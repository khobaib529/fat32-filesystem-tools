#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "fat32.h"

// Compute the number of sectors per FAT for a FAT32 volume.
uint32_t CalculateSectorsPerFAT(uint64_t disk_size_bytes,
                                uint16_t bytes_per_sector,
                                uint8_t sectors_per_cluster,
                                uint16_t reserved_sectors,
                                uint8_t number_of_fats) {
  uint32_t total_sectors = disk_size_bytes / bytes_per_sector;
  // Initial guess: ignore FAT space.
  uint32_t fat_size =
      (((total_sectors - reserved_sectors) / sectors_per_cluster + 2) * 4 +
       bytes_per_sector - 1) /
      bytes_per_sector;
  uint32_t clusters, new_fat_size;

  do {
    clusters = (total_sectors - reserved_sectors - number_of_fats * fat_size) /
               sectors_per_cluster;
    new_fat_size =
        (((clusters + 2) * 4) + bytes_per_sector - 1) / bytes_per_sector;
    if (new_fat_size == fat_size) break;
    fat_size = new_fat_size;
  } while (1);

  return fat_size;
}

void InitFAT32BootSector(FAT32BootSector* fat32BootSector,
                         uint64_t disk_size_in_bytes) {
  const uint8_t jump_inst[3] = {0xeb, 0x58, 0x90};
  memcpy(fat32BootSector->BootJumpInst, jump_inst, sizeof(jump_inst));
  memcpy(fat32BootSector->OemName, "MSWIN4.1", 8);
  fat32BootSector->BytesPerSector = 512;
  fat32BootSector->SectorsPerCluster = 8;
  fat32BootSector->ReservedSectors = 32;
  fat32BootSector->NumberOfFATs = 2;
  fat32BootSector->RootDirEntries = 0;
  fat32BootSector->TotalSectorsShort = 0;
  fat32BootSector->MediaDescriptor = 0xf8;
  fat32BootSector->SectorsPerFAT16 = 0;
  fat32BootSector->SectorsPerTrack = 63;
  fat32BootSector->NumberOfHeads = 64;
  fat32BootSector->HiddenSectors = 0;
  fat32BootSector->TotalSectorsLong =
      disk_size_in_bytes / fat32BootSector->BytesPerSector;
  fat32BootSector->SectorsPerFAT = CalculateSectorsPerFAT(
      disk_size_in_bytes, fat32BootSector->BytesPerSector,
      fat32BootSector->SectorsPerCluster, fat32BootSector->ReservedSectors,
      fat32BootSector->NumberOfFATs);
  fat32BootSector->ExtFlags = 0;
  fat32BootSector->FsVersion = 0;
  fat32BootSector->RootCluster = 2;
  fat32BootSector->FsInfo = 1;
  fat32BootSector->BackupBootSector = 6;
  memset(fat32BootSector->Reserved, 0, sizeof(fat32BootSector->Reserved));
  fat32BootSector->DriveNumber = 0x80;
  fat32BootSector->Reserved1 = 0x0;
  fat32BootSector->BootSignature = 0x29;
  fat32BootSector->VolumeID = 0xdeadbeef;
  memcpy(fat32BootSector->VolumeLabel, "NO NAME    ",
         sizeof(fat32BootSector->VolumeLabel));
  memcpy(fat32BootSector->FileSystemType, "FAT32   ",
         sizeof(fat32BootSector->FileSystemType));
}

void InitFAT32FSInfo(FAT32FSInfo* fs_info, FAT32BootSector* fat32BootSector) {
  fs_info->LeadSignature = 0x41615252;
  memset(fs_info->Reserved1, 0, sizeof(fs_info->Reserved1));
  fs_info->StructSignature = 0x61417272;
  fs_info->FreeCount =
      (fat32BootSector->TotalSectorsLong - fat32BootSector->ReservedSectors -
       (fat32BootSector->SectorsPerFAT * fat32BootSector->NumberOfFATs)) /
          fat32BootSector->SectorsPerCluster -
      1;
  fs_info->NextFree = 2;

  // Zero out the Reserved2 area.
  memset(fs_info->Reserved2, 0, sizeof(fs_info->Reserved2));

  // Set the trail signature.
  fs_info->TrailSignature = 0xaa550000;
}

void SetFATEntry(std::vector<uint8_t>& FATTable, size_t index, uint32_t value) {
  // Each FAT entry is 4 bytes, so we write the value byte-by-byte in
  // little-endian order
  FATTable[index * 4] = static_cast<uint8_t>(value & 0xFF);  // LSB
  FATTable[index * 4 + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
  FATTable[index * 4 + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
  FATTable[index * 4 + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);  // MSB
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    const char* usage = "Usage: format_fat32 <image_file>\n";
    std::fprintf(stderr, "%s", usage);
    std::exit(1);
  }
  const char* image_file_path = argv[1];

  std::ofstream image_file(image_file_path,
                           std::ios::binary | std::ios::in | std::ios::out);
  if (!image_file) {
    std::fprintf(stderr, "Failed to open image file %s for writing.\n",
                 image_file_path);
    std::exit(1);
  }

  // Move the file pointer to the end to calculate the file size
  image_file.seekp(0, std::ios::end);

  // Get the current position of the file pointer, which gives the size
  uint64_t sizeMB = static_cast<uint64_t>(image_file.tellp()) / (1024 * 1024);

  // Initialize FAT32 boot sector and FSInfo
  FAT32BootSector fat32BootSector;
  InitFAT32BootSector(&fat32BootSector, sizeMB * 1024 * 1024);

  FAT32FSInfo fat32FSInfo;
  InitFAT32FSInfo(&fat32FSInfo, &fat32BootSector);

  // Initialize FAT Table with the first few entries
  std::vector<uint8_t> FATTable(
      fat32BootSector.SectorsPerFAT * fat32BootSector.BytesPerSector, 0);
  SetFATEntry(FATTable, 0, 0x0FFFFFF8);
  SetFATEntry(FATTable, 1, 0x0FFFFFFF);
  SetFATEntry(FATTable, 2, 0x0FFFFFFF);

  image_file.seekp(0, std::ios::beg);
  // Write the FAT32 boot sector to the file
  image_file.write(reinterpret_cast<const char*>(&fat32BootSector),
                   sizeof(fat32BootSector));
  if (!image_file) {
    std::fprintf(stderr, "Failed to write FAT32 boot sector to image file.\n");
    std::exit(1);
  }

  // TODO: write the fsinfo sector
  image_file.seekp(fat32BootSector.FsInfo * fat32BootSector.BytesPerSector,
                   std::ios::beg);
  // Write the FAT32 FSInfo sector to the file
  image_file.write(reinterpret_cast<const char*>(&fat32FSInfo),
                   sizeof(fat32FSInfo));
  if (!image_file) {
    std::fprintf(stderr,
                 "Failed to write FAT32 FSInfo sector to image file.\n");
    std::exit(1);
  }

  // Write the first copy of the FAT table
  image_file.seekp(
      fat32BootSector.ReservedSectors * fat32BootSector.BytesPerSector,
      std::ios::beg);
  image_file.write(reinterpret_cast<const char*>(FATTable.data()),
                   FATTable.size());
  if (!image_file) {
    std::fprintf(stderr,
                 "Failed to write first copy of FAT32 table to image file.\n");
    std::exit(1);
  }

  // Write the second copy of the FAT table
  image_file.write(reinterpret_cast<const char*>(FATTable.data()),
                   FATTable.size());
  if (!image_file) {
    std::fprintf(stderr,
                 "Failed to write second copy of FAT32 table to image file.\n");
    std::exit(1);
  }

  // Output success message
  std::printf("FAT32 image file %s formatted successfully with size %lu MB.\n",
              image_file_path, sizeMB);

  return 0;
}
