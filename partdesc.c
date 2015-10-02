#include "includes.h"

// @@@ lots more part definitions need to go here
// @@@ may need to add other attributes to control differences in programming requirements

// RAM size is currently only used for reporting.
// block size is the unit which is transfered/written to flash (typically want as large as available RAM will allow)
// block address is the address in RAM used for the data block to write to flash (must avoid locations used by the bootloader)
// some devices have two IDs.  if the second is unused set to ~0.

// the amount of flash is indicated (mostly) by the base part number:
//  LPC1111:          8kB
//  LPC1112:          16kB
//  LPC1113:          24kB
//  LPC1114/LPC11D14: 32kB
//  LPC1114/323:      48kB
//  LPC1114/333:      56kB
//  LPC1115:          64kB
// the letter after LPC111x indicates the temperature range (F=-40 to 85C, J=-40 to 105C)
// the next two digits indicate the package (e.g., 33 indicates it's a 33 pad QFN package)
// the three digit code after the slash indicates the processor series and resources (see page 4 of the User Manual):
// 	first digit:
// 		1: 2k SRAM
// 		2: 4k SRAM
// 		3: 8k SRAM
// 	third digit:
// 		1: LPC100
// 		2: LPC100L (LPC1000 plus power profiles for lower power consumption)
// 		3: LPC100XL (LPC1000L plus IAP flash page erase, extra pinning, second SPI block)
// it's not clear what differentiates the HI33 parts from the HN33 parts.
//
// this table is incomplete and likely has errors.  please forward corrections and additions via this page:
//    http://www.cosmodog.com/contact/

static const int
	sectorMap1k[]=
	{
		// 1kB uniform sector map
		1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,
	},
	sectorMap4k[]=
	{
		// 1kB uniform sector map
		4096,4096,4096,4096,4096,4096,4096,4096,
		4096,4096,4096,4096,4096,4096,4096,4096,
	},
	sectorMapLpc17xx[]=
	{
		// LPC17xx non-uniform sector map
		4096,4096,4096,4096,4096,4096,4096,4096,
		4096,4096,4096,4096,4096,4096,4096,4096,
		32768,32768,32768,32768,32768,32768,32768,32768,
		32768,32768,32768,32768,32768,32768,32768,32768,
	};


const partinfo_t
	partDef[]=
	{
		// the following are from UM10601, LPC800 User manual, Rev. 1.3, 22 July 2013.
		//																									# of										block	block RAM
		//		id			alt. ID			name															sectors							ram			size	address		flags
		{	{ 0x00008100,	~0			},	"LPC810M021FN8",												4,			sectorMap1k,		1024,		256,	0x10000300,	0			},
		{	{ 0x00008110,	~0			},	"LPC811M001JDH16",												8,			sectorMap1k,		2048,		1024,	0x10000300,	0			},
		{	{ 0x00008120,	~0			},	"LPC812M101JDH16",												16,			sectorMap1k,		4096,		1024,	0x10000300,	0			},
		{	{ 0x00008121,	~0			},	"LPC812M101JD20",												16,			sectorMap1k,		4096,		1024,	0x10000300,	0			},
		{	{ 0x00008122,	~0			},	"LPC812M101JDH20",												16,			sectorMap1k,		4096,		1024,	0x10000300,	0			},

		// the following are from UM10398, LPC111x/LPC11Cxx User manual, Rev. 12.1, August 2013.
		//																									# of										block	block RAM
		//		id			alt. ID			name															sectors							ram			size	address
		{	{ 0x0a07102b,	0x1a07102b	},	"LPC1111FD20",													2,			sectorMap4k,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0a16d02b,	0x1a16d02b	},	"LPC1111FDH20/002",												2,			sectorMap4k,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x041e502b,	~0			},	"LPC1111FHN33/101",												2,			sectorMap4k,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2516d02b,	~0			},	"LPC1111FHN33/[101|102]",										2,			sectorMap4k,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0416502b,	~0			},	"LPC1111FHN33/201",												2,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2516902b,	~0			},	"LPC1111FHN33/[201|202]",										2,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00010013,	~0			},	"LPC1111FHN33/103",												2,			sectorMap4k,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00010012,	~0			},	"LPC1111FHN33/203",												2,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0a24902b,	0x1a24902b	},	"LPC1112[FD20|FDH20|FDH28]/102",								4,			sectorMap4k,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x042d502b,	~0			},	"LPC1112FHN33/101",												4,			sectorMap4k,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2524d02b,	~0			},	"LPC1112FHN33/[101|102]",										4,			sectorMap4k,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0425502b,	~0			},	"LPC1112FHN33/201",												4,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2524902b,	~0			},	"LPC1112[FHN33/201|FHN33/202|FHN24/202|FHI33/202]",				4,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00020023,	~0			},	"LPC1112FHN33/103",												4,			sectorMap4k,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00020022,	~0			},	"LPC1112FH[N|I]33/203",											4,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0434502b,	~0			},	"LPC1113FHN33/201",												6,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2532902b,	~0			},	"LPC1113[FBD|FHN]/[201|202]",									6,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0434102b,	~0			},	"LPC1113FHN33/301",												6,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2532102b,	~0			},	"LPC1113[FBD48|FHN33]/[301/302]",								6,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00030030,	~0			},	"LPC1113[FBD48|FHN33]/303",										6,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00030032,	~0			},	"LPC1113FHN33/203",												6,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0a40902b,	0x1a40902b	},	"LPC1114[FDH|FN]28/102",										8,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0444502b,	~0			},	"LPC1114FHN33/201",												8,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2540902b,	~0			},	"LPC1114FHN33/[201|202]",										8,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0444102b,	~0			},	"LPC1114[FHN33|FBD48]/301",										8,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2540102b,	~0			},	"LPC1114F[FHN33/301|FHN33/302|FHI33/302|FBD48/302|FBD100/302]",	8,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040040,	~0			},	"LPC1114[FBD48|FHN33]/303",										8,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040042,	~0			},	"LPC1114FHN33/203",												8,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040060,	~0			},	"LPC1114FBD48/323",												12,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040070,	~0			},	"LPC1114[FBD48|FHN33]/333",										14,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040040,	~0			},	"LPC1114FHI33/303",												8,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00050080,	~0			},	"LPC1115FBD48/303",												16,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x1421102b,	~0			},	"LPC11C12FBD48/301",											4,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x1440102b,	~0			},	"LPC11C14FBD48/301",											8,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x1431102b,	~0			},	"LPC11C22FBD48/301",											4,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x1430102b,	~0			},	"LPC11C24FBD48/301",											8,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},

		// the following are from UM10375, LPC1311/13/42/43 User manual, Rev. 5 -- 21 June 2012.
		//																									# of										block	block RAM
		//		id			alt. ID			name															sectors							ram			size	address
		{	{0x2C42502B,	~0			},	"LPC1311FHN33",													2,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{0x1816902B,	~0			},	"LPC1311FHN33/01",												2,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{0x2C40102B,	~0			},	"LPC1313F[HN33|BD48]",											8,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{0x1830102B,	~0			},	"LPC1313F[HN33|BD48]/01",										8,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{0x3D01402B,	~0			},	"LPC1342F[HN33|BD48]",											4,			sectorMap4k,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{0x3D00002B,	~0			},	"LPC1343F[HN33|BD48]",											8,			sectorMap4k,		8192,		1024,	0x10000300,	UUENCODE	},

		// the following are from UM10360, LPC176x/5x User manual, Rev. 3.1 -- 2 April 2014.
		//																									# of										block	block RAM
		//		id			alt. ID			name															sectors							ram			size	address
		{	{0x25001118,	0x25001110	},	"LPC1751FBD80",													8,			sectorMap4k,		8192,		1024,	0x10000300,	0			},
		{	{0x25001121,	~0			},	"LPC1752FBD80",													16,			sectorMap4k,		16384,		1024,	0x10000300,	0			},

		// the following are from UM10470, LPC178x/7x User manual, Rev. 3.1 -- 15 September 2014.
		//		id			alt. ID			name															sectors							main ram	size	address
		{	{0x27011132,	~0			},	"LPC1774",														18,			sectorMapLpc17xx,	32*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 128kB flash
		{	{0x27191F43,	~0			},	"LPC1776",														22,			sectorMapLpc17xx,	64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 256kB flash
		{	{0x27193747,	~0			},	"LPC1777",														30,			sectorMapLpc17xx,	64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 512kB flash
		{	{0x27193F47,	~0			},	"LPC1778",														30,			sectorMapLpc17xx,	64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 512kB flash
		{	{0x281D1743,	~0			},	"LPC1785",														22,			sectorMapLpc17xx,	64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 256kB flash
		{	{0x281D1F43,	~0			},	"LPC1786",														22,			sectorMapLpc17xx,	64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 256kB flash
		{	{0x281D3747,	~0			},	"LPC1787",														30,			sectorMapLpc17xx,	64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 512kB flash
		{	{0x281D3F47,	~0			},	"LPC1788",														30,			sectorMapLpc17xx,	64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 512kB flash
	},
	partDefUnknown=
	{
		.id				= {0,0},
		.name			= "unknown",
		.numSectors		= 0,
		.sectorSizeMap	= (int *)NULL,
		.ramSize		= 0,
	};

void DumpPartList(FILE *fp)
{
	int
		i;

	fprintf(fp,"supported parts:\n");
	for(i=0;i<ARRAY_SIZE(partDef);i++)
	{
		fprintf(fp,"  %s\n",partDef[i].name);
	}
}

int GetSectorAddr(unsigned int addr, partinfo_t *p)
// given an address locate the number of the sector containing 
// that address or -1 if out of range (address off end of device)
// or sector map undefined.
{
	int
		curAddr,
		sectorNum;
	
	if(p->sectorSizeMap)
	{
		curAddr=0;
		for(sectorNum=0;sectorNum<p->numSectors;sectorNum++)
		{
			curAddr+=p->sectorSizeMap[sectorNum];
			if(addr<curAddr)
			{
				return(sectorNum);
			}
		}
	}
	return(-1);	
}

static unsigned int GetFlashSize(partinfo_t *p)
// return size of flash (add up size of all sectors) or zero
// if sector size map is undefined.
{
	unsigned int
		i,
		size;
	
	size=0;
	if(p->sectorSizeMap)
	{
		for(i=0;i<p->numSectors;i++)
		{
			size+=p->sectorSizeMap[i];
		}
	}
	return(size);
}

void ReportPartInfo(int level,partinfo_t *p)
// report information about specifed part
{
	int
		i;
	if(p)
	{
		ReportString(level,"device information:\n");
		ReportString(level,"   part number:  %s\n",p->name);
		ReportString(level,"   ID:           0x%08x\n",p->id[p->idIdx]);
		ReportString(level,"   flash size:   %d bytes\n",GetFlashSize(p));
		ReportString(level,"   SRAM size:    %d bytes\n",p->ramSize);
		ReportString(level,"   UID:          ");
		for(i=0;i<4;i++)
		{
			ReportString(level,"%08x ",p->uid[i]);
		}
		ReportString(level,"\n");
		ReportString(level,"   boot version: %d.%d\n",p->bootMajor,p->bootMinor);
	}
}

int GetPartInfo(int fd,partinfo_t *p)
// get part information from the target (if possible), fill out passed structure.
// if part ID can't be matched still return the part ID, UID, and boot code version.
{
	int
		i,
		j,
		found;
	unsigned int
		id;

	found=0;
	memcpy(p,&partDefUnknown,sizeof(partinfo_t));
	id=ReadPartID(fd);
	p->id[0]=id;
	p->idIdx=0;
	if(id!=~0)
	{
		for(i=0;!found&&(i<ARRAY_SIZE(partDef));i++)
		{
			for(j=0;!found&&(j<2);j++)
			{
				if(partDef[i].id[j]==id)
				{
					memcpy(p,&partDef[i],sizeof(partinfo_t));
					p->idIdx=j;
					found=1;
				}
			}
		}
		ReadPartUID(fd,p->uid);
		ReadBootCode(fd,&p->bootMajor,&p->bootMinor);
		return(0);
	}
	return(-1);
}
