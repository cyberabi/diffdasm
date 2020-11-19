/*
 * os9stuff.h
 *
 *  Created on: Nov 22, 2019
 *      Author: cburke
 */

#ifndef OS9STUFF_H_
#define OS9STUFF_H_

#define SYNC_1	0x87
#define SYNC_2	0xCD

#define SWI2_1	0x10
#define SWI2_2	0x3F

/*
* Module Type Values
*/
#define MT_DEVIC	0xF0                 // Device Descriptor Module
#define MT_DRIVR    0xE0                 // Physical Device Driver
#define MT_FLMGR	0xD0                 // File Manager
#define MT_SYSTM	0xC0                 // System Module
#define MT_SHELLSUB	0x50                 // Shell+ shell sub module
#define MT_DATA		0x40                 // Data Module
#define MT_MULTI    0x30                 // Multi-Module
#define MT_SBRTN	0x20                 // Subroutine Module
#define MT_PRGRM	0x10                 // Program Module

/*
* Module Language Values
*/
#define ML_OBJCT	0x01                 // 6809 Object Code Module
#define ML_ICODE	0x02                 // Basic09 I-code
#define ML_PCODE	0x03                 // Pascal P-code
#define ML_CCODE	0x04                 // C I-code
#define ML_CBLCODE	0x05                 // Cobol I-code
#define ML_FRTNCODE	0x06                 // Fortran I-code
#define ML_OBJ6309	0x07                 // 6309 object code

#endif /* OS9STUFF_H_ */
