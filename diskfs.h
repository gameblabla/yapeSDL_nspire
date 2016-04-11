#ifndef _DISKFS_H
#define _DISKFS_H

#include <stdio.h>
#include "archdep.h"
#include "device.h"

#define NAMEBUF_LENGTH MAX_PATH

class CIECFSDrive : public CIECDrive {
public:
	CIECFSDrive(char *path);
	virtual ~CIECFSDrive();
	virtual Uint8 Open(int channel);
	virtual Uint8 Close(int channel);
	virtual Uint8 Read(int channel, Uint8 *data);
	virtual Uint8 Write(int channel, Uint8 data, Uint32 cmd, bool eoi);
	virtual void Reset();

private:
	virtual unsigned char OpenFile(int channel, char *filename);
	virtual unsigned char OpenDirectory(int channel, char *filename);
	virtual void ParseFileName(char *srcname, char *destname, int *filemode, int *filetype, bool *wildflag);
	virtual void ExecuteCommand(char *command);
	void FindFirstFile(char *name);
	bool ChangeDir(char *dirpath);
	void ChangeDirCmd(char *dirpath);
	char dir_path[MAX_PATH];
	char orig_dir_path[MAX_PATH];
	char dir_title[16];
	FILE *file[16];

	char cmd_buffer[44];
	int cmd_len;

	unsigned char read_char[16];
};

#endif // _DISKFS_H
