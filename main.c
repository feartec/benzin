#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "brlan.h"
#include "brlyt.h"
#include "types.h"
#include "endian.h"
#include "general.h"

int main(int argc, char* argv[])
{
	int brlanarg;
	int brlytarg;
	int currentarg = 1;
	char helpstr[256];
	sprintf(helpstr, "%s", argv[0]);
#ifdef USE_BRLAN
	brlanarg = currentarg++;
	strcat(helpstr, " <*.brlan>");
#endif //USE_BRLAN
#ifdef USE_BRLYT
	brlytarg = currentarg++;
	strcat(helpstr, " <*.brlyt>");
#endif //USE_BRLYT
	printf("Benzin %d.%d.%d%s.\n" \
	       "Written by SquidMan (Alex Marshall), Archangel (Takeda Toshinaka), and comex.\n" \
	       "(c) 2009 HACKERCHANNEL\n", BENZIN_VERSION_MAJOR, BENZIN_VERSION_MINOR, BENZIN_VERSION_BUILD, BENZIN_VERSION_OTHER);
	if(argc < currentarg) {
		printf("Invalid arguments. Use like so:\n\t%s\n", helpstr);
		exit(1);
	}
	
#ifdef USE_BRLAN
	parse_brlan(argv[brlanarg]);
#endif //USE_BRLAN
#ifdef USE_BRLYT
	parse_brlyt(argv[brlytarg]);
#endif //USE_BRLYT
	return 0;
}


















