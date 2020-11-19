/*
 * memorymap.h
 *
 *  Created on: Nov 22, 2019
 *      Author: cburke
 */

#ifndef MEMORYMAP_H_
#define MEMORYMAP_H_

typedef struct MemoryMap {
  unsigned char *storage;
  unsigned char *end;  // Last byte of storage
  int maxElements;
} MemoryMap;

// One byte in the map represents one byte of the loaded module.
// Each byte is marked as unknown, code, or data prior to disassenbly
// MSB is set if this is a "label"

#define MM_UNKNOWN	'U'
#define MM_FCB		'B'
#define	MM_FDB		'D'
#define	MM_FDB2		'd'
#define MM_FCC		'C'
#define MM_FCS      'S'
#define MM_FCSN     's'
#define MM_CODE1    'I'
#define MM_CODE     'i'
#define MM_CODEX    'y'
#define MM_INVALID	'X'

#define MM_LABEL	0b10000000

// Initialize a memory map
void mm_init(MemoryMap* map, int mapSize, char *id);

// Return the type of the byte at a given position in a map
unsigned char mm_type(MemoryMap* map, int offset);

// Return whether there is a label at a given position in a map
int mm_isLabel(MemoryMap* map, int offset);

// Set the type of the byte at a given position in a map
void mm_setType(MemoryMap* map, int offset, unsigned char type);

// Set whether there is a label at a given position in a map
void mm_setLabel(MemoryMap* map, int offset, int value);

// Set a range of bytes to the specified type
void mm_set(MemoryMap* map, int offset, unsigned char type, int count);

// Set a range of bytes as CODE; 1st byte gets a CODE1 type
void mm_setCode(MemoryMap* map, int offset, int count);

// Set a range of bytes as FCS; last byte gets an FCSN type
void mm_setString(MemoryMap* map, int offset, int count);

// Set a range of bytes as FDB; alternate FDB and FDB2
void mm_setFDB(MemoryMap* map, int offset, int count);

// Count the number of map bytes in a "run" of the same type
int mm_runLength(MemoryMap* map, int offset);

// Dump the memory map with a given number of bytes per line
void mm_dump(MemoryMap* map, int perLine);

#endif /* MEMORYMAP_H_ */
