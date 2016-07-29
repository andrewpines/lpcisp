/*

	ISP commands and support

*/

#include "includes.h"

// @@@ consider allowing this to be set from command line
#define RESPONSE_TIMEOUT	1000000		// in ms, fail if no response from device within this time

static int
	resetPin,
	ispPin,
	echo=1,		// true when sent characters are expected to echo back, false when not (echo disabled by command to device)
	ispHold;	// true to hold ISP asserted through programming sequence

static unsigned char
	lineTermination=TERM_ANY;

//============ private functions ==============================================

static int GetCheckSum(unsigned char *data, int length)
// return the sum of bytes in a buffer
{
	int
		cs;
	cs=0;
	while(length--)
	{
		cs+=*data++;
	}
	return(cs);
}

static int WriteString(int fd, const char *string)
// send a string to the device, return true on success.
{
	unsigned int
		len;

	// report to console if debugging is enabled
	ReportString(REPORT_DEBUG_FULL,"--> [");
	ReportStringCtrl(REPORT_DEBUG_FULL,string);
	ReportString(REPORT_DEBUG_FULL,"]\n");

	// get length, write to device, return true if wrote all bytes
	len=strlen(string);
	return(WriteBytes(fd,(unsigned char *)string,len)==len);
}

static int WriteBuffer(int fd, const unsigned char *buffer, unsigned int len)
// write a block of binary data.  used for parts which don't uuencode their data.
{
	// report to console if debugging is enabled
	ReportString(REPORT_DEBUG_FULL,"--> ");
	ReportBufferCtrl(REPORT_DEBUG_FULL,buffer,len);

	// write to device, return true if wrote all bytes
	return(WriteBytes(fd,(unsigned char *)buffer,len)==len);
}

static int WriteChar(int fd, char c)
// send a single character, return true on success
{
	// report to console if debugging is enabled
	ReportString(REPORT_DEBUG_FULL,"--> ");
	ReportChar(REPORT_DEBUG_FULL,c);
	ReportChar(REPORT_DEBUG_FULL,'\n');

	// write to device, return true if wrote the byte
	return(WriteBytes(fd,(unsigned char *)&c,1)==1);
}

static int ReadBuffer(int fd, unsigned char *buffer, unsigned int length)
// read length bytes into buffer or until timeout, return number of
// bytes read.  this is used for binary reads for parts which don't uuencode
// data transfers.
{

	unsigned char
		c;
	int
		i;

	i=0;
	// read one character
	while((i<length)&&(ReadBytes(fd,&c,1,RESPONSE_TIMEOUT)==1))
	{
		buffer[i++]=c;
	}

	if(i)
	{
		// if something came in report it
		ReportString(REPORT_DEBUG_FULL,"<-- ");
		ReportBufferCtrl(REPORT_DEBUG_FULL,buffer,i);
	}
	return(i);
}
	
void SetLineTermination(unsigned char t)
// set line termination mode
{
	lineTermination=t;
}

static int ReadStringAny(int fd, char *string)
// read a string from the device, return non-zero on success (complete string read),
// zero on failure (didn't read complete string within timeout period).  stop on the
// first CR or LF.  Throw away any leading CRs or LFs, assume they're left over from
// the end of the previous message.  this will work with CR, LF, or CRLF termination
// but creates ambiguity when changing modes in devices which transfer data in binary
// instead of uuencoded-ASCII.
{
	unsigned char
		c;
	int
		i;

	i=0;
	ReportString(REPORT_DEBUG_FULL,"<-- ");
	while(ReadBytes(fd,&c,1,RESPONSE_TIMEOUT)==1)
	{
		ReportCharCtrl(REPORT_DEBUG_FULL,c);
		// if not first character or not carriage return or newline, add to buffer (throw away leading CRs and LFs)
		if(i||((c!='\r')&&(c!='\n')))
		{
			string[i]=c;
			if(i&&((c=='\n')||(c=='\r')))
			{
				// not first character and is CR or LF, zero-terminate the string.
				string[i]='\0';
				ReportString(REPORT_DEBUG_FULL," [");
				ReportStringCtrl(REPORT_DEBUG_FULL,string);
				ReportString(REPORT_DEBUG_FULL,"]\n");
				return(1);
			}
			i++;
		}
	}
	// failed, exit false
	return(0);
}


static int ReadStringCRLF(int fd, char *string)
// read a string from the device, return non-zero on success (complete string read),
// zero on failure (didn't read complete string within timeout period).  stop on the
// CRLF.  Ignore any embedded control characters (including CR or LF) prior to seeing
// the CRLF sequence.
{
	unsigned char
		c;
	int
		i,
		t;

	i=0;
	t=0;
	ReportString(REPORT_DEBUG_FULL,"<-- ");
	while(ReadBytes(fd,&c,1,RESPONSE_TIMEOUT)==1)
	{
		ReportCharCtrl(REPORT_DEBUG_FULL,c);
		
		if(c=='\r')
		{
			// carriage return.  start of termination sequence (do not store character)
			t=1;
		}
		else if(c=='\n')
		{
			// line feed.  end of termination sequence (if preceeded by CR), otherwise ignore
			if(t)
			{
				string[i]='\0';
				ReportString(REPORT_DEBUG_FULL," [");
				ReportStringCtrl(REPORT_DEBUG_FULL,string);
				ReportString(REPORT_DEBUG_FULL,"]\n");
				return(1);
			}
			t=0;
		}
		else if(c>=' ')
		{
			// non-control character, add to string
			string[i++]=c;
			t=0;
		}
		else
		{
			// other control character, ignore
			t=0;
		}
	}
	// failed, exit false
	return(0);
}

static int ReadStringLF(int fd, char *string)
// read a string from the device, return non-zero on success (complete string read),
// zero on failure (didn't read complete string within timeout period).  stop on the
// first LF and ignore (drop) CRs.  this will correctly stop after either CRLF or LF.
{
	unsigned char
		c;
	int
		i;

	i=0;
	ReportString(REPORT_DEBUG_FULL,"<-- ");
	while(ReadBytes(fd,&c,1,RESPONSE_TIMEOUT)==1)
	{
		ReportCharCtrl(REPORT_DEBUG_FULL,c);
		// ignore all CRs
		if(c!='\r')
		{
			// if not first character or not newline, add to buffer (throw away leading LFs)
			if(i||(c!='\n'))
			{
				string[i]=c;
				if(i&&(c=='\n'))
				{
					// not first character and is LF, zero-terminate the string.
					string[i]='\0';
					ReportString(REPORT_DEBUG_FULL," [");
					ReportStringCtrl(REPORT_DEBUG_FULL,string);
					ReportString(REPORT_DEBUG_FULL,"]\n");
					return(1);
				}
				i++;
			}
		}
	}
	// failed, exit false
	return(0);
}

static int ReadStringCR(int fd, char *string)
// read a string from the device, return non-zero on success (complete string read),
// zero on failure (didn't read complete string within timeout period).  stop on the
// first CR and ignore (drop) LFs.  this is here for completeness, nothing currently
// uses it.
{
	unsigned char
		c;
	int
		i;

	i=0;
	ReportString(REPORT_DEBUG_FULL,"<-- ");
	while(ReadBytes(fd,&c,1,RESPONSE_TIMEOUT)==1)
	{
		ReportCharCtrl(REPORT_DEBUG_FULL,c);
		// ignore all LFs
		if(c!='\n')
		{
			// if not first character or not carriage return, add to buffer (throw away leading CRs)
			if(i||(c!='\r'))
			{
				string[i]=c;
				if(i&&(c=='\r'))
				{
					// not first character and is CR, zero-terminate the string.
					string[i]='\0';
					ReportString(REPORT_DEBUG_FULL," [");
					ReportStringCtrl(REPORT_DEBUG_FULL,string);
					ReportString(REPORT_DEBUG_FULL,"]\n");
					return(1);
				}
				i++;
			}
		}
	}
	// failed, exit false
	return(0);
}

static int ReadString(int fd, char *string)
{
	switch(lineTermination)
	{
		case TERM_CR:
			return(ReadStringCR(fd,string));
		case TERM_LF:
			return(ReadStringLF(fd,string));
		case TERM_CRLF:
			return(ReadStringCRLF(fd,string));
		default:
			return(ReadStringAny(fd,string));
	}
}

static void SetControlPin(int fd, int pin, int state)
// set selected pin to selected state.  do nothing if
// pin is mapped PIN_NONE.
{
	switch(pin)
	{
		case PIN_DTR:
			SERIAL_SetDTR(fd,state);
			ReportString(REPORT_DEBUG_PROCESS," (DTR=%d)\n",state);
			break;
		case PIN_nDTR:
			SERIAL_SetDTR(fd,!state);
			ReportString(REPORT_DEBUG_PROCESS," (DTR=%d)\n",!state);
			break;
		case PIN_RTS:
			SERIAL_SetRTS(fd,state);
			ReportString(REPORT_DEBUG_PROCESS," (RTS=%d)\n",state);
			break;
		case PIN_nRTS:
			SERIAL_SetRTS(fd,!state);
			ReportString(REPORT_DEBUG_PROCESS," (RTS=%d)\n",!state);
			break;
		default:
			ReportString(REPORT_DEBUG_PROCESS," (not mapped)\n",!state);
			break;
	}
}

static void SetReset(int fd, int state)
// set the handshake line which is used for reset.  state indicates the
// state of the pin (0=low, reset asserted).
{
	ReportString(REPORT_DEBUG_PROCESS,"reset=%d",state);
	SetControlPin(fd,resetPin,state);
}

static void SetISP(int fd, int state)
// set the handshake line which is used for ISP.  state indicates the
// state of the pin (0=low, ISP asserted).
{
	ReportString(REPORT_DEBUG_PROCESS,"ISP=%d",state);
	SetControlPin(fd,ispPin,state);
}

static char *AddTermination(char *str)
// add current line termination to string.
{
	size_t
		len;
	
	len=strlen(str);
	switch(lineTermination)
	{
		case TERM_CR:
			str[len++]='\r';
			break;
		case TERM_LF:
			str[len++]='\n';
			break;
		case TERM_CRLF:
		default:
			str[len++]='\r';
			str[len++]='\n';
			break;
	}
	str[len]='\0';
	return(str);
}


static int SendCommand(int fd, const char *command, char *response, const char *match)
// send a command, return the response.
// the device echos everything we send it so watch for the same
// string coming back followed by a response string.
// if match is defined compare the response to the match string.
// return:
//   -1 on error
//   0 on success but no match
//   1 on success and match
{
	int
		fail=0;
	size_t
		len;
	char
		str[256];

	// get a copy of the string, get length without termination, add necessary termination.  Line termination isn't handled consistently across
	// parts, apply termination as set by the variable "termination"
	strcpy(str,command);
	len=strlen(str);
	AddTermination(str);

	// send string, read response
	WriteString(fd,str);
	if(ReadString(fd,response))
	{
		if(echo)
		{
			// fail if we don't see it echoed back or if we can't read another string
			fail=((strncmp(command,response,len)!=0)||!ReadString(fd,response));
		}
		if(!fail)
		{

			if(match)
			{
				// return 1 if response = match, 0 otherwise
				return(strncmp(response,match,strlen(match))==0);
			}
			return(0);
		}
	}
	return(-1);
}

static int SendCommandNoResponse(int fd, char *command)
// same as SendCommand() but expect no response other than echoed characters if enabled
{
	char
		str[256],
		response[256];
	size_t
		len;

	strcpy(str,command);
	len=strlen(str);
	AddTermination(str);

	if(WriteString(fd,str))
	{
		if(echo&&(!ReadString(fd,response)||(strncmp(command,response,len)!=0)))
		{
			return(-1);
		}
	}
	return(0);
}

//============ public functions ==============================================

void ConfigureISPHold(int state)
{
	ispHold=state;
}

void ConfigureResetPin(int state)
{
	resetPin=state;
}

void ConfigureISPPin(int state)
{
	ispPin=state;
}

int ResetTarget(int fd)
// attempt to reset target.  return -1 if reset pin is not mapped,
// 0 otherwise.
{
	if(resetPin!=PIN_NONE)
	{
		SetReset(fd,0);
		usleep(1000000);
		usleep(100000);
		SetReset(fd,1);
		return(0);
	}
	return(-1);
}

void EnterISPMode(int fd, int hold)
// enter the ISP mode.  Drive RESET and ISP low, then release RESET.
// if hold is true then leave ISP asserted on exit.
{
	ReportString(REPORT_DEBUG_PROCESS,"entering ISP mode\n");

	// echo is enabled by default, re-enable here since the part is being reset
	echo=1;

	// set ISP low, toggle reset if pin is mapped
	SetISP(fd,0);
	ResetTarget(fd);
	
	// allow target processor to boot
	usleep(100000);

	// if not asked to hold ISP low, set it back high here
	if(!hold)
	{
		SetISP(fd,1);
	}
}

void ExitISPMode(int fd)
{
	char
		response[256];

	ReportString(REPORT_DEBUG_PROCESS,"exiting ISP mode\n");

	// ensure ISP is high
	SetISP(fd,1);
	if(ResetTarget(fd)<0)
	{
		// reset is not mapped; use GO command here to try to start program
		ReportString(REPORT_DEBUG_PROCESS,"reset not mapped, attempting to jump to 0x00000000\n");
		SendCommand(fd,"G 0 A",response,"0");
	}
}

int Unlock(int fd)
// unlock flash write/erase and go commands
{
	char
		response[256];

	if(SendCommand(fd,"U 23130",response,"0")>0)
	{
		ReportString(REPORT_DEBUG_PROCESS,"unlocked\n");
		return(1);
	}
	ReportString(REPORT_DEBUG_PROCESS,"failed to unlock: %s\n",GetErrorString(response));
	return(0);
}

int SetBaudRate(int fd, int baud, int stop)
// set the baud rate and stop bits
// @@@ test me
{
	char
		buffer[256];

	sprintf(buffer,"B %d %d",baud,stop);
	switch(SendCommand(fd,buffer,buffer,"0"))
	{
		case 1:
			// success, change to new rate
			// @@@ change baud rate here
			ReportString(REPORT_DEBUG_PROCESS,"changed baud rate to %d, %d stop bits\n",baud,stop);
			return(1);
		case 0:
			// executed okay but returned an error, handle it
			ReportString(REPORT_ERROR,"set baud rate: %s\n",GetErrorString(buffer));
			break;
		default:
			// failed
			break;
	}
	return(0);
}

int Echo(int fd, int state)
// enable/disable serial echo
{
	char
		response[256];

	if(SendCommand(fd,state?"A 1":"A 0",response,"0")>=0)
	{
		ReportString(REPORT_DEBUG_PROCESS,"echo %s\n",state?"enabled":"disabled");
		echo=state;
		return(1);
	}
	return(0);
}

static int PrepareSectorsForWrite(int fd, int startSector, int endSector, int bank, partinfo_t *partInfo)
// prepare a range of sectors for erase/write.  return 1 on success, 0 on failure
{
	char
		buffer[256];

	if(startSector<=endSector)
	{
		sprintf(buffer,"P %d %d",startSector,endSector);
		if(partInfo->numBanks>1)
		{
			sprintf(buffer,"%s %d",buffer,bank);
		}
		switch(SendCommand(fd,buffer,buffer,"0"))
		{
			case 1:
				// success
				if(startSector!=endSector)
				{
					ReportString(REPORT_DEBUG_PROCESS,"prepared %d through %d for write\n",startSector,endSector);
				}
				else
				{
					ReportString(REPORT_DEBUG_PROCESS,"prepared %d for write\n",startSector);
				}
				return(1);
			case 0:
				// executed okay but returned an error, handle it
				ReportString(REPORT_ERROR,"prepare sectors for write: %s\n",GetErrorString(buffer));
				break;
			default:
				// failed
				break;
		}
	}
	return(0);
}

int CopyRAMtoFlash(int fd, unsigned int src, unsigned int dest, unsigned int length)
// write a block of RAM to flash.
// @@@ should qualify the arguments
{
	char
		buffer[256];

	sprintf(buffer,"C %d %d %d",dest,src,length);
	switch(SendCommand(fd,buffer,buffer,"0"))
	{
		case 1:
			// success
			ReportString(REPORT_DEBUG_PROCESS,"wrote %d bytes from 0x%08x to 0x%08x\n",length,src,dest);
			return(1);
		case 0:
			// executed okay but returned an error, handle it
			ReportString(REPORT_ERROR,"copy RAM to flash: %s\n",GetErrorString(buffer));
			break;
		default:
			// failed
			break;
	}
	return(0);
}

int Erase(int fd, int startSector, int endSector, int bank, partinfo_t *partInfo)
// erase a range of sectors.  return 1 on success, 0 on failure
{
	char
		buffer[256];

	if(startSector<=endSector)
	{
		if(PrepareSectorsForWrite(fd,startSector,endSector,0,partInfo))	// @@@ only support bank 0 for now
		{
			sprintf(buffer,"E %d %d",startSector,endSector);
			if(partInfo->numBanks>1)
			{
				sprintf(buffer,"%s %d",buffer,bank);
			}
			switch(SendCommand(fd,buffer,buffer,"0"))
			{
				case 1:
					// success
					ReportString(REPORT_INFO,"erased\n");
					return(1);
				case 0:
					// executed okay but returned an error, handle it
					ReportString(REPORT_ERROR,"erase: %s\n",GetErrorString(buffer));
					break;
				default:
					// failed
					break;
			}
		}
	}
	return(0);
}

int BlankCheck(int fd, int startSector, int endSector)
// blank check a range of sectors.  return 1 on success, 0 on not blank, -1 on error.
{
	char
		buffer[256];
	int
		addr,
		value;

	if(startSector<=endSector)
	{
		sprintf(buffer,"I %d %d",startSector,endSector);
		switch(SendCommand(fd,buffer,buffer,"0"))
		{
			case 1:
				// success
				ReportString(REPORT_DEBUG_PROCESS,"blank\n");
				return(1);
			case 0:
				// executed okay but returned an error, handle it
				ReportString(REPORT_ERROR,"blank check: %s\n",GetErrorString(buffer));
				if(atoi(buffer)==RTN_SECTOR_NOT_BLANK)
				{
					if(ReadString(fd,buffer))
					{
						addr=atoi(buffer);
						if(ReadString(fd,buffer))
						{
							value=atoi(buffer);
							ReportString(REPORT_ERROR,"  0x%08x: 0x%08x\n",addr,value);
						}
					}
				}
				break;
			default:
				// failed
				break;
		}
	}
	return(-1);
}

unsigned int ReadPartID(int fd, unsigned int *id1)
// read and return part ID or ~0 on error
// some parts (LPC18xxx) return an extra byte
// for the part ID.  to cope with this we try
// to read the extra byte but don't flag an
// error if we time out waiting for it.
{
	char
		buffer[256];
	int
		id;

	*id1=0;
	switch(SendCommand(fd,"J",buffer,"0"))
	{
		case 1:
			// success
			if(ReadString(fd,buffer))
			{
				id=atoi(buffer);
				if(id)
				{
					if(ReadString(fd,buffer))
					{
						*id1=atoi(buffer);
					}
					return(id);
				}
			}
			break;
		case 0:
			// executed okay but returned an error, handle it
			ReportString(REPORT_ERROR,"read part ID: %s\n",GetErrorString(buffer));
			break;
		default:
			// failed
			break;
	}
	return(~0);
}

int ReadPartUID(int fd, unsigned int *uid)
// read and return 0 on success or -1 on error.  uid must point to an array of four 32-bits each.
// not all parts have a UID; will return -1 if unsupported.
{
	char
		buffer[256];
	int
		i;

	switch(SendCommand(fd,"N",buffer,"0"))
	{
		case 1:
			// success
			for(i=0;i<4;i++)
			{
				if(ReadString(fd,buffer))
				{
					uid[3-i]=atoll(buffer);
				}
				else
				{
					break;
				}
			}
			if(i==4)
			{
				return(0);
			}
			break;
		case 0:
			// executed okay but returned an error, handle it
			ReportString(REPORT_ERROR,"read part UID: %s\n",GetErrorString(buffer));
			break;
		default:
			// failed
			break;
	}
	return(-1);

}

int ReadBootCode(int fd, unsigned char *major, unsigned char *minor)
// read and return boot code version.  return 0 on success or -1 on error
{
	char
		buffer[256];

	switch(SendCommand(fd,"K",buffer,"0"))
	{
		case 1:
			// success
			if(ReadString(fd,buffer))
			{
				*major=atoi(buffer);
				if(ReadString(fd,buffer))
				{
					*minor=atoi(buffer);
					return(0);
				}
			}
			break;
		case 0:
			// executed okay but returned an error, handle it
			ReportString(REPORT_ERROR,"read boot code: %s\n",GetErrorString(buffer));
			break;
		default:
			// failed
			break;
	}
	return(-1);
}

int WriteRAMAddress(int fd, unsigned char *data, unsigned int addr, unsigned int count)
// prepare to write to RAM.  send the address and the count.  return
// 0 on success, -1 on error.
{
	char
		buffer[256];

	if((count&0x03)==0)
	{
		sprintf(buffer,"W %d %d",addr,count);
		switch(SendCommand(fd,buffer,buffer,"0"))
		{
			case 1:
				// success, may start sending data
				ReportString(REPORT_DEBUG_PROCESS,"%s: set address = 0x%04x, count = %d\n",__FUNCTION__,addr,count);
				return(0);
			case 0:
				// got a response other than '0', @@@ handle error
				// @@@ possible values are ADDR_ERROR, ADDR_NOT_MAPPED, COUNT_ERROR,
				// @@@ PARAM_ERROR, CODE_READ_PROTECTION_ENABLED
				ReportString(REPORT_ERROR,"write RAM address: %s\n",GetErrorString(buffer));
				break;
			default:
				// no response, handle error
				ReportString(REPORT_ERROR,"%s: no response\n",__FUNCTION__);
				break;
		}
	}
	else
	{
		ReportString(REPORT_ERROR,"%s: count (%d) must be a multiple of four\n",__FUNCTION__,count);
	}
	return(-1);
}

int ReadFromTarget(int fd, unsigned char *data, unsigned int addr, unsigned int count, partinfo_t *partInfo)
// read a block of data from RAM.
{

	char
		buffer[256];
	unsigned int
		n,
		cs,
		numRead,
		lineCnt;

	sprintf(buffer,"R %d %d",addr,count);
	switch(SendCommand(fd,buffer,buffer,"0"))
	{
		case 1:
			// success, may start reading data
			cs=0;
			lineCnt=0;
			while(count)
			{
				if(partInfo->flags&UUENCODE)
				{
					// read block of uuencoded data
					numRead=ReadString(fd,buffer);
					if(numRead)
					{
						lineCnt++;
						n=uudecode(buffer,data);
						cs+=GetCheckSum(data,n);
						count-=n;
						data+=n;
						if((count==0)||(lineCnt==20))
						{
							if(ReadString(fd,buffer))
							{
								if(cs==atoi(buffer))
								{
									SendCommandNoResponse(fd,"OK");
									ReportString(REPORT_DEBUG_PROCESS,"checksum okay\n");
									cs=0;
									lineCnt=0;
									if(count==0)
									{
										return(1);
									}
								}
								else
								{
									// report error and stop
									ReportString(REPORT_ERROR,"checksum error\n");
									count=0;
								}
							}
						}
					}
				}
				else
				{
					// read block of binary data
					numRead=ReadBuffer(fd,data,MIN(64,count));
					if(numRead)
					{
						count-=numRead;
						data+=numRead;
						if(count==0)
						{
							return(1);
						}
					}
				}

				if(numRead==0)
				{
					// report error and stop
					ReportString(REPORT_ERROR,"failed to read from target\n");
					count=0;
				}
			}
			break;;
		case 0:
			// got a response other than '0', @@@ handle error
			// @@@ possible values are ADDR_ERROR, ADDR_NOT_MAPPED, COUNT_ERROR,
			// @@@ PARAM_ERROR, CODE_READ_PROTECTION_ENABLED
			ReportString(REPORT_ERROR,"read from target: %s\n",GetErrorString(buffer));
			break;
		default:
			// no response, handle error
			ReportString(REPORT_ERROR,"%s: no response\n",__FUNCTION__);
			break;
	}
	return(0);

}

int WriteToRAM(int fd, unsigned char *data,partinfo_t *partInfo)
// write a block of data to RAM.
//   data -- pointer to data
//   addr -- start address in device (must be a multiple of four)
//   length -- number of bytes to write (must be a multiple of four)
// return 1 on success, 0 on error
{
	int
		fail;
	unsigned int
		addr,
		length,
		bytesThisBlock,
		bytesRemaining,
		n,
		resend,
		offset,
		cs;
	char
		buffer[64];

	addr=partInfo->flashBlockRAMBase;
	length=partInfo->flashBlockSize;
	fail=0;
	if((fd>=0)&&data&&((addr&3)==0)&&((length&3)==0))
	{
		// everything appears valid, try to write data to RAM
		resend=5;
		while(!fail&&length&&resend)
		{
			// set the address and count (never write more than 128 bytes in a single block)
			bytesThisBlock=MIN(length,512);
			bytesRemaining=bytesThisBlock;
			offset=0;
			if(resend==5)
			{
				fail=(WriteRAMAddress(fd,data,addr,bytesThisBlock)<0);
			}
			if(!fail)
			{
				cs=0;
				while(bytesRemaining&&!fail)
				{
					// write up to 45 bytes in a line
					n=MIN(bytesRemaining,45);
					cs+=GetCheckSum(&data[offset],n);
					if(partInfo->flags&UUENCODE)
					{
						uuencode(&data[offset],(unsigned char *)buffer,n);
						fail=(SendCommandNoResponse(fd,buffer)<0);
					}
					else
					{
						fail=!WriteBuffer(fd,&data[offset],n);
					}
					if(!fail)
					{
						bytesRemaining-=n;
						offset+=n;
					}
				}
			}
			if(partInfo->flags&UUENCODE)
			{
				if(!fail)
				{
					// seem to have written up to 128 bytes okay, send the checksum
					sprintf(buffer,"%d",cs);
					fail=(SendCommandNoResponse(fd,buffer)<0);
					if(!fail)
					{
						do
						{
							n=ReadString(fd,buffer);
							// the response is very inconsistent.  sometimes it echos even when echo is disabled,
							// sometimes it doesn't.  pull lines until we see either "OK", "RESEND", or timeout.
							fail=(n==0);
							if(!fail)
							{
								if(strncmp(buffer,"OK",2)==0)
								{
									// success, reset the resend counter for the next block, break out of here
									resend=5;
									data+=bytesThisBlock;
									addr+=bytesThisBlock;
									length-=bytesThisBlock;
									n=0;
								}
								else if(strncmp(buffer,"RESEND",6)==0)
								{
									// target is asking to resend the block, break out of here and try again if resend hasn't counted out.
									resend--;
									n=0;
								}
								else
								{
									// else ignore anything else that came back
									ReportString(REPORT_DEBUG_FULL,"unknown response \"%s\"\n",buffer);
								}
							}
							else
							{
								ReportString(REPORT_DEBUG_FULL,"no response\n");
							}
						}while(n);
					}
				}
			}
			else
			{
				data+=bytesThisBlock;
				addr+=bytesThisBlock;
				length-=bytesThisBlock;
				n=0;
			}
		}
	}
	return(!fail);
}

int WriteToFlash(int fd,unsigned char *data,unsigned int addr,unsigned int length,partinfo_t *partInfo)
// write 256 bytes of flash at a time (the minimum block size)
//   data = image data
//   addr = start address (where first byte of data maps into target device)
//   length = number of bytes to write
//   partInfo = pointer to structure which describes the identified device
// @@@ this will currently fail if addr isn't page-aligned @@@
{
	unsigned int
		bytesToWrite;
	int
		fail;
	unsigned int
		curSector;
	unsigned char
		*buffer;	// use a secondary buffer so we can fill the tail of short writes with 0xff

	fail=0;
	ReportString(REPORT_INFO,"writing... 0x%08x",addr);

	buffer=(unsigned char *)malloc(partInfo->flashBlockSize);
	if(buffer)
	{
		while(!fail&&length)
		{
			bytesToWrite=MIN(length,partInfo->flashBlockSize);
			memcpy(buffer,data,bytesToWrite);
			memset(&buffer[bytesToWrite],0xff,partInfo->flashBlockSize-bytesToWrite);
			if(WriteToRAM(fd,buffer,partInfo))
			{
				ReportString(REPORT_DEBUG_PROCESS,"copied %d bytes into RAM for 0x%08x\n",bytesToWrite,addr);
				// prepare the sector for writing
				curSector=GetSectorAddr(addr,partInfo);	// get number of the sector containing address of this block
				if(PrepareSectorsForWrite(fd,curSector,curSector,0,partInfo))	// @@@ only support bank 0 for now
				{
					// write to flash
					if(CopyRAMtoFlash(fd,partInfo->flashBlockRAMBase,addr,partInfo->flashBlockSize))
					{
						// prepare for next
						length-=bytesToWrite;
						data+=bytesToWrite;
						addr+=bytesToWrite;
						ReportString(REPORT_INFO,"\b\b\b\b\b\b\b\b%08x",addr);
					}
					else
					{
						ReportString(REPORT_ERROR,"failed to write %d bytes from 0x%08x to 0x%08x\n",bytesToWrite,partInfo->flashBlockRAMBase,addr);
						fail=1;
					}

				}
				else
				{
					ReportString(REPORT_ERROR,"failed to prepare to write to sector %d\n",curSector);
					fail=1;
				}
			}
			else
			{
				ReportString(REPORT_ERROR,"failed to copy %d bytes to RAM\n",bytesToWrite);
				fail=1;
			}
		}
		free(buffer);
	}
	ReportString(REPORT_INFO,"\n");
	return(!fail);
}

int Sync(int fd, int freq,int retries)
// freq=frequency in Hz
// perform synchronization sequence
//   --> ?
//   <-- Synchronized
//   --> Synchronized
//   <-- OK
//   --> 12000 (crystal frequency in kHz, decimal)
//   <-- OK
{
	char
		buffer[256];
	int
		retry;

	ReportString(REPORT_INFO,"synchronizing...");
	retry=retries;
	while(retry--)
	{
		// enter ISP mode (drive ISP low, toggle reset low then high, if possible)
		EnterISPMode(fd,ispHold);

		// send autobaud character ('?')
		WriteChar(fd,'?');
		if(ReadString(fd,buffer))
		{
			// expect to read back "Synchronized"
			if(strncmp(buffer,"Synchronized",12)==0)
			{
				if(SendCommand(fd,"Synchronized",buffer,"OK")>0)
				{
						// send frequency in kHz
					sprintf(buffer,"%d",freq/1000);
					if(SendCommand(fd,buffer,buffer,"OK")>0)
					{
						ReportString(REPORT_INFO,"done\n");
						return(1);
					}
				}
			}
		}
		ReportString(REPORT_INFO,".");
	}
	return(0);
}
