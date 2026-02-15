#include "fs/mbr.h"
#include "drivers/ata.h"
#include "utils/io.h"

uint8_t mbr_buffer[512];

uint32_t MBRParser::find_first_partition() {
	ATA::read_sectors(0, 1, mbr_buffer);

	MBR* mbr = (MBR*)mbr_buffer;

	if (mbr->signature != 0xAA55) {
		return 0;
	}

	// Scan the 4 partition slots
	for (int i = 0; i < 4; i++) {
		if (mbr->partitions[i].type == 0x0B || mbr->partitions[i].type == 0x0C)     {				
			return mbr->partitions[i].lba_start;
		}
	}

	return 0;
}
