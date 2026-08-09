/*
 * memoryfile.h
 *
 *  Created on: Nov 23, 2019
 *      Author: cburke
 */

#ifndef MEMORYFILE_H_
#define MEMORYFILE_H_

typedef struct MemoryFile {
  unsigned abs_base;	// Absolute base address of module (or 0x0000)
  unsigned char *storage;
  unsigned char *end;  // Last byte of storage
  int length;
} MemoryFile;

// Initialize a memory file
void mf_init(MemoryFile* file, int fileSize, char *id);
void mf_set_base(MemoryFile* file, unsigned base);

int mf_get_byte(MemoryFile* file, int offset);
int mf_get_word(MemoryFile* file, int offset);
long mf_get_dword(MemoryFile* file, int offset);

int mf_get_abs_word(MemoryFile* file, int address);
int mf_get_abs_byte(MemoryFile* file, int address);
long mf_get_abs_dword(MemoryFile* file, int address);

#endif /* MEMORYFILE_H_ */
