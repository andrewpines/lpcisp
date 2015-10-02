
#include "includes.h"

#define TTYIN	0
#define TTYOUT	1

#define MIN_CHARS		1		// DEBUG something is amiss with this, if VTIME is non-zero we get EAGAIN returned instead of zero (and no delay)
#define CHAR_TIMEOUT	0		// character timeout (read fails and returns if this much time passes without a character) in 1/10's sec

static int
	done;

static char
	resetCode='\033',		// if this character is encountered from the keyboard, reset the target (ESC)
	exitCode='\030';		// if this character is encountered from the keyboard, exit (ctrl-x)

static void ReadDevice(int fd,int theTTY)
// read the serial device, send characters to the tty
{
	char
		buffer[2048];
	int
		numRead;

	while(1)
	{
		numRead=read(fd,buffer,2048);
		if(numRead>0)
		{
			// read return value to silence the compiler
			numRead=write(theTTY,buffer,numRead);
		}
	}
}

static void WriteDevice(int theTTY,int fd)
// read the tty, send characters to the serial device
{
	char
		buffer[2048];
	int
		action,		// indicates if external action is to be taken -- don't send the command characters to the device
		numRead,
		idx;

	while(!done)
	{
		// read some characters from the tty
		numRead=read(theTTY,buffer,2048);
		if(numRead>0)
		{
			// scan for the exit and reset codes
			action=0;
			for(idx=0;!done&&idx<numRead;idx++)
			{
				if(buffer[idx]==exitCode)
				{
					action=1;
					done=1;
				}
				if(buffer[idx]==resetCode)
				{
					// reset the target
					action=1;
					ReportString(REPORT_INFO,"\n   resetting target...\n");
					ExitISPMode(fd);
				}
			}
			// send characters to the serial device (read return value to silence the compiler)
			if(!action)
			{
				numRead=write(fd,buffer,numRead);
			}
		}
	}
}

static void DoTasks(int fd)
{
	int
		processID;

	processID=fork();
	if(processID>=0)
	{
		if(processID)				// =0 for the child, =PID of the child for the parent
		{
			WriteDevice(TTYIN,fd);	// will run until done=true
			if(kill(processID,SIGKILL)<0)
			{
				ReportString(REPORT_ERROR,"terminal: failed to kill sub-task\n");
			}
		}
		else
		{
			ReadDevice(fd,TTYOUT);		// child task, will run until done=true
		}
	}
	else
	{
		ReportString(REPORT_ERROR,"terminal: failed to fork\n");
	}
}

int Terminal(int fd)
{
	struct termios
		params,
		oldParams;

	done=0;
	ReportString(REPORT_INFO,"entered terminal mode.  type <ctrl-X> to exit, <ESC> to reset the target\n");
	if(tcgetattr(TTYIN,&oldParams)>=0)
	{
		params=oldParams;
		params.c_iflag=ISTRIP;
		params.c_oflag=OPOST|ONLCR;
		params.c_lflag=0;
		params.c_cc[VMIN]=1;			// at least 1 character in
		params.c_cc[VTIME]=1;			// or, 1/10th of a second
		if(tcsetattr(TTYIN,TCSANOW,&params)>=0)
		{
			DoTasks(fd);
			if(tcsetattr(TTYIN,TCSANOW,&oldParams)>=0)
			{
				ReportString(REPORT_INFO,"\n   exiting terminal...\n");
				return(1);
			}
		}
	}
	return(0);
}
