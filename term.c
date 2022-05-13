
#include "includes.h"

static char
	resetCode='\033',		// if this character is encountered from the keyboard, reset the target (ESC)
	exitCode='\030';		// if this character is encountered from the keyboard, exit (ctrl-x)

#if defined __UNIX__

#define TTYIN	0
#define TTYOUT	1

#define MIN_CHARS		1		// DEBUG something is amiss with this, if VTIME is non-zero we get EAGAIN returned instead of zero (and no delay)
#define CHAR_TIMEOUT	0		// character timeout (read fails and returns if this much time passes without a character) in 1/10's sec

static int
	done;

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

static void WriteDevice(int theTTY,int fd, lpcispcfg_t *cfg)
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
					// reset the target, if able
					if(LPCISP_ResetTarget(fd,cfg)>=0)
					{
						action=1;
						ReportString(REPORT_INFO,"\n   resetting target...\n");
					}
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

static void DoTasks(int fd, lpcispcfg_t *cfg)
{
	int
		processID;

	processID=fork();
	if(processID>=0)
	{
		if(processID)				// =0 for the child, =PID of the child for the parent
		{
			WriteDevice(TTYIN,fd,cfg);	// will run until done=true
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

int Terminal(int fd, lpcispcfg_t *cfg)
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
			DoTasks(fd,cfg);
			if(tcsetattr(TTYIN,TCSANOW,&oldParams)>=0)
			{
				ReportString(REPORT_INFO,"\n   exiting terminal...\n");
				return(1);
			}
		}
	}
	return(0);
}

#elif defined __WIN__	// Windows
int Terminal(int fd, lpcispcfg_t *cfg)
{
	int
		i,
		numRead,
		c;
	unsigned char
		buf[256];

	ReportString(REPORT_INFO,"entered terminal mode.  type <ctrl-X> to exit, <ESC> to reset the target\n");
	do
	{
		// if one or more characters come in from serial port display to console
		numRead=LPCISP_SERIAL_ReadBytes(fd,buf,256,0);
		if(numRead>0)
		{
			write(1,buf,numRead);
			fflush(stdout);
		}
		// read character from stdin, send to serial device
		if(kbhit())
		{
			c=getch();
			if(c==resetCode)
			{
				// reset the target, if able
				if(LPCISP_ResetTarget(fd,cfg)>=0)
				{
					ReportString(REPORT_INFO,"\n   resetting target...\n");
				}
			}
			else if(c>=0)
			{
				buf[0]=c;
				LPCISP_SERIAL_WriteBytes(fd,buf,1);
			}
		}
	}while(c!=exitCode);
	ReportString(REPORT_INFO,"\n   exiting terminal...\n");
	return(1);
}
#endif
