/*
 * statsCoCo3.c
 *
 *  Created on: Nov 28, 2020
 *      Author: cburke
 */

#include "statsCoCo3.h"
#include "stats6809.h"

// Every CoCo3 I/O register in numerical order from 0xFF00
char *cc3IONames[] = {
	// 0xFF00 - 0xFF0F Keyboard Sense and Joystick
	"KBD Sense / Joy Compare",
	"HSYNC / Analog MUX 1",
	"KBD Strobe / RAM Size",
	"VSYNC / Analog MUX 2",
	"", "", "", "",
	"", "", "", "","", "", "", "",

	// 0xFF10 - 0xFF1F Unused (alias of above)
	"", "", "", "","", "", "", "",
	"", "", "", "","", "", "", "",

	// 0xFF20 - 0xFF2F D/A, Cassette, Serial, VDG, Cart
	// NOTE: FF22 and FF23 are duplicated in the GIME.
	// On the CoCo 3, FF22 bits 3-7 are meaningful only
	// internal to GIME
	"CASSIN / SEROUT / D/A",
	"CD / CASSMOT",
	"SERIN / SND1B / RAM Size / RGB / VDG / GIME",
	"CART / SNDEN / GIME",
	"", "", "", "",
	"", "", "", "","", "", "", "",

	// 0xFF30 - 0xFF3F Unused (alias of above)
	"", "", "", "","", "", "", "",
	"", "", "", "","", "", "", "",

	// 0xFF40 - 0xFF4F Cartridge I/O
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",

	// 0xFF50 - 0xFF5F Cartridge I/O
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",

	// 0xFF60 - 0xFF6F Fully Decoded Cartridge
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",

	// 0xFF70 - 0xFF7F Fully Decoded Cartridge, MultiPAK
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "MultiPAK",

	// 0xFF80 - 0xFF8F Fully Decoded Cartridge
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",
	"Cartridge", "Cartridge", "Cartridge", "Cartridge",

	// 0xFF90 - 0xFF9F GIME and DISTO 2MB Upgrade
	"GIME INIT0", "GIME INIT1", "GIME IRQENR", "GIME FIRQENR",
	"GIME TIMERMSB", "GIME TIMERLSB", "", "",
	"GIME VMODE", "GIME VRES", "GIME BRDR", "DISTO 2MB VBANK",
	"GIME VSC", "GIME VOFSMSB", "GIME VOFSLSB", "GIME HOFS",

	// 0xFFA0 - 0xFFAF GIME Task Registers
	"GIME Task 0.0", "GIME Task 0.2", "GIME Task 0.4", "GIME Task 0.6",
	"GIME Task 0.8", "GIME Task 0.A", "GIME Task 0.C", "GIME Task 0.E",
	"GIME Task 1.0", "GIME Task 1.2", "GIME Task 1.4", "GIME Task 1.6",
	"GIME Task 1.8", "GIME Task 1.A", "GIME Task 1.C", "GIME Task 1.E",

	// 0xFFB0 - 0xFFBF GIME Palette Registers
	"GIME Palette 0", "GIME Palette 1", "GIME Palette 2", "GIME Palette 3",
	"GIME Palette 4", "GIME Palette 5", "GIME Palette 6", "GIME Palette 7",
	"GIME Palette 8", "GIME Palette 9", "GIME Palette A", "GIME Palette B",
	"GIME Palette C", "GIME Palette D", "GIME Palette E", "GIME Palette F",

	// 0xFFC0 - 0xFFCF SAM Video Mode Registers
	"SAM Video Mode Bit 0 CLR", "SAM Video Mode Bit 0 SET",
	"SAM Video Mode Bit 1 CLR", "SAM Video Mode Bit 1 SET",
	"SAM Video Mode Bit 2 CLR", "SAM Video Mode Bit 2 SET",
	"SAM RAM Page Bit 0 CLR", "SAM RAM Page Bit 0 SET",
	"SAM RAM Page Bit 1 CLR", "SAM RAM Page Bit 1 SET",
	"SAM RAM Page Bit 2 CLR", "SAM RAM Page Bit 2 SET",
	"SAM RAM Page Bit 3 CLR", "SAM RAM Page Bit 3 SET",
	"SAM RAM Page Bit 4 CLR", "SAM RAM Page Bit 4 SET",

	// 0xFFD0 - 0xFFDF SAM Video Mode Registers
	"SAM RAM Page Bit 5 CLR", "SAM RAM Page Bit 5 SET",
	"SAM RAM Page Bit 6 CLR", "SAM RAM Page Bit 6 SET",
	"SAM PAG1CLR", "SAM PAG1SET",
	"SAM Clock Speed Bit 0 CLR", "SAM Clock Speed Bit 0 SET",
	"SAM Clock Speed Bit 1 CLR", "SAM Clock Speed Bit 1 SET",
	"SAM DRAM Size Bit 0 CLR", "SAM DRAM Size Bit 0 SET",
	"SAM DRAM Size Bit 1 CLR", "SAM DRAM Size Bit 1 SET",
	"SAM ROM/RAM Mode", "SAM All RAM Mode",

	// 0xFFE0 - 0xFFEF Reserved Future Interrupt Vectors
	"", "", "", "","", "", "", "",
	"", "", "", "","", "", "", "",

	// 0xFFF0 - 0xFFFF Interrupt Vectors
	"", "", "", "","", "", "", "",
	"", "", "", "","", "", "", "",
};


/* offset points to the first byte of the potentially immediate instruction */
char* CoCo3_ioNameCode(MemoryFile* mod, int offset) {
	// Simple heuristic; since we don't know what's in the
	// registers at this time, we look to see if this is a
	// 16-bit immediate load, and if so, check for an I/O address.
	int mode = M6809_mode(mod, offset);
	int length = M6809_bytes(mod, offset);
	int	postbyte;

	switch (mode) {
	case IMMED_16:
	case EXTENDED:
		postbyte = M6809_get16(mod, offset+length-2);
		if ((postbyte & 0xFF00) == 0xFF00) {
			// Reference to an I/O page, possibly
			return cc3IONames[postbyte & 0x00FF];
		}
		break;
	default:
		break;
	}
	return "";
}

/* offset points to the first byte of the potentially literal data */
char* CoCo3_ioNameData(MemoryFile* mod, int offset) {
	if (mod->storage[offset+0] == 0xFF) {
		// Reference to an I/O page, possibly
		return cc3IONames[mod->storage[offset+1]];
	}
	return "";
}

