
#include "includes.h"

static const char
	*errorCodes[]=
	{
		"success",
		"invalid command",
		"source address error",
		"destination address error",
		"source address not mapped",
		"destination address not mapped",
		"count error",
		"invalid sector",
		"sector not blank",
		"sector not prepared for write operation",
		"compare error",
		"busy",
		"param error",
		"address error",
		"address not mapped",
		"command locked",
		"invalid code",
		"invalid baud rate",
		"invalid stop bit",
		"code read protection enabled",
	};

static int
	reportLevel=REPORT_INFO;

const char *GetErrorString(const char *s)
// report error code
{
	int
		code;
	static char
		string[256];

	code=atoi(s);
	if(code<=ARRAY_SIZE(errorCodes))
	{
		return(errorCodes[code]);
	}
	sprintf(string,"unknown response code: \"%s\"\n",s);
	return(string);
}


void ReportString(int level,const char *format,...)
// print a diagnostic message if level<=reportLevel
{
	va_list
		p;

	if(level<=reportLevel)
	{
		va_start(p,format);     // Initialize start of variable-length argument list
		vprintf(format,p);
		va_end(p);
		fflush(stdout);
	}
}

void ReportChar(int level,const char c)
// print a character if level<=reportLevel
{
	if(level<=reportLevel)
	{
		fprintf(stderr,"%c",c);
	}
}

void ReportBufferCtrl(int level,const unsigned char *buffer, unsigned int length)
// print a diagnostic message, if enabled.  convert control characters to printable, e.g,
// control-C becomes <03>
{
	ReportChar(level,'[');
	ReportString(level,"(%d) ",length);
	while(length--)
	{
		ReportString(level,"%02x ",(unsigned char)*buffer);
		buffer++;
	}
	ReportString(level,"]\n");
}

void ReportCharCtrl(int level, const char c)
{
	if((c>=' ')&&(c<='~'))
	{
		ReportChar(level,c);
	}
	else
	{
		ReportString(level,"<%02x>",c);
	}
}

void ReportStringCtrl(int level,const char *string)
// print a diagnostic message, if enabled.  convert control characters to printable, e.g,
// control-C becomes <03>
{
	while(*string)
	{
		ReportCharCtrl(level,*string);
		string++;
	}
}


void SetReportLevel(int level)
{
	reportLevel=level;
}

