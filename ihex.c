
#include "includes.h"

static int ASCII2HexDigit(char c)
// convert an ascii character to its hex value.  return -1 if invalid.
{
	c=tolower(c);
	if((c>='0') && (c<='9'))
	{
		return(c-'0');
	}
	else if((c>='a') && (c<='f'))
	{
		return(c-'a'+0x0a);
	}
	return(-1);									// not a valid ascii character, return -1
}

static int ASCII2HexByte(char hi, char lo)
// convert two ascii characters to a hex value.  return -1 if invalid.
{
	int
		i,j;

	i=ASCII2HexDigit(hi);
	j=ASCII2HexDigit(lo);
	return(((i>=0)&&(j>=0))?(i*0x10+j):-1);
}

static unsigned char *ParseRecord(unsigned char *buffer, char *line, int *baseAddr, int *extAddr, int *length, int *size, int *eof)
// parse an Intel hex record, put the contents into the buffer.
//  buffer: pointer to the buffer
//  line: pointer to the line containing the hex record
//  baseAddr: pointer to address of first byte in buffer (lowest address in file)
//  extAddr: pointer to current linear base address (b31:16 only; b15:0 are zero)
//  length: pointer to size of the image in the buffer
//  size: pointer to the size of the buffer (may be resized)
//  return pointer to buffer on success, NULL on any error (free buffer on error)
//
//  0   - colon (start of record)
//  2:1 - record length field (number of data bytes)
//  6:3 - address field
//  8:7 - type (00=data record, 01=EOF record, 02=extended segment address, 03=start segment address, 04=extended linear address, 05=start linear address
//  data starts at 10:9
{
	unsigned char
		type,
		idx,
		checksum,
		reclen;
	unsigned int
		recAddr,	// base address of this record
		endOffset;
	int
		fail;
	unsigned char
		*p;
	int
		oldSize;

	fail=0;
	// must start with a colon
	if(line[0]==':')
	{
		// read the parameters common to all records
		reclen=ASCII2HexByte(line[1],line[2]);
		recAddr=*extAddr|(ASCII2HexByte(line[3],line[4])<<8)|ASCII2HexByte(line[5],line[6]);
		type=ASCII2HexByte(line[7],line[8]);

		// test the record's checksum regardless of the record type
		checksum=0;
		for(idx=0;idx<(reclen+5);idx++)	// length of area to be checksummed (data plus reclen plus offset plus type plus checksum)
		{
			checksum+=ASCII2HexByte(line[idx*2+1],line[idx*2+2]);
		}

		if(checksum==0)
		{
			switch(type)
			{
				case 0:	// data record
					if(*baseAddr<0)
					{
						// if base address hasn't been assigned yet then use this first record's address as the base of the buffer
						*baseAddr=recAddr;
					}
					if(recAddr<*baseAddr)
					{
						// this record is lower than the base, allocate space for it and adjust the base
						oldSize=*size;
						*size+=*baseAddr-recAddr;
						p=(unsigned char *)realloc(buffer,*size);
						if(p)
						{
							// move existing data to end of buffer, clear new space at front to 0xff (erased flash)
							memmove(&p[*baseAddr-recAddr],p,oldSize);
							memset(p,0xff,*baseAddr-recAddr);
							*baseAddr=recAddr;
						}
						else
						{
							fail=1;
						}
					}
					endOffset=recAddr+reclen-*baseAddr;
					if(endOffset>*size)
					{
						oldSize=*size;
						*size=(endOffset/1024+1)*1024;
						p=(unsigned char *)realloc(buffer,*size);	// increase buffer size, round up to next 1k
						if(p)
						{
							// fill new region with 0xff (as if erased flash)
							memset(&p[oldSize],0xff,*size-oldSize);
							buffer=p;
						}
						else
						{
							ReportString(REPORT_ERROR,"failed to reallocate buffer to %d bytes\n",*size);
							fail=1;
						}
					}
					if(!fail)
					{
						for(idx=0;idx<reclen;idx++)
						{
							buffer[recAddr+idx-*baseAddr]=ASCII2HexByte(line[9+idx*2],line[10+idx*2]);
						}
						if(endOffset>*length)
						{
							*length=endOffset;
						}
					}
					break;
				case 1:	// EOF record
					*eof=1;
					break;
				case 3: // start segment address
					// @@@ ignore
					break;
				case 4: // extended linear address
					*extAddr=(ASCII2HexByte(line[9],line[10])<<24)|(ASCII2HexByte(line[11],line[12])<<16);
					break;
				case 5: // start linear address
					printf("start linear address: 0x%08x\n",(ASCII2HexByte(line[9],line[10])<<24)|(ASCII2HexByte(line[11],line[12])<<16));
					break;
				case 2: // extended segment address
					*extAddr=(ASCII2HexByte(line[9],line[10])<<8)<<4;
					break;
				default:
					// don't know how to handle this type of record, fall through and return -1
					ReportString(REPORT_ERROR,"unknown record type \"%02x\"\n",type);
					fail=1;
					break;
			}
		}
		else
		{
			ReportString(REPORT_ERROR,"hex record checksum error\n");
			fail=1;
		}
	}
	else
	{
		ReportString(REPORT_ERROR,"not intel hex record\n");
		fail=1;
	}
	if(fail)
	{
		free(buffer);
		return((unsigned char *)NULL);
	}
	return(buffer);
}

#ifdef __WIN__
static int getline(char **linep, int *linecapp, FILE *fp)
// reduced function getline to support what we need since Windows doesn't provide getline.
// not a full implementation.  Assumes line buffer was allocated and is large enough.
{
	int
		n,
		c;
	char
		*p;
	
	p=*linep;
	n=0;
	do
	{
		c=fgetc(fp);
		*p++=c;
		*p='\0';
		n++;
	}while((c!='\n')&&(c>=0)&&(n<*linecapp));
	n--;
	*--p='\0';
	return(n);
}
#endif

unsigned char *LPCISP_ReadHexFile(const char *fileName,int *baseAddr,int *length)
// read hex file into buffer, return pointer to buffer or NULL if error.
// caller must free returned buffer.
//   fileName: name of hex file to load
//   start: pointer to start address (to be filled in) -- this is the address of the first byte of the buffer (offset)
//   length: pointer to length of image (to be filled in)
{
	FILE
		*fp;
	size_t
		n;
	unsigned char
		*buffer;
	char
		*line;
	int
		size,
		extAddr;
	int
		eof;

	buffer=(unsigned char *)NULL;
	fp=fopen(fileName,"r");
	if(fp)
	{
		size=1024;
		if((buffer=(unsigned char *)malloc(size)))
		{
			memset(buffer,0xff,size);
			n=256;
			line=(char *)malloc(n);
			if(line)
			{
				extAddr=0;
				*baseAddr=-1;
				*length=0;
				eof=0;
				while(!eof && buffer && (getline(&line,&n,fp)>0))
				{
					// place the contents of the line into the buffer
					// on success address and reclen are filled with the
					// address and length of the record read in.  returns
					// pointer to (possibly resized) buffer or NULL if
					// error (buffer will have been freed, so no need to
					// deal with that here).
					buffer=ParseRecord(buffer,line,baseAddr,&extAddr,length,&size,&eof);
					if(!buffer)
					{
						ReportString(REPORT_ERROR,"failed to parse \"%s\"\n",fileName);
						errno=EINVAL;
					}
				}
				free(line);
			}
		}
		fclose(fp);
	}
	else
	{
		ReportString(REPORT_ERROR,"failed to open \"%s\", %s\n",fileName,strerror(errno));
	}
	return(buffer);
}
