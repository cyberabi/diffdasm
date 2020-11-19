/*
 * memorymap.c
 *
 *  Created on: Nov 22, 2019
 *      Author: cburke
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "memorymap.h"

void mm_init(MemoryMap* map, int mapSize, char *id) {
	// WARNING: id might be an ephemeral string; copy if needed
	// Try to allocate memory for map
	map->storage = (unsigned char *)malloc(mapSize*sizeof(unsigned char));
	if (NULL == map->storage) {
		map->maxElements = 0;
		fprintf(stderr, "ERROR: mm_init: Insufficient memory to map '%s'.\n", id);
		exit(1);
	}
	memset(map->storage, MM_UNKNOWN, (size_t)(mapSize*sizeof(unsigned char)));
	map->maxElements = mapSize;
	map->end = map->storage + mapSize - 1;
}

unsigned char mm_type(MemoryMap* map, int offset) {
	if (offset < map->maxElements) {
		return map->storage[offset] & ~MM_LABEL;
	}
	return MM_INVALID;
}

int mm_isLabel(MemoryMap* map, int offset) {
	if (offset < map->maxElements) {
		return ((map->storage[offset] & MM_LABEL) != 0);
	}
	return 0;
}

void mm_setType(MemoryMap* map, int offset, unsigned char type) {
	if (offset < map->maxElements) {
		int labelFlag = map->storage[offset] & MM_LABEL;
		map->storage[offset] = type | labelFlag;
	}
}

void mm_setLabel(MemoryMap* map, int offset, int value) {
	if (offset < map->maxElements) {
		if (value != 0) {
			map->storage[offset] |= MM_LABEL;
		}
		else {
			map->storage[offset] &= ~MM_LABEL;
		}
	}
}

void _mm_set(MemoryMap* map, int offset, unsigned char type_1, unsigned char type, unsigned char type_n, int count) {
	//printf("_mm_set $%04X %c %c %c %d\n", offset, type_1, type, type_n, count);
	if (offset+count > map->maxElements) {
		fprintf(stderr, "ERROR: _mm_set: Map offset $%05x is beyond the end of the map\n", offset+count);
	}
	// Error if these bytes aren't all currently "unknown"
	unsigned short i;
	unsigned char v, t;
	if (1 == count) {
		// Special case if we have a "first" and a "last" rule for 1 byte
		t = type_n;
		if (type_1 != type) {
			// Assume the 1st byte rule has priority
			t = type_1;
		}
		if (((v = mm_type(map, offset)) != MM_UNKNOWN) && (v != t)) {
			fprintf(stderr, "ERROR: _mm_set: Map offset $%04X has two different data types (%c,%c)\n", offset, t, v);
		} else {
			mm_setType(map, offset, t);
		}
	} else {
		for (i=0; i<count; i++) {
			t = (0 == i) ? type_1 : (((count-1) == i) ? type_n : type);
			if ((v = mm_type(map, offset+i)) != MM_UNKNOWN) {
				fprintf(stderr, "ERROR: _mm_set: Offset $%04X has two different data types (%c,%c)\n", offset+i, t, v);
			}
			else {
				mm_setType(map, offset+i, t);
			}
		}
	}
}

void mm_set(MemoryMap* map, int offset, unsigned char type, int count) {
	_mm_set(map, offset, type, type, type, count);
}

void mm_setCode(MemoryMap* map, int offset, int count) {
	// For code, first byte is treated differently
	_mm_set(map, offset, MM_CODE1, MM_CODE, MM_CODE, count);
}

void mm_setString(MemoryMap* map, int offset, int count) {
	// For strings, last byte is treated differently
	_mm_set(map, offset, MM_FCS, MM_FCS, MM_FCSN, count);
}

void mm_setFDB(MemoryMap* map, int offset, int count) {
	if (count & 1) {
		fprintf(stderr, "ERROR: mm_setfdb: Byte count must be even (%d)\n", count);
	} else {
		// Set in pairs so we alternate FDB and FDB2
		int i;
		for (i=0; i<count; i+=2) {
			_mm_set(map, offset+i, MM_FDB, MM_FDB, MM_FDB2, 2);
		}
	}
}

int mm_runLength(MemoryMap* map, int offset) {
	// Whatever the data type at offset, return the number of bytes
	// starting there with the same (or compatible) type (e.g. one instruction)
	int count = 0;
	int source;
	unsigned char type_1, type;
	if (offset >= map->maxElements) {
		fprintf(stderr, "ERROR: mm_runlength: Map offset $%05X is beyond the end of the map\n", offset);
	}
	type_1 = mm_type(map, offset);
	switch (type_1) {
		case MM_FCSN:
			// This is a single-byte string -- done
			count = 1;
			break;
		case MM_FCS:
			// Multi-byte string -- keep going until MM_FCSN
			while ((type=mm_type(map, (source=offset+count))) == MM_FCS) ++count;
			if (type != MM_FCSN) {
				fprintf(stderr, "ERROR: mm_runlength: String at offset $%04X ended with unexpected type (%c)\n", offset, type);
			} else {
				++count;
			}
			break;
		case MM_FDB:
			// Keep going while FDB or FDB2
			while (((type=mm_type(map, (source=offset+count))) == MM_FDB) || (type == MM_FDB2)) ++count;
			break;
		case MM_CODE1:
		case MM_CODEX:
			// Start of an instruction; rest of instruction is MM_CODE
			++count;
			while ((type=mm_type(map, (source=offset+count))) == MM_CODE) ++count;
			break;
		case MM_CODE:
			// Should never happen
			fprintf(stderr, "ERROR: mm_runlength: Not on instruction boundary at offset $%04X\n", offset);
			count = 1;
			break;
		case MM_FDB2:
			// Should never happen
			fprintf(stderr, "ERROR: mm_runlength: Not on FDB boundary at offset $%04X\n", offset);
			count = 1;
			break;
		default:
			// Other cases are easy - just look for a continuous run
			while ((source=offset+count) < map->maxElements &&
				   (type=mm_type(map, source)) == type_1) ++count;
			break;
	}
	return count;
}

void mm_dump(MemoryMap* map, int perLine) {
	// Note, this dump doesn't show label flags
	// One way to do this would be, two characters per byte
	// with space for no label, * for label
	int i, pos = 0;
	if (perLine < 0) perLine = 16;
	if (perLine > 64) perLine = 64;

	// Generate header
	printf("      ");
	for (i=0; i<perLine; i++) {
		printf("%X",i/16);
	}
	printf("\n      ");
	for (i=0; i<perLine; i++) {
		printf("%X",i%16);
	}
	printf("\n      ");
	for (i=0; i<perLine; i++) {
		printf("-");
	}

	// Generate dump
    for (pos = 0; pos < map->maxElements; pos++) {
    	if (pos%perLine == 0) {
    		printf("\n%04X: ", pos);
    	}
    	printf("%c", map->storage[pos] & ~MM_LABEL);
    }
    printf("\n");
}
