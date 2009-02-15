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
#include <ctype.h>
#include <mxml.h>

#include "memfile.h"
#include "types.h"
#include "brlan.h"
#include "xml.h"

#ifdef DEBUGBRLAN
#define dbgprintf	printf
#else
#define dbgprintf	//
#endif //DEBUGBRLAN

#define MAXIMUM_TAGS_SIZE		(0x1000)

fourcc tag_FourCCs[] = { "RLPA", "RLTS", "RLVI", "RLVC", "RLMC", "RLTP" };

char tag_types_list[15][24];

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

static void DisplayTagData(tag_data data, int z)
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
//		if(FourCCsMatch(head.magic, tag_FourCCs[0]) == 1) {
			if(be16(entryinfos[i].type) < 16)
				printf("				Type: %s (%04x)\n", tag_types_list[be16(entryinfos[i].type)], be16(entryinfos[i].type));
			else
				printf("				Type: Unknown (%04x)\n", be16(entryinfos[i].type));
//		}
// User doesn't need to know the offset
//		printf("				Offset: %lu\n", be32(entries[i].offset));

// Yet again, these are always the same, and seem to maybe be some marker. don't bother to show.
/*		printf("				Unk1: %04x\n", be16(entryinfos[i].pad1));
		printf("				Unk2: %04x\n", be16(entryinfos[i].unk1));
		printf("				Unk3: %08x\n", be32(entryinfos[i].unk2)); */
		printf("				Triplet Count: %u\n", be16(entryinfos[i].coord_count));
		printf("				Triplets:\n");
		for(z = 0; z < be16(entryinfos[i].coord_count); z++)
			DisplayTagData(datas[i][z], z);
	}
}

static void ReadTagFromBRLAN(int idx, u8* brlan_file, tag_header *head, tag_entry** entries,
		       tag_entryinfo** entryinfo, tag_data*** data)
{
	int taghead_location = BRLAN_fileoffset;
	BRLAN_ReadDataFromMemory(&head[idx], brlan_file, sizeof(tag_header));
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
	int i;
	for(i = 0; i < 16; i++)
		memset(tag_types_list[i], 0, 24);
	strcpy(tag_types_list[0], "X Translation");
	strcpy(tag_types_list[1], "Y Translation");
	strcpy(tag_types_list[2], "Z Translation");
	strcpy(tag_types_list[3], "0x03");
	strcpy(tag_types_list[4], "0x04");
	strcpy(tag_types_list[5], "Angle");
	strcpy(tag_types_list[6], "X Zoom");
	strcpy(tag_types_list[7], "Y Zoom");
	strcpy(tag_types_list[8], "Width");
	strcpy(tag_types_list[9], "Height");
	strcpy(tag_types_list[10], "0x0A");
	strcpy(tag_types_list[11], "0x0B");
	strcpy(tag_types_list[12], "0x0C");
	strcpy(tag_types_list[13], "0x0D");
	strcpy(tag_types_list[14], "0x0E");
	strcpy(tag_types_list[15], "0x0F");
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
		printf("		Type: %s\n", be32(tag_entries[i].flags) == 0x01000000 ? "Normal" : "Strange");
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

void WriteBRLANTagHeader(tag_header* head, FILE* fp)
{
	fwrite(head, sizeof(tag_header), 1, fp);
}

void WriteBRLANTagEntries(tag_entry* entry, u8 count, FILE* fp)
{
	tag_entry writeentry;
	int i;
	for(i = 0; i < count; i++) {
		writeentry.offset = be32(entry[i].offset);
		fwrite(&writeentry, sizeof(tag_entry), 1, fp);
	}
}

void WriteBRLANTagEntryinfos(tag_entryinfo entryinfo, FILE* fp)
{
	tag_entryinfo writeentryinfo;
	writeentryinfo.type = be16(entryinfo.type);
	writeentryinfo.unk1 = be16(entryinfo.unk1);
	writeentryinfo.coord_count = be16(entryinfo.coord_count);
	writeentryinfo.pad1 = be16(entryinfo.pad1);
	writeentryinfo.unk2 = be32(entryinfo.unk2);
	fwrite(&writeentryinfo, sizeof(tag_entryinfo), 1, fp);
}

void WriteBRLANTagData(tag_data* data, u16 count, FILE* fp)
{
	tag_data writedata;
	int i;
	for(i = 0; i < count; i++) {
		printf("%08x -> %08x\n", data[i].part1, be32(data[i].part1));
		writedata.part1 = be32(data[i].part1);
		printf("%08x -> %08x\n", data[i].part2, be32(data[i].part2));
		writedata.part2 = be32(data[i].part2);
		printf("%08x -> %08x\n\n", data[i].part3, be32(data[i].part3));
		writedata.part3 = be32(data[i].part3);
		fwrite(&writedata, sizeof(tag_data), 1, fp);
	}
}

void WriteBRLANEntry(brlan_entry *entr, FILE* fp)
{
	brlan_entry writeentr;
	memset(writeentr.name, 0, 20);
	strncpy(writeentr.name, entr->name, 20);
	writeentr.flags = be32(entr->flags);
	writeentr.anim_header_len = be32(entr->anim_header_len);
	fwrite(&writeentr, sizeof(brlan_entry), 1, fp);
}

u32 create_entries_from_xml(mxml_node_t *tree, mxml_node_t *node, brlan_entry *entr, tag_header* head, u8** tagblob, u32* blobsize)
{
	tag_entry* entry = NULL;
	tag_entryinfo* entryinfo = NULL;
	tag_data** data = NULL;
	mxml_node_t *tempnode = NULL;
	mxml_node_t *subnode = NULL;
	mxml_node_t *subsubnode = NULL;
	char temp[256];
	char temp2[256];
	char temp3[15][24];
	int i, x;

	for(i = 0; i < 16; i++)
		memset(temp3[i], 0, 24);
	for(x = 0; x < 16; x++)
		for(i = 0; i < strlen(tag_types_list[x]); i++)
			temp3[x][i] = toupper(tag_types_list[x][i]);
	head->entry_count = 0;
	subnode = node;
	for (x = 0, subnode = mxmlFindElement(subnode, node, "entry", NULL, NULL, MXML_DESCEND); subnode != NULL; subnode = mxmlFindElement(subnode, node, "entry", NULL, NULL, MXML_DESCEND), x++) {
		head->entry_count++;
		entry = realloc(entry, sizeof(tag_entry) * head->entry_count);
		entryinfo = realloc(entryinfo, sizeof(tag_entryinfo) * head->entry_count);
		if(data == NULL)
			data = (tag_data**)malloc(sizeof(tag_data*) * head->entry_count);
		else
			data = (tag_data**)realloc(data, sizeof(tag_data*) * head->entry_count);
		data[x] = NULL;
		tempnode = mxmlFindElement(subnode, tree, "type", NULL, NULL, MXML_DESCEND);
		if(tempnode == NULL) {
			printf("Couldn't find attribute \"type\"!\n");
			exit(1);
		}
		memset(temp, 0, 256);
		memset(temp2, 0, 256);
		get_value(tempnode, temp, 24);
		for(i = 0; i < strlen(temp); i++)
			temp2[i] = toupper(temp[i]);
		for(i = 0; (i < 16) && (strcmp(temp3[i - 1], temp2) != 0); i++);
		if(i == 16)
			i = atoi(temp2);
		else
			i--;
		entry[x].offset = 0;
		entryinfo[x].type = i;
		entryinfo[x].unk1 = 0x0200;
		entryinfo[x].pad1 = 0x0000;
		entryinfo[x].unk2 = 0x0000000C;
		entryinfo[x].coord_count = 0;
		subsubnode = subnode;
		for (i = 0, subsubnode = mxmlFindElement(subsubnode, subnode, "triplet", NULL, NULL, MXML_DESCEND); subsubnode != NULL; subsubnode = mxmlFindElement(subsubnode, subnode, "triplet", NULL, NULL, MXML_DESCEND), i++) {
			entryinfo[x].coord_count++;
			data[x] = realloc(data[x], sizeof(tag_data) * entryinfo[x].coord_count);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "frame", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"frame\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data[x][i].part1)) = atof(temp);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "value", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"value\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data[x][i].part2)) = atof(temp);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "interpolation", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"interpolation\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data[x][i].part3)) = atof(temp);
		}
	}
	FILE* fp = fopen("temp.blan", "wb+");
	if(fp == NULL) {
		printf("Couldn't open temporary temp.blan file\n");
		exit(1);
	}
	fseek(fp, 0, SEEK_SET);
	entr->anim_header_len = 0;
	WriteBRLANEntry(entr, fp);
	WriteBRLANTagHeader(head, fp);
	u32 entryloc = ftell(fp);
	WriteBRLANTagEntries(entry, head->entry_count, fp);
	u32* entryinfolocs = (u32*)calloc(head->entry_count, sizeof(u32));
	for(x = 0; x < head->entry_count; x++) {
		entryinfolocs[x] = ftell(fp);
		entry[x].offset = entryinfolocs[x] - sizeof(brlan_entry);
		WriteBRLANTagEntryinfos(entryinfo[x], fp);
		WriteBRLANTagData(data[x], entryinfo[x].coord_count, fp);
	}
	u32 oldpos = ftell(fp);
	fseek(fp, entryloc, SEEK_SET);
	WriteBRLANTagEntries(entry, head->entry_count, fp);
	fseek(fp, oldpos, SEEK_SET);
	u32 filesz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	entr->anim_header_len = sizeof(tag_header) + (sizeof(tag_entry) * head->entry_count);
	WriteBRLANEntry(entr, fp);
	*blobsize = filesz;
	*tagblob = (u8*)malloc(*blobsize);
	fseek(fp, 0, SEEK_SET);
	fread(*tagblob, *blobsize, 1, fp);
	free(entry);
	free(entryinfo);
	free(data);
	printf("All done.\n");
	return filesz;
}

void create_tag_from_xml(mxml_node_t *tree, mxml_node_t *node, u8** tagblob, u32* blobsize)
{
	tag_header head;
	brlan_entry entr;
	mxml_node_t *tempnode = mxmlFindElement(node, tree, "name", NULL, NULL, MXML_DESCEND);
	if(tempnode == NULL) {
		printf("Couldn't find attribute \"name\"!\n");
		exit(1);
	}
	memset(entr.name, 0, 20);
	char temp[256];
	get_value(tempnode, temp, 20);
	strncpy(entr.name, temp, 20);
	tempnode = mxmlFindElement(node, tree, "magic", NULL, NULL, MXML_DESCEND);
	if(tempnode == NULL) {
		printf("Couldn't find attribute \"magic\"!\n");
		exit(1);
	}
	get_value(tempnode, temp, 5);
	head.magic[0] = temp[0];
	head.magic[1] = temp[1];
	head.magic[2] = temp[2];
	head.magic[3] = temp[3];
	head.pad1 = 0;
	head.pad2 = 0;
	head.pad3 = 0;
	tempnode = mxmlFindElement(node, tree, "type", NULL, NULL, MXML_DESCEND);
	if(tempnode == NULL) {
		printf("Couldn't find attribute \"type\"!\n");
		exit(1);
	}
	int x;
	get_value(tempnode, temp, 256);
	for(x = 0; x < strlen(temp); x++)
		temp[x] = toupper(temp[x]);
	printf("%s\n", temp);
	if(strcmp(temp, "NORMAL") == 0)
		entr.flags = 0x01000000;
	else if(strcmp(temp, "STRANGE") == 0)
		entr.flags = 0x02000000;
	create_entries_from_xml(tree, node, &entr, &head, tagblob, blobsize);
}

void WriteBRLANHeader(brlan_header rlanhead, FILE* fp)
{
	brlan_header writehead;
	writehead.magic[0] = rlanhead.magic[0];
	writehead.magic[1] = rlanhead.magic[1];
	writehead.magic[2] = rlanhead.magic[2];
	writehead.magic[3] = rlanhead.magic[3];
	writehead.unk1 = be32(rlanhead.unk1);
	writehead.file_size = be32(rlanhead.file_size);
	writehead.pai1_offset = be16(rlanhead.pai1_offset);
	writehead.pai1_count = be16(rlanhead.pai1_count);
	fwrite(&writehead, sizeof(brlan_header), 1, fp);
}

void WriteBRLANPaiHeader(brlan_pai1_header_type1 paihead, FILE* fp)
{
	brlan_pai1_header_type1 writehead;
	writehead.magic[0] = paihead.magic[0];
	writehead.magic[1] = paihead.magic[1];
	writehead.magic[2] = paihead.magic[2];
	writehead.magic[3] = paihead.magic[3];
	writehead.size = be32(paihead.size);
	writehead.framesize = be16(paihead.framesize);
	writehead.flags = paihead.flags;
	writehead.unk1 = paihead.unk1;
	writehead.num_timgs = be16(paihead.num_timgs);
	writehead.num_entries = be16(paihead.num_entries);
	writehead.entry_offset = be32(paihead.entry_offset);
	fwrite(&writehead, sizeof(brlan_pai1_header_type1), 1, fp);
}

void write_brlan(char *infile, char* outfile)
{
	int i;
	for(i = 0; i < 16; i++)
		memset(tag_types_list[i], 0, 24);
	strcpy(tag_types_list[0], "X Translation");
	strcpy(tag_types_list[1], "Y Translation");
	strcpy(tag_types_list[2], "Z Translation");
	strcpy(tag_types_list[3], "0x03");
	strcpy(tag_types_list[4], "0x04");
	strcpy(tag_types_list[5], "Angle");
	strcpy(tag_types_list[6], "X Zoom");
	strcpy(tag_types_list[7], "Y Zoom");
	strcpy(tag_types_list[8], "Width");
	strcpy(tag_types_list[9], "Height");
	strcpy(tag_types_list[10], "0x0A");
	strcpy(tag_types_list[11], "0x0B");
	strcpy(tag_types_list[12], "0x0C");
	strcpy(tag_types_list[13], "0x0D");
	strcpy(tag_types_list[14], "0x0E");
	strcpy(tag_types_list[15], "0x0F");
	FILE* fpx = fopen(infile, "r");
	if(fpx == NULL) {
		printf("xmlan couldn't be opened!\n");
		exit(1);
	}
	mxml_node_t *hightree = mxmlLoadFile(NULL, fpx, MXML_TEXT_CALLBACK);
	if(hightree == NULL) {
		printf("Couldn't open hightree!\n");
		exit(1);
	}
	mxml_node_t *tree = mxmlFindElement(hightree, hightree, "xmlan", NULL, NULL, MXML_DESCEND);
	if(hightree == NULL) {
		printf("Couldn't get tree!\n");
		exit(1);
	}
	mxml_node_t *node;
	FILE* fp = fopen(outfile, "wb+");
	if(fpx == NULL) {
		printf("destination brlan couldn't be opened!\n");
		exit(1);
	}
	u8* tagblob;
	u32 blobsize;
	u16 blobcount = 0;
	u32 bloboffset;
	brlan_header rlanhead;
	rlanhead.magic[0] = 'R';
	rlanhead.magic[1] = 'L';
	rlanhead.magic[2] = 'A';
	rlanhead.magic[3] = 'N';
	rlanhead.unk1 = 0xFEFF0008;
	rlanhead.file_size = 0;
	rlanhead.pai1_offset = sizeof(brlan_header);
	rlanhead.pai1_count = 1;
	WriteBRLANHeader(rlanhead, fp);
	brlan_pai1_header_type1 paihead;
	paihead.magic[0] = 'p';
	paihead.magic[1] = 'a';
	paihead.magic[2] = 'i';
	paihead.magic[3] = '1';
	paihead.size = 0;
	char temp[256];
	mxml_node_t *tempnode = mxmlFindElement(tree, tree, "framesize", NULL, NULL, MXML_DESCEND);
	if(tempnode == NULL) {
		printf("Couldn't find attribute \"framesize\"!\n");
		exit(1);
	}
	get_value(tempnode, temp, 256);
	paihead.framesize = atoi(temp);
	paihead.flags = 1;
	paihead.unk1 = 0;
	paihead.num_timgs = 0;
	paihead.num_entries = 0;
	paihead.entry_offset = sizeof(brlan_pai1_header_type1);
	WriteBRLANPaiHeader(paihead, fp);
	// Do header stuff here...
	u8* tagchunksbig = (u8*)calloc(MAXIMUM_TAGS_SIZE, 1);
	MEMORY* tagsmem = mopen(tagchunksbig, MAXIMUM_TAGS_SIZE, 3);
	u32 totaltagsize = 0;
	for(node = mxmlFindElement(tree, tree, "tag", NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlFindElement(node, tree, "tag", NULL, NULL, MXML_DESCEND)) {
		blobcount++;
		bloboffset = ftell(fp) + mtell(tagsmem) - (4 * (blobcount + 1));
		bloboffset = be32(bloboffset);
		fwrite(&bloboffset, sizeof(u32), 1, fp);
		create_tag_from_xml(tree, node, &tagblob, &blobsize);
		mwrite(tagblob, blobsize, 1, tagsmem);
		totaltagsize += blobsize;
	}
	tagchunksbig = (u8*)mclose(tagsmem);
	fwrite(tagchunksbig, totaltagsize, 1, fp);
	paihead.num_entries = blobcount;
	fseek(fp, 0, SEEK_END);
	paihead.size = ftell(fp) - rlanhead.pai1_offset;
	rlanhead.file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	WriteBRLANHeader(rlanhead, fp);
	WriteBRLANPaiHeader(paihead, fp);
}

void make_brlan(char* infile, char* outfile)
{
	printf("Starting xmlan file @ %s parsing.\n", infile);
	write_brlan(infile, outfile);
}

