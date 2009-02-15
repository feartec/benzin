/*
 *  memfile.c
 *  0CFMaker
 *
 *  Created by Alex Marshall on 09/02/08.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "memfile.h"

void ReadMemory(void* dst, size_t size, size_t count, MEMORY* src)
{
	if((src->mode & 0x1) == 0) {
		printf("Invalid mode for reading!\n");
		return;
	}
	u8* data = (u8*)dst;
	if(((size * count) + src->position) > src->memorysize) {
		printf("Tried to over read.\n");
		return;
	}
	memcpy(data, ((u8*)src->memory) + src->position, size * count);
	src->position += size * count;
}

char ReadMemoryChar(MEMORY* src)
{
	char ret;
	ReadMemory(&ret, 1, 1, src);
	return ret;
}

void WriteMemory(void* dst, size_t size, size_t count, MEMORY* src)
{
	if((src->mode & 0x2) == 0) {
		printf("Invalid mode for writing!\n");
		return;
	}
	u8* data = (u8*)dst;
	if(((size * count) + src->position) > src->memorysize) {
		printf("Tried to over read.\n");
		return;
	}
	memcpy(((u8*)src->memory) + src->position, data, size * count);
	src->position += size * count;
}

void WriteMemoryChar(char inchar, MEMORY* src)
{
	WriteMemory(&inchar, 1, 1, src);
}

MEMORY* OpenMemory(void* indata, size_t size, char mode)
{
	MEMORY* mem = malloc(sizeof(MEMORY));
	if((mode & 0x2) == 0x0)
		mem->memory = indata;
	else
		mem->memory = calloc(size, 1);
	if(mem->memory == NULL)
		return NULL;
	mem->memorysize = size;
	mem->position = 0;
	mem->mode = mode;
	return mem;
}

void* CloseMemory(MEMORY* mem)
{
	void* ret = mem->memory;
	mem->memory = NULL;
	mem->memorysize = 0;
	mem->position = 0;
	mem->mode = 0;
	free(mem);
	return ret;
}

void* GetMemory(MEMORY* mem)
{
	return mem->memory;
}

void SeekMemory(MEMORY* mem, size_t location, int type)
{
	switch(type) {
		case SEEK_SET:
			mem->position = location;
			break;
			
		case SEEK_END:
			mem->position = mem->memorysize - location;
			break;
	}
}

size_t TellMemory(MEMORY* mem)
{
	return mem->position;
}

size_t SizeMemory(MEMORY* mem)
{
	return mem->memorysize;
}
