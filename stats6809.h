/*
 * stats6809.h
 *
 *  Created on: Nov 23, 2019
 *      Author: cburke
 */

#ifndef STATS6809_H_
#define STATS6809_H_

#include "memoryfile.h"
#include "memorymap.h"

typedef struct Instruction {
	char *mnemonic;
	char *altMnemonic; // Used when obscuring offset sizes (e.g. lbra and bra -> jbra)
	unsigned short  mode;
	unsigned short  bytes;
	unsigned short	flags;
} Instruction;

// Addressing modes
// Note that for INDEXED, PREBYTE10, and PREBYTE11 the byte count is incorrect
#define	DIRECT		0x00
#define PREBYTE10 	0x01
#define PREBYTE11	0x02
#define INHERENT	0x03
#define INVALID		0x04
#define REL_8		0x05
#define REL_16		0x06
#define IMMED_8		0x07
#define IMMED_16	0x08
#define IMMED_32	0x09
#define REGISTER	0x0A
#define INDEXED		0x0B
#define EXTENDED	0x0C
#define SINGLE_BIT	0x0D
#define WINDEXED    0x0E
#define REG_PULL_S  0x0F
#define REG_PULL_U  0x10
#define REG_PUSH_S  0x11
#define REG_PUSH_U  0x12

// Indexed modes
#define OFFSET_0	0x20
#define OFFSET_5	0x21
#define OFFSET_8	0x22
#define OFFSET_16	0x23
#define OFFSET_A	0x24
#define OFFSET_B	0x25
#define OFFSET_D	0x26
#define OFFSET_E	0x27
#define OFFSET_F	0x28
#define OFFSET_W	0x29
#define POSTINC_1	0x2A
#define POSTINC_2	0x2B
#define PREDEC_1	0x2C
#define PREDEC_2	0x2D
#define PCR_8		0x2E
#define PCR_16		0x2F
#define IOFFSET_0	0x30
#define IOFFSET_8	0x31
#define IOFFSET_16	0x32
#define IOFFSET_A	0x33
#define IOFFSET_B	0x34
#define IOFFSET_D	0x35
#define IOFFSET_E	0x36
#define IOFFSET_F	0x37
#define IOFFSET_W	0x38
#define IPOSTINC_2	0x39
#define IPREDEC_2	0x3A
#define IPCR_8		0x3B
#define IPCR_16		0x3C
#define IEXTENDED	0x3D
#define IDXINVALID	0x3E

// Register bit mask (indexed addressing)
#define IREG_X		0b00000000
#define IREG_Y		0b00100000
#define IREG_U		0b01000000
#define IREG_S		0b01100000

#define IREG_MASK	0b01100000

// Register bit mask (psh / pul)
// Note that U and S are same bit; psh/puls uses u; psh/pulu uses s
#define PREG_CCR	0b00000001
#define PREG_B		0b00000010
#define PREG_A		0b00000100
#define PREG_DPR	0b00001000
#define PREG_X		0b00010000
#define PREG_Y		0b00100000
#define PREG_U		0b01000000
#define PREG_S		0b01000000
#define PREG_PC		0b10000000

// Register ID (exg / tfr)
#define TREG_D		0b00000000
#define TREG_X		0b00000001
#define TREG_Y		0b00000010
#define TREG_U		0b00000011
#define TREG_S		0b00000100
#define TREG_PC		0b00000101
#define TREG_A		0b00001000
#define TREG_B		0b00001001
#define TREG_CC		0b00001010
#define TREG_DPR	0b00001011

#define TREG_MASK	0b00001111

// NOTE: PSHx pc and PULx pc can also cause a control transfer, but aren't LEAF tagged
// NOTE: SWIx can also cause a control transfer, but handling is OS specific special case
// LEAF means the contiguous execution thread of instructions ends here.
// TRANSFER means the contiguous execution thread continues, but there's also another entry point
#define NEITHER		0
#define HAS_6809	0b00000001
#define HAS_6309	0b00000010

#define LEAF    	0b10000000
#define TRANSFER	0b01000000

#define CPU_MASK	0b00000011

// Return the number of bytes for the opcode at the specified pointer
short M6809_bytes(MemoryFile* mod, int offset);

// Return the flags for the opcode at the specified pointer
unsigned char M6809_flags(MemoryFile* mod, int offset);

// Fetch two bytes
unsigned short M6809_get16(MemoryFile *mod, int offset);

// Return the destination address for any transfer associated with the opcode
// -1 means no transfer address or can't figure it out (e.g. it depends on register state)
int M6809_transfer(MemoryFile* mod, int offset);

// Return PC relative effective address of the opcode (if any)
// -1 means no relevant address or can't figure it out
int M6809_pcrel(MemoryFile* mod, int offset);

// Return the label (if any) of the instruction
char* M6809_label(MemoryMap* map, int offset);

// Return the opcode of the instruction
char* M6809_opcode(MemoryFile* mod, int offset);

// Return the addressing mode name of the instruction
char* M6809_modeName(MemoryFile* mod, int offset);

// Return operands of the instruction
char* M6809_operands(char* buffer, MemoryFile* mod, MemoryMap* map, int offset);

#endif /* STATS6809_H_ */
