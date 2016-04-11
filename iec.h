#ifndef _IEC_H
#define _IEC_H

#include "SDL/SDL.h"

class CIECInterface {
  public:
    // IEC command & control codes
	enum {
		CMD_LISTEN = 0x20,
		CMD_UNLISTEN = 0x30,
		CMD_TALK = 0x40,
		CMD_UNTALK = 0x50,	    
		CMD_DATA = 0x60,	// Data transfer
		CMD_CLOSE = 0xe0,	// Close channel
		CMD_OPEN = 0xf0		// Open channel
	};      
    virtual void Reset() = 0;
    virtual Uint32 Listen() = 0;
    virtual Uint32 Unlisten() = 0;
    virtual void Talk() = 0;
    virtual void Untalk() = 0;
    virtual Uint32 In(Uint8 *data) = 0;
    virtual Uint32 Out(Uint8 data) = 0;
    virtual Uint32 OutCmd(Uint8 data) = 0;
	virtual Uint32 OutSec(Uint8 data) = 0;    
    virtual Uint8 Status() = 0;
};

#include "device.h"

class CFakeIEC : public CIECInterface {
  protected:
  	enum { STATE_IDLE = 0, STATE_TALKING, STATE_LISTENING };
  	enum { ST_OK = 0, ST_EOF = 0x40, ST_ERROR = 0x80 };
	Uint32	state;
	Uint8	status;
	Uint32 received_cmd;
	Uint32	prev_cmd;
	Uint32	sec_addr;
	Uint32	prev_addr;
	Uint32 dev_nr;
	class CIECDevice *Device;
	Uint32 DispatchIECCmd(Uint8 cmd);
  public:
//	CFakeIEC() {}
    CFakeIEC(Uint32 dn) { dev_nr = dn; };
    virtual void Reset();
    virtual Uint32 Listen();
    virtual Uint32 Unlisten();
    virtual void Talk();
    virtual void Untalk();
    virtual Uint32	In(Uint8 *data);
    virtual Uint32 Out(Uint8 data);
    virtual Uint32 OutCmd(Uint8 data);
	virtual Uint32 OutSec(Uint8 data);    
    virtual Uint8 Status() { return status; };
    void AddIECDevice(CIECDevice *dev) { Device = dev; };
};

class CRealIEC : public CFakeIEC {
  public:
	CRealIEC();
	CRealIEC(Uint32 dn) : CFakeIEC(dn) { dev_nr = dn; };
	Uint32 Init();
	Uint32 RawRead(Uint32 sec_addr, Uint8 *data);
	Uint32 RawWrite(Uint32 sec_addr, Uint8 data);
    virtual void Reset();
    virtual Uint32 Listen();
    virtual Uint32 Unlisten();
    virtual void Talk();
    virtual void Untalk();
    virtual Uint32 In(Uint8 *data);
    virtual Uint32 Out(Uint8 data);
	virtual Uint32 OutCmd(Uint8 data);
	virtual Uint32 OutSec(Uint8 data);	
  protected:
	Uint8	cmd_buffer[255];
	int		cmd_len;
};

#endif // _IEC_H
