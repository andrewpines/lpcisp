#
#	  lpcisp
#

APP=lpcisp

.PHONY:	install

CFLAGS=-O2 -Wall

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
	@rm -f *.o $(APP)

install:
	cp $(APP) /usr/local/bin/.
	
