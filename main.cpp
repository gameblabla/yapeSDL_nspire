/*
	YAPE - Yet Another Plus/4 Emulator

	The program emulates the Commodore 264 family of 8 bit microcomputers

	This program is free software, you are welcome to distribute it,
	and/or modify it under certain conditions. For more information,
	read 'Copying'.

	(c) 2000, 2001, 2004, 2005, 2007 Attila Grósz
	(c) 2005 VENESZ Roland
*/

#define TITLE   "YAPESDL 0.32"
#define NAME    "Yape for SDL 0.32.5"

#ifdef _TINSPIRE
#include <os.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <SDL/SDL.h>
#include "keyboard.h"
#include "cpu.h"
#include "tedmem.h"
#include "tape.h"
#include "archdep.h"
#include "iec.h"
#include "device.h"
#include "tcbm.h"
#include "diskfs.h"
#include "prg.h"
#include "interface.h"

// function prototypes
extern void FrameUpdate(void);

#include "interface.h"

// SDL stuff
static SDL_Event		event;
static SDL_Surface     *screen, *surface;
static SDL_Palette		palette;
static SDL_Color		*g_pal;
static SDL_Rect			dest;

// class pointer for the user interface
static UI				*uinterface = NULL;

// color
static Uint8	p4col_calc[768];

///////////////////
// Screen variables
static Uint8	*pixel;

////////////////
// Supplementary
static Uint32			g_TotFrames = 0;
static Uint32			timeelapsed;

static TED				*ted8360 = new TED;
static CPU				*machine = new CPU(ted8360, &(ted8360->Ram[0xFF09]),
							&(ted8360->Ram[0x0100]));
static CTCBM			*tcbm = NULL;
static CIECInterface	*iec = NULL;
static CIECDrive		*fsdrive = NULL;
static Uint32			i;
static char				textout[40];
static char				inipath[512] = "";
static char				inifile[512] = "";

static Uint8			*screenptr;
static bool				g_bActive = true;
static bool				g_inDebug = false;
static bool				g_FrameRate = true;
static bool				g_50Hz = true;
static bool				g_bSaveSettings = true;
static bool				g_FullScreen = false;
static bool				g_SoundOn = true;

/* ---------- Inline functions ---------- */

inline double myMin(double a, double b)
{
    return a>b ? b : a;
}

inline double myMax(double a, double b)
{
    return a>b ? a : b;
}

//-----------------------------------------------------------------------------
// Name: ShowFrameRate()
//-----------------------------------------------------------------------------
inline static void ShowFrameRate()
{
	char fpstxt[80];
	sprintf( fpstxt, "%dFPS", ad_get_fps());
	ted8360->texttoscreen(56,36,fpstxt);
}

//-----------------------------------------------------------------------------
// Name: DebugInfo()
//-----------------------------------------------------------------------------
inline static void DebugInfo()
{
	const Uint32 hpos = 36, vpos = 12;

	sprintf(textout,"OPCODE: %02x ",machine->getcins());
	ted8360->texttoscreen(hpos,vpos,textout);
	ted8360->texttoscreen(hpos,vpos+8,"  PC  SR AC XR YR SP");
	sprintf(textout,";%04x %02x %02x %02x %02x %02x",machine->getPC(),machine->getST(),machine->getAC(),machine->getX(),machine->getY(), machine->getSP());
	ted8360->texttoscreen(hpos,vpos+16,textout);
	sprintf(textout,"TAPE: %08d ",ted8360->tap->TapeSoFar);
	ted8360->texttoscreen(hpos,vpos+24,textout);
}

/* ---------- File loading functions ---------- */

void CopyToKbBuffer(char *text, unsigned int length)
{
	ted8360->Write(0xEF,length);
	while (length--)
		ted8360->Write(0x0527+length,text[length]);
}

//-----------------------------------------------------------------------------

void machineReset()
{
  ted8360->RAMenable = false;
  ted8360->ChangeMemBankSetup();
  ted8360->Reset();
  // this should not be HERE, but where else could I've put it??
  ted8360->tap->rewind();
  machine->Reset();
}

bool start_file(char *szFile )
{
	if(strstr(szFile, ".prg") != NULL) 
	{
		PrgLoad( szFile, 0, (MemoryHandler*)ted8360 );
		CopyToKbBuffer("RUN:\r",5);
		return true;
	}
	else if(strstr(szFile, ".tap") != NULL) 
	{
		ted8360->tap->detach_tap();
		strcpy(ted8360->tap->tapefilename, szFile);
		ted8360->tap->attach_tap();
		return true;
	}
	else if(strstr(szFile, ".d64") != NULL) 
	{
		/*if (!LoadD64Image(szFile)) {
			return false;
		}*/
		CopyToKbBuffer("LOAD\042*\042,8,1\rRUN\r", 15);
		//CopyToKbBuffer("L\317\042*\042,8,1\rRUN:\r", 15);
		return true;
	}
	return false;
}

bool autostart_file(char *szFile)
{
	machineReset();
	// do some frames
	g_TotFrames = 0;
	while (g_TotFrames<20) {
		ted8360->ted_process();
		FrameUpdate();
		g_TotFrames++;
	}
	// and then try to load the parameter as file
	return start_file(szFile);
}

/* ---------- Display functions ---------- */

void init_palette()
{
	Uint32 colorindex;
	double	Uc, Vc, Yc,  PI = 3.14159265 ;
	double bsat = 45.0;

    /* Allocate 256 color palette */
    int ncolors = 256;
    g_pal  = (SDL_Color *)malloc(ncolors*sizeof(SDL_Color));

	// calculate palette based on the HUE values
	memset(p4col_calc, 0, 768);
	for ( i=1; i<16; i++)
		for ( register int j = 0; j<8; j++) {
		    Uint8 col;
			if (i == 1)
				Uc = Vc = 0;
			else {
				Uc = bsat * ((double) cos( HUE[i] * PI / 180.0 ));
				Vc = bsat * ((double) sin( HUE[i] * PI / 180.0 ));
			}
			Yc = (luma[j+1] - 2.0)* 255.0 / (5.0 - 2.0); // 5V is the base voltage
			// RED, GREEN and BLUE component
			colorindex = (j)*16*3 + i*3;
			col = (Uint8) myMax(myMin((Yc + 1.14 * Vc),255.0),0);
			p4col_calc[ colorindex ] = p4col_calc[ 384 + colorindex ] = col;
			col = (Uint8) myMax(myMin((Yc - 0.39465 * Uc - 0.58060 * Vc ),255.0),0);
			p4col_calc[ colorindex + 1] = p4col_calc[ 384 + colorindex + 1] = col;
			col = (Uint8) myMax(myMin((Yc + 2.029 * Uc),255.0),0);
			p4col_calc[ colorindex + 2] = p4col_calc[ 384 + colorindex + 2] = col;
		}

	for (i=0 ; i<256; ++i) {
		// Creating an 8 bit SDL_Color structure
		g_pal[i].b=p4col_calc[i*3+2];
		g_pal[i].g=p4col_calc[i*3+1];
		g_pal[i].r=p4col_calc[i*3];
		g_pal[i].unused=0;
	}

	palette.ncolors = 256;
	palette.colors = g_pal;

}

inline void FrameUpdate(void)
{
    SDL_Rect rc, dest;
    rc.x = 64;
    rc.y = 36;
    rc.w = 384;
    rc.h = 288;
    dest.x = 0;
    dest.y = 0;
    dest.w = 320;
    dest.h = 240;
    

	SDL_BlitSurface( surface, &rc , screen , &dest);
	SDL_Flip (screen);
}

/* ---------- Management of settings ---------- */

//-----------------------------------------------------------------------------
// Name: SaveSettings
//-----------------------------------------------------------------------------
bool SaveSettings()
{
	FILE *ini;
	unsigned int rammask;

	if (ini=fopen(inifile,"wt")) {

		fprintf(ini, "[Yape configuration file]\n");
		fprintf(ini,"DisplayFrameRate = %d\n",g_FrameRate);
		fprintf(ini,"DisplayQuickDebugInfo = %d\n",g_inDebug);
		fprintf(ini,"50HzTimerActive = %d\n",g_50Hz);
		fprintf(ini,"JoystickEmulation = %d\n",ted8360->joyemu);
		fprintf(ini,"ActiveJoystick = %d\n",ted8360->keys->activejoy);
		rammask = ted8360->getRamMask();
		fprintf(ini,"RamMask = %x\n",rammask);
		fprintf(ini,"256KBRAM = %x\n",ted8360->bigram);
		fprintf(ini,"SaveSettingsOnExit = %x\n",g_bSaveSettings);

		for (i = 0; i<4; i++) {
			fprintf(ini,"ROMC%dLOW = %s\n",i, ted8360->romlopath[i]);
			fprintf(ini,"ROMC%dHIGH = %s\n",i, ted8360->romhighpath[i]);
		}

		fclose(ini);
		return true;
	}
	return false;
}

bool LoadSettings()
{
	FILE *ini;
	unsigned int rammask;
	char keyword[256], line[256], value[256];

	if (ini=fopen(inifile,"r")) {

		fscanf(ini,"%s configuration file]\n", keyword);
		if (strcmp(keyword, "[Yape"))
			return false;

		while(fgets(line, 255, ini)) {
			strcpy(value, "");
			if (sscanf(line, "%s = %s\n", keyword, value) ) {
				if (!strcmp(keyword, "DisplayFrameRate")) {
					g_FrameRate = !!atoi(value);
					//	fprintf( stderr, "Display frame rate: %i\n", g_FrameRate);
				} else if (!strcmp(keyword, "DisplayQuickDebugInfo"))
					g_inDebug = !!atoi(value);
				else if (!strcmp(keyword, "50HzTimerActive"))
					g_50Hz = !!atoi(value);
				else if (!strcmp(keyword, "JoystickEmulation"))
					ted8360->joyemu = !!atoi(value);
				else if (!strcmp(keyword, "ActiveJoystick"))
					ted8360->keys->activejoy = atoi(value);
				else if (!strcmp(keyword, "RamMask")) {
					sscanf( value, "%04x", &rammask);
					ted8360->setRamMask( rammask );
				}
				else if (!strcmp(keyword, "256KBRAM"))
					ted8360->bigram = !!atoi(value);
				else if (!strcmp(keyword, "SaveSettingsOnExit"))
					g_bSaveSettings = !!atoi(value);
				else if (!strcmp(keyword, "ROMC0LOW"))
					strcpy(ted8360->romlopath[0], value);
				else if (!strcmp(keyword, "ROMC1LOW"))
					strcpy(ted8360->romlopath[1], value);
				else if (!strcmp(keyword, "ROMC2LOW"))
					strcpy(ted8360->romlopath[2], value);
				else if (!strcmp(keyword, "ROMC3LOW"))
					strcpy(ted8360->romlopath[3], value);
				else if (!strcmp(keyword, "ROMC0HIGH"))
					strcpy(ted8360->romhighpath[0], value);
				else if (!strcmp(keyword, "ROMC1HIGH"))
					strcpy(ted8360->romhighpath[1], value);
				else if (!strcmp(keyword, "ROMC2HIGH"))
					strcpy(ted8360->romhighpath[2], value);
				else if (!strcmp(keyword, "ROMC3HIGH"))
					strcpy(ted8360->romhighpath[3], value);
			}
		}
		fclose(ini);

		/*if (!g_50Hz)
			g_SoundOn = false;*/

		return true;
	}
	return false;
}

inline void PopupMsg(char *msg)
{
	Uint32 ix;
	size_t len = strlen(msg);
	char dummy[40];

	ix = Uint32(len);
	while( ix-->0)
		dummy[ix] = 32;
	dummy[len] = '\0';

	ted8360->texttoscreen(220-(len<<2),144,dummy);
	ted8360->texttoscreen(220-(len<<2),152,msg);
	ted8360->texttoscreen(220-(len<<2),160,dummy);
	FrameUpdate();
	SDL_Delay(750);
}

//-----------------------------------------------------------------------------
// Name: SaveBitmap()
// Desc: Saves the SDL surface to Windows bitmap file named as yapeXXXX.bmp
//-----------------------------------------------------------------------------
/*int SaveBitmap()
{
	bool			success = true;
    char			bmpname[16];
	FILE			*fp;
	int				ix = 0;

    // finding the last yapeXXXX.bmp image
    while (success) {
        sprintf( bmpname, "yape%.4d.bmp", ix);
        fp=fopen( bmpname,"rb");
        if (fp)
            fclose(fp);
		else
			success=false;
        ix++;
    };
	//fprintf( stderr, "%s\n", bmpname);
	return SDL_SaveBMP( screen, bmpname);
}*/
//-----------------------------------------------------------------------------
// Name: poll_events()
// Desc: polls SDL events if there's any in the message queue
//-----------------------------------------------------------------------------
inline static void poll_events(void)
{
    if ( SDL_PollEvent(&event) ) {
        switch (event.type) {
	        case SDL_KEYDOWN:

            /*printf("The %d,%d,%s key was pressed!\n",event.key.keysym.sym,event.key.keysym.scancode,
                   SDL_GetKeyName(event.key.keysym.sym));*/

				if (event.key.keysym.mod & KMOD_LALT) {
						switch (event.key.keysym.sym) {
							case SDLK_j :
								ted8360->joyemu=!ted8360->joyemu;
								if (ted8360->joyemu) {
									/*sprintf(textout , " JOYSTICK EMULATION IS ON ");*/
									ted8360->keys->joyinit();
								} else {
									/*sprintf(textout , " JOYSTICK EMULATION IS OFF ");*/
									ted8360->keys->releasejoy();
								};
								/*PopupMsg(textout);*/
								break;
							case SDLK_i :
								ted8360->keys->swapjoy();
								/*sprintf(textout, "ACTIVE JOY IS : %d", ted8360->keys->activejoy);
								PopupMsg(textout);*/
								break;
							case SDLK_w :
								g_50Hz = ! g_50Hz ;
								/*if (g_50Hz) {
									sprintf(textout , " 50 HZ TIMER IS ON ");
								} else {
									sprintf(textout , " 50 HZ TIMER IS OFF ");
								}
								PopupMsg(textout);*/
								// restart counter
								g_TotFrames = 0;
								timeelapsed = SDL_GetTicks();
								break;
							case SDLK_s:
								g_TotFrames = 0;
								g_FrameRate = !g_FrameRate;
								break;
						};
						return;
				}
				switch (event.key.keysym.sym) 
				{

					case SDLK_PAUSE :
						/*if (g_bActive)
							PopupMsg(" PAUSED ");*/
						g_bActive=!g_bActive;
						break;
					/*case SDLK_F5 :
					case SDLK_F6 :
						ted8360->tap->PressTapeButton(ted8360->GetClockCount());
						break;

						break;
					case SDLK_F7:
						if (!SaveBitmap( ))
							fprintf( stderr, "Screenshot saved.\n");
						break;*/
					case SDLK_ESCAPE:
						// close audio
						// release the palette
						if (g_pal)
							free(g_pal);
						if (g_bSaveSettings)
							SaveSettings();
						SDL_Quit();
						exit(0);
						break;
					/*case SDLK_F9 :
						g_inDebug=!g_inDebug;
						break;
					case SDLK_F10 :
						if (SaveSettings())
						  fprintf( stderr, "Settings saved to %s.\n", inifile);
						else
						  fprintf( stderr, "Oops! Could not save settings... %s\n", inifile);
						break;
					case SDLK_F11 :
						g_bActive = false;
						break;
					case SDLK_F12 :
						// Save settings if required
						if (g_bSaveSettings)
							SaveSettings();
						exit(0);*/
					case SDLK_LEFT:
						ted8360->keys->pushkey(122);
					break;
					case SDLK_RIGHT:
						ted8360->keys->pushkey(120);
					break;
					default :
						ted8360->keys->pushkey(event.key.keysym.sym&0x1FF);
				}
				break;

	        case SDL_KEYUP:
				switch (event.key.keysym.sym) {
					case SDLK_F11 :
						g_bActive = true;
		                if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT) ) {
				  machineReset();
							break;
						}
						if (event.key.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL) ) {
							ted8360->RAMenable = false;
							ted8360->ChangeMemBankSetup();
							break;
			            }
						machine->softreset();
						break;
					case SDLK_LEFT:
						ted8360->keys->releasekey(122);
					break;
					case SDLK_RIGHT:
						ted8360->keys->releasekey(120);
					break;
					default:
						ted8360->keys->releasekey(event.key.keysym.sym&0x1FF);
				}
				break;

            case SDL_QUIT:
				// close audio
				// release the palette
				if (g_pal)
					free(g_pal);
				if (g_bSaveSettings)
					SaveSettings();
				SDL_Quit();
                exit(0);
        }
    }
}

static void MachineInit()
{
	CFakeTCBM *tcbm_l = new CFakeTCBM();
	CFakeIEC *iec_l = new CFakeIEC(8);
	fsdrive = new CIECFSDrive(".");

	iec = iec_l;
	tcbm = tcbm_l;
	tcbm_l->AddIECInterface((CIECInterface*)iec);
	iec_l->AddIECDevice((CIECDevice*)fsdrive);
	ted8360->loadroms();
	ted8360->HookTCBM(tcbm);
	tcbm_l->Reset();
	iec->Reset();
	/*fsdrive->Reset();*/
	CSerial::InitPorts();

	uinterface = new UI(ted8360, screen, surface, inipath);
}

static void app_initialise()
{
	SDL_Init(SDL_INIT_VIDEO);
	// set the appropriate video mode
	//screen = SDL_SetVideoMode(456, 312, 8, SDL_SRCCOLORKEY|SDL_SWSURFACE  );
	screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE  );
    surface = SDL_CreateRGBSurface(0,456,312,8,0,0,0,0);

	// set the window range to be updated
    dest.x = 0;
    dest.y = 0;
    dest.w = surface->w;
    dest.h = surface->h;

	// calculate and initialise palette
	init_palette();

	// set colors to the screen buffer
	int SDLresult = SDL_SetColors(surface, g_pal, 0, 256);

	// change the pointer to the pixeldata of the backbuffer
	surface->pixels = ted8360->screen;

	/*if (SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL))
		fprintf(stderr,"Oops... could not set keyboard repeat rate.\n");*/
}

/* ---------- MAIN ---------- */

int main(int argc, char *argv[])
{
	app_initialise();
	MachineInit();

	KEYS::initPcJoys();
	machine->Reset();
	ted8360->cpuptr = machine;
	/* ---------- Command line parameters ---------- */
	if (argv[1]!='\0') {
		/*printf("Parameter 1 :%s\n", argv[1]);*/
		// and then try to load the parameter as file
		autostart_file(argv[1]);
	}
	/*ad_vsync_init();*/
	for (;;) {
		// hook into the emulation loop if active
			ted8360->ted_process();
			poll_events();
			/*if (g_inDebug)
				DebugInfo();*/
			/*ad_vsync(g_50Hz );*/
			/*ShowFrameRate();*/
			/*if (g_FrameRate)
				ShowFrameRate();*/
			// frame update
			FrameUpdate();
	}
	return 0;
}

