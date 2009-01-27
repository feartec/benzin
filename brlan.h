#ifndef BRLAN_H_
#define BRLAN_H_

#include "types.h"

typedef char fourcc[4];

typedef enum
{
	RLPA_ENTRY	= 0
} entry_type;

typedef struct
{
	char		magic[4];		// "pai1" in ASCII.
	u32		size;			// Size of section, which is rest of the file. (header.file_size - header.offset_pai1)
	u32		unk1;			// Unknown?
	u16		flags;			// Flags?
	u16		num_entries;		// Number of tags in the brlan.
	u32		unk2;			// Only if bit 25 of flags is set.
	u32		entry_offset;		// Offset to entries. (Relative to start of pai1 header.)
} pai1_header_type2;

typedef struct
{
	char		magic[4];		// "RLAN" in ASCII.
	u32		unk1;			// Always 0xFEFF 0x0008. Possibly a versioning string.
	u32		file_size;		// Size of whole file, including the header.
	u16		pai1_offset;		// The offset to the pai1 header from the start of file.
	u16		pai1_count;		// How many pai1 sections there are (duh, only 1... wtf?)
} brlan_header;

typedef struct
{
	char		magic[4];		// "pai1" in ASCII.
	u32		size;			// Size of section, which is rest of the file. (header.file_size - header.offset_pai1)
	u32		unk1;			// Unknown
	u16		flags;			// Flags
	u16		num_entries;		// Number of tags in the brlan.
} pai1_universal;

typedef struct
{
	char		magic[4];		// "pai1" in ASCII.
	u32		size;			// Size of section, which is rest of the file. (header.file_size - header.offset_pai1)
	u32		unk1;			// Unknown?
	u16		flags;			// Flags?
	u16		num_entries;		// Number of tags in the brlan.
	u32		entry_offset;		// Offset to entries. (Relative to start of pai1 header.)
} pai1_header_type1;

typedef struct
{
	char		name[20];		// Name of the BRLAN entry. (Must be defined in the BRLYT)
	u32		flags;			// Flags? (If bit 25 is set, we have another u32 after the entry. It's use is unknown.)
	u32		anim_header_len;	// Length of the animation header which is directly after this entry.
} brlan_entry;

typedef struct
{
	char		magic[4];		// "RLPA" in ASCII
	u8		entry_count;		// How many entries in this chunk.
	u8		pad1;			// All cases I've seen is zero.
	u8		pad2;			// All cases I've seen is zero.
	u8		pad3;			// All cases I've seen is zero.
} RLPA_header;

typedef struct
{
	u32		offset;			// Offset to the data pointed to by this entry.
						// Relative to the start of the RLPA header.
} RLPA_entry;

typedef struct
{
	u32		flags;			// Flags?
	u16		coord_count;		// How many coordinates.
	u16		pad1;			// All cases I've seen is zero.
	u32		unk1;			// ??? In every case I've seen, it is \x00\x00\x00\x0C.
} RLPA_entryinfo;

typedef struct
{						// Bits not listed here are currently unknown.
	u32		part1;			// If Bit 9 is set in flags, this is an f32, with a coordinate. (Bit 17 seems to act the same)
	u32		part2;			// If Bit 16 is set in flags, this is an f32, with another coordinate. (Bit 17 seems to act the same)
	u32		part3;			// With Bit 16 set in flags, this seems to be yet another coordinate. (Bit 17 seems to act the same)
} RLPA_data;















#endif //BRLAN_H_

