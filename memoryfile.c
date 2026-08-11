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
	file->abs_base = 0;
}

void mf_set_base(MemoryFile* file, unsigned base) {
	file->abs_base = base;
}

int mf_get_byte(MemoryFile* file, int offset) {
	if ( offset >= 0 && offset < file->length)
		return file->storage[offset];
	fprintf(stderr, "ERROR: mf_get_byte: Offset '$%05X' out of range.\n", offset);
	return 0;
}

int mf_get_word(MemoryFile* file, int offset) {
	if ( offset >= 0 && offset < file->length-1)
		return	((file->storage[offset+0]<<8) |
			 (file->storage[offset+1]));
	fprintf(stderr, "ERROR: mf_get_word: Offset '$%05X' out of range.\n", offset);
	return 0;
}

long mf_get_dword(MemoryFile* file, int offset) {
	if ( offset >= 0 && offset < file->length-3)
		return	((file->storage[offset+0]<<24) |
			 (file->storage[offset+1]<<16) |
			 (file->storage[offset+2]<<8)  |
			 (file->storage[offset+3]));
	fprintf(stderr, "ERROR: mf_get_dword: Offset '$%05X' out of range.\n", offset);
	return 0;
}

int mf_get_abs_byte(MemoryFile* file, int address) {
	return mf_get_byte(file, address - file->abs_base);
}

int mf_get_abs_word(MemoryFile* file, int address) {
	return mf_get_word(file, address - file->abs_base);
}

long mf_get_abs_dword(MemoryFile* file, int address) {
	return mf_get_dword(file, address - file->abs_base);
}

int mf_checksum(MemoryFile* file) {
    int csum = 0;
    for (int i=0; i < file->length; i++)
        csum += file->storage[i];
    return (csum & 0xFF);
}




