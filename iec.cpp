#include "iec.h"

void CFakeIEC::Reset()
{
    state = STATE_IDLE;
	status = ST_OK;
}

Uint32 CFakeIEC::Listen()
{ 
	state = STATE_LISTENING;
	return ST_OK;
};

Uint32 CFakeIEC::Unlisten()
{ 
    if (state & STATE_LISTENING) {
        state = STATE_IDLE;
        if (prev_cmd == CMD_OPEN) {
			status = Device->Write(sec_addr, 0, CMD_OPEN, true);
            status = Device->Open(sec_addr);
        } else if (prev_cmd == CMD_DATA) {
            status = Device->Write(sec_addr, 0, CMD_DATA, true);
        }    
    } else
    	status = ST_OK;
	return status;
}

void CFakeIEC::Talk()
{ 
	state = STATE_TALKING;
}

void CFakeIEC::Untalk()
{ 
	state = STATE_IDLE;
}

Uint32 CFakeIEC::In(Uint8 *data)
{
	if ((state&STATE_TALKING) && (received_cmd == CMD_DATA))
		return Device->Read(sec_addr, data);

	return ST_ERROR;   
}

Uint32 CFakeIEC::DispatchIECCmd(Uint8 cmd)
{
	switch (cmd&0xF0) {
		case CMD_LISTEN:
			return Listen();
		case CMD_UNLISTEN:
			return Unlisten();
		case CMD_TALK:
			Talk();
			return ST_OK;
		case CMD_UNTALK:
			Untalk();
			return ST_OK;
		default: // illegal command
		    return ST_ERROR;
	}
}
    
Uint32 CFakeIEC::OutCmd(Uint8 data)
{
	prev_cmd = received_cmd;
	received_cmd = data&0xF0;
	return status = DispatchIECCmd(data);
}

Uint32 CFakeIEC::Out(Uint8 data)
{
	if ((state&STATE_LISTENING) /*&& (received_cmd == CMD_DATA)*/) {
		status = Device->Write( sec_addr, data, received_cmd, false);
		return status;
	}

	return ST_ERROR;  
}

Uint32 CFakeIEC::OutSec(Uint8 data)
{
	prev_addr = sec_addr;
    sec_addr = data&0x0F;

	prev_cmd = received_cmd;
	received_cmd = data&0xF0;

    switch (state) {
        case STATE_IDLE:
			status = ST_ERROR;
            break;        
        case STATE_LISTENING:
			switch (received_cmd) {
				case CMD_OPEN:	// Prepare for receiving the file name
					status = ST_OK;
					break;

				case CMD_CLOSE: // Close channel
					status = Device->Close( sec_addr);
					break;
				
				case CMD_DATA: // Data comes
					break;
				    
				default:
				    status = ST_ERROR;
				    state = STATE_IDLE;
				    break;
			}
            break;

        case STATE_TALKING:
            break;
    }
    return status;
}


