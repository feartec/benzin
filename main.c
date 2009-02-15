#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "brlan.h"
#include "brlyt.h"
#include "types.h"
#include "endian.h"
#include "general.h"

#ifdef DEBUGMAIN
#define dbgprintf	printf
#else
#define dbgprintf	//
#endif //DEBUGMAIN

int main(int argc, char* argv[])
{
	int brlanargread;
	int brlytargread;
	int brlanargmake;
	int brlytargmake;
	int brlanargdestmake;
	int brlytargdestmake;
	int reqargs;
	int currentarg = 2;
	char helpstrmake[256];
	char helpstrread[256];
	sprintf(helpstrmake, "%s m", argv[0]);
	sprintf(helpstrread, "%s r", argv[0]);
#ifdef USE_BRLAN
	brlanargread = currentarg;
	strcat(helpstrread, " <*.brlan>");
	brlanargmake = currentarg++;
	strcat(helpstrmake, " <*.xmlan>");
#endif //USE_BRLAN
#ifdef USE_BRLYT
	brlytargread = currentarg;
	strcat(helpstrread, " <*.brlyt>");
	brlytargmake = currentarg++;
	strcat(helpstrmake, " <*.xmlyt>");
#endif //USE_BRLYT
	reqargs = currentarg;
#ifdef USE_BRLAN
	brlanargdestmake = currentarg++;
	strcat(helpstrmake, " <out.brlan>");
#endif //USE_BRLAN
#ifdef USE_BRLYT
	brlytargdestmake = currentarg++;
	strcat(helpstrmake, " <out.brlyt>");
#endif //USE_BRLYT

	printf("Benzin %d.%d.%d%s.\n" \
	       "Written by SquidMan (Alex Marshall), Archangel (Takeda Toshinaka), and comex.\n" \
	       "(c) 2009 HACKERCHANNEL\n", BENZIN_VERSION_MAJOR, BENZIN_VERSION_MINOR, BENZIN_VERSION_BUILD, BENZIN_VERSION_OTHER);
	if(argc < reqargs) {
		printf("Invalid arguments. Use like:\n\t%s\n\t\tor:\n\t%s\n", helpstrread, helpstrmake);
		exit(1);
	}
	if(argv[1][0] == 'r') {
#ifdef USE_BRLAN
		parse_brlan(argv[brlanargread]);
#endif //USE_BRLAN
#ifdef USE_BRLYT
		parse_brlyt(argv[brlytargread]);
#endif //USE_BRLYT
	}else if(argv[1][0] == 'm') {
#ifdef USE_BRLAN
		dbgprintf("1:%d 2:%d f1:%s f2:%s\n", brlanargmake, brlanargdestmake, argv[brlanargmake], argv[brlanargdestmake]);
		make_brlan(argv[brlanargmake], argv[brlanargdestmake]);
#endif //USE_BRLAN
#ifdef USE_BRLYT
		dbgprintf("1:%d 2:%d f1:%s f2:%s\n", brlytargmake, brlytargdestmake, argv[brlytargmake], argv[brlytargdestmake]);
		make_brlyt(argv[brlytargmake], argv[brlytargdestmake]);
#endif //USE_BRLYT
	}
	return 0;
}


















