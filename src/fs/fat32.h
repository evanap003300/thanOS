#pragma once
#include <stdint.h>
#include <std/string.h>

struct BPB {
	uint8_t jump_boot[3];
	uint8_t oem_name[8];
	uint16_t bytes_per_sector; 
	uint8_t sectors_per_cluster;
	uint16_t reserved_sector_count;
	uint8_t num_fats;
	uint16_t root_entry_count;
	uint16_t total_sectors_16;
	uint8_t media;
	uint16_t fat_size_16;
	uint16_t sectors_per_track;
	uint16_t num_heads;
	uint32_t hidden_sectors;
	uint32_t total_sectors_32;

	uint32_t fat_size_32;
	uint16_t ext_flags;
	uint16_t fs_version;
	uint32_t root_cluster;
	uint16_t fs_info;
	uint16_t backup_boot_sector;
	uint8_t reserved[12];
	uint8_t drive_numbre;
	uint8_t reserved1;
	uint8_t boot_signature;
	uint32_t volume_id;
	uint8_t volume_label[11];
	uint8_t fs_type[8];
} __attribute__((packed));

struct DirectoryEntry {
	uint8_t name[8];
	uint8_t ext[3];
	uint8_t attributes;
	uint8_t reserved;
	uint8_t creation_time_tenths;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t access_date;
	uint16_t cluster_high;
	uint16_t time;
	uint16_t date;
	uint16_t cluster_low;
	uint32_t file_size;
} __attribute__((packed));

class Fat32 {
public:
	uint32_t partition_start_lba;
	uint32_t root_cluster;
	uint32_t fat_start_lba;	
	uint32_t data_start_lba;
	uint8_t sectors_per_cluster;
	
	bool init(uint32_t partition_offset);
	void print_info();
	uint32_t cluster_to_lba(uint32_t cluster);
	bool read_cluster(uint32_t cluster, uint8_t* buffer);
	void list_directory(uint32_t cluster);
	void cat(char* filename);
	uint32_t read_file(const char* filename, uint8_t* out_buffer);
};

extern Fat32 fat32;
