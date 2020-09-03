enum
{
	ISP_ERR_NONE=0,
	ISP_ERR_BAUD,
	ISP_ERR_SYNC,
	ISP_ERR_COMM,
	ISP_ERR_INFO,
	ISP_ERR_UNKNOWN_DEV,	// read ID okay but don't recognize it
};

enum
{
	PIN_NONE=0,		// no mapping, do not use handshake lines
	PIN_DTR,		// map this control to DTR
	PIN_nDTR,		// map this control to DTR, inverted
	PIN_RTS,		// map this control to RTS
	PIN_nRTS,		// map this control to RTS, inverted
};

typedef struct
{
	unsigned int
		bank,		// bank number
		base,		// base address
		size;		// size in bytes
}sectormap_t;

typedef struct
{
	// these are definitions which are set by the part table in partdesc.c
	unsigned int
		id[2],				// some parts have more than one ID.  set the second to ~0 if unused.
		id1;				// some parts have a second ID word.  set to ~0 if unused.
	const char
		*name;
	const int
		numSectors;
	const sectormap_t
		*sectorMap;
	const int
		numBanks,
		ramSize,
		flashBlockSize,		// number of bytes to write at once.  depends on available RAM.
		flashBlockRAMBase;	// start address in RAM of buffer to use for writing to flash
	const unsigned char
		flags;
	// these are variables which are read from the target device and filled in
	unsigned int
		uid[4];
	unsigned char
		bootMajor,
		bootMinor;
	int
		idIdx;
}partinfo_t;

// b0   = 1 if data is uuencoded, 0 if not
// b2:1 = expected line termination (00=any, 01=CR, 10=LF, 11=CRLF)
// b3   = 1 if device has a UID, 0 if not
// b4   = 1 if device remaps the first 64 bytes (vectors) in ISP mode
// b5   = 1 if device remaps the first 256 bytes (vectors) in ISP mode
// b6   = 1 if device cannot read the first word of flash (LPC546xx, possibly others)

#define UUENCODE		(1<<0)	// set if part expects data to be uuencoded and checksummed
#define TERM_ANY		(0<<1)	// default, no flags set
#define TERM_CR			(1<<1)
#define TERM_LF			(2<<1)
#define TERM_CRLF		(3<<1)
#define TERM_MASK		(3<<1)
#define HAS_UID			(1<<3)	// device has a UID, supports UID command
#define VECT_REMAP64	(1<<4)	// set true if device remaps the first 64 bytes (vectors) in ISP mode, thus making that section un-verifiable
#define VECT_REMAP256	(1<<5)	// set true if device remaps the first 256 bytes (vectors) in ISP mode, thus making that section un-verifiable
#define SKIP_0			(1<<6)	// when reading skip first word of flash

int LPCISP_SetBaudRate(int fd, int baud, int stop);
int LPCISP_CopyRAMtoFlash(int fd, unsigned int src, unsigned int dest, unsigned int length);
int LPCISP_BlankCheck(int fd, int startSector, int endSector);
int LPCISP_GetSectorAddr(unsigned int addr, partinfo_t *p);
int LPCISP_ResetTarget(int fd);
void LPCISP_ExitISPMode(int fd);
unsigned int LPCISP_ReadPartID(int fd, unsigned int *id1);
int LPCISP_ReadPartUID(int fd, unsigned int *uid);
int LPCISP_ReadBootCodeVersion(int fd, unsigned char *major, unsigned char *minor);
int LPCISP_Erase(int fd, int startSector, int endSector, int bank, partinfo_t *partInfo);
int LPCISP_ReadFromTarget(int fd, unsigned char *data, unsigned int addr, unsigned int count,partinfo_t *partInfo);
int LPCISP_WriteToFlash(int fd,unsigned char *data,unsigned int addr,unsigned int length,partinfo_t *partInfo);
int LPCISP_Sync(int fd, int baud, int clock, int retries, int setecho, int hold, int reset, int isp, partinfo_t *partInfo);

// read hex file into buffer, return pointer to buffer or NULL if error.
// caller must free returned buffer.
//   fileName: name of hex file to load
//   start: pointer to start address (to be filled in)
//   length: pointer to length of image (to be filled in)
extern unsigned char *LPCISP_ReadHexFile(const char *fileName,int *start,int *length);

