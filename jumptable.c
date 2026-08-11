
//
// Jump table handler for the disassembler
//
// Created on: Aug 10, 2026
//     Author: cburke
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "intstack.h"
#include "memoryfile.h"
#include "memorymap.h"

#include "jumptable.h"

extern MemoryMap map;
extern IntStack addrStack;	// Stack of known-good execution addresses
extern int _debug;		// Non-zero to print debug information

extern void usage();

/* Every EVEN address in the range is the start of a 2-byte table entry */
void jt_worker(MemoryFile *mod, unsigned start, unsigned end, int mapType) {
	int count;

    if (end < start) return;
    if (start < 0 || start >= mod->length) return;
    if (end < 0 || end >= mod->length) return;

	if (!(end & 1)) ++end;
    count = (end - start + 1) / 2;
    if (_debug) printf("jumptable: Processing %d entries at $%04X...\n", count, start + mod->abs_base);
    if (count) {
        // Each entry in the table will be a specialized FDB
        unsigned at = start;
        unsigned effectiveAddr;
        while (at >= 0 && at < mod->length && count) {
            unsigned address = mf_get_word(mod, at) & 0xFFFF;
            switch (mapType) {
            case MAPTYPE_EXT:
                effectiveAddr = address - mod->abs_base;
                break;
            case MAPTYPE_PIC:
                effectiveAddr = address + at;
                break;
            case MAPTYPE_REL:
                effectiveAddr = address + start;
                break;
            default:
                break;
            }
            effectiveAddr &= 0xFFFF;
            if (_debug) printf("jumptable: Known address [$%04X] = $%04X\n", at + mod->abs_base, effectiveAddr + mod->abs_base);
            intstack_push(&addrStack, effectiveAddr);
            mm_setjtFDB(&map, at, 2, mapType);
            at += 2;
        }
    }
}

/* Process jumptable of EXTENDED (absolute) addresses */
void jt_extended(MemoryFile *mod, unsigned start, unsigned end) {
    jt_worker(mod, start, end, MAPTYPE_EXT);
}

/* Process jumptable of position-independent (* relative) addresses */
void jt_pic(MemoryFile *mod, unsigned start, unsigned end) {
    jt_worker(mod, start, end, MAPTYPE_PIC);
}

/* Process jumptable of PIC addresses relative to base of table */
void jt_pic_relative(MemoryFile *mod, unsigned start, unsigned end) {
    jt_worker(mod, start, end, MAPTYPE_REL);
}
