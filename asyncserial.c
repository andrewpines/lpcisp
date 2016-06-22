
#include "includes.h"


#ifdef __OSX__
#include <IOKit/serial/ioss.h>
#endif

#define MIN_CHARS		1		// DEBUG something is amiss with this, if VTIME is non-zero we get EAGAIN returned instead of zero (and no delay)
#define CHAR_TIMEOUT	0		// character timeout (read fails and returns if this much time passes without a character) in 1/10's sec

static int SerialByteWaiting(int fd,unsigned int timeOut)
// See if there is unread data waiting on device.
// This is used to poll device without reading any characters
// from it.
// As soon as there is data, or timeOut expires, return.
// NOTE: timeOut is measured in microseconds.
// if timeOut is 0, this will return the status immediately.
{
	fd_set
		readSet;
	struct timeval
		timeVal;

	FD_ZERO(&readSet);					// clear the set
	FD_SET(fd,&readSet);				// add this descriptor to the set
	timeVal.tv_sec=timeOut/1000000;		// set up the timeout waiting for one to come ready
	timeVal.tv_usec=timeOut%1000000;
	return(select(FD_SETSIZE,&readSet,NULL,NULL,&timeVal)==1);	// if our descriptor is ready, then report it
}

int ReadBytes(int fd,unsigned char *buf,unsigned int maxBytes,unsigned int timeOut)
// Attempt to read the given number of bytes before timeout (uS) occurs during any read attempt.
// Return the number of bytes received or -1 on error.  if maxBytes is zero will simply return zero.
//  fd -- file descriptor of serial device
//  buf -- pointer to buffer to be filled
//  maxBytes -- max number of bytes to read
//  timeout -- time to wait on any read, in microseconds
{
	int
		numRead;
	unsigned int
		readSoFar;

	readSoFar=0;
	numRead=0;
	while((numRead>=0)&&(readSoFar<maxBytes)&&SerialByteWaiting(fd,timeOut))
	{
		if((numRead=read(fd,&buf[readSoFar],maxBytes-readSoFar))>0)
		{
			readSoFar+=numRead;
		}
	}

	return(numRead>=0?readSoFar:numRead);
}

unsigned int WriteBytes(int fd,const unsigned char *buf,unsigned int numBytes)
// Write bytes to device.  return number of bytes written (should be same as numBytes unless error).
//  fd -- file descriptor
//  buf -- pointer to buffer to be sent
//  numBytes -- number of bytes to send
{
	return(write(fd,buf,numBytes));
}

void SERIAL_SetDTR(int fd, int state)
// Set the state of the DTR handshake line
{
	int
		flags;

	ioctl(fd,TIOCMGET,&flags);
	if(state)
	{
		flags|=TIOCM_DTR;
	}
	else
	{
		flags&=~TIOCM_DTR;
	}
	ioctl(fd,TIOCMSET,&flags);
}

void SERIAL_SetRTS(int fd, int state)
// Set the state of the RTS handshake line
{
	int
		flags;

	ioctl(fd,TIOCMGET,&flags);
	if(state)
	{
		flags|=TIOCM_RTS;
	}
	else
	{
		flags&=~TIOCM_RTS;
	}
	ioctl(fd,TIOCMSET,&flags);
}

typedef struct
{
	int rate;			// numerical baud rate
	speed_t code;		// code for baud rate
}baud_t;

static const baud_t baudRates[] =
{
	{50,B50},
	{75,B75},
	{110,B110},
	{134,B134},
	{150,B150},
	{200,B200},
	{300,B300},
	{600,B600},
	{1200,B1200},
	{1800,B1800},
	{2400,B2400},
	{4800,B4800},
	{9600,B9600},
	{19200,B19200},
	{38400,B38400},
	{115200,B115200},
	{230400,B230400},
};

static int GetBaudCode(unsigned int baud)
// get the code for a specified baud rate
{
	int
		i;

	for(i=0;i<ARRAY_SIZE(baudRates);i++)
	{
		if(baudRates[i].rate==baud)
		{
			return(baudRates[i].code);
		}
	}
	return(-1);
}

int ChangeBaudRate(int fd, int baud)
{
	struct termios
		terminalParams;			// parameters for the serial port
	int
		baudCode;

	baudCode=GetBaudCode(baud);
	if(baudCode>=0)
	{
		if(tcgetattr(fd,&terminalParams)>=0)
		{
			cfsetspeed(&terminalParams,baudCode);		// Set baud
			if(tcsetattr(fd,TCSANOW,&terminalParams)>=0)
			{
				return(0);
			}
		}
	}
	else
	{
#ifdef __OSX__
		// if OSX, set arbitrary baud rate
		if(ioctl(fd,IOSSIOSPEED,&baud)>=0)	
		{
			return(0);
		}
#endif // __OSX__
		ReportString(REPORT_ERROR,"invalid baud rate: %d\n",baud);
	}
	return(-1);
}

int OpenDevice(char *name)
// open the device that we are going to communicate through
// set it to the passed baud rate, raw mode
// return descriptor number on success, -1 on error
{
	int
		fd;
	struct termios
		terminalParams;			// parameters for the serial port

	fd=open(name,O_NDELAY|O_RDWR|O_NOCTTY|O_NONBLOCK);
	if(fd>=0)
	{
		cfmakeraw(&terminalParams);					// start with basic raw settings
		terminalParams.c_cflag=(CREAD|CLOCAL|CS8);
//		terminalParams.c_cflag|=(CREAD|CLOCAL|CS8);
//		terminalParams.c_cflag&=~CRTSCTS;			// clear flow control (no hardware handshake)
		terminalParams.c_iflag&=~(IXON|IXOFF);		// disable XON/XOFF
		terminalParams.c_cc[VMIN]=MIN_CHARS;		// read returns immediately if no characters
		terminalParams.c_cc[VTIME]=CHAR_TIMEOUT;
		cfsetspeed(&terminalParams,GetBaudCode(115200));	// Set default baud (can't fail)
		if(tcsetattr(fd,TCSANOW,&terminalParams)>=0)
		{
			tcflow(fd,TCOON);
			tcflush(fd,TCIOFLUSH);
			return(fd);
		}
		else
		{
			ReportString(REPORT_ERROR,"failed to set terminal parameters for device '%s'\n",name);
		}
		close(fd);
	}
	else
	{
		ReportString(REPORT_ERROR,"failed to open device '%s': %s\n",name,strerror(errno));
	}
	return(-1);
}


void CloseDevice(int fd)
// close the device that was opened by OpenDevice
{
	close(fd);
}
