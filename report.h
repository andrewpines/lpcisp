
enum
{
	REPORT_ERROR=0,
	REPORT_MINIMUM,
	REPORT_INFO,
	REPORT_DEBUG_PROCESS,
	REPORT_DEBUG_FULL,
};

const char *GetErrorString(const char *s);
void ReportCharCtrl(int level, const char c);
void ReportStringCtrl(int level,const char *string);
void ReportChar(int level,const char c);
void ReportString(int level,const char *format,...);
void ReportBufferCtrl(int level,const unsigned char *buffer, unsigned int length);
void ReportStringCtrl(int level,const char *string);
void SetReportLevel(int level);
