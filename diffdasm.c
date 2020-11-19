
//
// Disassemble OS9 module to a diffable format
//
// diffdasm --entry=xxxx ... <binary>
//
// Created on: Nov 23, 2019
//     Author: cburke
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "intstack.h"
#include "memorymap.h"
#include "memoryfile.h"
#include "linelist.h"
#include "os9stuff.h"
#include "stats6809.h"

#define STACKLIMIT 65536
#define STRMAX 4096
#define LINEMAX 65536
#define CODE_THRESHOLD 3
#define STRING_THRESHOLD 4

char* inFileName = NULL;
unsigned baseAddr = 0;

MemoryFile input;
MemoryMap map;
IntStack addrStack; // Stack of known-good execution addresses
IntStack labelStack; // Stack of labels (e.g. ,pcr references) that could be execution addresses
LineList lines[LINEMAX]; // Used to map line numbers to byte ranges
int lineCount = 0;

char operands[32]; // Disassembly operand buffer

int source = 0; // Non-zero to disassemble in source format
int f9info = 0; // Non-zero to output only code / data map info for f9dasm
int _debug = 0; // Non-zero to print debug information

void usage() {
	fflush(stderr);
	printf("Usage:\ndiffdasm <options> <module>\nDisassemble 6809 OS9 module to a diffable format.\n\n");
	printf("Options:\n--base xxxx Specifies a hex base address (defaults to zero)\n");
	printf("--exec xxxx Specifies a hex execution address. Can use multiple times.\n");
	printf("--source Output in assembler source format rather than diff format.\n");
	printf("--f9info Output in f9dasm info file format rather than diff format.\n");
	printf("--debug  Output debugging information.\n");
    exit(1);
}

void init() {
	intstack_init(&addrStack, STACKLIMIT);
	intstack_init(&labelStack, STACKLIMIT);
}

void processArgs(int argc, char **argv) {
	unsigned address;
	if (argc < 2) {
		fprintf(stderr, "ERROR: input module required\n");
	    usage();
	}
	while (++argv,--argc) {
		if (!strcmp(*argv,"--exec")) {
			// Push the specified entry address onto the stack
			if ( argc < 2) {
				fprintf(stderr, "ERROR: --exec requires argument\n");
				usage();
			}
			++argv, --argc;
			sscanf(*argv, "%x", &address);
			intstack_push(&addrStack, address);
		} else if (!strcmp(*argv,"--base")) {
			// Save the specified base address
			if ( argc < 2) {
				fprintf(stderr, "ERROR: --base requires argument\n");
				usage();
			}
			++argv, --argc;
			sscanf(*argv, "%x", &baseAddr);
		} else if (!strcmp(*argv,"--source")) {
			// Flag that we want source output
			source = 1;
		} else if (!strcmp(*argv,"--f9info")) {
			// Flag that we want f9dasm info output
			f9info = 1;
		} else if (!strcmp(*argv,"--debug")) {
			// Flag that we want debug output
			_debug = 1;
		} else {
			// Treat as path to module
			inFileName = *argv;
		}
	}
}

void loadFile(char *fName) {
	FILE		*fp1;
	unsigned char *mp;
	size_t		moduleLength;

	if (_debug) printf("loadFile(%s)...\n", fName);
	if (NULL == fName) {
		usage();
	}
	if (!(fp1 = fopen(fName,"rb"))) {
		fprintf(stderr, "ERROR: unable to open '%s'\n", fName);
		usage();
	}
	fseek(fp1, 0, SEEK_END);
	moduleLength = ftell(fp1);
	rewind(fp1);

	// Try to allocate memory for module(s)
	mf_init(&input, moduleLength, fName);

	// Read the module(s) in
	if (moduleLength != fread(input.storage, 1, moduleLength, fp1)) {
		fprintf(stderr, "ERROR: Couldn't load '%s'.\n", fName);
		exit(1);
	}
	fclose(fp1);

	// Try to allocate memory for map
	if (_debug) printf("loadFile(%s): allocating $%04X bytes for map\n", fName, (int)moduleLength);
	mm_init(&map, moduleLength, fName);

	if (_debug) printf("loadFile(%s): loaded $%04X bytes\n", fName, (int)moduleLength);
}

// Calculate an indirect address (load from offset, then adjust by jtOffset)
unsigned short getAddrInd(MemoryFile *mod, unsigned offset, unsigned jtOffset) {
	unsigned short entryPoint;
	entryPoint = M6809_get16(mod, offset) + jtOffset;
	return entryPoint;
}

// Push an indirect address (load from offset, then adjust by jtOffset)
void pushAddrInd(MemoryFile *mod, unsigned base, unsigned offset, unsigned jtOffset) {
	intstack_push(&addrStack, base + getAddrInd(mod, base + offset, jtOffset));
}

char strTmp[STRMAX];
char* stringAt(MemoryFile *mod, unsigned offset) {
	// Copies an MSB-terminated string to a null-terminated string
	// WARNING: Returns a static buffer that is overwritten on each call
	char *p = strTmp;
	char *pastEnd = strTmp + STRMAX - 2;
	char *src = (char*)(mod->storage + offset);
	char c;
	while ((p != pastEnd) && (src <= (char*)mod->end) && ((c = *src++) > 0)) {
		*p++ = c;
	}
	*p++ = c & 0x7f;
	*p++ = '\0';
	return strTmp;
}

void inferEntry(MemoryFile *mod) {
	// Check for concatenation of OS9 modules
	int offset = 0;
	while (offset < mod->length) {
		// An executable OS9 module has at least a 12-byte module header and a 3-byte CRC
		if (mod->length - offset >= 12 &&
				mod->storage[offset+0] == SYNC_1 &&
				mod->storage[offset+1] == SYNC_2) {
			// First nine bytes of all OS9 modules are the same
			short moduleType;
			int modSize = getAddrInd(mod, offset+2, 0);
			int nameStart = getAddrInd(mod, offset+4, 0);
			char* moduleName = stringAt(mod, offset+nameStart);
			//printf("Found module: '%s' ($%04X bytes)\n", moduleName, modSize);
			mm_setFDB(&map, offset+0, 6); // Sync, size, name
			mm_set(&map, offset+6, MM_FCB, 3); // TYLA, ATRV, parity
			mm_setString(&map, offset+nameStart, strlen(moduleName));
			mm_setLabel(&map, offset+nameStart, 1);
			// Last three bytes are the CRC, always
			mm_set(&map, offset+modSize-3, MM_FCB, 3);
			if (mod->length - offset >= 15 && (mod->storage[offset+6] & 0x0f) == ML_OBJCT) {
				// For OS9 modules we can stack the module entry point,
				// and possibly dispatch table entry points depending
				// on the module type.
				moduleType = mod->storage[offset+6] & 0xF0;
				switch (moduleType) {
					case MT_PRGRM: // Program
					case MT_SBRTN: // Subroutine
					case MT_SYSTM: // System
						//printf("Processing Executable Module: '%s'\n", moduleName);
						mm_setFDB(&map, offset+9, 4); // exec, storage
						pushAddrInd(mod, offset, 9, 0); // Single entry address
						break;
					case MT_FLMGR: // File Manager
						//printf("Processing File Manager: '%s'\n", moduleName);
						mm_setFDB(&map, offset+9, 4); // exec, storage
						pushAddrInd(mod, offset, 9, 0); // Create
						pushAddrInd(mod, offset, 9, 3); // Open
						pushAddrInd(mod, offset, 9, 6); // MakDir
						pushAddrInd(mod, offset, 9, 9); // ChgDir
						pushAddrInd(mod, offset, 9, 12); // Delete
						pushAddrInd(mod, offset, 9, 15); // Seek
						pushAddrInd(mod, offset, 9, 18); // Read
						pushAddrInd(mod, offset, 9, 21); // Write
						pushAddrInd(mod, offset, 9, 24); // ReadLn
						pushAddrInd(mod, offset, 9, 27); // WriteLn
						pushAddrInd(mod, offset, 9, 30); // GetStt
						pushAddrInd(mod, offset, 9, 33); // SetStt
						pushAddrInd(mod, offset, 9, 36); // Close
						break;
					case MT_DRIVR: // Device Driver
						//printf("Processing Device Driver: '%s'\n", moduleName);
						mm_setFDB(&map, offset+9, 4); // exec, storage
						pushAddrInd(mod, offset, 9, 0); // Init
						pushAddrInd(mod, offset, 9, 3); // Read
						pushAddrInd(mod, offset, 9, 6); // Write
						pushAddrInd(mod, offset, 9, 9); // GetStt
						pushAddrInd(mod, offset, 9, 12); // SetStt
						pushAddrInd(mod, offset, 9, 15); // Term
						break;
					default:
						// There's no execution entry point for these types
						break;
				}
			}
			offset += modSize;
		} else {
			// Not a module; skip to end
			// NOTE: Could scan for sync bytes here
			offset = mod->length;
		}
	}

	// If no other entry point, start at offset zero
	if (intstack_isEmpty(&addrStack)) {
		intstack_push(&addrStack,0x0000);
	}
}

void dumpStack() {
	int depth, at;
	printf("Address Stack:\n");
	depth = intstack_size(&addrStack);
	for (at=0; at<depth; at++) {
		printf("%s$%04X\n", (at==0)?"  -> ":"     ", intstack_probe(&addrStack, at));
	}
}

int couldBeString(MemoryFile *mod, int entryPoint) {
	// Here, we speculatively look forward from offset checking
	// for 7-bit ASCII sequences with only certain control
	// characters allowed, and ending with MSB set. If there
	// are no invalid characters this "could be" a string.
	//
	// Returns 0 if bad, else # bytes processed including leaf.
	// Does not set a label.
	//
	// NOTE: Just because something passes this check, doesn't
	// mean it's really text. The longer the returned segment
	// is, the higher the probability that it's text.
	unsigned char flags, type;
	int length, offset = 0;
	//printf("Speculatively checking for string at $%04X...\n", entryPoint);
	do {
		char c = mod->storage[entryPoint + offset];
		char cc = c & 0x7F;
		int valid = ((cc >= 0x20) || (cc == '\n') || (cc == '\r') || (c == '\t') || (c == 0x1b));
		if (valid) {
			// One of the chars we're willing to speculatively consider valid
			if ((type=mm_type(&map, entryPoint + offset)) == MM_UNKNOWN) {
				// We haven't visited this byte before
				++offset;
				if (c != cc) break; // Stop at end of string
			} else {
				// Already visited this byte; it's not a string.
				return 0;
			}
		} else {
			// Invalid character; say it's not a string
			return 0;
		}
	} while (entryPoint + offset < mod->length);
	return offset;
}

int couldBeCode(MemoryFile *mod, int entryPoint) {
	// Here, we speculatively disassemble forward from offset
	// checking only the linear instruction stream; if it has
	// no invalid opcodes until the leaf this "could be" code.
	//
	// Returns 0 if bad, else # bytes processed including leaf.
	// Does not set a label.
	//
	// NOTE: Just because something passes this check, doesn't
	// mean it's really code. The longer the returned segment
	// is, the higher the probability that it's code.
	unsigned char flags, type;
	int length, offset = 0;
	//printf("Speculatively disassembling at $%04X...\n", entryPoint);
	do {
		flags = M6809_flags(mod, entryPoint + offset);
		if (flags & HAS_6809) {
			// Valid opcode
			if ((type=mm_type(&map, entryPoint + offset)) == MM_UNKNOWN) {
				// We haven't visited this code before - linear
				// Don't worry about any branch destinations, etc
				length = M6809_bytes(mod, entryPoint + offset);
				offset += length;
			} else {
				// Already visited this code; stop looking at code here
				// by faking that this is a "leaf" (e.g. JMP, BRA, RTS)
				// However, if the middle of an instruction is next,
				// is not code.
				if (type == MM_CODE) return 0;
				flags |= LEAF;
			}
		} else {
			// Invalid opcode; this is not code
			return 0;
		}
	} while ((entryPoint + offset < mod->length) && !(flags & LEAF));
	return offset;
}

void mapCode(MemoryFile *mod) {
	int entryPoint, length, dest, eff, run;
	unsigned char flags, type;

	// Build the map based on linear and (easy) branch traversal
	while (!intstack_isEmpty(&addrStack)) {
		entryPoint = intstack_pop(&addrStack);
		//printf("Popping $%04X...\n", entryPoint);
		mm_setLabel(&map, entryPoint, 1);  // We know this has a label
		do {
			flags = M6809_flags(mod, entryPoint);
			if (flags & HAS_6809) {
				// Valid opcode
				if ((type=mm_type(&map, entryPoint)) == MM_UNKNOWN) {
					// We haven't visited this code before
					length = M6809_bytes(mod, entryPoint);
					mm_setCode(&map, entryPoint, length);
					// If there's a transfer address, push it
					dest = -1;
					if (flags & TRANSFER) {
						dest = M6809_transfer(mod, entryPoint);
						if (dest != -1) {
							// We know what the transfer address is! Save it for later
							intstack_push(&addrStack, dest);
						}
					}
					// Save other PC relative references for later
					// These could be to data rather than code, so label them immediately
					eff = M6809_pcrel(mod, entryPoint);
					if ((eff != dest) && (eff != -1)) {
						type = mm_type(&map, eff);
						mm_setLabel(&map, eff, 1);
						if (type == MM_UNKNOWN) {
							intstack_push(&labelStack, eff);
						}
					}
					// Advance past instruction
					entryPoint += length;
				} else {
					// Already visited this code; stop looking at code here
					// by faking that this is a "leaf" (e.g. JMP, BRA, RTS)
					// Ideally this means we are at type = MM_CODE but there could
					// be other valid options with hand-optimized code
					flags |= LEAF;
				}
			} else {
				// Invalid opcode; stop looking at code here
				// by faking that this is a "leaf" (e.g. JMP, BRA, RTS)
				flags |= LEAF;
			}
		} while ((entryPoint < mod->length) && !(flags & LEAF));
	}

	// Address stack is empty

	// Extend the map based on speculative disassembly from the labelStack
	length = 0;
	//printf("Speculatively checking labelStack for referenced regions...\n");
	while (!intstack_isEmpty(&labelStack)) {
		eff = intstack_pop(&labelStack);
		if (mm_type(&map, eff) == MM_UNKNOWN) {
			if ((run=couldBeCode(mod, eff)) >= CODE_THRESHOLD) {
				// Assume a long-enough potential code run is code
				intstack_push(&addrStack, eff);
				++length;
			} else if ((run=couldBeString(mod, eff)) >= STRING_THRESHOLD) {
				// Assume a long-enough potential string is a string
				mm_setString(&map, eff, run);
			}
		}
	}
	if (!length) {
		// No changes this pass due to PC relative references
		// Try walking through the whole map looking for code or strings.
		// This is a higher-risk speculation
		//printf("Speculatively checking map for UNKNOWN regions...\n");
		eff = 0;
		while (eff < map.maxElements) {
			if (mm_type(&map, eff) == MM_UNKNOWN) {
				if ((run=couldBeCode(mod, eff)) >= CODE_THRESHOLD) {
					// Assume a long-enough potential code run is code
					intstack_push(&addrStack, eff);
					++length;
					eff += run;
				} else if ((run=couldBeString(mod, eff)) >= STRING_THRESHOLD) {
					// Assume a long-enough potential string is a string
					mm_setString(&map, eff, run);
					eff += run;
				} else {
					++eff;
				}
			} else {
				++eff;
			}
		}
	}
	if (length) {
		// There were changes to the stack; recurse
		mapCode(mod);
	}
}

void dumpMap() {
	mm_dump(&map, 64);
}

void dumpBytes(MemoryFile *mod, int offset, int length) {
	int i;
	for (i=0; i<length; i++)
	{
		printf(" %02X", mod->storage[offset+i]);
	}
	printf("\n");
}

void dumpPairs(MemoryFile *mod, int offset, int length) {
	int i, odd = (length & 1);
	if (odd) {
		--length;
	}
	for (i=0; i<length; i+=2)
	{
		printf(" %02X%02X", mod->storage[offset+i], mod->storage[offset+i+1]);
	}
	printf("\n");
	if (odd) {
		dumpBytes(mod, offset+length, 1);
	}
}

void dumpString(MemoryFile *mod, int offset, int length) {
	int i;
	printf(" \"");
	for (i=0; i<length; i++)
	{
		unsigned char c = mod->storage[offset+i] & 0x7f;
		switch (c) {
			case '\t':
				printf("\\t");
				break;
			case '\n':
				printf("\\n");
				break;
			case '\r':
				printf("\\r");
				break;
			case 0x1B:
				printf("\\1B");
				break;
			default:
				printf("%c",c);
				break;
		}
	}
	printf("\"\n");
}

void disassemble(MemoryFile *mod, MemoryMap *map) {
	// TODO: map line numbers to eff values
	int run, length, type;
	int eff = 0;
	char *label;
	//printf("Disassembling...\n");
	while (eff < map->maxElements) {
		type = mm_type(map, eff);
		run = mm_runLength(map, eff);
		//printf("%d: ", lineCount+1);
		if (source) {
			if ((label=M6809_label(map,eff))) {
				printf("%s", label);
			}
		}
		switch (type) {
			case MM_UNKNOWN:
			case MM_FCB:
				// Get the run length, crop to 16
				if (run > 16) run = 16;
				printf(" FCB");
				dumpBytes(mod, eff, run);
				break;
			case MM_FCC:
				// Output as-is
				printf(" FCC");
				dumpString(mod, eff, run);
				break;
			case INVALID:
				// Get the run length, crop to 16
				if (run > 16) run = 16;
				printf(" ???");
				dumpBytes(mod, eff, run);
				break;
			case MM_FDB:
				// Get the run length, crop to 16
				if (run > 16) run = 16;
				printf(" FDB");
				dumpPairs(mod, eff, run);
				break;
			case MM_FCS:
				// Output as-is
				printf(" FCS");
				dumpString(mod, eff, run);
				break;
			case MM_CODE1:
				// Output as-is
				printf(" %s %s\n", M6809_opcode(mod, eff), M6809_operands(operands, mod, map, eff));
				break;
			default:
				// Output as a single FCB
				run = 1;
				printf(" FCB");
				dumpBytes(mod, eff, run);
				break;
		}
		// Track lines for later diff reversal
		++lineCount;
		lines[lineCount].lineNumner = lineCount;
		lines[lineCount].startOffset = eff;
		lines[lineCount].endOffset = eff + run - 1;
		// Advance to next part of module
		eff += run;
	}
}

void infogen(MemoryFile *mod) {
	int run, length, type;
	int eff = 0;
	//printf("Disassembling...\n");
	while (eff < map.maxElements) {
		type = mm_type(&map, eff);
		run = mm_runLength(&map, eff);
		//printf("%d: ", lineCount+1);
		switch (type) {
			case INVALID:
			case MM_UNKNOWN:
			case MM_FCB:
				printf("HEX 0x%04x-0x%04x\n", eff, eff+run-1);
				break;
			case MM_FCS:
			case MM_FCC:
				printf("CHAR 0x%04x-0x%04x\n", eff, eff+run-1);
				break;
			case MM_FDB:
				printf("WORD 0x%04x-0x%04x\n", eff, eff+run-1);
				break;
			case MM_CODE1:
				printf("CODE 0x%04x-0x%04x\n", eff, eff+run-1);
				break;
			default:
				// Output as a single FCB
				run = 1;
				printf("HEX 0x%04x-0x%04x\n", eff, eff+run-1);
				break;
		}
		// Advance to next part of module
		eff += run;
	}
}

int main(int argc, char **argv) {
	init();
	processArgs(argc, argv);
	loadFile(inFileName);
	inferEntry(&input);
	//dumpStack();
	mapCode(&input);
	//dumpMap();
	if (f9info) {
		infogen(&input);
	} else {
		disassemble(&input, &map);
	}
	return 0;
}

