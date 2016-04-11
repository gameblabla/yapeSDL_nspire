/*
	YAPE - Yet Another Plus/4 Emulator

	The program emulates the Commodore 264 family of 8 bit microcomputers

	This program is free software, you are welcome to distribute it,
	and/or modify it under certain conditions. For more information,
	read 'Copying'.

	(c) 2000, 2001, 2004, 2007 Attila Gr�sz
*/
#ifndef _TEDMEM_H
#define _TEDMEM_H

#define RAMSIZE 65536
#define ROMSIZE 16384
#define SCR_HSIZE 384
#define SCR_VSIZE 288

#include "types.h"
#include "mem.h"
#include "serial.h"

class TED;
class CPU;
class KEYS;
class TAP;
class CTCBM;

class TED: 
public CSerial , public MemoryHandler 
{
  public:
  	TED();
  	~TED();
	KEYS *keys;
	TAP	*tap;
	virtual void UpdateSerialState(unsigned char portval);
	virtual void Reset();
	// read memory through memory decoder
  	virtual unsigned char Read(unsigned int addr);
  	virtual void Write(unsigned int addr, unsigned char value);
	// read memory directly
	unsigned char readDMA(unsigned int addr) { return Ram[addr]; }
	// same as above but with writing
	void wrtDMA(unsigned int addr, unsigned char value) { Ram[addr]=value; }
	void setRamMask(unsigned int value) { RAMMask=value;}
	unsigned int getRamMask(void) { return RAMMask;}
	// are the ROMs disabled?
  	bool RAMenable;
	// indicates whether 256K RAM is on
	bool bigram, bramsm;
	// /ram/rom path/load variables
	void loadroms(void);
	void loadloromfromfile(int nr, char fname[256]);
	void loadhiromfromfile(int nr, char fname[256]);
	char romlopath[4][256];
	char romhighpath[4][256];
	// this is for the FRE support
  	void dump(void *img);
	void memin(void *img);
	// screen rendering
	// raster co-ordinates and boundaries
	unsigned int beamx, beamy;
	unsigned char screen[SCR_HSIZE*(SCR_VSIZE*2)];
	bool render_ok;
	void texttoscreen(int x,int y, char *scrtxt);
	void chrtoscreen(int x,int y, char scrchr);
	// cursor stuff
	unsigned int crsrpos;
	int crsrphase;
	bool crsrblinkon;
	// is joy emulated?
	bool joyemu;
	// CPU class pointer
	CPU	*cpuptr;
	// TED process (main loop of emulation)
	void ted_process();
	void setDS( void *ds);

 	unsigned char Ram[RAMSIZE];
  	unsigned char RomHi[4][ROMSIZE];
	void ChangeMemBankSetup();

	// timer stuff
	bool t1on, t2on, t3on;
	unsigned int timer1, timer2, timer3, t1start;

	void HookTCBM(CTCBM *pTcbmbus) { tcbmbus = pTcbmbus; };
	ClockCycle GetClockCount();
	static TED *instance() { return instance_; };

  private:
	static TED *instance_;
    CTCBM *tcbmbus;
	// memory variables
  	unsigned char RomLo[4][ROMSIZE];
	unsigned char *actromlo, *actromhi;
	unsigned char *mem_8000_bfff, *mem_c000_ffff, *mem_fc00_fcff;
  	unsigned int RAMMask;
	unsigned char RamExt[4][RAMSIZE];	// Ram slots for 256 K RAM
	unsigned char *actram;
	unsigned char prp, prddr;
	unsigned char pio1;
	// indicates if screen blank is off
	bool scrblank;
	// for vertical/horizontal smooth scroll
	unsigned int hshift, vshift;
	unsigned int nrwscr, fltscr;
	// char/color buffers
	unsigned char DMAbuf[64*3];
	unsigned char *chrbuf, *clrbuf, *tmpClrbuf;
	int cposy;
	// rendering functions
	void	(TED::*scrmode)();
	inline void	hi_text();
	void	mc_text();
	void 	mc_text_rvs();
	void	ec_text();
	void	mcec();
	void	rv_text();
	void	hi_bitmap();
	void	mc_bitmap();
	void	illegalbank();
	bool	charrom;
	int		rvsmode, grmode, ecmode;
	int		scrattr, charbank;

	// border color
	unsigned int framecol;
	// various memory pointers
	unsigned char *colorbank, *charrombank, *charrambank;
	unsigned char *grbank;
	unsigned char *scrptr, *endptr, *ramptr;
	const char *DMAptr;
	unsigned int fastmode, irqline;
	unsigned char hcol[2], mcol[4], ecol[4], bmmcol[4], *cset;

	//
	void DoDMA( unsigned char *Buf, unsigned int Offset  );
};

#define theTed TED::instance()

const short HUE[16] = { 0, 0,
/*RED*/	103, /*CYAN	*/ 283,
/*MAGENTA*/	53,/*GREEN*/ 241, /*BLUE*/347,
/*YELLOW*/ 167,/*ORANGE*/123, /*BROWN*/	148,
/*YLLW-GRN*/ 195, /*PINK*/ 83, /*BLU-GRN*/ 265,
/*LT-BLU*/ 323, /*DK-BLU*/ /*23 original, but wrong...*/ 355, /*LT-GRN	*/ 213 
};

const double luma[9] = {
/*
	Luminancia Voltages
*/
	2.00, 2.4, 2.55, 2.7, 2.9, 3.3, 3.6, 4.1, 4.8 };

#endif //_TEDMEM_H

/*

int brightness =  ...;	//* 0..255
int contrast = ...;	//* 0..255

for (i=0; i < 128; i++) {
	int k = (brightness-128) + (contrast*i)/128;
	if (k < 0)
		k=0;
	else if (k > 255)
		k = 255;
	table[i] = k;
}
*/
