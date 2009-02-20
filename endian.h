#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include "types.h"

//#define be16(x)		((x>>8)|(x<<8))
//#define be32(x)		((x>>24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x<<24))
//#define be64(x)		((x>>56)|((x<<40)&(u64)0x00FF000000000000)|((x<<24)&(u64)0x0000FF0000000000)|((x<<8)&(u64)0x000000FF00000000)|((x>>8)&(u64)0x00000000FF000000)|((x>>24)&(u64)0x0000000000FF0000)|((x<<40)&(u64)0x000000000000FF00)|(x<<56))

u16 be16(u16 x);

u32 be32(u32 x);

// __int64 for MSVC, "long long" for gcc
u64 be64(u64 x);

#endif //_ENDIAN_H_

