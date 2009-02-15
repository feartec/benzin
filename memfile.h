/*
 *  memfile.h
 *  0CFMaker
 *
 *  Created by Alex Marshall on 09/02/08.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MEMFILE_H_
#define _MEMFILE_H_

#include "types.h"

#define mread		ReadMemory
#define mwrite		WriteMemory
#define mopen		OpenMemory
#define mclose		CloseMemory
#define mseek		SeekMemory
#define mgetc		ReadMemoryChar
#define mputc		WriteMemoryChar
#define mtell		TellMemory
#define msize		SizeMemory
#define mgetm		GetMemory

typedef struct
{
	void*	memory;			// Pointer.
	size_t	memorysize;		// Size.
	size_t	position;		// Where we are.
	char	mode;			// What mode opened (1 = read, 2 = write, 3 = both)
} MEMORY;

void ReadMemory(void* dst, size_t size, size_t count, MEMORY* src);
char ReadMemoryChar(MEMORY* src);
void WriteMemory(void* dst, size_t size, size_t count, MEMORY* src);
void WriteMemoryChar(char inchar, MEMORY* src);
MEMORY* OpenMemory(void* indata, size_t size, char mode);
void* CloseMemory(MEMORY* mem);
void* GetMemory(MEMORY* mem);
void SeekMemory(MEMORY* mem, size_t location, int type);
size_t TellMemory(MEMORY* mem);
size_t SizeMemory(MEMORY* mem);

#endif //_MEMFILE_H_
