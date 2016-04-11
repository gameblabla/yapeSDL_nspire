#ifndef _MEM_H
#define _MEM_H

#include "types.h"

// Abstract Memory handler superclass for all memory handler objects
class MemoryHandler {
public:
	virtual void Write(Uint32 addr, Uint8 data) = 0;
	virtual Uint8 Read(Uint32 addr) = 0;
	virtual void Reset() = 0;
};

#endif // _MEM_H
