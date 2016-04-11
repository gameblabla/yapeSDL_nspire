#include "device.h"
#include "diskfs.h"
#include "drive.h"
#include "iec.h"
#include "tcbm.h"

class Drive {
public:
	Drive(unsigned int dn) : devNr(dn) {
	};
	virtual ~Drive() {	};
	virtual void Reset() = 0;
	virtual void AttachDisk(char *fname) = 0;
	virtual void DetachDisk() = 0;
	static void ChangeEmulation();
protected:
	unsigned int devNr;
	unsigned char *ram;
	unsigned char *rom;
};

class FakeDrive : public Drive {
public:
	FakeDrive();
	virtual ~FakeDrive() {	};
	virtual void Reset();
	virtual void AttachDisk(char *fname);
	virtual void DetachDisk();
private:
	CTCBM *tcbm;
	CIECInterface *iec;
	CIECDrive	*device;
};

FakeDrive::FakeDrive() : Drive(8)
{
	CFakeTCBM *tcbm_l = new CFakeTCBM();
	CFakeIEC *iec_l = new CFakeIEC(8);
	CIECFSDrive *fsdrive = new CIECFSDrive(".");

	iec = iec_l;
	tcbm = tcbm_l;
	tcbm_l->AddIECInterface((CIECInterface*)iec);
	iec_l->AddIECDevice((CIECDevice*)fsdrive);
	
	FakeDrive::Reset();
}

void FakeDrive::Reset()
{
	tcbm->Reset();
	iec->Reset();
	device->Reset();	
}

void FakeDrive::AttachDisk(char *fname)
{
	
}

void FakeDrive::DetachDisk()
{
	
}
