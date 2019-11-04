#
#	  lpcisp
#
#  Use this Makefile for Unix-based systems
#
#  invoke as:
#    make clean
#    make
#

APP=lpcisp

.PHONY:	install

CFLAGS=-O2 -Wall -D__UNIX__

ifeq ($(OSTYPE),)
OSTYPE		= $(shell uname)
endif

ifneq ($(findstring Darwin,$(OSTYPE)),)
CFLAGS		+= -D__OSX__
endif

OBJECTS = \
	main.o \
	asyncserial.o \
	uuencode.o \
	ihex.o \
	partdesc.o \
	report.o \
	isp.o \
	term.o

all : $(APP)

$(APP) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(APP)

clean :
	@rm -f *.o *.obj $(APP) $(APP).exe

install:
	@cp $(APP) /usr/local/bin/.
