/*
 * statsOS9.c
 *
 *  Created on: Feb 15, 2020
 *      Author: cburke
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "os9stuff.h"
#include "os9.h"
#include "memoryfile.h"

// Every OS9 SVC in numerical order from 0x00
char *svcNames[] = {
	"F$LINK",	/* 00 Link to Module */
	"F$LOAD",	/* 01 Load Module from File */
	"F$UNLINK",	/* 02 Unlink Module */
	"F$FORK",	/* 03 Start New Process */
	"F$WAIT",	/* 04 Wait for Child Process to Die */
	"F$CHAIN",	/* 05 Chain Process to New Module */
	"F$EXIT",	/* 06 Terminate Process */
	"F$MEM",	/* 07 Set Memory Size */
	"F$SEND",	/* 08 Send Signal to Process */
	"F$ICPT",	/* 09 Set Signal Intercept */
	"F$SLEEP",	/* 0a Suspend Process */
	"F$SSPD",	/* 0b Suspend Process */
	"F$ID",		/* 0c Return Process ID */
	"F$SPRIOR",	/* 0d Set Process Priority */
	"F$SSWI",	/* 0e Set Software Interrupt */
	"F$PERR",	/* 0f Print Error */

	"F$PRSNAM",	/* 10 Parse Pathlist Name */
	"F$CMPNAM",	/* 11 Compare Two Names */
	"F$SCHBIT",	/* 12 Search Bit Map */
	"F$ALLBIT",	/* 13 Allocate in Bit Map */
	"F$FELBIT",	/* 14 Deallocate in Bit Map */
	"F$TIME",	/* 15 Get Current Time */
	"F$STIME",	/* 16 Set Current Time */
	"F$CRC",	/* 17 Generate CRC */
	"F$GPRDSC",	/* 18 get Process Descriptor copy */
	"F$GBLKMP",	/* 19 get System Block Map copy */
	"F$GMODDR",	/* 1a get Module Directory copy */
	"F$CPYMEM",	/* 1b Copy External Memory */
	"F$SUSER",	/* 1c Set User ID number */
	"F$UNLOAD",	/* 1d Unlink Module by name */
	"$1e",
	"$1f",

	"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27",
	"F$SRQMEM",	/* 28 System Memory Request */
	"F$SRTMEM",	/* 29 System Memory Return */
	"F$IRQ",	/* 2a Enter IRQ Polling Table */
	"F$IOQU",	/* 2b Enter I/O Queue */
	"F$APROC",	/* 2c Enter Active Process Queue */
	"F$NPROC",	/* 2d Start Next Process */
	"F$VMODUL",	/* 2e Validate Module */
	"F$FIND64",	/* 2f Find Process/Path Descriptor */

	"F$ALL64",	/* 30 Allocate Process/Path Descriptor */
	"F$RET64",	/* 31 Return Process/Path Descriptor */
	"F$SSVC",	/* 32 Service Request Table Initialization */
	"F$IODEL",	/* 33 Delete I/O Module */
	"F$SLINK",	/* 34 System Link */
	"F$BOOT",	/* 35 Bootstrap System */
	"F$BTMEM",	/* 36 Bootstrap Memory Request */
	"F$GPROCP",	/* 37 Get Process ptr */
	"F$MOVE",	/* 38 Move Data (low bound first) */
	"F$ALLRAM",	/* 39 Allocate RAM blocks */
	"F$ALLIMG",	/* 3a Allocate Image RAM blocks */
	"F$FELIMG",	/* 3b Deallocate Image RAM blocks */
	"F$SETIMG",	/* 3c Set Process DAT Image */
	"F$FREELB",	/* 3d Get Free Low Block */
	"F$FREEHB",	/* 3e Get Free High Block */
	"F$ALLTSK",	/* 3f Allocate Process Task number */

	"F$FELTSK",	/* 40 Deallocate Process Task number */
	"F$SETTSK",	/* 41 Set Process Task DAT registers */
	"F$RESTSK",	/* 42 Reserve Task number */
	"F$RELTSK",	/* 43 Release Task number */
	"F$FATLOG",	/* 44 Convert DAT Block/Offset to Logical */
	"F$FATTMP",	/* 45 Make temporary DAT image */
	"F$LDAXY",	/* 46 Load A [X,[Y]] */
	"F$LDAXYP",	/* 47 Load A [X+,[Y]] */
	"F$LDDDXY",	/* 48 Load D [D+X,[Y]] */
	"F$LDABX",	/* 49 Load A from 0,X in task B */
	"F$STABX",	/* 4a Store A at 0,X in task B */
	"F$ALLPRC",	/* 4b Allocate Process Descriptor */
	"F$FELPRC",	/* 4c Deallocate Process Descriptor */
	"F$ELINK",	/* 4d Link using Module Directory Entry */
	"F$FMODUL",	/* 4e Find Module Directory Entry */
	"F$MAPBLK",	/* 4f Map Specific Block */

	"F$CLRBLK",	/* 50 Clear Specific Block */
	"F$FELRAM",	/* 51 Deallocate RAM blocks */
	"$52", "$53", "$54", "$55", "$56", "$57",
	"$58", "$59", "$5A", "$5B", "$5C", "$5D", "$5E", "$5F",

	"$60", "$61", "$62", "$63", "$64", "$65", "$66", "$67",
	"$68", "$69", "$6A", "$6B", "$6C", "$6D", "$6E", "$6F",

	"$70", "$71", "$72", "$73", "$74", "$75", "$76", "$77",
	"$78", "$79", "$7A", "$7B", "$7C", "$7D", "$7E", "$7F",

	"I$ATTACH",	/* 80 Attach I/O Device */
	"I$DETACH",	/* 81 Detach I/O Device */
	"I$FUP",	/* 82 Duplicate Path */
	"I$CREATE",	/* 83 Create New File */
	"I$OPEN",	/* 84 Open Existing File */
	"I$MAKDIR",	/* 85 Make Directory File */
	"I$CHGDIR",	/* 86 Change Default Directory */
	"I$FELETE",	/* 87 Delete File */
	"I$SEEK",	/* 88 Change Current Position */
	"I$READ",	/* 89 Read Data */
	"I$WRITE",	/* 8a Write Data */
	"I$READLN",	/* 8b Read Line of ASCII Data */
	"I$WRITLN",	/* 8c Write Line of ASCII Data */
	"I$GETSTT",	/* 8d Get Path Status */
	"I$SETSTT",	/* 8e Set Path Status */
	"I$CLOSE",	/* 8f Close Path */

	"I$FELETX",	/* 90 Delete from current exec dir */
	"$91", "$92", "$93", "$94", "$95", "$96", "$97",
	"$98", "$99", "$9A", "$9B", "$9C", "$9D", "$9E", "$9F",

	"$A0", "$A1", "$A2", "$A3", "$A4", "$A5", "$A6", "$A7",
	"$A8", "$A9", "$AA", "$AB", "$AC", "$AD", "$AE", "$AF",

	"$B0", "$B1", "$B2", "$B3", "$B4", "$B5", "$B6", "$B7",
	"$B8", "$B9", "$BA", "$BB", "$BC", "$BD", "$BE", "$BF",

	"$C0", "$C1", "$C2", "$C3", "$C4", "$C5", "$C6", "$C7",
	"$C8", "$C9", "$CA", "$CB", "$CC", "$CD", "$CE", "$CF",

	"$D0", "$D1", "$D2", "$D3", "$D4", "$D5", "$D6", "$D7",
	"$D8", "$D9", "$DA", "$DB", "$DC", "$DD", "$DE", "$DF",

	"$E0", "$E1", "$E2", "$E3", "$E4", "$E5", "$E6", "$E7",
	"$E8", "$E9", "$EA", "$EB", "$EC", "$ED", "$EE", "$EF",

	"$F0", "$F1", "$F2", "$F3", "$F4", "$F5", "$F6", "$F7",
	"$F8", "$F9", "$FA", "$FB", "$FC", "$FD", "$FE", "$FF"
};

/* offset points to the first byte of the SWI2 instruction */
char* OS9_svcName(MemoryFile* mod, int offset) {
	if ((mod->storage[offset+0] == SWI2_1) && (mod->storage[offset+1] == SWI2_2)) {
		// OS9 system call has pseudo-operand
		return svcNames[mod->storage[offset+2]];
	}
	return "";
}

