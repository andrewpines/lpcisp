
typedef struct
{
	// these are definitions which are set by the part table in partdesc.c
	unsigned int
		id[2];				// some parts have more than one ID.  set the second to ~0 if unused.
	const char
		*name;
	const int
		numSectors,
		*sectorSizeMap,
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
// b2:1 = expected line termination
// b3   = 1 if device remaps the first 64 bytes (vectors) in ISP mode
// b4   = 1 if device has a UID, 0 if not
 
#define UUENCODE	(1<<0)	// set if part expects data to be uuencoded and checksummed
#define TERM_ANY	(0<<1)	// default, no flags set
#define TERM_CR		(1<<1)
#define TERM_LF		(2<<1)
#define TERM_CRLF	(3<<1)
#define TERM_MASK	(3<<1)
#define VECT_REMAP	(1<<3)	// set true if device remaps the first 64 bytes (vectors) in ISP mode, thus making that section un-verifiable
#define HAS_UID		(1<<4)	// device has a UID, supports UID command

void ReportPartInfo(int level,partinfo_t *p);
int GetPartInfo(int fd,partinfo_t *p);
int GetSectorAddr(unsigned int addr, partinfo_t *p);
void DumpPartList(FILE *fp);

