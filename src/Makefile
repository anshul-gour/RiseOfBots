# RISE OF BOTS Makefile

LIBDIR= -L/usr/X11R6/lib
DEFINE= -DLINUX
CC= gcc ${DEFINE} ${LIBDIR}
GLIBS= -lGL -lGLU -lglut
CURSES= -lcurses
LIBS= -lm -lnsl

all: combat display mapediter
	cp combat ..
	cp display ..
	cp mapediter ..
	rm combat
	rm display
	rm mapediter
	rm *.o

text: combat textdisplay textmapediter
	cp combat ..
	cp display ..
	cp mapediter ..
	rm combat
	rm display
	rm mapediter
	rm *.o

combat:	combat.o OsDeps.o
	${CC} -o combat combat.o OsDeps.o ${LIBS}

textdisplay:textdisplay.o OsDeps.o
	${CC} -o display textdisplay.o OsDeps.o ${CURSES} ${LIBS}

textmapediter:textmapediter.o
	${CC} -o mapediter textmapediter.o ${CURSES}

display:display.o OsDeps.o
	${CC} -o display display.o OsDeps.o ${GLIBS} ${LIBS}

mapediter:mapediter.o
	${CC} -o mapediter mapediter.o ${GLIBS}

OsDeps.o: OsDeps.c
	${CC} -c -o OsDeps.o OsDeps.c

combat.o: combat.c dispcomm.h objects.h
	${CC} -c -o combat.o combat.c

textdisplay.o: textdisplay.c dispcomm.h objects.h
	${CC} -c -o textdisplay.o textdisplay.c

textmapediter.o: textmapediter.c
	${CC} -c -o textmapediter.o textmapediter.c

display.o: display.c dispcomm.h objects.h
	${CC} -c -o display.o display.c

mapediter.o: mapediter.c
	${CC} -c -o mapediter.o mapediter.c

clean:
	rm -r *.o
