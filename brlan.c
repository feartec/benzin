/*
 *  brlan.c
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
#include "brlan.h"

//#define DEBUGBRLAN

#ifdef DEBUGBRLAN
#define dbgprintf	printf
#else
#define dbgprintf	//
#endif //DEBUGBRLAN

fourcc tag_FourCCs[] = { "RLPA", "RLTS", "RLVI", "RLVC", "RLMC", "RLTP" };

char RLPA_Types[15][24];

static size_t BRLAN_fileoffset = 0;

static void BRLAN_ReadDataFromMemoryX(void* destination, void* input, size_t size)
{
	u8* out = (u8*)destination;
	u8* in = ((u8*)input) + BRLAN_fileoffset;
	memcpy(out, in, size);
}

static void BRLAN_ReadDataFromMemory(void* destination, void* input, size_t size)
{
	BRLAN_ReadDataFromMemoryX(destination, input, size);
	BRLAN_fileoffset += size;
}

static void CreateGlobal_pai1(brlan_pai1_header_type2 *pai1_header, brlan_pai1_header_type1 pai1_header1,
			      brlan_pai1_header_type2 pai1_header2, int pai1_header_type)
{
	if(pai1_header_type == 1) {
		pai1_header->magic[0]		= pai1_header1.magic[0];
		pai1_header->magic[1]		= pai1_header1.magic[1];
		pai1_header->magic[2]		= pai1_header1.magic[2];
		pai1_header->magic[3]		= pai1_header1.magic[3];
		pai1_header->size		= pai1_header1.size;
		pai1_header->framesize		= pai1_header1.framesize;
		pai1_header->flags		= pai1_header1.flags;
		pai1_header->unk1		= pai1_header1.unk1;
		pai1_header->num_timgs		= pai1_header1.num_timgs;
		pai1_header->num_entries	= pai1_header1.num_entries;
		pai1_header->unk2		= 0;
		pai1_header->entry_offset	= pai1_header1.entry_offset;
	}else
		memcpy(pai1_header, &pai1_header2, sizeof(brlan_pai1_header_type2));
}

static void DisplayTagData(u32 flags, tag_data data, int z)
{
	u32 p1 = be32(data.part1);
	u32 p2 = be32(data.part2);
	u32 p3 = be32(data.part3);
	printf("					Triplet %d:\n", z);
	printf("						Frame number: %f\n", *(f32*)(&p1));
	printf("						Value: %f\n", *(f32*)(&p2));
	printf("						Interpolation Value: %f\n", *(f32*)(&p3));
}

static int FourCCsMatch(fourcc cc1, fourcc cc2)
{
	if((cc1[0] == cc2[0]) && (cc1[1] == cc2[1]) && (cc1[2] == cc2[2]) && (cc1[3] == cc2[3]))
		return 1;
	else
		return 0;
}

static int FourCCInList(fourcc cc)
{
	int i;
	for(i = 0; i < 6; i++)
		if(FourCCsMatch(cc, tag_FourCCs[i])) return 1;
	return 0;
}

static void DisplayTagInformation(int idx, tag_header* heads, tag_entry** entriesx,
			    tag_entryinfo** entryinfosx, tag_data*** datasxx)
{
	tag_header head = heads[idx];
	tag_entry* entries = entriesx[idx];
	tag_entryinfo* entryinfos = entryinfosx[idx];
	tag_data** datas = datasxx[idx];
	
	printf("		Number of entries: %u\n", head.entry_count);

// Why should we show padding? It's 0 every single time.
/*	printf("		Unk1: %02x\n", head.pad1);
	printf("		Unk2: %02x\n", head.pad2);
	printf("		Unk3: %02x\n", head.pad3); */

	int i, z;
	printf("		Entries:\n");
	for(i = 0; i < head.entry_count; i++) {
		printf("			Entry %u:\n", i);
	if(FourCCsMatch(head.magic, tag_FourCCs[0]) == 1)
		printf("				Type: %s\n", RLPA_Types[i]);

// User doesn't need to know the offset
//		printf("				Offset: %lu\n", be32(entries[i].offset));
		printf("				Flags: %08x\n", be32(entryinfos[i].flags));

// Yet again, these are always the same, and seem to maybe be some marker. don't bother to show.
/*		printf("				Unk1: %04x\n", be16(entryinfos[i].pad1));
		printf("				Unk2: %08x\n", be32(entryinfos[i].unk1)); */
		printf("				Triplet Count: %u\n", be16(entryinfos[i].coord_count));
		printf("				Triplets:\n");
		for(z = 0; z < be16(entryinfos[i].coord_count); z++)
			DisplayTagData(be32(entryinfos[i].flags), datas[i][z], z);
	}
}

static void ReadTagFromBRLAN(int idx, u8* brlan_file, tag_header *head, tag_entry** entries,
		       tag_entryinfo** entryinfo, tag_data*** data)
{
	int taghead_location = BRLAN_fileoffset;
	BRLAN_ReadDataFromMemory(&head[idx], brlan_file, sizeof(tag_header));
	dbgprintf("read header. entry count %d\n", head[idx].entry_count);
	int i, z;
	entries[idx] = realloc(entries[idx], sizeof(tag_entry) * head[idx].entry_count);
	dbgprintf("reallocated entries[idx]\n");
	entryinfo[idx] = realloc(entryinfo[idx], sizeof(tag_entryinfo) * head[idx].entry_count);
	dbgprintf("reallocated entryinfo[idx]\n");
	if(data[idx] == NULL) {
		dbgprintf("callocating data[idx]\n");
		data[idx] = (tag_data**)malloc(sizeof(tag_data*) * head[idx].entry_count);
		dbgprintf("callocated data[idx]\n");
	}else{
		dbgprintf("reallocating data[idx] %08x\n", data[idx]);
		data[idx] = (tag_data**)realloc(data[idx], sizeof(tag_data*) * head[idx].entry_count);
		dbgprintf("reallocated data[idx]\n");
	}
	for(i = 0; i < head[idx].entry_count; i++) {
		dbgprintf("reading entry %d 0x%08x\n", i, &entries[idx][i]);
		BRLAN_ReadDataFromMemory(&entries[idx][i], brlan_file, sizeof(tag_entry));
		dbgprintf("read entry.\n");
	}
	for(i = 0; i < head[idx].entry_count; i++) {
		dbgprintf("i %d cnt %d\n", i, head[idx].entry_count);
		BRLAN_fileoffset = be32(entries[idx][i].offset) + taghead_location;
		dbgprintf("read entry infos. 0x%08x\n", &entryinfo[idx][i]);
		BRLAN_ReadDataFromMemory(&entryinfo[idx][i], brlan_file, sizeof(tag_entryinfo));
		dbgprintf("read entry info 0x%08x. %u\n", &data[idx][i][0], be16(entryinfo[idx][i].coord_count));
		data[idx][i] = realloc(NULL, sizeof(tag_data) * be16(entryinfo[idx][i].coord_count));
		dbgprintf("reallocated data[i].\n");
		for(z = 0; z < be16(entryinfo[idx][i].coord_count); z++) {
			dbgprintf("reading data 0x%08x\n", &data[idx][i][z]);
			BRLAN_ReadDataFromMemory(&data[idx][i][z], brlan_file, sizeof(tag_data));
			dbgprintf("read data.\n");
		}
	}
}

void parse_brlan(char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL) {
		printf("Error! Couldn't open %s!\n", filename);
		exit(1);
	}
	strcpy(RLPA_Types[0], "X Translation");
	strcpy(RLPA_Types[1], "Y Translation");
	strcpy(RLPA_Types[2], "Z Translation");
	strcpy(RLPA_Types[3], "Unknown (Alpha?) (0x03)");
	strcpy(RLPA_Types[4], "Unknown (0x04)");
	strcpy(RLPA_Types[5], "Angle");
	strcpy(RLPA_Types[6], "X Zoom");
	strcpy(RLPA_Types[7], "Y Zoom");
	strcpy(RLPA_Types[8], "Width");
	strcpy(RLPA_Types[9], "Height");
	strcpy(RLPA_Types[10], "Unknown (0x0A)");
	strcpy(RLPA_Types[11], "Unknown (0x0B)");
	strcpy(RLPA_Types[12], "Unknown (0x0C)");
	strcpy(RLPA_Types[13], "Unknown (0x0D)");
	strcpy(RLPA_Types[14], "Unknown (0x0E)");
	strcpy(RLPA_Types[15], "Unknown (0x0F)");
	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	dbgprintf("Filesize is %d\n", file_size);
	u8* brlan_file = (u8*)malloc(file_size);
	dbgprintf("brlan_file allocated\n");
	fseek(fp, 0, SEEK_SET);
	fread(brlan_file, file_size, 1, fp);
	dbgprintf("brlan_file read to.\n");
	BRLAN_fileoffset = 0;
	brlan_header header;
	BRLAN_ReadDataFromMemoryX(&header, brlan_file, sizeof(brlan_header));
	dbgprintf("brlan_header read to.\n");
	BRLAN_fileoffset = be16(header.pai1_offset);
	brlan_pai1_universal universal;
	BRLAN_ReadDataFromMemoryX(&universal, brlan_file, sizeof(brlan_pai1_universal));
	dbgprintf("pa1_universal read to.\n");
	
	int pai1_header_type;
	brlan_pai1_header_type1 pai1_header1;
	brlan_pai1_header_type2 pai1_header2;
	brlan_pai1_header_type2 pai1_header;
	
	if((be32(universal.flags) & (1 << 25)) >= 1) {
		pai1_header_type = 2;
		BRLAN_ReadDataFromMemory(&pai1_header2, brlan_file, sizeof(brlan_pai1_header_type2));
	} else {
		pai1_header_type = 1;
		BRLAN_ReadDataFromMemory(&pai1_header1, brlan_file, sizeof(brlan_pai1_header_type1));
	}
	dbgprintf("pai1 headers gotten.\n");
	
	CreateGlobal_pai1(&pai1_header, pai1_header1, pai1_header2, pai1_header_type);
	dbgprintf("pai1 global created.\n");
	
	int tagcount = be16(pai1_header.num_entries);
	dbgprintf("tagcount done.\n");
	u32 *taglocations = (u32*)calloc(tagcount, sizeof(u32));
	dbgprintf("allocated tag locations.\n");
	fourcc CCs[256];
	memset(CCs, 0, 256*4);
	dbgprintf("allocated fourccs.\n");
	BRLAN_ReadDataFromMemory(taglocations, brlan_file, tagcount * sizeof(u32));
	dbgprintf("read tag locations.\n");
	brlan_entry *tag_entries = (brlan_entry*)calloc(tagcount, sizeof(brlan_entry));
	dbgprintf("tag entries allocated.\n");
	tag_header *intag_heads = NULL;
	dbgprintf("tag heads created.\n");
	tag_entry **intag_entries = NULL;
	dbgprintf("tag entries created.\n");
	tag_entryinfo **intag_entryinfos = NULL;
	dbgprintf("rlpa entry infos created.\n");
	tag_data ***intag_datas = NULL;
	dbgprintf("rlpa datas created.\n");
	int intag_cnt = 1;
	int i;
	for(i = 0; i < tagcount; i++) {
		BRLAN_fileoffset = be32(taglocations[i]) + be16(header.pai1_offset);
		dbgprintf("fileoffset set.\n");
		BRLAN_ReadDataFromMemory(&(tag_entries[i]), brlan_file, sizeof(brlan_entry));
		dbgprintf("got tag entry.\n");
		if((be32(tag_entries[i].flags) & (1 << 25)) >= 1) {
			BRLAN_fileoffset += sizeof(u32);
			dbgprintf("skipped extra.\n");
		}
		fourcc magick;
		BRLAN_ReadDataFromMemoryX(magick, brlan_file, 4);
		memcpy(CCs[i], magick, 4);
		dbgprintf("read fourcc from %04x %02x%02x%02x%02x.\n", BRLAN_fileoffset, magick[0], magick[1], magick[2], magick[3]);
		if(FourCCInList(CCs[i]) == 1) {
			dbgprintf("we found a tag.\n");
			intag_heads = realloc(intag_heads, sizeof(tag_header) * intag_cnt);
			dbgprintf("reallocated tag heads.\n");
			intag_entries = realloc(intag_entries, sizeof(tag_entry) * intag_cnt * 20);
			dbgprintf("reallocated tag entries.\n");
			intag_entryinfos = realloc(intag_entryinfos, sizeof(tag_entryinfo) * intag_cnt * 20);
			dbgprintf("reallocated tag entryinfos.\n");
			intag_datas = realloc(intag_datas, sizeof(tag_data) * intag_cnt * 20 * 6);
			dbgprintf("reallocated tag datas.\n");
			if(intag_heads == NULL) {
				printf("Error allocating heads.\n");
				exit(2);
			}
			if(intag_entries == NULL) {
				printf("Error allocating entries.\n");
				exit(2);
			}
			if(intag_entryinfos == NULL) {
				printf("Error allocating entryinfos.\n");
				exit(2);
			}
			if(intag_datas == NULL) {
				printf("Error allocating datas.\n");
				exit(2);
			}
			ReadTagFromBRLAN(intag_cnt - 1, brlan_file, intag_heads, intag_entries,
					  intag_entryinfos, intag_datas);
			intag_cnt++;
		}
		dbgprintf("looping.\n");
	}
	
	printf("Parsed BRLAN! Information:\n");
	printf("Main header:\n");
	printf("	Magic: %c%c%c%c\n", header.magic[0], header.magic[1], header.magic[2], header.magic[3]);
	printf("	Filesize: %lu\n", be32(header.file_size));
	printf("		%s real file size!\n", be32(header.file_size) == file_size ? "Matches" : "Does not match");

// Not important to user, why bother.
/*	printf("	pai1 Offset: %04x\n", be16(header.pai1_offset));
	printf("	pai1 Count: %04x\n", be16(header.pai1_count)); */
	printf("\npai1 header:\n");
	printf("	Type: %d\n", pai1_header_type);
	printf("	Magic: %c%c%c%c\n", pai1_header.magic[0], pai1_header.magic[1], pai1_header.magic[2], pai1_header.magic[3]);
	printf("	Size: %lu\n", be32(pai1_header.size));
	printf("	Framesize: %u\n", be16(pai1_header.framesize));
	printf("	Flags: %02x\n", pai1_header.flags);
	printf("	unk1: %02x\n", pai1_header.unk1);
	printf("	Number of Textures: %u\n", be16(pai1_header.num_timgs));
	printf("	Number of Entries: %u\n", be16(pai1_header.num_entries));
	printf("	unk2: %08lx\n", be32(pai1_header.unk2));
		
// Not important to user, why bother.
//	printf("	Offset to Entries: 0x%08lx\n", be32(pai1_header.entry_offset));

	printf("\nBRLAN entries:");
	intag_cnt = 0;
	for(i = 0; i < tagcount; i++) {
		printf("\n	Entry %u:\n", i);
		printf("		Name: %s\n", tag_entries[i].name);
		printf("		Flags: %08x\n", be32(tag_entries[i].flags));
// Not important to user, why bother.
//		printf("		Animation Header Length: %lu\n", be32(tag_entries[i].anim_header_len));		
		printf("		FourCC: %c%c%c%c\n", CCs[i][0], CCs[i][1], CCs[i][2], CCs[i][3]);
		if(FourCCInList(CCs[i]) == 1) {
			dbgprintf("found tag. cnt %d.\n", intag_cnt);
			DisplayTagInformation(intag_cnt, intag_heads, intag_entries, intag_entryinfos, intag_datas);
			intag_cnt++;
		}else{
			printf("		Sorry, this type is currently unknown.\n");
		}
	}
	free(brlan_file);
	fclose(fp);
}
