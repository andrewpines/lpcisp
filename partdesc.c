#include "includes.h"

// @@@ lots more part definitions need to go here
// @@@ may need to add other attributes to control differences in programming requirements

// RAM size is currently only used for reporting.
// block size is the unit which is transfered/written to flash (typically want as large as available RAM will allow)
// block address is the address in RAM used for the data block to write to flash (must avoid locations used by the bootloader)
// some devices have two IDs.  if the second is unused set to ~0.
//
// some devices use uuencoded ASCII to transfer binary data while others transfer binary data directly.  Transfering binary data
// as binary is problematic because the command protocol is ASCII and it needs to switch between ASCII and binary.  This creates
// issues at the edges as it changes modes because line termination is not handled consistently across all parts.  When sending or
// receiving binary data the presence/absence/ambiguity of the second character of line termination makes it unclear whether
// that second character is the second part of the termination or the first binary character.  Historically, different OS's use
// different termination:
//  CR -- pre-OSX Mac
//  LF -- Unix
//  CRLF -- Windows
// Without getting into a holy war over which is optimal, the reality is that both CRLF and LF exist in the world and both should
// be supported.  ideally NXP would have engineered all parts to expect LF as the terminator and would ignore any CRs, since even
// Apple has abandonded CR-only line termination.  this would mean that CRLF or LF would work equally well, plus switching to
// binary would always work as expected because it would skip over any CRs and terminate on the LF.  No such luck.  inspection
// reveals that they couldn't even manage to be consistent within a single part.  Here are some examples of tested components:
//   LPC1788: consistently accepts CR, LF, or CRLF; returns either CRLF or echos the termination that was sent to it (context-dependent)
//   LPC1313: accepts CRLF; sometimes returns CR (after 'Synchronized'), other times returns CRLF
//
//  to handle this consistently an extra flag is needed to indicate what form of termination to use for a given part.
//
// this table is incomplete and likely has errors.  please forward corrections and additions via this page:
//    https://github.com/andrewpines/lpcisp/issues


static const sectormap_t
	sectorMap1k[]=
	{
		// 1kB uniform sector map
		//	bank	base		size
		{	0,		0x00000000,	0x400,	},
		{	0,		0x00000400,	0x400,	},
		{	0,		0x00000800,	0x400,	},
		{	0,		0x00000c00,	0x400,	},
		{	0,		0x00001000,	0x400,	},
		{	0,		0x00001400,	0x400,	},
		{	0,		0x00001800,	0x400,	},
		{	0,		0x00001c00,	0x400,	},
		{	0,		0x00002000,	0x400,	},
		{	0,		0x00002400,	0x400,	},
		{	0,		0x00002800,	0x400,	},
		{	0,		0x00002c00,	0x400,	},
		{	0,		0x00003000,	0x400,	},
		{	0,		0x00003400,	0x400,	},
		{	0,		0x00003800,	0x400,	},
		{	0,		0x00003c00,	0x400,	},
		{	0,		0x00004000,	0x400,	},
		{	0,		0x00004400,	0x400,	},
		{	0,		0x00004800,	0x400,	},
		{	0,		0x00004c00,	0x400,	},
		{	0,		0x00005000,	0x400,	},
		{	0,		0x00005400,	0x400,	},
		{	0,		0x00005800,	0x400,	},
		{	0,		0x00005c00,	0x400,	},
		{	0,		0x00006000,	0x400,	},
		{	0,		0x00006400,	0x400,	},
		{	0,		0x00006800,	0x400,	},
		{	0,		0x00006c00,	0x400,	},
		{	0,		0x00007000,	0x400,	},
		{	0,		0x00007400,	0x400,	},
		{	0,		0x00007800,	0x400,	},
		{	0,		0x00007c00,	0x400,	},
	},
	sectorMap4k[]=
	{
		// 4kB uniform sector map
		//	bank	base		size
		{	0,		0x00000000,	0x1000,	},
		{	0,		0x00001000,	0x1000,	},
		{	0,		0x00002000,	0x1000,	},
		{	0,		0x00003000,	0x1000,	},
		{	0,		0x00004000,	0x1000,	},
		{	0,		0x00005000,	0x1000,	},
		{	0,		0x00006000,	0x1000,	},
		{	0,		0x00007000,	0x1000,	},
		{	0,		0x00008000,	0x1000,	},
		{	0,		0x00009000,	0x1000,	},
		{	0,		0x0000a000,	0x1000,	},
		{	0,		0x0000b000,	0x1000,	},
		{	0,		0x0000c000,	0x1000,	},
		{	0,		0x0000d000,	0x1000,	},
		{	0,		0x0000e000,	0x1000,	},
		{	0,		0x0000f000,	0x1000,	},

		{	0,		0x00010000,	0x1000,	},
		{	0,		0x00011000,	0x1000,	},
		{	0,		0x00012000,	0x1000,	},
		{	0,		0x00013000,	0x1000,	},
		{	0,		0x00014000,	0x1000,	},
		{	0,		0x00015000,	0x1000,	},
		{	0,		0x00016000,	0x1000,	},
		{	0,		0x00017000,	0x1000,	},
		{	0,		0x00018000,	0x1000,	},
		{	0,		0x00019000,	0x1000,	},
		{	0,		0x0001a000,	0x1000,	},
		{	0,		0x0001b000,	0x1000,	},
		{	0,		0x0001c000,	0x1000,	},
		{	0,		0x0001d000,	0x1000,	},
		{	0,		0x0001e000,	0x1000,	},
		{	0,		0x0001f000,	0x1000,	},

		{	0,		0x00020000,	0x1000,	},
		{	0,		0x00021000,	0x1000,	},
		{	0,		0x00022000,	0x1000,	},
		{	0,		0x00023000,	0x1000,	},
		{	0,		0x00024000,	0x1000,	},
		{	0,		0x00025000,	0x1000,	},
		{	0,		0x00026000,	0x1000,	},
		{	0,		0x00027000,	0x1000,	},
		{	0,		0x00028000,	0x1000,	},
		{	0,		0x00029000,	0x1000,	},
		{	0,		0x0002a000,	0x1000,	},
		{	0,		0x0002b000,	0x1000,	},
		{	0,		0x0002c000,	0x1000,	},
		{	0,		0x0002d000,	0x1000,	},
		{	0,		0x0002e000,	0x1000,	},
		{	0,		0x0002f000,	0x1000,	},

		{	0,		0x00030000,	0x1000,	},
		{	0,		0x00031000,	0x1000,	},
		{	0,		0x00032000,	0x1000,	},
		{	0,		0x00033000,	0x1000,	},
		{	0,		0x00034000,	0x1000,	},
		{	0,		0x00035000,	0x1000,	},
		{	0,		0x00036000,	0x1000,	},
		{	0,		0x00037000,	0x1000,	},
		{	0,		0x00038000,	0x1000,	},
		{	0,		0x00039000,	0x1000,	},
		{	0,		0x0003a000,	0x1000,	},
		{	0,		0x0003b000,	0x1000,	},
		{	0,		0x0003c000,	0x1000,	},
		{	0,		0x0003d000,	0x1000,	},
		{	0,		0x0003e000,	0x1000,	},
		{	0,		0x0003f000,	0x1000,	},
	},
	sectorMap8k[]=
	{
		// 8kB uniform sector map
		//	bank	base		size
		{	0,		0x00000000,	0x2000,	},
		{	0,		0x00002000,	0x2000,	},
		{	0,		0x00004000,	0x2000,	},
		{	0,		0x00006000,	0x2000,	},
		{	0,		0x00008000,	0x2000,	},
		{	0,		0x0000a000,	0x2000,	},
		{	0,		0x0000c000,	0x2000,	},
		{	0,		0x0000e000,	0x2000,	},
		{	0,		0x00010000,	0x2000,	},
		{	0,		0x00012000,	0x2000,	},
		{	0,		0x00014000,	0x2000,	},
		{	0,		0x00016000,	0x2000,	},
		{	0,		0x00018000,	0x2000,	},
		{	0,		0x0001a000,	0x2000,	},
		{	0,		0x0001c000,	0x2000,	},
		{	0,		0x0001e000,	0x2000,	},
	},
	sectorMap32k[]=
	{
		// 32kB uniform sector map
		//	bank	base		size
		{	0,		0x00000000,	0x8000,	},
		{	0,		0x00008000,	0x8000,	},
		{	0,		0x00010000,	0x8000,	},
		{	0,		0x00018000,	0x8000,	},
		{	0,		0x00020000,	0x8000,	},
		{	0,		0x00028000,	0x8000,	},
		{	0,		0x00030000,	0x8000,	},
		{	0,		0x00038000,	0x8000,	},
		{	0,		0x00040000,	0x8000,	},
		{	0,		0x00048000,	0x8000,	},
		{	0,		0x00050000,	0x8000,	},
		{	0,		0x00058000,	0x8000,	},
		{	0,		0x00060000,	0x8000,	},
		{	0,		0x00068000,	0x8000,	},
		{	0,		0x00070000,	0x8000,	},
		{	0,		0x00078000,	0x8000,	},
	},
	sectorMapLpc17xx[]=
	{
		// LPC17xx non-uniform sector map
		{	0,		0x00000000,	0x1000,	},
		{	0,		0x00001000,	0x1000,	},
		{	0,		0x00002000,	0x1000,	},
		{	0,		0x00003000,	0x1000,	},
		{	0,		0x00004000,	0x1000,	},
		{	0,		0x00005000,	0x1000,	},
		{	0,		0x00006000,	0x1000,	},
		{	0,		0x00007000,	0x1000,	},
		{	0,		0x00008000,	0x1000,	},
		{	0,		0x00009000,	0x1000,	},
		{	0,		0x0000a000,	0x1000,	},
		{	0,		0x0000b000,	0x1000,	},
		{	0,		0x0000c000,	0x1000,	},
		{	0,		0x0000d000,	0x1000,	},
		{	0,		0x0000e000,	0x1000,	},
		{	0,		0x0000f000,	0x1000,	},
		{	0,		0x00010000,	0x8000,	},
		{	0,		0x00018000,	0x8000,	},
		{	0,		0x00020000,	0x8000,	},
		{	0,		0x00028000,	0x8000,	},
		{	0,		0x00030000,	0x8000,	},
		{	0,		0x00038000,	0x8000,	},
		{	0,		0x00040000,	0x8000,	},
		{	0,		0x00048000,	0x8000,	},
		{	0,		0x00050000,	0x8000,	},
		{	0,		0x00058000,	0x8000,	},
		{	0,		0x00060000,	0x8000,	},
		{	0,		0x00068000,	0x8000,	},
		{	0,		0x00070000,	0x8000,	},
		{	0,		0x00078000,	0x8000,	},
		{	0,		0x00080000,	0x8000,	},
		{	0,		0x00088000,	0x8000,	},
	},
	sectorMapLpc18x2[]=
	{
		// LPC18xx non-uniform sector map
		{	0,		0x1a000000,	0x02000,	},		// 0-7 are 8kB
		{	0,		0x1a002000,	0x02000,	},
		{	0,		0x1a004000,	0x02000,	},
		{	0,		0x1a006000,	0x02000,	},
		{	0,		0x1a008000,	0x02000,	},
		{	0,		0x1a00a000,	0x02000,	},
		{	0,		0x1a00c000,	0x02000,	},
		{	0,		0x1a00e000,	0x02000,	},

		{	0,		0x1a010000,	0x10000,	},		// 8-14 are 64kB
		{	0,		0x1a020000,	0x10000,	},
		{	0,		0x1a030000,	0x10000,	},
		{	0,		0x1a040000,	0x10000,	},
		{	0,		0x1a050000,	0x10000,	},
		{	0,		0x1a060000,	0x10000,	},
		{	0,		0x1a070000,	0x10000,	},
	},
	sectorMapLpc18x3[]=
	{
		// LPC18xx non-uniform sector map
		// bank A
		{	0,		0x1a000000,	0x02000,	},		// 0-7 are 8kB
		{	0,		0x1a002000,	0x02000,	},
		{	0,		0x1a004000,	0x02000,	},
		{	0,		0x1a006000,	0x02000,	},
		{	0,		0x1a008000,	0x02000,	},
		{	0,		0x1a00a000,	0x02000,	},
		{	0,		0x1a00c000,	0x02000,	},
		{	0,		0x1a00e000,	0x02000,	},

		{	0,		0x1a010000,	0x10000,	},		// 8-10 are 64kB
		{	0,		0x1a020000,	0x10000,	},
		{	0,		0x1a030000,	0x10000,	},

		// bank B
		{	1,		0x1b000000,	0x02000,	},		// 0-7 are 8kB
		{	1,		0x1b002000,	0x02000,	},
		{	1,		0x1b004000,	0x02000,	},
		{	1,		0x1b006000,	0x02000,	},
		{	1,		0x1b008000,	0x02000,	},
		{	1,		0x1b00a000,	0x02000,	},
		{	1,		0x1b00c000,	0x02000,	},
		{	1,		0x1b00e000,	0x02000,	},

		{	1,		0x1b010000,	0x10000,	},		// 8-10 are 64kB
		{	1,		0x1b020000,	0x10000,	},
		{	1,		0x1b030000,	0x10000,	},
	},
	sectorMapLpc18x5[]=
	{
		// LPC18xx non-uniform sector map
		// bank A
		{	0,		0x1a000000,	0x02000,	},		// 0-7 are 8kB
		{	0,		0x1a002000,	0x02000,	},
		{	0,		0x1a004000,	0x02000,	},
		{	0,		0x1a006000,	0x02000,	},
		{	0,		0x1a008000,	0x02000,	},
		{	0,		0x1a00a000,	0x02000,	},
		{	0,		0x1a00c000,	0x02000,	},
		{	0,		0x1a00e000,	0x02000,	},

		{	0,		0x1a010000,	0x10000,	},		// 8-12 are 64kB
		{	0,		0x1a020000,	0x10000,	},
		{	0,		0x1a030000,	0x10000,	},
		{	0,		0x1a040000,	0x10000,	},
		{	0,		0x1a050000,	0x10000,	},

		// bank B
		{	1,		0x1b000000,	0x02000,	},		// 0-7 are 8kB
		{	1,		0x1b002000,	0x02000,	},
		{	1,		0x1b004000,	0x02000,	},
		{	1,		0x1b006000,	0x02000,	},
		{	1,		0x1b008000,	0x02000,	},
		{	1,		0x1b00a000,	0x02000,	},
		{	1,		0x1b00c000,	0x02000,	},
		{	1,		0x1b00e000,	0x02000,	},

		{	1,		0x1b010000,	0x10000,	},		// 8-12 are 64kB
		{	1,		0x1b020000,	0x10000,	},
		{	1,		0x1b030000,	0x10000,	},
		{	0,		0x1b040000,	0x10000,	},
		{	0,		0x1b050000,	0x10000,	},
	},
	sectorMapLpc18x7[]=
	{
		// LPC18xx non-uniform sector map
		// bank A
		{	0,		0x1a000000,	0x02000,	},		// 0-7 are 8kB
		{	0,		0x1a002000,	0x02000,	},
		{	0,		0x1a004000,	0x02000,	},
		{	0,		0x1a006000,	0x02000,	},
		{	0,		0x1a008000,	0x02000,	},
		{	0,		0x1a00a000,	0x02000,	},
		{	0,		0x1a00c000,	0x02000,	},
		{	0,		0x1a00e000,	0x02000,	},

		{	0,		0x1a010000,	0x10000,	},		// 8-14 are 64kB
		{	0,		0x1a020000,	0x10000,	},
		{	0,		0x1a030000,	0x10000,	},
		{	0,		0x1a040000,	0x10000,	},
		{	0,		0x1a050000,	0x10000,	},
		{	0,		0x1a060000,	0x10000,	},
		{	0,		0x1a070000,	0x10000,	},

		// bank B
		{	1,		0x1b000000,	0x02000,	},		// 0-7 are 8kB
		{	1,		0x1b002000,	0x02000,	},
		{	1,		0x1b004000,	0x02000,	},
		{	1,		0x1b006000,	0x02000,	},
		{	1,		0x1b008000,	0x02000,	},
		{	1,		0x1b00a000,	0x02000,	},
		{	1,		0x1b00c000,	0x02000,	},
		{	1,		0x1b00e000,	0x02000,	},

		{	1,		0x1b010000,	0x10000,	},		// 8-14 are 64kB
		{	1,		0x1b020000,	0x10000,	},
		{	1,		0x1b030000,	0x10000,	},
		{	1,		0x1b040000,	0x10000,	},
		{	1,		0x1b050000,	0x10000,	},
		{	1,		0x1b060000,	0x10000,	},
		{	1,		0x1b070000,	0x10000,	},
	},
	sectorMapLpc21xx[]=
	{
		// LPC21xx/LPC22xx non-uniform sector map
		//	bank	base		size
		{	0,		0x00000000,	0x2000,		},		// 0-7 are 8kB
		{	0,		0x00002000,	0x2000,		},
		{	0,		0x00004000,	0x2000,		},
		{	0,		0x00006000,	0x2000,		},
		{	0,		0x00008000,	0x2000,		},
		{	0,		0x0000a000,	0x2000,		},
		{	0,		0x0000c000,	0x2000,		},
		{	0,		0x0000e000,	0x2000,		},
		{	0,		0x00010000,	0x10000,	},		// 8 and 9 are 64kB
		{	0,		0x00020000,	0x10000,	},
		{	0,		0x00030000,	0x2000,		},		// 10-17 are 8kB
		{	0,		0x00032000,	0x2000,		},
		{	0,		0x00034000,	0x2000,		},
		{	0,		0x00036000,	0x2000,		},
		{	0,		0x00038000,	0x2000,		},
		{	0,		0x0003a000,	0x2000,		},
		{	0,		0x0003c000,	0x2000,		},
		{	0,		0x0003e000,	0x2000,		},
	},
	sectorMapLpc23xx[]=
	{
		// LPC23xx non-uniform sector map
		{	0,		0x00000000,	0x01000,	},		// 0-7 are 4kB
		{	0,		0x00001000,	0x01000,	},
		{	0,		0x00002000,	0x01000,	},
		{	0,		0x00003000,	0x01000,	},
		{	0,		0x00004000,	0x01000,	},
		{	0,		0x00005000,	0x01000,	},
		{	0,		0x00006000,	0x01000,	},
		{	0,		0x00007000,	0x01000,	},

		{	0,		0x00008000,	0x08000,	},		// 8-21 are 32kB
		{	0,		0x00010000,	0x08000,	},
		{	0,		0x00018000,	0x08000,	},
		{	0,		0x00020000,	0x08000,	},
		{	0,		0x00028000,	0x08000,	},
		{	0,		0x00030000,	0x08000,	},
		{	0,		0x00038000,	0x08000,	},
		{	0,		0x00040000,	0x08000,	},
		{	0,		0x00048000,	0x08000,	},
		{	0,		0x00050000,	0x08000,	},
		{	0,		0x00058000,	0x08000,	},
		{	0,		0x00060000,	0x08000,	},
		{	0,		0x00068000,	0x08000,	},
		{	0,		0x00070000,	0x08000,	},

		{	0,		0x00078000,	0x01000,	},		// 22-27 are 4kB
		{	0,		0x00079000,	0x01000,	},
		{	0,		0x0007a000,	0x01000,	},
		{	0,		0x0007b000,	0x01000,	},
		{	0,		0x0007c000,	0x01000,	},
		{	0,		0x0007d000,	0x01000,	},
	};

const partinfo_t
	partDef[]=
	{
		// the following are from UM10601, LPC800 User manual, Rev. 1.3, 22 July 2013.
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	ram			size	address		flags
		{	{ 0x00008100,	~0			},	~0,	"LPC810M021FN8",												4,			sectorMap1k,		1,		1024,		256,	0x10000300,	TERM_CRLF	},
		{	{ 0x00008110,	~0			},	~0,	"LPC811M001JDH16",												8,			sectorMap1k,		1,		2048,		1024,	0x10000300,	TERM_CRLF	},
		{	{ 0x00008120,	~0			},	~0,	"LPC812M101JDH16",												16,			sectorMap1k,		1,		4096,		1024,	0x10000300,	TERM_CRLF	},
		{	{ 0x00008121,	~0			},	~0,	"LPC812M101JD20",												16,			sectorMap1k,		1,		4096,		1024,	0x10000300,	TERM_CRLF	},
		{	{ 0x00008122,	~0			},	~0,	"LPC812M101JDH20",												16,			sectorMap1k,		1,		4096,		1024,	0x10000300,	TERM_CRLF	},

		// the following are from UM10800, LPC82x User manual, Rev. 1.2, 5 October 2016.
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	ram			size	address		flags
		{	{ 0x00008221,	~0			},	~0,	"LPC822M101JHI33",												16,			sectorMap1k,		1,		4*1024,		1024,	0x10000300,	TERM_CRLF	},
		{	{ 0x00008222,	~0			},	~0,	"LPC822M101JDH20",												16,			sectorMap1k,		1,		4*1024,		1024,	0x10000300,	TERM_CRLF	},
		{	{ 0x00008241,	~0			},	~0,	"LPC824M201JHI33",												32,			sectorMap1k,		1,		8*1024,		1024,	0x10000300,	TERM_CRLF	},
		{	{ 0x00008242,	~0			},	~0,	"LPC824M201JDH20",												32,			sectorMap1k,		1,		8*1024,		1024,	0x10000300,	TERM_CRLF	},

		// the following are from UM10398, LPC111x/LPC11Cxx User manual, Rev. 12.1, August 2013.
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	ram			size	address
		{	{ 0x0a07102b,	0x1a07102b	},	~0,	"LPC1111FD20",													2,			sectorMap4k,		1,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0a16d02b,	0x1a16d02b	},	~0,	"LPC1111FDH20/002",												2,			sectorMap4k,		1,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x041e502b,	~0			},	~0,	"LPC1111FHN33/101",												2,			sectorMap4k,		1,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2516d02b,	~0			},	~0,	"LPC1111FHN33/[101|102]",										2,			sectorMap4k,		1,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0416502b,	~0			},	~0,	"LPC1111FHN33/201",												2,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2516902b,	~0			},	~0,	"LPC1111FHN33/[201|202]",										2,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00010013,	~0			},	~0,	"LPC1111FHN33/103",												2,			sectorMap4k,		1,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00010012,	~0			},	~0,	"LPC1111FHN33/203",												2,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0a24902b,	0x1a24902b	},	~0,	"LPC1112[FD20|FDH20|FDH28]/102",								4,			sectorMap4k,		1,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x042d502b,	~0			},	~0,	"LPC1112FHN33/101",												4,			sectorMap4k,		1,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2524d02b,	~0			},	~0,	"LPC1112FHN33/[101|102]",										4,			sectorMap4k,		1,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0425502b,	~0			},	~0,	"LPC1112FHN33/201",												4,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2524902b,	~0			},	~0,	"LPC1112[FHN33/201|FHN33/202|FHN24/202|FHI33/202]",				4,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00020023,	~0			},	~0,	"LPC1112FHN33/103",												4,			sectorMap4k,		1,		2048,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00020022,	~0			},	~0,	"LPC1112FH[N|I]33/203",											4,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0434502b,	~0			},	~0,	"LPC1113FHN33/201",												6,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2532902b,	~0			},	~0,	"LPC1113[FBD|FHN]/[201|202]",									6,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0434102b,	~0			},	~0,	"LPC1113FHN33/301",												6,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2532102b,	~0			},	~0,	"LPC1113[FBD48|FHN33]/[301/302]",								6,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00030030,	~0			},	~0,	"LPC1113[FBD48|FHN33]/303",										6,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00030032,	~0			},	~0,	"LPC1113FHN33/203",												6,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0a40902b,	0x1a40902b	},	~0,	"LPC1114[FDH|FN]28/102",										8,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0444502b,	~0			},	~0,	"LPC1114FHN33/201",												8,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2540902b,	~0			},	~0,	"LPC1114FHN33/[201|202]",										8,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0444102b,	~0			},	~0,	"LPC1114[FHN33|FBD48]/301",										8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2540102b,	~0			},	~0,	"LPC1114[FHN33/301|FHN33/302|FHI33/302|FBD48/302|FBD100/302]",	8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040040,	~0			},	~0,	"LPC1114[FBD48|FHN33]/303",										8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040042,	~0			},	~0,	"LPC1114FHN33/203",												8,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040060,	~0			},	~0,	"LPC1114FBD48/323",												12,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040070,	~0			},	~0,	"LPC1114[FBD48|FHN33]/333",										14,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00040040,	~0			},	~0,	"LPC1114FHI33/303",												8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00050080,	~0			},	~0,	"LPC1115FBD48/303",												16,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x1421102b,	~0			},	~0,	"LPC11C12FBD48/301",											4,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x1440102b,	~0			},	~0,	"LPC11C14FBD48/301",											8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x1431102b,	~0			},	~0,	"LPC11C22FBD48/301",											4,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x1430102b,	~0			},	~0,	"LPC11C24FBD48/301",											8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},

		// the following are from UM10839, LPC112x User manual, Rev. 1.0, 12 February 2015
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	ram			size	address
		{	{ 0x00140040,	~0			},	~0,	"LPC1124JBD48/303",												8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE|HAS_UID	},
		{	{ 0x00150080,	~0			},	~0,	"LPC1125JBD48/303",												16,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE|HAS_UID	},


		// the following are from UM10518, LPC11Exx User manual, Rev. 3.5, 21 December 2016.
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	ram			size	address
		{	{ 0x293E902B,	~0			},	~0,	"LPC11E11FHN33/101",											2,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2954502B,	~0			},	~0,	"LPC11E12FBD48/201",											4,			sectorMap4k,		1,		6144,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x296A102B,	~0			},	~0,	"LPC11E13FBD48/301",											6,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2980102B,	~0			},	~0,	"LPC11E14FHN33/401",											8,			sectorMap4k,		1,		10240,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2980102B,	~0			},	~0,	"LPC11E14FBD48/401",											8,			sectorMap4k,		1,		10240,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x2980102B,	~0			},	~0,	"LPC11E14FBD64/401",											8,			sectorMap4k,		1,		10240,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00009C41,	~0			},	~0,	"LPC11E35FHI33/501",											16,			sectorMap4k,		1,		12288,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x0000BC41,	~0			},	~0,	"LPC11E36FBD64/501",											24,			sectorMap4k,		1,		12288,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00009C41,	~0			},	~0,	"LPC11E36FHN33/501",											24,			sectorMap4k,		1,		12288,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00007C41,	~0			},	~0,	"LPC11E37FBD48/501",											32,			sectorMap4k,		1,		12288,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00007C45,	~0			},	~0,	"LPC11E37HFBD64/401",											32,			sectorMap4k,		1,		12288,		1024,	0x10000300,	UUENCODE	},
		{	{ 0x00007C41,	~0			},	~0,	"LPC11E37FBD64/501",											32,			sectorMap4k,		1,		12288,		1024,	0x10000300,	UUENCODE	},

		// the following are from UM10375, LPC1311/13/42/43 User manual, Rev. 5 -- 21 June 2012.
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	ram			size	address
		{	{0x2C42502B,	~0			},	~0,	"LPC1311FHN33",													2,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE|HAS_UID	},
		{	{0x1816902B,	~0			},	~0,	"LPC1311FHN33/01",												2,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE|HAS_UID	},
		{	{0x2C40102B,	~0			},	~0,	"LPC1313F[HN33|BD48]",											8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE|HAS_UID	},
		{	{0x1830102B,	~0			},	~0,	"LPC1313F[HN33|BD48]/01",										8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE|HAS_UID	},
		{	{0x3D01402B,	~0			},	~0,	"LPC1342F[HN33|BD48]",											4,			sectorMap4k,		1,		4096,		1024,	0x10000300,	UUENCODE|HAS_UID	},
		{	{0x3D00002B,	~0			},	~0,	"LPC1343F[HN33|BD48]",											8,			sectorMap4k,		1,		8192,		1024,	0x10000300,	UUENCODE|HAS_UID	},

		// the following are from UM10470,	~0, LPC178x/7x User manual, Rev. 3.1 -- 15 September 2014.
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	main ram	size	address
		{	{0x27011132,	~0			},	~0,	"LPC1774",														18,			sectorMapLpc17xx,	1,		32*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 128kB flash
		{	{0x27191F43,	~0			},	~0,	"LPC1776",														22,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 256kB flash
		{	{0x27193747,	~0			},	~0,	"LPC1777",														30,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 512kB flash
		{	{0x27193F47,	~0			},	~0,	"LPC1778",														30,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 512kB flash
		{	{0x281D1743,	~0			},	~0,	"LPC1785",														22,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 256kB flash
		{	{0x281D1F43,	~0			},	~0,	"LPC1786",														22,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 256kB flash
		{	{0x281D3747,	~0			},	~0,	"LPC1787",														30,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 512kB flash
		{	{0x281D3F47,	~0			},	~0,	"LPC1788",														30,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE	},	// @@@ 512kB flash

		// the following are from UM10360, LPC176x/5x User manual, Rev. 4. 1 -- 19 December 2016
		//																										# of							flash				block	block RAM
		//		id			alt. ID			word1	name														sectors							banks	main ram	size	address
		{	{0x26113F37,	~0			},	~0,	"LPC1769",														30,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x26013F37,	~0			},	~0,	"LPC1768",														30,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x26012837,	~0			},	~0,	"LPC1767",														30,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x26013F33,	~0			},	~0,	"LPC1766",														22,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x26013733,	~0			},	~0,	"LPC1765",														22,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x26011922,	~0			},	~0,	"LPC1764",														18,			sectorMapLpc17xx,	1,		32*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x26012033,	~0			},	~0,	"LPC1763",														22,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x25113737,	~0			},	~0,	"LPC1759",														30,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x25013F37,	~0			},	~0,	"LPC1758",														30,			sectorMapLpc17xx,	1,		64*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x25011723,	~0			},	~0,	"LPC1756",														22,			sectorMapLpc17xx,	1,		32*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x25011722,	~0			},	~0,	"LPC1754",														18,			sectorMapLpc17xx,	1,		32*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x25001121,	~0			},	~0,	"LPC1752",														16,			sectorMapLpc17xx,	1,		16*1024,	1024,	0x10000300,	UUENCODE|VECT_REMAP256	},
		{	{0x25001118,	0x25001110	},	~0,	"LPC1751",														8,			sectorMapLpc17xx,	1,		8*1024,		1024,	0x10000300,	UUENCODE|VECT_REMAP256	},

		// the following are from UM10120, LPC2131/2/4/6/8 User manual, Rev. 4 -- 23 April 2012
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	main ram	size	address
		{	{0x0002ff01,	~0			},	~0,	"LPC2131/LPC2131/01",											8,			sectorMapLpc23xx,	1,		8*1024,		1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x0002ff11,	~0			},	~0,	"LPC2132/LPC2132/01",											9,			sectorMapLpc23xx,	1,		16*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x0002ff12,	~0			},	~0,	"LPC2134/LPC2134/01",											11,			sectorMapLpc23xx,	1,		16*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x0002ff23,	~0			},	~0,	"LPC2136/LPC2136/01",											15,			sectorMapLpc23xx,	1,		32*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x0002ff25,	~0			},	~0,	"LPC2138/LPC2138/01",											27,			sectorMapLpc23xx,	1,		32*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},

		// the following are from UM10114, LPC21xx and LPC22xx User manual, Rev. 4 -- 2 May 2012 @@@ these need to be tested (check line termination)
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	main ram	size	address
		{	{0x0201ff01,	~0			},	~0,	"LPC2109",														8,			sectorMap8k,		1,		8*1024,		1024,	0x40000200,	UUENCODE	},
		{	{0x0201ff12,	~0			},	~0,	"LPC2119",														16,			sectorMap8k,		1,		16*1024,	1024,	0x40000200,	UUENCODE	},
		{	{0x0201ff13,	~0			},	~0,	"LPC2129",														18,			sectorMapLpc21xx,	1,		16*1024,	1024,	0x40000200,	UUENCODE	},
		{	{0x0101ff12,	~0			},	~0,	"LPC2114",														16,			sectorMap8k,		1,		16*1024,	1024,	0x40000200,	UUENCODE	},
		{	{0x0101ff13,	~0			},	~0,	"LPC2124",														18,			sectorMapLpc21xx,	1,		16*1024,	1024,	0x40000200,	UUENCODE	},
		{	{0x0301ff13,	~0			},	~0,	"LPC2194",														18,			sectorMapLpc21xx,	1,		16*1024,	1024,	0x40000200,	UUENCODE	},
		{	{0x0401ff13,	~0			},	~0,	"LPC2292",														18,			sectorMapLpc21xx,	1,		16*1024,	1024,	0x40000200,	UUENCODE	},
		{	{0x0501ff13,	~0			},	~0,	"LPC2294",														18,			sectorMapLpc21xx,	1,		16*1024,	1024,	0x40000200,	UUENCODE	},
		{	{0x0601ff13,	~0			},	~0,	"LPC2214/01",													18,			sectorMapLpc21xx,	1,		16*1024,	1024,	0x40000200,	UUENCODE	},
		{	{0x0401ff12,	~0			},	~0,	"LPC2212/01",													16,			sectorMap8k,		1,		16*1024,	1024,	0x40000200,	UUENCODE	},

		// the LPC2210 has 16kB of RAM while the LPC2220 has 64kB but they have the same ID so they cannot be uniquely identified.
		{	{0x0301ff12,	~0			},	~0,	"LPC2210/LPC2220/LPC2290",										0,			0,					0,		16*1024,	1024,	0x10000300,	UUENCODE	},
		{	{0x0301ff32,	~0			},	~0,	"LPC2220C/LPC2290/01C",											0,			0,					0,		64*1024,	1024,	0x10000300,	UUENCODE	},

		// the following are from UM10211, LPC23xx User manual, Rev. 4.1 -- 5 September 2012
		//																										# of							flash				block	block RAM
		//		id			alt. ID				name															sectors							banks	main ram	size	address
		{	{0x1600f701,	~0			},	~0,	"LPC2361",														9,			sectorMapLpc23xx,	1,		8*1024,		1024,	0x40000200,	UUENCODE|VECT_REMAP64	},	// vectors are remapped in ISP
		{	{0x1600ff22,	~0			},	~0,	"LPC2362",														11,			sectorMapLpc23xx,	1,		32*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},	//   mode so skip that area when
		{	{0x1600f902,	~0			},	~0,	"LPC2364",														11,			sectorMapLpc23xx,	1,		8*1024,		1024,	0x40000200,	UUENCODE|VECT_REMAP64	},	//   verifying
		{	{0x1600e823,	~0			},	~0,	"LPC2365",														15,			sectorMapLpc23xx,	1,		32*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x1600f923,	~0			},	~0,	"LPC2366",														15,			sectorMapLpc23xx,	1,		32*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x1600e825,	~0			},	~0,	"LPC2367",														28,			sectorMapLpc23xx,	1,		32*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x1600f925,	~0			},	~0,	"LPC2368",														28,			sectorMapLpc23xx,	1,		32*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x1700e825,	~0			},	~0,	"LPC2377",														28,			sectorMapLpc23xx,	1,		32*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x1700fd25,	~0			},	~0,	"LPC2378",														28,			sectorMapLpc23xx,	1,		32*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x1700ff35,	~0			},	~0,	"LPC2387",														28,			sectorMapLpc23xx,	1,		64*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},
		{	{0x1800ff35,	~0			},	~0,	"LPC2388",														28,			sectorMapLpc23xx,	1,		64*1024,	1024,	0x40000200,	UUENCODE|VECT_REMAP64	},

		// the following are from UM10430, LPC18xx User manual, Rev. 2.8 -- 10 December 2015
		//																										# of							flash				block	block RAM
		//		id			alt. ID			word1	name														sectors							banks	main ram	size	address
		{	{0xf000d830,	~0			},	0x00,	"LPC1850FET[180|256]",										0,		(sectormap_t *)NULL,	0,		96*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf000d860,	~0			},	0x00,	"LPC18S50FET[180|256]",										0,		(sectormap_t *)NULL,	0,		96*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf000da30,	~0			},	0x00,	"LPC1830[[FET[100|180|256]|FBD144]",						0,		(sectormap_t *)NULL,	0,		96*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf000da60,	~0			},	0x00,	"LPC18S30[[FET[100|256]|FBD144]",							0,		(sectormap_t *)NULL,	0,		96*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf00adb3c,	~0			},	0x00,	"LPC1820[FET100|FBD[100|144]]",								0,		(sectormap_t *)NULL,	0,		96*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf00adb6c,	~0			},	0x00,	"LPC18S20FBD144",											0,		(sectormap_t *)NULL,	0,		96*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf00b5b3f,	~0			},	0x00,	"LPC1810[FET100|FBD144]",									0,		(sectormap_t *)NULL,	0,		64*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf00b5b6f,	~0			},	0x00,	"LPC18S10[FET[100|180]|FBD144]",							0,		(sectormap_t *)NULL,	0,		64*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001d830,	~0			},	0x00,	"LPC1857[FET[180|256]|FBD208]",								15,			sectorMapLpc18x7,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001d860,	~0			},	0x00,	"LPC18S57JBD208",											15,			sectorMapLpc18x7,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001d830,	~0			},	0x44,	"LPC1853[FET[180|256]|FBD208]",								11,			sectorMapLpc18x3,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001da30,	~0			},	0x00,	"LPC1837FET[[100|180|256]FBD144]",							15,			sectorMapLpc18x7,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001da60,	~0			},	0x00,	"LPC18S37[JET100|JBD144]",									15,			sectorMapLpc18x7,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001da30,	~0			},	0x44,	"LPC1833FET[[100|180|256]FBD144]",							11,			sectorMapLpc18x3,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001db3c,	~0			},	0x00,	"LPC1827[JBD144|JET100]",									15,			sectorMapLpc18x7,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001db3c,	~0			},	0x22,	"LPC1823[JBD144|JET100]",									11,			sectorMapLpc18x3,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf00bdb3c,	~0			},	0x44,	"LPC1823[JBD144|JET100]",									11,			sectorMapLpc18x3,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf00bdb3c,	~0			},	0x80,	"LPC1822[JBD144|JET100]",									15,			sectorMapLpc18x2,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001db3f,	~0			},	0x00,	"LPC1817[JBD144|JET100]",									15,			sectorMapLpc18x7,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf001db3f,	~0			},	0x22,	"LPC1815[JBD144|JET100]",									13,			sectorMapLpc18x5,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf00bdb3f,	~0			},	0x44,	"LPC1813[JBD144|JET100]",									11,			sectorMapLpc18x3,	2,		32*1024,	1024,	0x10000200,	UUENCODE			},
		{	{0xf00bdb3f,	~0			},	0x80,	"LPC1812[JBD144|JET100]",									15,			sectorMapLpc18x2,	1,		32*1024,	1024,	0x10000200,	UUENCODE			},

		// the following are from UM10736, LPC15xx User manual, Rev. 1.1 -- 3 March 2014
		//																										# of							flash				block	block RAM
		//		id			alt. ID			word1	name														sectors							banks	main ram	size	address
		{	{0x00001549,	~0			},	0x00,	"LPC1549",													64,			sectorMap4k,		1,		36*1024,	1024,	0x02000200,	TERM_CRLF			},
		{	{0x00001548,	~0			},	0x00,	"LPC1548",													32,			sectorMap4k,		1,		20*1024,	1024,	0x02000200,	TERM_CRLF			},
		{	{0x00001547,	~0			},	0x00,	"LPC1547",													16,			sectorMap4k,		1,		12*1024,	1024,	0x02000200,	TERM_CRLF			},
		{	{0x00001519,	~0			},	0x00,	"LPC1519",													64,			sectorMap4k,		1,		36*1024,	1024,	0x02000200,	TERM_CRLF			},
		{	{0x00001518,	~0			},	0x00,	"LPC1518",													32,			sectorMap4k,		1,		20*1024,	1024,	0x02000200,	TERM_CRLF			},
		{	{0x00001517,	~0			},	0x00,	"LPC1517",													16,			sectorMap4k,		1,		12*1024,	1024,	0x02000200,	TERM_CRLF			},

		// the following are from UM10912, LPC546xx User manual, Rev. 1.9 -- 16 June 2017
		//																										# of							flash				block	block RAM
		//		id			alt. ID			word1	name														sectors							banks	main ram	size	address
		{	{0x7f954605,	~0			},	~0,		"LPC54605J256ET180",										8,			sectorMap32k,		1,		136*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0xfff54605,	~0			},	~0,		"LPC54605J512ET180",										16,			sectorMap32k,		1,		200*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0x7f954606,	~0			},	~0,		"LPC54606J256[ET100|BD100|ET180]",							8,			sectorMap32k,		1,		136*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0xfff54606,	~0			},	~0,		"LPC54606J512[ET100|BD100|BD208]",							16,			sectorMap32k,		1,		200*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0x7f954607,	~0			},	~0,		"LPC54607J256[ET180|BD208]",								8,			sectorMap32k,		1,		136*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0xfff54607,	~0			},	~0,		"LPC54607J512ET180",										16,			sectorMap32k,		1,		200*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0xfff54608,	~0			},	~0,		"LPC54608J512[ET180|BD208]",								16,			sectorMap32k,		1,		200*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0xfff54616,	~0			},	~0,		"LPC54616J512[ET100|BD100|BD208]",							16,			sectorMap32k,		1,		200*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0x7f954616,	~0			},	~0,		"LPC54616J256ET180",										8,			sectorMap32k,		1,		136*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0xfff54618,	~0			},	~0,		"LPC54618J512[ET180|BD208]",								16,			sectorMap32k,		1,		200*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
		{	{0xfff54628,	~0			},	~0,		"LPC54628J512ET180",										16,			sectorMap32k,		1,		200*1024,	1024,	0x20000200,	TERM_CRLF|HAS_UID|SKIP_0	},
	},
	partDefUnknown=
	{
		.id				= {0,0},
		.name			= "unknown",
		.numSectors		= 0,
		.sectorMap		= (sectormap_t *)NULL,
		.ramSize		= 0,
	};

HIDDEN void DumpPartList(FILE *fp)
{
	int
		i;

	fprintf(fp,"supported parts:\n");
	for(i=0;i<ARRAY_SIZE(partDef);i++)
	{
		fprintf(fp,"  %s\n",partDef[i].name);
	}
}

int LPCISP_GetSectorAddr(unsigned int addr, partinfo_t *p)
// given an address locate the number of the sector containing
// that address or -1 if out of range (address off end of device)
// or sector map undefined.
{
	int
		sectorNum;

	if(p->sectorMap)
	{
		for(sectorNum=0;sectorNum<p->numSectors;sectorNum++)
		{
			if((p->sectorMap[sectorNum].base<=addr)&&((p->sectorMap[sectorNum].base+p->sectorMap[sectorNum].size)>addr))
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
	if(p->sectorMap)
	{
		for(i=0;i<p->numSectors;i++)
		{
			size+=p->sectorMap[i].size;
		}
	}
	return(size);
}

HIDDEN void ReportPartInfo(int level,partinfo_t *p)
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
		if(p->flags&HAS_UID)
		{
			// if device has a UID report it
			ReportString(level,"   UID:          ");
			for(i=0;i<4;i++)
			{
				ReportString(level,"%08x ",p->uid[i]);
			}
			ReportString(level,"\n");
		}
		ReportString(level,"   boot version: %d.%d\n",p->bootMajor,p->bootMinor);
	}
}

HIDDEN int GetPartInfo(int fd,partinfo_t *p)
// get part information from the target (if possible), fill out passed structure.
// if part ID can't be matched still return the part ID, UID, and boot code version.
{
	int
		i,
		j,
		found;
	unsigned int
		id,
		id1;

	found=0;
	memcpy(p,&partDefUnknown,sizeof(partinfo_t));
	id=LPCISP_ReadPartID(fd,&id1);
	p->id[0]=id;
	p->id1=id1;
	p->idIdx=0;
	if(id!=~0)
	{
		for(i=0;!found&&(i<ARRAY_SIZE(partDef));i++)
		{
			for(j=0;!found&&(j<2);j++)
			{
				if((partDef[i].id[j]==id)&&((partDef[i].id1==~0)||(partDef[i].id1==id1)))
				{
					memcpy(p,&partDef[i],sizeof(partinfo_t));
					p->idIdx=j;
					found=1;
				}
			}
		}
		if(found&&(p->flags&HAS_UID))
		{
			// if device identified and it supports UID
			LPCISP_ReadPartUID(fd,p->uid);
		}
		LPCISP_ReadBootCodeVersion(fd,&p->bootMajor,&p->bootMinor);
		return(0);
	}
	return(-1);
}
