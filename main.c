/*

	lpcisp -- NXP LPCxxxx ISP

	see ChangeLog for version information.

*/

#include "includes.h"

static const char
	*version="0.0.26";

static int
	fd,
	baud,
	tbaud,
	clock,
	retries,
	start,
	length,
	echo,
	vector,
	burn,
	erase,
	verify,
	term;

static unsigned char
	*data;

static partinfo_t
	partInfo;

static char
	*fileName;

static long
	dumpAddr,
	dumpLength;

static void ReportFileInfo(int level)
{
	ReportString(level,"file information:\n");
	ReportString(level,"   file name:     %s\n",fileName);
	ReportString(level,"   start address: 0x%08x\n",start);
	ReportString(level,"   length:        0x%08x\n",length);
}

//========== option handlers ==========

static int VerboseOpt(int *argc, char ***argv)
{
	if(*argc)
	{
		SetReportLevel(strtol(**argv,NULL,0));
		*argc=(*argc)-1;;
		*argv=(*argv)+1;
		return(0);
	}
	return(-1);
}

static int PartsOpt(int *argc, char ***argv)
{
	DumpPartList(stdout);
	return(0);
}

static int WriteOpt(int *argc, char ***argv)
{
	burn=1;
	return(0);
}

static int EraseOpt(int *argc, char ***argv)
{
	erase=1;
	return(0);
}

static int EchoOpt(int *argc, char ***argv)
{
	echo=1;
	return(0);
}

static int VectorOpt(int *argc, char ***argv)
{
	vector=1;
	return(0);
}

typedef struct
{
	const char
		*str;
	int
		code;
}pin_t;

static int GetCtrlArg(const char *arg)
{
	int
		i;
	const pin_t
		pinMap[]=
		{
			{	"dtr",	PIN_DTR		},
			{	"ndtr",	PIN_nDTR	},
			{	"rts",	PIN_RTS		},
			{	"nrts",	PIN_nRTS	},
		};
	for(i=0;i<ARRAY_SIZE(pinMap);i++)
	{
		if(strcmp(arg,pinMap[i].str)==0)
		{
			return(pinMap[i].code);
		}
	}
	return(-1);
}

static int ResetOpt(int *argc, char ***argv)
// using the next argument, map the reset control to specified serial device pin.
{
	int
		pin;

	if(*argc)
	{
		if((pin=GetCtrlArg(**argv))>=0)
		{
			ConfigureResetPin(pin);
			*argv=(*argv)+1;
			*argc=(*argc)-1;
			return(0);
		}
	}
	return(-1);
}

static int ISPOpt(int *argc, char ***argv)
// using the next argument, map the ISP control to specified serial device pin.
{
	int
		pin;

	if(*argc)
	{
		if((pin=GetCtrlArg(**argv))>=0)
		{
			ConfigureISPPin(pin);
			*argv=(*argv)+1;
			*argc=(*argc)-1;
			return(0);
		}
	}
	return(-1);
}

static int HoldOpt(int *argc, char ***argv)
{
	ConfigureISPHold(1);
	return(0);
}

static int BaudOpt(int *argc, char ***argv)
// using the next argument get the baud rate
{
	if(*argc)
	{
		if((baud=strtol(**argv,NULL,0)))
		{
			*argv=(*argv)+1;
			*argc=(*argc)-1;
			return(0);
		}
	}
	return(-1);
}

static int TBaudOpt(int *argc, char ***argv)
// using the next argument get the terminal baud rate
{
	if(*argc)
	{
		if((tbaud=strtol(**argv,NULL,0)))
		{
			*argv=(*argv)+1;
			*argc=(*argc)-1;
			return(0);
		}
	}
	return(-1);
}

static int ClockOpt(int *argc, char ***argv)
// using the next argument get the clock rate
{
	if(*argc)
	{
		if((clock=strtol(**argv,NULL,0)))
		{
			*argv=(*argv)+1;
			*argc=(*argc)-1;
			return(0);
		}
	}
	return(-1);
}


static int RetryOpt(int *argc, char ***argv)
// using the next argument get the number of sync retries
{
	if(*argc)
	{
		if((retries=strtol(**argv,NULL,0)))
		{
			*argv=(*argv)+1;
			*argc=(*argc)-1;
			return(0);
		}
	}
	return(-1);
}

static int VerifyOpt(int *argc, char ***argv)
{
	verify=1;
	return(0);
}

static int TermOpt(int *argc, char ***argv)
{
	term=1;
	return(0);
}

static int DumpOpt(int *argc, char ***argv)
{
	if(*argc>=2)
	{
		dumpAddr=strtol(**argv,NULL,0);
		*argv=(*argv)+1;
		dumpLength=strtol(**argv,NULL,0);
		*argv=(*argv)+1;
		*argc=(*argc)-2;
		return(0);
	}
	return(-1);
}


static int HexFileOpt(int *argc, char ***argv)
{
	if(*argc)
	{
		if(data)
		{
			// throw out a previously-loaded buffer if one exists
			free(data);
		}

		// read in a hex file
		fileName=**argv;
		data=ReadHexFile(**argv,&start,&length);
		if(data)
		{
			ReportFileInfo(REPORT_INFO);
			*argc=(*argc)-1;;
			*argv=(*argv)+1;
			return(0);
		}
	}
	return(-1);
}

static int StartOpt(int *argc, char ***argv)
{
	if(*argc)
	{
		start=strtol(**argv,NULL,0);
		*argc=(*argc)-1;
		*argv=(*argv)+1;
		return(0);
	}
	return(-1);
}

static int BinFileOpt(int *argc, char ***argv)
{
	FILE *
		fp;
	struct stat
		info;
	int
		numRead;

	if(*argc)
	{
		if(data)
		{
			// throw out a previously-loaded buffer if one exists
			free(data);
		}

		// get size (and existence) of file, allocate a buffer, read it in
		fileName=**argv;
		if(stat(**argv,&info)>=0)
		{
			start=0;
			length=info.st_size;
			data=(unsigned char *)malloc(length);
			if(data)
			{
				fp=fopen(**argv,"r");
				if(fp)
				{
					numRead=fread(data,1,length,fp);
					fclose(fp);
					if(numRead==length)
					{
						ReportFileInfo(REPORT_INFO);
						*argc=(*argc)-1;;
						*argv=(*argv)+1;
						return(0);
					}
				}
			}
			else
			{
				ReportString(REPORT_ERROR,"failed to allocate memory: %s\n",strerror(errno));
			}
		}
		ReportString(REPORT_ERROR,"failed to open \"%s\" for reading: %s\n",fileName,strerror(errno));
	}
	return(-1);
}

//========== token handling ==========

typedef int (funct_t)(int *argc, char ***argv);

typedef struct
{
	const char
		*token;
	funct_t
		*funct;
	const char
		*helpStr;
}token_t;

static token_t
	tokenList[]=
	{
			// token name	function		help string
		{	"-reset",		ResetOpt,		" ctrl            map reset to a serial control signal.  ctrl may be one of dtr, ndtr, rts, or nrts (n=inverted)"		},
		{	"-isp",			ISPOpt,			" ctrl              map ISP to a serial control signal.  ctrl may be one of dtr, ndtr, rts, or nrts (n=inverted)"		},
		{	"-hold",		HoldOpt,		"                  assert ISP through programming sequence (default is to negate at initialization)"					},
		{	"-baud",		BaudOpt,		" rate             set the baud rate for ISP mode (default=115200)"														},
		{	"-tbaud",		TBaudOpt,		" rate            set the baud rate for the terminal (default=115200)"													},
		{	"-clock",		ClockOpt,		" rate            set the clock rate in kHz (default=12000)"															},
		{	"-retry",		RetryOpt,		" num             set number of retries when synchronizing (default=25)"												},
		{	"-bin",			BinFileOpt,		" filename          specify name of binary file to load"																},
		{	"-hex",			HexFileOpt,		" filename          specify name of hex file to load"																	},
		{	"-start",		StartOpt,		" address         override start address (specify AFTER hex or binary file)"											},
		{	"-erase",		EraseOpt,		"                 erase target device"																					},
		{	"-echo",		EchoOpt,		"                  enable echo from target (default is no echo)"														},
		{	"-vector",		VectorOpt,		"                patch vector 7 to 2's complement of the sum of vectors 0 through 6 regardless of address of image"		},
		{	"-write",		WriteOpt,		"                 write image to target device"																			},
		{	"-verify",		VerifyOpt,		"                verify image"																							},
		{	"-dump",		DumpOpt,		" address length   read from length bytes from memory starting at address, dump to console"								},
		{	"-term",		TermOpt,		"                  enter terminal mode after other operations"															},
		{	"-verbose",		VerboseOpt,		" level         set reporting level (0=errors only, 1=minimum, 2=progress (default), 3=process debug, 4=full debug)"	},
		{	"-parts",		PartsOpt,		"                 list supported devices"																				},
	};

static void Usage(const char *progName)
{
	int
		i;

	ReportString(REPORT_ERROR,"%s: ver. %s (c) 2015, 2016 Cosmodog, Ltd.\n",progName,version);
	ReportString(REPORT_ERROR,"usage:\n");
	ReportString(REPORT_ERROR,"  %s [options] device\n",progName);
	ReportString(REPORT_ERROR,"where\n");
	ReportString(REPORT_ERROR,"  device is the serial device (/dev/ttyS0, etc.)\n");
	ReportString(REPORT_ERROR,"options:\n");
	for(i=0;i<ARRAY_SIZE(tokenList);i++)
	{
		ReportString(REPORT_ERROR,"  %s%s\n",tokenList[i].token,tokenList[i].helpStr);
	}
}


//========== main ==========

int main(int argc,char *argv[])
{
	int
		i,
		j,
		lineLen,
		fail;
	char
		*progName;
	unsigned char
		buffer[256];
	unsigned int
		*vect,
		cs;

	fail=0;
	progName=basename(*argv);
	argc--;
	argv++;
	if(argc)
	{
		// set the defaults.  may be changed by command line arguments
		fd=-1;
		baud=115200;
		tbaud=115200;
		clock=12000;
		retries=25;
		ConfigureResetPin(PIN_NONE);
		ConfigureISPPin(PIN_NONE);
		ConfigureISPHold(0);
		SetReportLevel(REPORT_INFO);
		erase=0;
		echo=0;
		vector=0;
		burn=0;
		verify=0;
		term=0;
		start=0;
		length=0;
		dumpAddr=-1;
		dumpLength=-1;
		data=(unsigned char *)NULL;

		// parse command line arguments
		while(!fail&&argc)
		{
			if(*argv[0]=='-')
			{
				// option, process it
				for(i=0;i<ARRAY_SIZE(tokenList);i++)
				{
					if(strcmp(tokenList[i].token,*argv)==0)
					{
						// skip over the token and call the function which may then consume more arguments.
						argc--;
						argv++;
						fail=(tokenList[i].funct(&argc,&argv)<0);
						if(fail)
						{
							ReportString(REPORT_ERROR,"invalid arguments\n");
						}
						break;
					}
				}
				if(!fail)
				{
					fail=(i==ARRAY_SIZE(tokenList));
					if(fail)
					{
						ReportString(REPORT_ERROR,"unknown option \"%s\"\n",*argv);
					}
				}
				if(!fail&&(start&0x03))
				{
					ReportString(REPORT_ERROR,"start address (0x%08x) must be on a word boundary (multiple of four)\n",start);
					fail=1;
				}
			}
			else
			{
				// expect a device name (e.g., /dev/ttyS0).  open it and try to communicate with
				// a target device and retrieve device information.
				fail=(fd>=0);
				if(!fail)
				{
					fd=OpenDevice(*argv);
					fail=(fd<0);
					if(!fail)
					{
						// if term was the only action requested skip the synchronization step and drop directly to terminal
						if(erase||data||(dumpLength>0)||!term)
						{
							fail=(ChangeBaudRate(fd,baud)<0);
							if(!fail)
							{
								// until we know what kind of device this is use default termination (accept CR, LF, or CRLF)
								SetLineTermination(TERM_ANY);
								fail=!Sync(fd,clock*1000,retries);
								if(!fail)
								{
									Echo(fd,echo);
									fail=(GetPartInfo(fd,&partInfo)<0);
									if(!fail)
									{
										SetLineTermination(partInfo.flags&TERM_MASK);
										ReportPartInfo(REPORT_INFO,&partInfo);
										if(partInfo.numSectors)
										{
											// only unlock device if it was identified
											Unlock(fd);
										}
									}
									else
									{
										ReportString(REPORT_ERROR,"failed to identify part\n");
									}
								}
								else
								{
									ReportString(REPORT_ERROR,"failed to detect target device on \"%s\" at %d baud\n",*argv,baud);
								}
							}
						}
					}
					else
					{
						ReportString(REPORT_ERROR,"failed to open \"%s\"\n",*argv);
					}
					argc--;
					argv++;
				}
				else
				{
					// one was already opened, close it and fail
					CloseDevice(fd);
				}
			}
		}

		// if a device was opened try to do operations on it
		if(!fail&&(fd>=0))
		{
			if(erase||data||(dumpLength>0))
			{
				// perform operations only if part was in our database
				if(partInfo.numSectors)
				{
					if(erase)
					{
						fail=(Erase(fd,0,partInfo.numSectors-1,0,&partInfo)!=1);	// @@@ only support one bank for now
						if(fail)
						{
							ReportString(REPORT_ERROR,"failed to erase device\n");
						}
					}

					// if data image was loaded we can write it to flash and/or verify it
					if(!fail&&data)
					{
						// if image starts at 0x0000 or forced fix up the vector table.
						if(vector||(start==0))
						{
							// vector 7 is the 2's comp of the arithmetic sum of vectors 0-6.
							vect=(unsigned int *)data;
							cs=0;
							for(i=0;i<7;i++)
							{
								cs+=*vect++;
							}
							*vect=-cs;
						}

						// write image to flash
						if(burn)
						{
							fail=!WriteToFlash(fd,data,start,length,&partInfo);
							ReportString(REPORT_INFO,"%s\n",!fail?"write complete":"failed to write\n");
						}

						// compare image against flash
						if(!fail&&verify)
						{
							unsigned int
								addr,
								off;
							
							off=0;
							if((partInfo.flags&SKIP_0)&&(start==0))
							{
								// device can't read word 0 in flash, skip over it
								length-=4;
								start=4;
								off=4;
							}
							if((partInfo.flags&VECT_REMAP64)&&(start<64))
							{
								// device remaps vector table in ISP mode, so we can't verify that section
								length-=(64-start);
								start=64;
								off=64;
							}
							else if((partInfo.flags&VECT_REMAP256)&&(start<256))
							{
								// device remaps vector table in ISP mode, so we can't verify that section
								length-=(256-start);
								start=256;
								off=256;
							}
							ReportString(REPORT_INFO,"verifying... 0x%08x",start);
							for(addr=start;!fail&&(addr<start+length);addr+=256)
							{
								// read remaining bytes, up to 256 (must be multiple of four)
								fail=(ReadFromTarget(fd,buffer,addr,MIN((length-(addr-start)+3)&~3,256),&partInfo)<=0);
								if(!fail)
								{
									for(i=0;!fail&&(i<MIN((length-(addr-start)+3)&~3,256));i++)
									{
										fail=buffer[i]!=data[off+addr-start+i];
										if(fail)
										{
											ReportString(REPORT_INFO," -- mismatch at 0x%08x, is 0x%02x, should be 0x%02x\n",addr+i,buffer[i],data[off+addr-start+i]);;
										}
									}
								}
								if(!fail)
								{
									ReportString(REPORT_INFO,"\b\b\b\b\b\b\b\b%08x",addr+MIN(length-(addr-start),256));
								}
							}
							ReportString(REPORT_INFO,"\nverify %s\n",!fail?"complete":"failed");
						}
						free(data);
					}

					if(dumpLength>0)
					{
						i=0;
						data=(unsigned char *)malloc(dumpLength);
						if(data)
						{
							// dumpAddr and dumpLength must be word-aligned
							dumpAddr&=~3;
							dumpLength=(dumpLength+3)&~3;
							fail=(ReadFromTarget(fd,data,dumpAddr,dumpLength,&partInfo)<=0);
							if(!fail)
							{
								while(i<dumpLength)
								{
									lineLen=16-(dumpAddr&0x0f);
									ReportString(REPORT_MINIMUM,"%08x: ",dumpAddr&~0x0f);
									for(j=0;j<16-lineLen;j++)
									{
										ReportString(REPORT_MINIMUM,"   ");
									}
									for(j=0;j<lineLen;j++)
									{
										if((i+j)<dumpLength)
										{
											ReportString(REPORT_MINIMUM,"%02x ",(unsigned char)data[i+j]);
										}
										else
										{
											ReportString(REPORT_MINIMUM,"   ");
										}
									}
									ReportString(REPORT_MINIMUM," - ");
									for(j=0;j<16-lineLen;j++)
									{
										ReportString(REPORT_MINIMUM," ");
									}
									for(j=0;(j<lineLen)&&((i+j)<dumpLength);j++)
									{
										ReportString(REPORT_MINIMUM,"%c",(data[i+j]>' '&&data[i+j]<='~')?data[i+j]:'.');
									}
									i+=lineLen;
									dumpAddr+=lineLen;
									ReportString(REPORT_MINIMUM,"\n");
								}
							}
							free(data);
						}
					}
				}
				else
				{
					ReportString(REPORT_ERROR,"ERROR: unknown part, cannot perform operations on it\n");
				}
			}
			ExitISPMode(fd);

			if(!fail&&term)
			{
				// go into terminal mode with current line settings
				if(tbaud!=baud)
				{
					fail=(ChangeBaudRate(fd,tbaud)<0);
				}
				if(!fail)
				{
					fail=!Terminal(fd);
				}
			}

			CloseDevice(fd);
		}
		if(fd<0)
		{
			ReportString(REPORT_ERROR,"ERROR: no serial device specified\n");
			fail=1;
		}
	}
	else
	{
		Usage(progName);
	}
	return(fail?-1:0);
}
