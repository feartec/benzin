/*
 *  brlyt.c
 *  
 *
 *  Created by Alex Marshall on 09/01/27.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "brlyt.h"

#ifdef DEBUGBRLYT
#define dbgprintf	printf
#else
#define dbgprintf	//
#endif //DEBUGBRLYT

char pic1_magic[] = "pic1";
char pan1_magic[] = "pan1";
char bnd1_magic[] = "bnd1";
char wnd1_magic[] = "wnd1";
char lyt1_magic[] = "lyt1";
char grp1_magic[] = "grp1";
char txl1_magic[] = "txl1";
char mat1_magic[] = "mat1";
char fnl1_magic[] = "fnl1";

static size_t BRLYT_fileoffset = 0;

static int FourCCsMatch(fourcc cc1, fourcc cc2)
{
	dbgprintf("FourCCs\n");
	int ret[4];
	dbgprintf("Let's go %08x %08x\n", cc1, cc2);
	ret[0] = (cc1[0] == cc2[0]);
	dbgprintf("Got zero |%02x| |%02x| %d\n", cc1[0], cc2[0], ret[0]);
	ret[1] = (cc1[1] == cc2[1]);
	dbgprintf("Got one |%02x| |%02x| %d\n", cc1[1], cc2[1], ret[1]);
	ret[2] = (cc1[2] == cc2[2]);
	dbgprintf("Got two |%02x| |%02x| %d\n", cc1[2], cc2[2], ret[2]);
	ret[3] = (cc1[3] == cc2[3]);
	dbgprintf("Got three |%02x| |%02x| %d\n", cc1[3], cc2[3], ret[3]);
	int retval;
	if(ret[0] && ret[1] && ret[2] && ret[3])
		retval = 1;
	else
		retval = 0;
	dbgprintf("Got retval %d\n", retval);
	return retval;
}

static void BRLYT_ReadDataFromMemoryX(void* destination, void* input, size_t size)
{
	u8* out = (u8*)destination;
	u8* in = ((u8*)input) + BRLYT_fileoffset;
	memcpy(out, in, size);
}

static void BRLYT_ReadDataFromMemory(void* destination, void* input, size_t size)
{
	BRLYT_ReadDataFromMemoryX(destination, input, size);
	BRLYT_fileoffset += size;
}

int BRLYT_ReadEntries(u8* brlyt_file, size_t file_size, brlyt_header header, brlyt_entry* entries)
{
/*	BRLYT_fileoffset = be16(header.lyt_offset);
	brlyt_entry_header tempentry;
	int i;
	dbgprintf("curr %08x max %08x\n", BRLYT_fileoffset, file_size);
	for(i = 0; BRLYT_fileoffset < file_size; i++) {
		BRLYT_ReadDataFromMemoryX(&tempentry, brlyt_file, sizeof(brlyt_entry_header));
		BRLYT_fileoffset += be32(tempentry.length);
		dbgprintf("curr %08x max %08x\n", BRLYT_fileoffset, file_size);
	}
	int entrycount = i;
	entries = (brlyt_entry*)calloc(i, sizeof(brlyt_entry));
	dbgprintf("%08x\n", entries);
	if(entries == NULL) {
		printf("Couldn't allocate for entries!\n");
		exit(1);
	}
	BRLYT_fileoffset = be16(header.lyt_offset);
	for(i = 0; i < entrycount; i++) {
		dbgprintf("&(entries[i].realhead) = %08x\n", &(entries[i].realhead));
		BRLYT_ReadDataFromMemoryX(&(entries[i].realhead), brlyt_file, sizeof(brlyt_entry_header));
		entries[i].data_location = BRLYT_fileoffset + sizeof(brlyt_entry_header);
		BRLYT_fileoffset += be32(tempentry.length);
	}
	return entrycount;*/
}

void BRLYT_CheckHeaderSanity(brlyt_header header, size_t filesize)
{
	if((header.magic[0] != 'R') || (header.magic[1] != 'L') || (header.magic[2] != 'Y') || (header.magic[3] != 'T')) {
		printf("BRLYT magic doesn't match! %c%c%c%c\n", header.magic[0], header.magic[1], header.magic[2], header.magic[3]);
		exit(1);
	}
	if(filesize != be32(header.filesize)) {
		printf("BRLYT filesize doesn't match!\n");
		exit(1);
	}
}

void PrintBRLYTEntry_pic1(brlyt_entry entry, u8* brlyt_file)
{
}

void PrintBRLYTEntry_lyt1(brlyt_entry entry, u8* brlyt_file)
{
	brlyt_lyt1_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_lyt1_chunk));
	printf("		Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("		unk1: %08x\n", be32(data.unk[0]));
	printf("		unk2: %08x\n", be32(data.unk[1]));
	printf("		unk3: %08x\n", be32(data.unk[2]));
}

void PrintBRLYTEntry_grp1(brlyt_entry entry, u8* brlyt_file)
{
	brlyt_grp1_chunkbase data;
	BRLYT_fileoffset = entry.data_location;
	char		name[16];
	u16		numsubs;
	u16		unk;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_grp1_chunkbase));
	printf("		Name: %s\n", data.name);
	printf("		Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("		Number of subs: %08x\n", be16(data.numsubs));
	printf("		unk: %08x\n", be16(data.unk));
}

void PrintBRLYTEntry_img(brlyt_entry entry, u8* brlyt_file)
{
	brlyt_img_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_img_chunk));
	printf("		Type: %c%c%c%c\n", entry.magic[0], entry.magic[1], entry.magic[2], entry.magic[3]);
	printf("		Number: %d\n", be16(data.num));
	printf("		Offset: %04x\n", be16(data.offs));
}

void PrintBRLYTEntries(brlyt_entry *entries, int entrycnt, u8* brlyt_file)
{
	int i;
	
	for(i = 0; i < entrycnt; i++) {
		printf("\n	Index %d (@%08x):\n", i, entries[i].data_location - 8);
		if((FourCCsMatch(entries[i].magic, pic1_magic) == 1) || (FourCCsMatch(entries[i].magic, pan1_magic) == 1) ||
		   (FourCCsMatch(entries[i].magic, bnd1_magic) == 1) || (FourCCsMatch(entries[i].magic, wnd1_magic) == 1)) {
			dbgprintf("pic1\n");
//			PrintBRLYTEntry_pic1(entries[i], brlyt_file);
			brlyt_pic1_chunk data;
			BRLYT_fileoffset = entries[i].data_location;
			BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_pic1_chunk));
			printf("%08x\n", BRLYT_fileoffset);
			printf("		Name: %s (%08x)\n", data.name);
			printf("		Type: %c%c%c%c\n", entries[i].magic[0], entries[i].magic[1], entries[i].magic[2], entries[i].magic[3]);
			printf("		Flags: %04x (%08x)\n", be16(data.flags));
			printf("		Alpha: %04x (%08x)\n", be16(data.alpha));
			printf("		X: %f (%08x) (%08x)\n", be32(data.x), &data.x, &data);
			printf("		Y: %f (%08x)\n", be32(data.y));
			printf("		X Magnification: %f (%08x)\n", be32(data.xmag));
			printf("		Y Magnification: %f (%08x)\n", be32(data.ymag));
			printf("		Width: %f (%08x)\n", be32(data.width));
			printf("		Height: %f (%08x)\n", be32(data.height));
			printf("		Angle: %f (%08x)\n", be32(data.angle));
			printf("		unk1: %f (%08x)\n", be32(data.unk[0]));
			printf("		unk2: %f (%08x)\n", be32(data.unk[1]));
			printf("		unk3: %f (%08x)\n", be32(data.unk[2]));
		}else if((FourCCsMatch(entries[i].magic, lyt1_magic) == 1)) {
			dbgprintf("lyt1\n");
			PrintBRLYTEntry_lyt1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, grp1_magic) == 1)) {
			dbgprintf("grp1\n");
			PrintBRLYTEntry_grp1(entries[i], brlyt_file);
		}else if((FourCCsMatch(entries[i].magic, txl1_magic) == 1) || (FourCCsMatch(entries[i].magic, mat1_magic) == 1) ||
			 (FourCCsMatch(entries[i].magic, fnl1_magic) == 1)) {
			dbgprintf("img\n");
			PrintBRLYTEntry_img(entries[i], brlyt_file);
		}else
			printf("		Unknown tag (%c%c%c%c)!\n",entries[i].magic[0],entries[i].magic[1],entries[i].magic[2],entries[i].magic[3]);
	}
}

void parse_brlyt(char *filename)
{
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL) {
		printf("Error! Couldn't open %s!\n", filename);
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
	dbgprintf("Filesize is %d\n", file_size);
	u8* brlyt_file = (u8*)malloc(file_size);
	dbgprintf("brlyt_file allocated\n");
	fseek(fp, 0, SEEK_SET);
	fread(brlyt_file, file_size, 1, fp);
	dbgprintf("brlyt_file read to.\n");
	BRLYT_fileoffset = 0;
	brlyt_header header;
	BRLYT_ReadDataFromMemory(&header, brlyt_file, sizeof(brlyt_header));
	BRLYT_CheckHeaderSanity(header, file_size);
	brlyt_entry *entries;
	BRLYT_fileoffset = be16(header.lyt_offset);
	brlyt_entry_header tempentry;
	int i;
	dbgprintf("curr %08x max %08x\n", BRLYT_fileoffset, file_size);
	for(i = 0; BRLYT_fileoffset < file_size; i++) {
		BRLYT_ReadDataFromMemoryX(&tempentry, brlyt_file, sizeof(brlyt_entry_header));
		BRLYT_fileoffset += be32(tempentry.length);
		dbgprintf("curr %08x max %08x\n", BRLYT_fileoffset, file_size);
	}
	int entrycount = i;
	entries = (brlyt_entry*)calloc(entrycount, sizeof(brlyt_entry));
	dbgprintf("%08x\n", entries);
	if(entries == NULL) {
		printf("Couldn't allocate for entries!\n");
		exit(1);
	}
	BRLYT_fileoffset = be16(header.lyt_offset);
	for(i = 0; i < entrycount; i++) {
		dbgprintf("&(entries[i]) = %08x\n", &(entries[i]));
		BRLYT_ReadDataFromMemoryX(&tempentry, brlyt_file, sizeof(brlyt_entry_header));
		memcpy(entries[i].magic, tempentry.magic, 4);
		entries[i].length = tempentry.length;
		entries[i].data_location = BRLYT_fileoffset + sizeof(brlyt_entry_header);
		BRLYT_fileoffset += be32(tempentry.length);
	}	
//	int entrycnt = BRLYT_ReadEntries(brlyt_file, file_size, header, entries);
	dbgprintf("%08x\n", entries);
	printf("Parsed BRLYT! Information:\n");
	printf("Main header:\n");
	printf("	Magic: %c%c%c%c\n", header.magic[0], header.magic[1], header.magic[2], header.magic[3]);
	printf("	Unk1: %08x\n", be32(header.unk1));
	printf("	Filesize: %lu\n", be32(header.filesize));
	printf("		%s real file size!\n", be32(header.filesize) == file_size ? "Matches" : "Does not match");
	printf("	Offset to lyt1: %04x\n", be16(header.lyt_offset));
	printf("	Unk2: %04x\n", be16(header.unk2));
	printf("\nBRLYT Entries:");
	PrintBRLYTEntries(entries, entrycount, brlyt_file);
}

void make_brlyt(char* infile, char* outfile)
{
}










































