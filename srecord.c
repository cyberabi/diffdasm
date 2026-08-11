
//
// S-record loader for the disassembler
//
// Created on: Aug 8, 2026
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

#include "srecord.h"

extern MemoryFile input;
extern MemoryMap map;
extern IntStack addrStack;	// Stack of known-good execution addresses
extern int _debug;		// Non-zero to print debug information

extern void usage();

#define MAX_SREC_SIZE 518

int loadMHXFile(char* fName) {
	FILE		*fp1;
	unsigned char *mp;
	size_t		moduleLength = 0;
	unsigned	lowAddress = 0xFFFF;
	unsigned	highAddress = 0x0000;
	unsigned	execAddress = 0x0000;	// Sentinel
	char		lineBuffer[MAX_SREC_SIZE+1];
	char		recType;
	unsigned	pairs, address, consumed;
	unsigned	csum;
	int		matches;

	if (_debug) printf("loadMHXFile(%s)...\n", fName);

	if (!(fp1 = fopen(fName,"r"))) {
		fprintf(stderr, "loadMHXFile(%s): ERROR: unable to open '%s'\n", fName, fName);
		usage();
	}

	// Pass 1. Determine lowest and highest address used
	// Pass 2. Load up the image
	for (int pass = 1; pass <= 2; pass++) {
		while (fgets(lineBuffer, MAX_SREC_SIZE, fp1)) {
			// Skip blank lines
			if (*lineBuffer == '\r' || *lineBuffer == '\n')
				continue;
			if (strlen(lineBuffer) < 8) goto badformat;
			// Parse the line
			matches = sscanf(lineBuffer, "S%c%02X%04X%n",
				&recType, &pairs, &address, &consumed);
			if (matches != 3 || consumed != 8)
				goto badformat;
			csum = pairs + ((address >> 8) & 0xFF) + (address & 0xFF);
			int dataBytes = pairs - 3;
			switch (recType) {
			case '0':
				// We don't care about this record
				break;
			case '1':
				if (pass == 1) {
					// Just collect address range
					if (address < lowAddress)
						lowAddress = address;
					if (address + dataBytes - 1 > highAddress)
						highAddress = address + dataBytes - 1;
				} else {
					// Parse and store the bytes
					// Update the checksum
					unsigned parsedByte;
					char* parsePtr = lineBuffer + consumed;
					unsigned char *loadPtr = input.storage + address - lowAddress;
					for (int i=0; i<dataBytes; i++) {
						sscanf(parsePtr, "%02X", &parsedByte);
						csum += parsedByte;
						*loadPtr++ = parsedByte;
						parsePtr += 2;
					}
					// Verify the checksum on each line
					sscanf(parsePtr, "%02X", &parsedByte);
					if (0xFF - (csum & 0xFF) != parsedByte)
						goto badformat;
				}
				break;
			case '9':
				// A potential execution address
				execAddress = address;
				break;
			default:
				goto badformat;
			}
		}
		// End of the current pass
		if (_debug) printf("loadMHXFile(%s): end of S-record loading pass %d...\n", fName, pass);
		if (pass == 1) {
			// Stats
			printf("Low address:  $%04X\n", lowAddress);
			printf("High address: $%04X\n", highAddress);
			printf("Exec address: $%04X\n", execAddress);
			moduleLength = highAddress - lowAddress + 1;
			// Try to allocate memory for module(s)
			if (_debug) printf("loadMHXFile(%s): allocating $%04X bytes for map\n", fName, (int)moduleLength);
			mf_init(&input, moduleLength, fName);
			mf_set_base(&input, lowAddress);
			// Rewind the file for pass 2
			rewind(fp1);
		} else {
			fclose(fp1);
            // Try to allocate memory for map
            if (_debug) printf("loadMHXFile(%s): allocating $%04X bytes for map\n", fName, (int)moduleLength);
            mm_init(&map, moduleLength, fName);
            mm_set_base(&map, lowAddress);
            if (_debug) printf("loadMHXFile(%s): loaded $%04X bytes\n", fName, (int)moduleLength);
			return execAddress;
		}
	}

// Because it's fun to use a goto just to piss people off ;-)
badformat:
    fclose(fp1);
	fprintf(stderr, "loadMHXFile(%s): ERROR: invalid S-record '%s' in '%s'\n", fName, lineBuffer, fName);
	usage();
	return 0;
}

