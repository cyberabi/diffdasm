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
	// Try to allocate memory for map
	file->storage = (unsigned char *)malloc(fileSize*sizeof(unsigned char));
	if (NULL == file->storage) {
		file->length = 0;
		fprintf(stderr, "ERROR: mm_init: Insufficient memory to map '%s'.\n", id);
		exit(1);
	}
	memset(file->storage, 0, (size_t)(fileSize*sizeof(unsigned char)));
	file->length = fileSize;
	file->end = file->storage + fileSize - 1;
}

