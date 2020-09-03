#
#	  lpcisp
#
#  Use this Makefile for Unix-based systems (see Makefile.vc to build for Windows)
#
#  invoke as:
#    make clean
#    make
#
#  builds two parts:
#    1) library which can be linked into a user application
#    2) front end for stand-alone application
#

APP=lpcisp

.PHONY:	install

CFLAGS=-O2 -Wall -D__UNIX__

ifeq ($(OSTYPE),)
OSTYPE		= $(shell uname)
endif

# detect if host is OSX (supports arbitrary baud rates, see asyncserial.c)
ifneq ($(findstring Darwin,$(OSTYPE)),)
CFLAGS		+= -D__OSX__
endif

LIBOJBECTS =		\
	asyncserial.o	\
	ihex.o			\
	isp.o			\
	partdesc.o		\
	report.o		\
	uuencode.o

OBJECTS =			\
	main.o			\
	term.o

all : lib$(APP).a $(APP)

%.o : %.c
	@echo "  compiling" $< to $@
	@$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

lib$(APP).a : $(LIBOJBECTS)
	@echo "  building library lib$(APP).a"
	@ar rcs lib$(APP).a $(LIBOJBECTS)

$(APP) : $(OBJECTS) lib$(APP).a
	@echo "  linking $(APP)"
	@$(CC) $(OBJECTS) lib$(APP).a -o $(APP)

clean :
	@rm -f *.a *.o *.obj $(APP) $(APP).exe

install:
	@cp $(APP) /usr/local/bin/.
