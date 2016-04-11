#include "serial.h"

unsigned char CSerial::Line[16];
class CSerial *CSerial::Devices[16];
unsigned int CSerial::NrOfDevicesAttached;
CSerial *CSerial::RootDevice = 0;
CSerial *CSerial::LastDevice = 0;

CSerial::CSerial()
{
}

void CSerial::InitPorts()
{
	for (int i=0; i<16; i++)
		Line[i] = 0xC0;
};

CSerial::CSerial(unsigned int DevNr) : DeviceNr(DevNr)
{
	NrOfDevicesAttached++;
	
	if (RootDevice == 0) {
	    PrevDevice = 0;
	    NextDevice = 0;
	    for (int i=0; i<4; i++)
	    	Devices[i] = 0;
	    RootDevice = this;
	    LastDevice = this;
	} else {
	    PrevDevice = LastDevice;
	    LastDevice->NextDevice = this;
	    LastDevice = this;
	    NextDevice = 0;
	}
	
	Devices[DevNr] = this;
	
	sprintf( Name, "Device #%u", DevNr);
	//cerr << Name << " with address " << this << " created." << endl;
}

CSerial::~CSerial()
{
    //cout << "Deleting device #" << DeviceNr << endl;
	if (!--NrOfDevicesAttached) {
	    RootDevice = 0;
	    LastDevice = 0;
 	} else if (Devices[DeviceNr]->PrevDevice) {
		Devices[DeviceNr]->PrevDevice->NextDevice = Devices[DeviceNr]->NextDevice;
		if ( this == LastDevice) {
		    LastDevice = PrevDevice;
		}    
 	}    		
	Devices[DeviceNr] = NULL;
}

Uint8 CSerial::ReadBus()
{
    return
        Line[0]
        &Line[4]&Line[5] // printers
        &Line[8]&Line[9]&Line[10]&Line[11]; // drives
}

