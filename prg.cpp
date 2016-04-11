#include <stdio.h>
#include <string.h>
#include "archdep.h"
#include "tedmem.h"
#include "types.h"

bool PrgLoad(char *fname, int loadaddress, MemoryHandler *the_ted)
{
	unsigned char   *lpBufPtr;
	unsigned short	loadaddr, endaddr;
	FILE			*prg;
	unsigned int	fsize;
	char			*fext;
	int				p00offset = 0;
	char			fullname[MAX_PATH+80];
	char			*purename;

	if ((prg = fopen(fname, "rb"))== NULL) {
    	return FALSE;
	} else {
		fext = strrchr( fname, '.');
		if ( !strcmp( fext+1, "p00" ) || !strcmp( fext+1, "P00" ))
			p00offset = 26;
		// load PRG file
		fseek(prg, 0L, SEEK_END);
		fsize=ftell(prg) - p00offset;
		fseek(prg, p00offset, SEEK_SET);
		lpBufPtr = new unsigned char[fsize];
		fread(lpBufPtr,fsize,1,prg);
		fclose(prg);
		// copy to memory
		if (loadaddress&0x10000)
			loadaddr=loadaddress&0xFFFF;
		else
			loadaddr=lpBufPtr[0]|(lpBufPtr[1]<<8);

		if (fsize<2)
			return FALSE;

		for (unsigned int i=0;i<fsize-2;i++)
			the_ted->Write(loadaddr+i,lpBufPtr[2+i]);

		endaddr = loadaddr+fsize-2;

		the_ted->Write(0x2D,(endaddr)&0xFF);
		the_ted->Write(0x2E,(endaddr)>>8);
		the_ted->Write(0x2F,(endaddr)&0xFF);
		the_ted->Write(0x30,(endaddr)>>8);
		the_ted->Write(0x31,(endaddr)&0xFF);
		the_ted->Write(0x32,(endaddr)>>8);
		the_ted->Write(0x9D,(endaddr)&0xFF);
		the_ted->Write(0x9E,(endaddr)>>8);
		delete [] lpBufPtr;
	};

	fprintf( stderr, "Loaded: %s at $%04X-$%04X\n", fname, loadaddr, endaddr);

	return TRUE;
}
