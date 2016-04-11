/*
	YAPE - Yet Another Plus/4 Emulator

	The program emulates the Commodore 264 family of 8 bit microcomputers

	This program is free software, you are welcome to distribute it,
	and/or modify it under certain conditions. For more information,
	read 'Copying'.

	(c) 2000, 2001, 2005, 2007 Attila Gr�z
	(c) 2005 VENESZ Roland
*/

#include "archdep.h"
#include <SDL/SDL.h>

unsigned int	tick_50hz, tick_vsync, fps;

/* ---------- UNIX ---------- */

#include <stdio.h>
#include <unistd.h>
#include <glob.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#define INVALID_HANDLE_VALUE 0x00

glob_t		gl;
unsigned int	gl_curr;
unsigned int	gl_currsize;
char		temp[512];
char		sem_vsync, sem_fps;

int ad_get_curr_dir(char *pathstring, size_t size)
{
        getcwd(pathstring, size);
        return 1;
}

char *ad_get_curr_dir(char *pathstring)
{
	size_t size = 512;
	getcwd(pathstring, size);
	return pathstring;
}

int ad_set_curr_dir(char *path)
{
        if (chdir(path) == 0) return 1;
        return 0;
}

int ad_find_first_file(char *filefilter)
{
        return 1;
}

char *ad_return_current_filename(void)
{
  //	fprintf(stderr, "File: %s parsed\n", gl.gl_pathv[gl_curr]);
  if (!gl.gl_pathc)
    return 0;
  return (char *) gl.gl_pathv[gl_curr];
}

UI_FileTypes ad_return_current_filetype(void)
{
  // Probably too kludgy but I dunno Unix so who cares...
  DIR *dirp;
  if ( (dirp = opendir(gl.gl_pathv[gl_curr])) != NULL) {
    closedir(dirp);
    return FT_DIR;
  } else
    return FT_FILE;
}

unsigned int ad_get_current_filesize(void)
{
        struct stat buf;

        // Size cache!
        if (gl_currsize != 0) return gl_currsize;

        // If the file size actually is 0, subsequent calls to this
        // function will stat the file over and over again. I don't care.
        if (stat(gl.gl_pathv[gl_curr], &buf) == 0) {
                return (gl_currsize = (unsigned int) buf.st_size);
        } else return 0;
}

int ad_find_next_file(void)
{
        if (++gl_curr >= gl.gl_pathc) {
                // No more matches: we don't need the glob any more.
                //globfree(&gl);
                return 0;
        }
        gl_currsize = 0;
        return 1;
}

int	ad_find_file_close(void)
{
	
}

int ad_makedirs(char *path)
{
	// Possible buffer overflow fixed.
	strncpy(temp, path, 512);
	if (strlen(temp) > 506) return 0;
	strcat(temp, "/yape");
	mkdir(temp, 0777);

	return 1;
}

char *ad_getinifilename(char *tmpchr)
{
	// Possible buffer overflow fixed.
	strncpy(temp, tmpchr, 512);
	if (strlen(temp) > 496) return NULL;
	strcat( temp, "/yape/yape.conf");
	return temp;
}

void _ad_vsync_sigalrm_handler(int signal) {
	sem_vsync = 0x01;

	// Refresh FPS every 3 seconds. To avoid race condition, do
	// it in the next call if tick_vsync is locked.
	if (++tick_50hz >= 3*20 && sem_fps != 0x00) {
		fps = (int) (50 * ((double) tick_vsync / (double) tick_50hz));
		tick_vsync = 0;
		tick_50hz = 0;
	}
}

unsigned int timeelapsed;

void ad_vsync_init(void)
{
	timeelapsed = SDL_GetTicks();
}

void ad_vsync(bool sync)
{
	unsigned int time_limit = timeelapsed + 20;
	timeelapsed = SDL_GetTicks();
	if (sync) {
		if (time_limit>timeelapsed) {
		   	int nr10ms = ((time_limit-timeelapsed)/10) * 10;
		   	SDL_Delay(nr10ms);
		   	timeelapsed = SDL_GetTicks();
		    while (time_limit>timeelapsed) {
		    	SDL_Delay(0);
		    	timeelapsed = SDL_GetTicks();
			}
		}		
	}	    
}

unsigned int ad_get_fps() 
{
    static Uint32 fps = 50;
	static Uint32 g_TotElapsed = SDL_GetTicks();
	static Uint32 g_TotFrames = 0;

    g_TotFrames++;
	if (g_TotElapsed + 2000 < timeelapsed) {
     	g_TotElapsed = SDL_GetTicks();
     	fps = g_TotFrames / 2;
     	g_TotFrames = 0;
  	}	
	return fps;
}
