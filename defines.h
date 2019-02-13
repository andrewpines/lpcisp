
#ifdef __WIN__
// Windows' Sleep() function is in milliseconds, divide by 1000 to go from microseconds to milliseconds.  Accurate
// enough for our purposes.
#define usleep(x) Sleep((x)/1000)
#endif

#define RTN_CMD_SUCCESS								0
#define RTN_INVALID_COMMAND							1
#define RTN_SRC_ADDR_ERROR							2
#define RTN_DST_ADDR_ERROR							3
#define RTN_SRC_ADDR_NOT_MAPPED						4
#define RTN_DST_ADDR_NOT_MAPPED						5
#define RTN_COUNT_ERROR								6
#define RTN_INVALID_SECTOR							7
#define RTN_SECTOR_NOT_BLANK						8
#define RTN_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION	9
#define RTN_COMPARE_ERROR							10
#define RTN_BUSY									11
#define RTN_PARAM_ERROR								12
#define RTN_ADDR_ERROR								13
#define RTN_ADDR_NOT_MAPPED							14
#define RTN_CMD_LOCKED								15
#define RTN_INVALID_CODE							16
#define RTN_INVALID_BAUD_RATE						17
#define RTN_INVALID_STOP_BIT						18
#define RTN_CODE_READ_PROTECTION_ENABLED			19

#define ARRAY_SIZE(x)		(sizeof(x)/sizeof((x)[0]))
#define MIN(a,b)			(((a)<(b))?(a):(b))
