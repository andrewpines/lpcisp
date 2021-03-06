
// standard C headers
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

// UNIX-specific headers
#ifdef __UNIX__
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <libgen.h>
#endif

// Windows-specific headers
#ifdef __WIN__
#include <windows.h>
#include <conio.h>
#endif

// local headers
#include "defines.h"
#include "lpcisp.h"
#include "asyncserial.h"
#include "uuencode.h"
#include "partdesc.h"
#include "report.h"
#include "term.h"

