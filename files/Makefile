# Makes all the samples

CC=g++ -DLINUX -I../src

all: aibots OsDeps
	
aibots: aibots.c aibots.h
	${CC} -c -o aibots.o aibots.c

OsDeps: OsDeps.c OsIncl.h
	${CC} -c -o OsDeps.o OsDeps.c
