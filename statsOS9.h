/*
 * statsOS9.h
 *
 *  Created on: Feb 15, 2020
 *      Author: cburke
 */

#ifndef STATSOS9_H_
#define STATSOS9_H_

#include "memoryfile.h"

// Return SVC name for OS9 instruction
char* OS9_svcName(MemoryFile* mod, int offset);

#endif /* STATSOS9_H_ */
