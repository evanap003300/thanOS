#include "fs/fat32.h"
#include "drivers/ata.h"
#include "graphics/render.h"

Fat32 fat32;
uint8_t fat_buffer[512];
uint8_t dir_buffer[512]; 

bool Fat32::init(uint32_t partition_offset) {
	this->partition_start_lba = partition_offset;
	ATA::read_sectors(partition_offset, 1, fat_buffer);
	BPB* bpb = (BPB*)fat_buffer;

	this->sectors_per_cluster = bpb->sectors_per_cluster;
	this->root_cluster = bpb->root_cluster;
	this->fat_start_lba = partition_offset + bpb->reserved_sector_count;
	uint32_t fat_size = bpb->fat_size_32;
	this->data_start_lba = this->fat_start_lba + (bpb->num_fats * fat_size);

	return true;
}

void Fat32::print_info() {
	terminal.printf("--- FAT32 INFO ---\n");
	terminal.printf("Partition Start: %d\n", partition_start_lba);
    	terminal.printf("Sectors/Cluster: %d\n", sectors_per_cluster);
    	terminal.printf("Root Cluster: %d\n", root_cluster);
    	terminal.printf("FAT Start LBA: %d\n", fat_start_lba);
    	terminal.printf("Data Start LBA: %d\n", data_start_lba);
}

uint32_t Fat32::cluster_to_lba(uint32_t cluster) { 
	if (cluster < 2) {
		return 0;
	}
	return this->data_start_lba + ((cluster - 2) * this->sectors_per_cluster);
}

bool Fat32::read_cluster(uint32_t cluster, uint8_t* buffer) {
	uint32_t lba = cluster_to_lba(cluster);
	if (lba == 0) return false;
	ATA::read_sectors(lba, this->sectors_per_cluster, buffer);

	return true;
}

void Fat32::list_directory(uint32_t cluster) {
	if (!read_cluster(cluster, dir_buffer)) {
		return;
	}

	DirectoryEntry* entries = (DirectoryEntry*)dir_buffer;

	uint32_t cluster_size_bytes = this->sectors_per_cluster * 512;
	uint32_t entries_count = cluster_size_bytes / 32;

	terminal.printf("--- Listing Cluster %d (%d entries) ---\n", cluster, entries_count);

	for (uint32_t i = 0; i < entries_count; i++) {
		DirectoryEntry* entry = &entries[i];

		if (entry->name[0] == 0x00) break;
		if (entry->name[0] == 0xE5) continue;
		if (entry->attributes == 0x0F) continue;

		char name_buffer[13];
		int out_idx = 0;

		for (int j = 0; j < 8; j++) {
			if (entry->name[j] != ' ') {
				name_buffer[out_idx++] = entry->name[j];
			}
		}

		if ((entry->attributes & 0x10) == 0) {
			name_buffer[out_idx++] = '.';
			for (int j = 0; j < 3; j++) {
				if (entry->ext[j] != ' ') {
					name_buffer[out_idx++] = entry->ext[j];
				}
			}
		}

		name_buffer[out_idx] = '\0';

		terminal.printf("%s\n", name_buffer);

		if (entry->attributes & 0x10) {
			terminal.printf("/");
		}
	}
}
