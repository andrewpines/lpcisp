
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

#define UUENCODE	(1<<0)	// set if part expects data to be uuencoded and checksummed
#define TERM_CRLF	(0<<1)	// default, no flags set
#define TERM_CR		(1<<1)
#define TERM_LF		(2<<1)
#define TERM_MASK	(3<<1)

void ReportPartInfo(int level,partinfo_t *p);
int GetPartInfo(int fd,partinfo_t *p);
int GetSectorAddr(unsigned int addr, partinfo_t *p);
void DumpPartList(FILE *fp);

