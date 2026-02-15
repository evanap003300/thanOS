#pragma once
#include <stdint.h>

struct PartitionEntry {
	uint8_t attributes;
	uint8_t start_head;
	uint8_t start_sec;
	uint8_t start_cyl;
	uint8_t type;
	uint8_t end_head;
	uint8_t end_sec;
	uint8_t end_cyl;
	uint32_t lba_start;
	uint32_t sector_count;
} __attribute__((packed));

struct MBR {
	uint8_t boot_code[446];
	PartitionEntry partitions[4];
	uint16_t signature;
} __attribute__((packed));

class MBRParser {
public:
	static uint32_t find_first_partition();
};
