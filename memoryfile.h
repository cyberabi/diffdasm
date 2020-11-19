/*
 * memoryfile.h
 *
 *  Created on: Nov 23, 2019
 *      Author: cburke
 */

#ifndef MEMORYFILE_H_
#define MEMORYFILE_H_

typedef struct MemoryFile {
  unsigned char *storage;
  unsigned char *end;  // Last byte of storage
  int length;
} MemoryFile;

// Initialize a memory file
void mf_init(MemoryFile* file, int fileSize, char *id);

#endif /* MEMORYFILE_H_ */
