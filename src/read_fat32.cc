// A utility to read and display information from a FAT32 formatted disk image.

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "fat32.h"

// Function to convert a 4-byte buffer to a uint32_t in little-endian format
uint32_t toLittleEndian(uint8_t* buffer) {
  return (static_cast<uint32_t>(buffer[0])) |
         (static_cast<uint32_t>(buffer[1]) << 8) |
         (static_cast<uint32_t>(buffer[2]) << 16) |
         (static_cast<uint32_t>(buffer[3]) << 24);
}

void PrintBootSector(FAT32BootSector& fat32BootSector) {
  std::cout << "BootJumpInst: ";
  for (int i = 0; i < 3; ++i) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<uint64_t>(fat32BootSector.BootJumpInst[i]) << " ";
  }
  std::cout << std::dec << std::endl;

  // Print other fields with uint64_t casting
  std::cout << "OemName: "
            << std::string(reinterpret_cast<char*>(fat32BootSector.OemName), 8)
            << std::endl;
  std::cout << "BytesPerSector: "
            << static_cast<uint64_t>(fat32BootSector.BytesPerSector)
            << std::endl;
  std::cout << "SectorsPerCluster: "
            << static_cast<uint64_t>(fat32BootSector.SectorsPerCluster)
            << std::endl;
  std::cout << "ReservedSectors: "
            << static_cast<uint64_t>(fat32BootSector.ReservedSectors)
            << std::endl;
  std::cout << "NumberOfFATs: "
            << static_cast<uint64_t>(fat32BootSector.NumberOfFATs) << std::endl;
  std::cout << "RootDirEntries: "
            << static_cast<uint64_t>(fat32BootSector.RootDirEntries)
            << std::endl;
  std::cout << "TotalSectorsShort: "
            << static_cast<uint64_t>(fat32BootSector.TotalSectorsShort)
            << std::endl;
  std::cout << "MediaDescriptor: 0x" << std::hex
            << static_cast<uint64_t>(fat32BootSector.MediaDescriptor)
            << std::dec << std::endl;
  std::cout << "SectorsPerFAT16: "
            << static_cast<uint64_t>(fat32BootSector.SectorsPerFAT16)
            << std::endl;
  std::cout << "SectorsPerTrack: "
            << static_cast<uint64_t>(fat32BootSector.SectorsPerTrack)
            << std::endl;
  std::cout << "NumberOfHeads: "
            << static_cast<uint64_t>(fat32BootSector.NumberOfHeads)
            << std::endl;
  std::cout << "HiddenSectors: "
            << static_cast<uint64_t>(fat32BootSector.HiddenSectors)
            << std::endl;
  std::cout << "TotalSectorsLong: "
            << static_cast<uint64_t>(fat32BootSector.TotalSectorsLong)
            << std::endl;
  std::cout << "SectorsPerFAT: "
            << static_cast<uint64_t>(fat32BootSector.SectorsPerFAT)
            << std::endl;
  std::cout << "ExtFlags: 0x" << std::hex
            << static_cast<uint64_t>(fat32BootSector.ExtFlags) << std::dec
            << std::endl;
  std::cout << "FsVersion: " << static_cast<uint64_t>(fat32BootSector.FsVersion)
            << std::endl;
  std::cout << "RootCluster: "
            << static_cast<uint64_t>(fat32BootSector.RootCluster) << std::endl;
  std::cout << "FsInfo: " << static_cast<uint64_t>(fat32BootSector.FsInfo)
            << std::endl;
  std::cout << "BackupBootSector: "
            << static_cast<uint64_t>(fat32BootSector.BackupBootSector)
            << std::endl;
  std::cout << "DriveNumber: 0x" << std::hex
            << static_cast<uint64_t>(fat32BootSector.DriveNumber) << std::dec
            << std::endl;
  std::cout << "BootSignature: 0x" << std::hex
            << static_cast<uint64_t>(fat32BootSector.BootSignature) << std::dec
            << std::endl;
  std::cout << "VolumeID: 0x" << std::hex
            << static_cast<uint64_t>(fat32BootSector.VolumeID) << std::dec
            << std::endl;
  std::cout << "VolumeLabel: "
            << std::string(reinterpret_cast<char*>(fat32BootSector.VolumeLabel),
                           11)
            << std::endl;
  std::cout << "FileSystemType: "
            << std::string(
                   reinterpret_cast<char*>(fat32BootSector.FileSystemType), 8)
            << std::endl;
}

void PrintFSInfoSector(const FAT32FSInfo& fat32FSInfoSector) {
  std::cout << "LeadSignature: 0x" << std::hex
            << static_cast<uint64_t>(fat32FSInfoSector.LeadSignature)
            << std::dec << std::endl;

  // Print Reserved1 as a hex dump
  std::cout << "Reserved1: ";
  for (size_t i = 0; i < sizeof(fat32FSInfoSector.Reserved1); ++i) {
    if (i % 16 == 0)
      std::cout << "\n  ";  // New line every 16 bytes for readability
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<uint64_t>(fat32FSInfoSector.Reserved1[i]) << " ";
  }
  std::cout << std::dec << std::endl;

  std::cout << "StructSignature: 0x" << std::hex
            << static_cast<uint64_t>(fat32FSInfoSector.StructSignature)
            << std::dec << std::endl;

  std::cout << "FreeCount: "
            << static_cast<uint64_t>(fat32FSInfoSector.FreeCount) << std::endl;

  std::cout << "NextFree: " << static_cast<uint64_t>(fat32FSInfoSector.NextFree)
            << std::endl;

  // Print Reserved2 as a hex dump
  std::cout << "Reserved2: ";
  for (size_t i = 0; i < sizeof(fat32FSInfoSector.Reserved2); ++i) {
    if (i % 16 == 0)
      std::cout << "\n  ";  // New line every 16 bytes for readability
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<uint64_t>(fat32FSInfoSector.Reserved2[i]) << " ";
  }
  std::cout << std::dec << std::endl;

  std::cout << "TrailSignature: 0x" << std::hex
            << static_cast<uint64_t>(fat32FSInfoSector.TrailSignature)
            << std::dec << std::endl;
}

void PrintFAT(std::vector<uint8_t>& fatBuffer, uint64_t numFATEntries) {
  uint64_t totalFATEntries = fatBuffer.size() / 4;
  std::cout << "\n===== FAT Table (" << totalFATEntries
            << " entries available) =====" << std::endl;

  // Make sure we don't try to print more entries than available.
  if (numFATEntries > totalFATEntries) {
    numFATEntries = totalFATEntries;
  }

  // Print FAT32 entries
  for (size_t i = 0; i < numFATEntries; ++i) {
    uint32_t entry;
    uint8_t* entryBuffer = fatBuffer.data() + i * 4;

    // Convert the entry bytes from little-endian format
    entry = toLittleEndian(entryBuffer);

    // Print the entry in hex, padded to 8 digits
    std::cout << std::hex << std::setw(8) << std::setfill('0') << entry << " ";

    // After every 16 entries, output a newline
    if ((i + 1) % 16 == 0) {
      std::cout << std::dec << std::endl;
    }
  }
  // Print a newline if the last line was not complete.
  if (numFATEntries % 16 != 0) {
    std::cout << std::dec << std::endl;
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0]
              << " <FAT32_image_file> [num_FAT_entries_to_print]" << std::endl;
    return 1;
  }

  const char* imageFile = argv[1];
  std::ifstream file(imageFile, std::ios::binary);
  if (!file) {
    std::cerr << "Error: Could not open file " << imageFile << std::endl;
    return 1;
  }

  // Read the Boot Sector (assumed to be the first 512 bytes)
  FAT32BootSector fat32BootSector;
  file.read(reinterpret_cast<char*>(&fat32BootSector), sizeof(FAT32BootSector));
  if (!file) {
    std::cerr << "Error: Could not read boot sector from file." << std::endl;
    return 1;
  }

  std::cout << "===== FAT32 Boot Sector =====" << std::endl;
  PrintBootSector(fat32BootSector);

  // Read the FS Info Sector.
  // The FS Info sector is typically located at sector number bootSector.FsInfo.
  // Calculate the byte offset: offset = BytesPerSector * FsInfo.
  std::streampos fsInfoOffset =
      fat32BootSector.BytesPerSector * fat32BootSector.FsInfo;
  file.seekg(fsInfoOffset);
  if (!file) {
    std::cerr << "Error: Could not seek to FS Info sector at offset "
              << fsInfoOffset << std::endl;
    return 1;
  }

  FAT32FSInfo fat32FSInfoSector;
  file.read(reinterpret_cast<char*>(&fat32FSInfoSector), sizeof(FAT32FSInfo));
  if (!file) {
    std::cerr << "Error: Could not read FS Info sector." << std::endl;
    return 1;
  }

  std::cout << "\n===== FAT32 FS Info Sector =====" << std::endl;
  PrintFSInfoSector(fat32FSInfoSector);

  // --- Read the FAT Table ---
  std::streampos fatOffset =
      fat32BootSector.ReservedSectors * fat32BootSector.BytesPerSector;
  file.seekg(fatOffset);
  if (!file) {
    std::cerr << "Error: Could not seek to FAT table at offset " << fatOffset
              << std::endl;
    return 1;
  }

  // Calculate the FAT table size in bytes.
  // (Each FAT occupies SectorsPerFAT sectors.)
  size_t fatSize =
      fat32BootSector.SectorsPerFAT * fat32BootSector.BytesPerSector;

  // Read the entire FAT into a buffer.
  std::vector<uint8_t> fatBuffer(fatSize);
  file.read(reinterpret_cast<char*>(fatBuffer.data()), fatSize);
  if (!file) {
    std::cerr << "Error: Could not read FAT table from file." << std::endl;
    return 1;
  }

  // Determine the number of FAT entries to print.
  // If a second command-line argument is provided, use it.
  uint64_t numFATEntriesToPrint = 16;  // default value
  if (argc >= 3) {
    numFATEntriesToPrint = std::stoul(argv[2]);
  }
  PrintFAT(fatBuffer, numFATEntriesToPrint);

  return 0;
}
