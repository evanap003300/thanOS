#include "fs/fat32.h"
#include "drivers/ata.h"
#include "graphics/render.h"

Fat32 fat32;
uint8_t fat_buffer[512];

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
