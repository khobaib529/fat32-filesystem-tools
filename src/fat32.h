// FAT32 file system structures for boot sector, FS info, and directory entries.

#ifndef FILESYSTEM_FAT32_H_
#define FILESYSTEM_FAT32_H_

#include <cstdint>

struct FAT32BootSector {
    uint8_t  BootJumpInst[3];       // Jump instruction
    uint8_t  OemName[8];            // OEM Name
    uint16_t BytesPerSector;        // Usually 512
    uint8_t  SectorsPerCluster;     // e.g., 8, 16, 32, etc.
    uint16_t ReservedSectors;       // Typically 32 for FAT32
    uint8_t  NumberOfFATs;          // Usually 2
    uint16_t RootDirEntries;        // Should be 0 for FAT32
    uint16_t TotalSectorsShort;     // If zero, use TotalSectorsLong
    uint8_t  MediaDescriptor;       // Media type
    uint16_t SectorsPerFAT16;       // Should be 0 for FAT32
    uint16_t SectorsPerTrack;
    uint16_t NumberOfHeads;
    uint32_t HiddenSectors;
    uint32_t TotalSectorsLong;      // Total sectors on the volume

    // FAT32-specific fields
    uint32_t SectorsPerFAT;         // Size of one FAT in sectors
    uint16_t ExtFlags;
    uint16_t FsVersion;
    uint32_t RootCluster;           // Typically 2
    uint16_t FsInfo;                // Sector number of FS Info sector (usually 1)
    uint16_t BackupBootSector;      // Sector number of backup boot sector (usually 6)
    uint8_t  Reserved[12];

    // Extended Boot Record
    uint8_t  DriveNumber;
    uint8_t  Reserved1;
    uint8_t  BootSignature;         // Extended boot signature (should be 0x29)
    uint32_t VolumeID;              // Serial number
    uint8_t  VolumeLabel[11];       // Volume label (padded with spaces)
    uint8_t  FileSystemType[8];     // Should be "FAT32   "
    
    // Followed by boot code and the boot signature (0x55, 0xAA)
} __attribute__((packed));

struct FAT32FSInfo {
    uint32_t LeadSignature;         // Must be 0x41615252
    uint8_t  Reserved1[480];        // Typically zero-filled
    uint32_t StructSignature;       // Must be 0x61417272
    uint32_t FreeCount;             // Free cluster count (or 0xFFFFFFFF if unknown)
    uint32_t NextFree;              // Next free cluster (or 0xFFFFFFFF if unknown)
    uint8_t  Reserved2[12];         // Typically zero-filled
    uint32_t TrailSignature;        // Must be 0xAA550000 (some implementations may use 0x0000AA55)
} __attribute__((packed));

struct FAT32DirectoryEntry {
    uint8_t  Name[11];              // 8-character name + 3-character extension (no null terminator)
    uint8_t  Attr;                  // File attributes (e.g., read-only, hidden, system, volume label, directory, archive)
    uint8_t  NtReserved;            // Reserved for Windows NT; typically zero
    uint8_t  CreationTimeTenth;     // Millisecond stamp at file creation time
    uint16_t CreationTime;          // File creation time
    uint16_t CreationDate;          // File creation date
    uint16_t LastAccessDate;        // Last access date
    uint16_t FirstClusterHigh;      // High 16 bits of the first cluster number (for FAT32)
    uint16_t WriteTime;             // Last write time
    uint16_t WriteDate;             // Last write date
    uint16_t FirstClusterLow;       // Low 16 bits of the first cluster number
    uint32_t FileSize;              // File size in bytes (0 for directories)
} __attribute__((packed));

#endif // FILESYSTEM_FAT32_H_
