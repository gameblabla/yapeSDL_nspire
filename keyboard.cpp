/*
	YAPE - Yet Another Plus/4 Emulator

	The program emulates the Commodore 264 family of 8 bit microcomputers

	This program is free software, you are welcome to distribute it,
	and/or modify it under certain conditions. For more information,
	read 'Copying'.

	(c) 2000, 2001, 2004 Attila Grósz
*/

#include <memory.h>
#include "keyboard.h"

/*
 Values		: C16 keyboard matrix positions
 Positions	: SDL key scan codes
 Eg. C16 has the "return" at matrix postition 8
	 SDL has it at position 13 (0x0D), ie. the 13. position has the value 8
*/

enum {
  	P4K_INS = 0, P4K_RETURN, P4K_POUND, P4K_HELP,  P4K_F1, P4K_F2, P4K_F3, P4K_AT,
  	P4K_3, P4K_W, P4K_A, P4K_4,  P4K_Z, P4K_S, P4K_E, P4K_SHIFT,
  	P4K_5, P4K_R, P4K_D, P4K_6,  P4K_C, P4K_F, P4K_T, P4K_X,
  	P4K_7, P4K_Y, P4K_G, P4K_8,  P4K_B, P4K_H, P4K_U, P4K_V,
  	
  	P4K_9, P4K_I, P4K_J, P4K_0,  P4K_M, P4K_K, P4K_O, P4K_N,
  	P4K_DOWN, P4K_P, P4K_L, P4K_UP,  P4K_PERIOD, P4K_DPOINT, P4K_MINUS, P4K_COMMA,
  	P4K_LEFT, P4K_MULTIPLY, P4K_SEMICOLON, P4K_RIGHT,  P4K_ESC, P4K_EQUAL, P4K_PLUS, P4K_SLASH,
  	P4K_1, P4K_HOME, P4K_CTRL, P4K_2,  P4K_SPACE, P4K_COMMIE, P4K_Q, P4K_STOP
};
    
static unsigned char sdltrans[512];

const unsigned int joystick[5]={
	SDLK_KP8, SDLK_KP6, SDLK_KP2, SDLK_KP4, SDLK_KP0 }; // PC keycodes up, right, down, left and fire

const unsigned int joykeys[2][5]=
	{ {P4K_5, P4K_6, P4K_R, P4K_D, P4K_T }, // keys for joystick 1 and 2 up, right, down, left and fire
	{P4K_3, P4K_4, P4K_W, P4K_A, P4K_SHIFT } };

const unsigned int origkeys[5]={
	P4K_UP, P4K_RIGHT, P4K_DOWN, P4K_LEFT, 255};

//--------------------------------------------------------------
unsigned int KEYS::nrOfJoys;

KEYS::KEYS() {
	register int i;

	memset(sdltrans, 0xFF,sizeof(sdltrans));

	/* NEW MAPPINGS */
	sdltrans[SDLK_BACKSPACE] = P4K_INS;
	sdltrans[SDLK_RETURN] = P4K_RETURN;
	sdltrans[SDLK_END] = P4K_POUND;
	sdltrans[SDLK_F4] = P4K_HELP;
	sdltrans[SDLK_F1] = P4K_F1;
	sdltrans[SDLK_F2] = P4K_F2;
	sdltrans[SDLK_F3] = P4K_F3;
	sdltrans[SDLK_MINUS] = P4K_AT;
	
	sdltrans[SDLK_3] = P4K_3;
	sdltrans[SDLK_w] = P4K_W;
	sdltrans[SDLK_a] = P4K_A;
	sdltrans[SDLK_4] = P4K_4;
	sdltrans[SDLK_z] = P4K_Z;
	sdltrans[SDLK_s] = P4K_S;
	sdltrans[SDLK_e] = P4K_E;
	sdltrans[SDLK_LSHIFT] = P4K_SHIFT;
	sdltrans[SDLK_RSHIFT] = P4K_SHIFT;
	
	sdltrans[SDLK_5] = P4K_5;
	sdltrans[SDLK_r] = P4K_R;
	sdltrans[SDLK_d] = P4K_D;
	sdltrans[SDLK_6] = P4K_6;
	sdltrans[SDLK_c] = P4K_C;
	sdltrans[SDLK_f] = P4K_F;
	sdltrans[SDLK_t] = P4K_T;
	sdltrans[SDLK_x] = P4K_X;
 
	sdltrans[SDLK_7] = P4K_7;
	sdltrans[SDLK_y] = P4K_Y;
	sdltrans[SDLK_g] = P4K_G;
	sdltrans[SDLK_8] = P4K_8;
	sdltrans[SDLK_b] = P4K_B;
	sdltrans[SDLK_h] = P4K_H;
	sdltrans[SDLK_u] = P4K_U;
	sdltrans[SDLK_v] = P4K_V;
	
	sdltrans[SDLK_9] = P4K_9;
	sdltrans[SDLK_i] = P4K_I;
	sdltrans[SDLK_j] = P4K_J;
	sdltrans[SDLK_0] = P4K_0;
	sdltrans[SDLK_m] = P4K_M;
	sdltrans[SDLK_k] = P4K_K;
	sdltrans[SDLK_o] = P4K_O;
	sdltrans[SDLK_n] = P4K_N;
	
	sdltrans[SDLK_DOWN] = P4K_DOWN;
	sdltrans[SDLK_p] = P4K_P;
	sdltrans[SDLK_l] = P4K_L;
	sdltrans[SDLK_UP] = P4K_UP;
	sdltrans[SDLK_PERIOD] = P4K_PERIOD;
	sdltrans[SDLK_SEMICOLON] = P4K_DPOINT;
	sdltrans[SDLK_RIGHTBRACKET] = P4K_MINUS;
	sdltrans[SDLK_COMMA] = P4K_COMMA;

	sdltrans[SDLK_RIGHT] = P4K_RIGHT;
	sdltrans[SDLK_BACKSLASH] = P4K_MULTIPLY;
	sdltrans[SDLK_QUOTE] = P4K_SEMICOLON;
	sdltrans[SDLK_LEFT] = P4K_LEFT;
	sdltrans[SDLK_BACKQUOTE] = P4K_ESC;
	sdltrans[SDLK_EQUALS] = P4K_EQUAL;
	sdltrans[SDLK_LEFTBRACKET] = P4K_PLUS;
	sdltrans[SDLK_SLASH] = P4K_SLASH;
	
	sdltrans[SDLK_1] = P4K_1;
	sdltrans[SDLK_HOME] = P4K_HOME;
	sdltrans[SDLK_RCTRL] = P4K_CTRL;
	sdltrans[SDLK_2] = P4K_2;
	sdltrans[SDLK_SPACE] = P4K_SPACE;
	sdltrans[SDLK_LCTRL] = P4K_COMMIE;
	sdltrans[SDLK_q] = P4K_Q;
	sdltrans[SDLK_TAB] = P4K_STOP;
	
	memset(joytrans,0xFF,sizeof(joytrans));
	activejoy=0;
	for (i=0;i<5;++i)
		joytrans[joystick[i]]=joykeys[activejoy][i];

	empty();

	//initPcJoys();
}

void KEYS::initPcJoys()
{
}

void KEYS::empty(void)
{
	memset(keybuffer,0,256);
	memset(joybuffer,0,256);
}

void KEYS::pushkey(unsigned int code)
{
	*(keybuffer+sdltrans[code])=0x80;
	*(joybuffer+joytrans[code])=0x80;
}

void KEYS::releasekey(unsigned int code)
{
	*(keybuffer+sdltrans[code])=0;
	*(joybuffer+joytrans[code])=0;
}

unsigned char KEYS::key_trans(unsigned char r)
{
	return ~((keybuffer[r*8+0]>>7)
 		|(keybuffer[r*8+1]>>6)
		|(keybuffer[r*8+2]>>5)
      	|(keybuffer[r*8+3]>>4)
       	|(keybuffer[r*8+4]>>3)
        |(keybuffer[r*8+5]>>2)
        |(keybuffer[r*8+6]>>1)
        |(keybuffer[r*8+7]>>0));
}

unsigned char KEYS::feedkey(unsigned char latch)
{
	static unsigned char tmp;

	tmp=0xFF;

	if ((latch&0x01)==0) tmp&=key_trans(0);
	if ((latch&0x02)==0) tmp&=key_trans(1);
	if ((latch&0x04)==0) tmp&=key_trans(2);
	if ((latch&0x08)==0) tmp&=key_trans(3);
	if ((latch&0x10)==0) tmp&=key_trans(4);
	if ((latch&0x20)==0) tmp&=key_trans(5);
	if ((latch&0x40)==0) tmp&=key_trans(6);
	if ((latch&0x80)==0) tmp&=key_trans(7);

	return tmp;
}

unsigned char KEYS::joy_trans(unsigned char r)
{
	return ~((joybuffer[r*8+0]>>7)
 		|(joybuffer[r*8+1]>>6)
		|(joybuffer[r*8+2]>>5)
      	|(joybuffer[r*8+3]>>4)
       //	|(joybuffer[r*8+4]>>3)
       // |(joybuffer[r*8+5]>>2)
        |(joybuffer[r*8+6]>>1)
        |(joybuffer[r*8+7]>>0));
}

unsigned char KEYS::getPcJoyState(unsigned int joyNr, unsigned int activeJoy)
{
	return 0;
}

unsigned char KEYS::feedjoy(unsigned char latch)
{
	static unsigned char tmp;

	tmp=0xFF;

	if ((latch&0x02)==0) {
		tmp&=joy_trans(1);
	}
	if ((latch&0x04)==0) {
		tmp&=joy_trans(2); // Joy2 is wired two times...
	}
	
	return tmp;
}

void KEYS::joyinit(void)
{
	register int i;

	for (i=0;i<5;++i) {
		joytrans[joystick[i]]=joykeys[activejoy][i];
		sdltrans[joystick[i]]=0xFF;
	}
}

void KEYS::swapjoy()
{
	register int i;

	activejoy=1-activejoy;
	for (i=0;i<5;++i)
		joytrans[joystick[i]]=joykeys[activejoy][i];
}

void KEYS::releasejoy()
{
	register int i;

	for (i=0;i<5;++i)
		sdltrans[joystick[i]]=origkeys[i];
}

KEYS::~KEYS() {
	int i = nrOfJoys;
	while (i) {
		i -= 1;
	}
}
