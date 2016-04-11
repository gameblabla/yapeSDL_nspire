#ifndef _DRIVE_H
#define _DRIVE_H

#include "types.h"
#include <string.h>

class CIECDevice {
public:
	virtual Uint8 Open(int channel)=0;
	virtual Uint8 Close(int channel)=0;
	virtual Uint8 Read(int channel, Uint8 *data)=0;
	virtual Uint8 Write(int channel, Uint8 data, Uint32 cmd, bool eoi)=0;
	virtual void Reset()=0;
};

class CIECDrive : public CIECDevice {
public:
	CIECDrive();
	~CIECDrive();
	virtual Uint8 Open(int channel) = 0;
	virtual Uint8 Close(int channel) = 0;
	virtual Uint8 Read(int channel, Uint8 *data) = 0;
	virtual Uint8 Write(int channel, Uint8 data, Uint32 cmd, bool eoi) = 0;
	virtual void Reset() = 0;
	static bool Match(char *p, char *n);
	static char ToASCII(char c);
	static char ToPETSCII(char c);	

protected:
	void CloseAllChannels();
	virtual unsigned char OpenFile(int channel, char *filename) = 0;
	virtual unsigned char OpenDirectory(int channel, char *filename) = 0;
	virtual void ExecuteCommand(char *command) = 0;		
	// 1541/1551 error codes
	enum {
		ERR_OK,				// 00 OK
		ERR_FILESSCRATCHED,	// 01 FILES SCRATCHED (track nr. shows how many)
		ERR_WRITEERROR,		// 25 WRITE ERROR
		ERR_WRITEPROTECT,	// 26 WRITE PROTECT ON
		ERR_SYNTAX30,		// 30 SYNTAX ERROR (unknown command)
		ERR_SYNTAX33,		// 33 SYNTAX ERROR (wildcards on writing)
		ERR_WRITEFILEOPEN,	// 60 WRITE FILE OPEN
		ERR_FILENOTOPEN,	// 61 FILE NOT OPEN
		ERR_FILENOTFOUND,	// 62 FILE NOT FOUND
		ERR_FILEEXISTS,		// 63 FILE EXISTS
		ERR_NOBLOCK,		// 65 NO BLOCK
		ERR_ILLEGALTS,		// 67 ILLEGAL TRACK OR SECTOR
		ERR_NOCHANNEL,		// 70 NO CHANNEL
		ERR_DISKFULL,		// 72 DISK FULL
		ERR_STARTUP,		// 73 Power-up message
		ERR_NOTREADY		// 74 DRIVE NOT READY
	};
	// Status codes
	enum { ST_OK, ST_ERROR = 0x80, ST_EOF = 0x40 };
	// Access modes
	enum { FMODE_READ, FMODE_WRITE, FMODE_APPEND };
	// File types
	enum { FTYPE_PRG, FTYPE_SEQ, FTYPE_REL, FTYPE_USR, FTYPE_DEL };
	// Block commands
	enum {
		BLOCK_ALLOCATE = 0,	BLOCK_FREE
	};
	// Channel modes
	enum {
		CHMOD_FREE,
		CHMOD_COMMAND,
		CHMOD_DIRECTORY,
		CHMOD_FILE,
		CHMOD_DIRECT
	};
	struct Buffer {
		int mode;
		int number;
		unsigned char *data;
		unsigned char *ptr;
		int length;
		bool writeFlag;
	} ch[16];
	bool bufferFree[4];
	//
	void SetError(Uint32 error, Uint32 track, Uint32 sector);
	int BufferAlloc(int buf);
	void BufferFree(int buf);
	void ParseFileName(char *srcName, char *targetName, int &fileMode, 
		int &fileType, int &recLength, bool convert);
	//	
	Uint8 *ram;		// 2KB 1541/1551 RAM
	int currentError;
	char *errorPtr;
	int error_len;
	char current_error[80];
	char name_buf[256];
	char *name_ptr;
	Uint32 name_length;
};

#endif // _DRIVE_H

