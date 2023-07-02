/*
 * memoryfile.c
 *
 *  Created on: Nov 23, 2019
 *      Author: cburke
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "memoryfile.h"

void mf_init(MemoryFile* file, int fileSize, char *id) {
	// WARNING: id might be an ephemeral string; copy if needed
	if (fileSize > 65536) {
		file->length = 0;
		fprintf(stderr, "ERROR: mf_init: '%s' is > 64K.\n", id);
		exit(1);
	}
	// Try to allocate memory for map
	file->storage = (unsigned char *)malloc(fileSize*sizeof(unsigned char));
	if (NULL == file->storage) {
		file->length = 0;
		fprintf(stderr, "ERROR: mf_init: Insufficient memory to map '%s'.\n", id);
		exit(1);
	}
	memset(file->storage, 0, (size_t)(fileSize*sizeof(unsigned char)));
	file->length = fileSize;
	file->end = file->storage + fileSize - 1;
}

int mf_get_word(MemoryFile* file, int offset) {
	if ( offset < file->length-1)
		return ((file->storage[offset]<<8) | file->storage[offset+1]);
	fprintf(stderr, "ERROR: mf_get_word: Offset '0x%04X' out of range.\n", offset);
	return 0;
}
