#pragma once
#include <stdint.h>

class ATA {
public:
	static void read_sectors(uint32_t target_lba, uint8_t sector_count, uint8_t* buffer);
};
