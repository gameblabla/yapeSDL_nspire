/*
	YAPE - Yet Another Plus/4 Emulator

	The program emulates the Commodore 264 family of 8 bit microcomputers

	This program is free software, you are welcome to distribute it,
	and/or modify it under certain conditions. For more information,
	read 'Copying'.

	(c) 2000, 2001, 2005 Attila Grósz
*/

#include "SDL/SDL.h"
#include "tedmem.h"
#include "interface.h"
#include "archdep.h"
#include "prg.h"
#include "tape.h"
#include <string.h>
#include <stdlib.h>

extern bool autostart_file(char *szFile );

#define COLOR( COL, SHADE) (SHADE<<4)|COL
#define MAX_LINES 25
#define MAX_INDEX (MAX_LINES - 1)

// TODO this is ugly and kludgy like hell but I don't have time for better
static menu_t main_menu = {
	"Main menu",
	"",
	{
		{"Load PRG file...", 0, UI_FILE_LOAD_PRG },
		{"Attach disk image...", 0, UI_DRIVE_ATTACH_IMAGE },
		//{"Emulator snapshot...", 0, UI_MENUITEM_MENULINK },
		{"                ", 0, UI_MENUITEM_SEPARATOR },
		{"Tape controls...", 0, UI_MENUITEM_MENULINK },
		{"             ", 0, UI_MENUITEM_SEPARATOR },
		{"Options..."	, 0, UI_MENUITEM_MENULINK },
		{"             ", 0, UI_MENUITEM_SEPARATOR },
		{"Resume emulation"	, 0, UI_EMULATOR_RESUME },
		{"             ", 0, UI_MENUITEM_SEPARATOR },
		{"Quit YAPE"	, 0, UI_EMULATOR_EXIT }
	},
	0,
	10,
	0,
	0,
	0
};

static menu_t tape_menu = {
	"Tape control menu",
	"",	
	{
		{"Attach TAP image...", 0, UI_TAPE_ATTACH_TAP },
		{"             ", 0, UI_MENUITEM_SEPARATOR },
		{"Create TAP image", 0, UI_TAPE_CREATE_TAP },
		{"             ", 0, UI_MENUITEM_SEPARATOR },
		{"Detach TAP image", 0, UI_TAPE_DETACH_TAP },
		{"             ", 0, UI_MENUITEM_SEPARATOR },
		{"Press PLAY"	, 0, UI_TAPE_PLAY },
		{"Press RECORD"	, 0, UI_TAPE_RECORD },
		{"Press STOP"	, 0, UI_TAPE_STOP },
		{"Rewind tape"	, 0, UI_TAPE_REWIND }
	},
	0,
	10,
	0,
	0,
	0
};

static menu_t snapshot_menu = {
	"Emulator snapshot menu",
	"",	
	{
		{"Load snapshot...", 0, UI_SNAPSHOT_LOAD },
		{"Save quicksnapshot...", 0, UI_SNAPSHOT_QUICKSAVE }
	},
	0,
	2,
	0,
	0,
	0
};

static menu_t drive_menu = {
	"Drive control menu",
	"",	
	{
		{"Attach disk image...", 0, UI_DRIVE_ATTACH_IMAGE },
		{"Set drive directory...", 0, UI_DRIVE_SET_DIR },
	},
	0,
	2,
	0,
	0,
	0
};

static menu_t options_menu = {
	"Options menu",
	"",	
	{
		{"ROM settings", 0, UI_MENUITEM_MENULINK },
		{"             ", 0, UI_MENUITEM_SEPARATOR },
		{"RAM size     ", 0, UI_MENUITEM_MENULINK },
		{"             ", 0, UI_MENUITEM_SEPARATOR },
		{"Monitor settings"	, 0, UI_MENUITEM_MENULINK }
	},
	0,
	5,
	0,
	0,
	0
};

static SDL_Surface *screen, *image;
static menu_t file_menu;
static menu_t tap_list;
static menu_t d64_list;
//static menu_t snapshot_list;

UI::UI(class TED *ted, SDL_Surface *buf1, SDL_Surface *buf2, char *path) :
	display(ted->screen), charset((ted->RomHi[0])+0x1400), ted8360(ted)
{
	screen = buf1;
	image = buf2;

	// Link menu structure
	main_menu.element[0].child = &file_menu;
	main_menu.element[1].child = &d64_list;
	main_menu.element[2].child = &snapshot_menu;
	main_menu.element[3].child = &tape_menu;
	main_menu.element[5].child = &options_menu;
	tape_menu.element[0].child = &tap_list;
	
	curr_menu = &main_menu;
	strcpy( file_menu.title, "PRG file selection menu");
	strcpy( tape_menu.title, "Tape controls menu");
	strcpy( tap_list.title, "TAP image selection menu");
	strcpy( d64_list.title, "Disk image selection menu");
	strcpy( snapshot_menu.title, "Snapshot selection menu");
	file_menu.parent = curr_menu;
	options_menu.parent = curr_menu;
	tape_menu.parent = curr_menu;
	tap_list.parent = &tape_menu;
	d64_list.parent = curr_menu;
	snapshot_menu.parent = curr_menu;
}

void UI::screen_update(void)
{
	SDL_BlitSurface( image, NULL , screen, NULL);
	SDL_Flip ( screen );
}

void UI::clear(char color, char shade)
{
	for (register int i=0; i<SCR_HSIZE*SCR_VSIZE; ++i)
		display[i] = COLOR( color, shade );
	screen_update();
}

void UI::set_color(char foreground, char background)
{
	bgcol = background;
	fgcol = foreground;
}

void UI::texttoscreen(int x,int y, char *scrtxt)
{
	register int i = 0;

	while (scrtxt[i]!=0)
		chrtoscreen(x+i*8,y,asc2pet(scrtxt[i++]));
}

void UI::chrtoscreen(int x,int y, char scrchr)
{
	register int j, k;
	unsigned char *cset;

	cset = charset + (scrchr<<3);
	for ( j=0; j<8; j++)
		for (k=0;k<8;k++)
			(*(cset+j) & (0x80>>k)) ? display[(y+j)*456+x+k]=fgcol : display[(y+j)*456+x+k]=bgcol;
}

void UI::hide_sel_bar(struct menu_t *menu)
{
	if ( menu->element[ menu->curr_sel ].menufunction == UI_DIR_ITEM)
		set_color( COLOR(2,1), COLOR(1,5) );
	else	
		set_color( COLOR(0,0), COLOR(1,5) );
	texttoscreen(100,64+(curr_menu->curr_line<<3), 
		menu->element[ curr_menu->curr_sel ].name);
	screen_update();
}

void UI::show_sel_bar(struct menu_t * menu)
{
	set_color( COLOR(0,0), COLOR(1,7) );
	texttoscreen(100,64+(curr_menu->curr_line<<3), 
		menu->element[ curr_menu->curr_sel ].name);
	screen_update();
}

void UI::show_file_list(struct menu_t * menu, UI_MenuClass type)
{
	int nf = 0, res;
	char cfilename[256];
	char *fileFoundName;

	menu->nr_of_elements = 0;
	res = ad_find_first_file("*");
	fileFoundName = ad_return_current_filename();
	if (fileFoundName && !strcmp(fileFoundName, ".") )
		res = ad_find_next_file();
	if (!fileFoundName || strcmp(fileFoundName, "..") ) {
	  strcpy( menu->element[nf++].name, "..");
	  menu->element[0].menufunction = UI_DIR_ITEM;
	}
	while(res) {
	  // fprintf(stderr,"Parsed: %s\n", ad_return_current_filename());
		if ( ad_return_current_filetype() == FT_DIR ) {
			strcpy( menu->element[nf].name, ad_return_current_filename());
			menu->element[nf].menufunction = UI_DIR_ITEM;
			++nf;
		}
		res = ad_find_next_file();
	}
	ad_find_file_close();
	
	switch ( type ) {
		case UI_PRG_ITEM:
			strcpy( cfilename, "*.prg");
			break;
		case UI_TAP_ITEM:
			strcpy( cfilename, "*.tap");
			break;
		case UI_D64_ITEM:
			strcpy( cfilename, "*.d64");
			break;
		case UI_FRE_ITEM:
			strcpy( cfilename, "*.fre");
			break;
		case UI_DRIVE_SET_DIR:
			menu->nr_of_elements = nf;
			return;
	}

	res = ad_find_first_file( cfilename);
	while(res) {
		strcpy( menu->element[nf].name, ad_return_current_filename());
		menu->element[nf].menufunction = type;
		++nf;
		res = ad_find_next_file();
	}
	menu->nr_of_elements = nf;
	ad_find_file_close();
}

bool UI::handle_menu_command( struct element_t *element)
{
  //if ( !strcmp( element->name, "..") ) {
  //	ad_set_curr_dir( ".." );
  //	return false;
  //}
	int menuSelection;
	UI_MenuClass ptype;

	// FIXME this must be nicer
	if (!curr_menu->parent) {
		menuSelection = element->menufunction;
	} else {
		ptype = curr_menu->parent->element[ curr_menu->parent->curr_sel ].menufunction;
		menuSelection = element->menufunction;
	}
	
	switch ( menuSelection ) {
		case UI_DIR_ITEM:
			ad_set_curr_dir( element->name );
			ad_get_curr_dir( (char*)&(curr_menu->subtitle) );
			if ( ptype == UI_FILE_LOAD_PRG )
				show_file_list( &file_menu, UI_PRG_ITEM );
			else if ( ptype == UI_TAPE_ATTACH_TAP )
				show_file_list( &tap_list, UI_TAP_ITEM );
			else if ( ptype == UI_DRIVE_ATTACH_IMAGE )
				show_file_list( &d64_list, UI_D64_ITEM );
			curr_menu->curr_sel = 0;
			curr_menu->curr_line = 0;
			curr_menu->uppermost = 0;
			/*if ( curr_menu->curr_sel >= file_menu.nr_of_elements )
				curr_menu->curr_sel = file_menu.nr_of_elements - 1;*/
			return false;

		case UI_PRG_ITEM:
			{
				Uint8 *keystate = SDL_GetKeyState(NULL);
				if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT]) {
					autostart_file(element->name);
				} else {
					PrgLoad( element->name, 0, (MemoryHandler*)ted8360);
				}
			}
			clear (0, 0);
			break;
		case UI_TAP_ITEM:
			strcpy( ted8360->tap->tapefilename, element->name);
			ted8360->tap->attach_tap();
			break;
		case UI_D64_ITEM:
			break;
		case UI_FRE_ITEM:
			break;

		case UI_TAPE_DETACH_TAP:
			ted8360->tap->detach_tap();
			break;
		case UI_TAPE_REWIND:
			ted8360->tap->rewind();
			break;
		case UI_FILE_LOAD_PRG:
			show_file_list( &file_menu, UI_PRG_ITEM );
			return false;
		case UI_TAPE_ATTACH_TAP:
			show_file_list( &tap_list, UI_TAP_ITEM );
			return false;
		case UI_DRIVE_ATTACH_IMAGE:
			show_file_list( &d64_list, UI_D64_ITEM );
			return false;
		case UI_SNAPSHOT_LOAD:
			show_file_list( &snapshot_menu, UI_FRE_ITEM );
			return false;
		case UI_DRIVE_DETACH_IMAGE:
			return false;	
		case UI_MENUITEM_MENULINK:
			return false;
		case UI_EMULATOR_RESUME:
			void ();
			return true;
		case UI_EMULATOR_EXIT:
			exit(1);
			break;
		default:
			break;
	}
	return true;
}

void UI::show_title(struct menu_t * menu)
{
	size_t titlen = strlen( menu->title ) << 3;
	set_color( COLOR(8,7), COLOR(7,0) );
	texttoscreen( (456 - int(titlen))/2, 24,menu->title);
	
	titlen = strlen( menu->subtitle ) << 3;
	set_color( COLOR(8,0), COLOR(1,5) );
	texttoscreen( (456 - int(titlen))/2, 40,petstr2ascstr(menu->subtitle));
	
	char helptxt[] = "Arrow keys to navigate, ENTER to select, ESC to resume";
	texttoscreen( (456 - int(strlen(helptxt)*8)) / 2, 312 - 32, petstr2ascstr(helptxt));
	set_color( fgcol, bgcol );
}

void UI::show_menu(struct menu_t * menu)
{
	int i;
	int shown = menu->nr_of_elements - menu->uppermost;
	
	show_title( menu );
	if ( shown > 0 ) {
		if ( shown > MAX_LINES ) shown = MAX_LINES + 1;

		set_color( COLOR(0,0), COLOR(1,5) );
		for ( i = 0; i < shown; ++i) {
			if ( menu->element[menu->uppermost + i].menufunction == UI_DIR_ITEM)
				set_color( COLOR(2,1), COLOR(1,5) );
			else
				set_color( COLOR(0,0), COLOR(1,5) );
			texttoscreen(100,64+(i<<3),menu->element[menu->uppermost + i].name);
		}
	}
	screen_update();
}

void UI::enterMenu()
{
	SDL_EnableKeyRepeat(200, SDL_DEFAULT_REPEAT_INTERVAL);

	ad_get_curr_dir( (char*)&(tap_list.subtitle) );
	ad_get_curr_dir( (char*)&(file_menu.subtitle) );

	clear (1, 5);
	show_menu( curr_menu);	
	show_sel_bar( curr_menu );
	wait_events();	
	
	clear (0, 0);
	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
}

int UI::wait_events()
{
	SDL_Event event;
	element_t *elem;
	
	while (SDL_WaitEvent(&event)) {
		switch (event.type) {
	        case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE :
						/*if ( curr_menu->parent ) {
							clear (1, 5);
							curr_menu = curr_menu->parent;
							show_menu( curr_menu );
							show_sel_bar( curr_menu );
							break;
						}*/
					case SDLK_F8:
						clear (0, 0);
						return 0;
						
					case SDLK_HOME:
						hide_sel_bar( curr_menu);
						clear (1, 5);
						curr_menu->curr_sel = 0;
						curr_menu->curr_line = 0;
						curr_menu->uppermost = 0;
						show_menu( curr_menu );
						show_sel_bar( curr_menu);
						break;
					case SDLK_END:
						/*hide_sel_bar( curr_menu);
						clear (1, 5);
						curr_menu->curr_sel = 
						curr_menu->uppermost = curr_menu->nr_of_elements - 1;
						curr_menu->curr_line = 
							curr_menu->nr_of_elements >= MAX_LINES ? 
								MAX_LINES - 1 : curr_menu->nr_of_elements;
						show_menu( curr_menu );
						show_sel_bar( curr_menu);*/
						break;
					case SDLK_UP:
						if (curr_menu->curr_sel > 0) {
							int step = 0;
							hide_sel_bar( curr_menu);
							// Step and skip menu separators
							do {
								step++;
							} while (curr_menu->element[curr_menu->curr_sel - step].menufunction 
								== UI_MENUITEM_SEPARATOR);
							if (curr_menu->curr_line == 0) {
								curr_menu->uppermost -= 1;
								clear (1, 5);
								show_menu( curr_menu );
							} else
								curr_menu->curr_line -= step;
							curr_menu->curr_sel -= step;
							show_sel_bar( curr_menu);
						}
						break;
					case SDLK_DOWN:
						if (curr_menu->curr_sel < curr_menu->nr_of_elements - 1) {
							int step = 0;
							hide_sel_bar( curr_menu);
							// Step and skip menu separators
							do {
								step++;
							} while (curr_menu->element[curr_menu->curr_sel + step].menufunction 
								== UI_MENUITEM_SEPARATOR);
							if (curr_menu->curr_line==MAX_LINES) {
								curr_menu->uppermost += 1;
								clear (1, 5);
								show_menu( curr_menu );
							} else
								curr_menu->curr_line += step;
							curr_menu->curr_sel += step;
							show_sel_bar( curr_menu);
						}
						break;
					case SDLK_PAGEUP:
						hide_sel_bar( curr_menu);
						if (curr_menu->curr_line != 0 ) {
							curr_menu->curr_sel = curr_menu->uppermost;
							curr_menu->curr_line = 0;
						} else {
							curr_menu->curr_sel -= MAX_LINES;
							if ( curr_menu->curr_sel < 0 )
								curr_menu->curr_sel = 0;
							curr_menu->uppermost = curr_menu->curr_sel;
							clear (1, 5);
							show_menu( curr_menu );
						}
						show_sel_bar( curr_menu);
						break;
					case SDLK_PAGEDOWN:
						hide_sel_bar( curr_menu );
						if (curr_menu->curr_line != MAX_LINES ) {
							int nr_el = curr_menu->nr_of_elements - 1;
							curr_menu->curr_sel = curr_menu->curr_line = 
								nr_el < MAX_LINES ? nr_el : MAX_LINES;
						} else {
							int nr_el = curr_menu->nr_of_elements - 1;
							curr_menu->curr_sel += MAX_LINES;
							if ( curr_menu->curr_sel > nr_el )
								curr_menu->curr_sel = nr_el;
							curr_menu->uppermost = curr_menu->curr_sel - MAX_LINES;
							clear (1, 5);
							show_menu( curr_menu );							
						}
						show_sel_bar( curr_menu);
						break;
					case SDLK_LEFT:
						if ( curr_menu->parent ) {
							curr_menu = curr_menu->parent;
							clear (1, 5);
							show_menu( curr_menu );
							show_sel_bar( curr_menu );
						}
						break;
					case SDLK_RIGHT:
					case SDLK_RETURN:
						if ( curr_menu->element[ curr_menu->curr_sel ].child ) {
							elem = &(curr_menu->element[ curr_menu->curr_sel ]);
							curr_menu = elem->child;
							/*int sel = elem->menufunction;
							if ( sel == UI_FILE_LOAD_PRG ) {
								show_file_list( &file_menu, "*.prg", UI_PRG_ITEM );
							} else if ( sel == UI_TAPE_ATTACH_TAP ) {
								show_file_list( &tap_list, "*.tap", UI_TAP_ITEM );
							}
							*/
						} else {
							elem = &(curr_menu->element[ curr_menu->curr_sel ]);
						}
						if ( handle_menu_command( elem ) ) {
							clear (0, 0);
							return 0;
						} else {
							clear (1, 5);
							show_menu( curr_menu );
							show_sel_bar( curr_menu);
						}
						break;
					default :
						break;
				};
				break;
			case SDL_QUIT:
				SDL_Quit();
				exit(0);
		}
	}
	return 1;
}

char UI::pet2asc(char c)
{
	if ((c == '\\'))
		return char(0x4D|0x80);
	if (c == 0x5F)
		return char(0x6F|0x80);
	/*if ((c >= 'A') && (c <= 'Z') || (c >= 'a') && (c <= 'z'))
		return c ^ 0x20;*/
	/*if ((c >= 0xc1) && (c <= 0xda))
		return c ^ 0x80;*/

	return c;
}

char *UI::petstr2ascstr(char *string)
{
	char *p = string;
	
	while (*p) {
		*p = pet2asc( *p );
		p++;
	}
	return string;
}

char UI::asc2pet(char c)
{
	if ((c >= 'A') && (c <= 'Z'))
		return c ;
	if ((c >= 'a') && (c <= 'z'))
		return c - 96;
	/*if ((c >= 0xc1) && (c <= 0xda))
		return c ^ 0x80;*/
	return c;
}

UI::~UI()
{
}
