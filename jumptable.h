/*
 * jumptable.h
 *
 *  Created on: Aug 10, 2026
 *      Author: cburke
 */

#ifndef JUMPTABLE_H_
#define JUMPTABLE_H_

#define MAPTYPE_EXT 0
#define MAPTYPE_PIC 1
#define MAPTYPE_REL 2
#define MAPTYPE_LBR 3

/* Process jumptable of EXTENDED (absolute) addresses */
void jt_extended(MemoryFile *mod, unsigned start, unsigned end);

/* Process jumptable of position-independent (* relative) addresses */
void jt_pic(MemoryFile *mod, unsigned start, unsigned end);

/* Process jumptable of PIC addresses relative to base of table */
void jt_pic_relative(MemoryFile *mod, unsigned start, unsigned end);

/* Process jumptable of LBRA instructions (e.g. in OS9 module) */
void jt_lbra(MemoryFile *mod, unsigned start, unsigned end);

#endif /* JUMPTABLE_H_ */
