/*
 *  brlyt.h
 *  
 *
 *  Created by Alex Marshall on 09/01/27.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef BRLYT_H_
#define BRLYT_H_

#include "types.h"

typedef struct
{
	fourcc		magic;			// RLYT
	u32		unk1;			// 0xFEFF0008
	u32		filesize;		// The filesize of the brlyt.
	u16		lyt_offset;		// Offset to the lyt1 section.
	u16		unk2;			// Unknown.
} brlyt_header;

typedef struct
{
	fourcc		magic;			// The magic.
	u32		length;			// How long the entry is.
} brlyt_entry_header;

typedef struct
{
	fourcc		magic;			// The magic.
	u32		length;			// How long the entry is.
	u32		data_location;		// The location of the data in the BRLYT file.
} brlyt_entry;

typedef struct
{
	u16		flags;
	u16		alpha;
	char		name[0x18];
	f32		x;
	f32		y;
	f32		unk[3];
	f32		angle;
	f32		xmag;
	f32		ymag;
	f32		width;
	f32		height;
} brlyt_pic1_chunk;

typedef struct
{
	u32		unk[3];
} brlyt_lyt1_chunk;

typedef struct
{
	char		name[16];
	u16		numsubs;
	u16		unk;
} brlyt_grp1_chunkbase;

typedef struct
{
	u16 num;
	u16 offs;
} brlyt_img_chunk;
// grs1, gre1, pas1, and pae1 all don't have anything special. they seem to just be prefixes to some sections.

void parse_brlyt(char *filename);
void make_brlyt(char* infile, char* outfile);

#endif //BRLYT_H_