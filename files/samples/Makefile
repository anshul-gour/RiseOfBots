# Makes all the samples

CC=g++ -DLINUX
LIBS= -lnsl -lm

all: BlackCats CheckWall Fire UpgradeEdan Messager UpgradeNost Power ResourceLover

BlackCats: aibots.o OsDeps.o BlackCats.c
	${CC} -o../../BlackCats BlackCats.c aibots.o OsDeps.o $(LIBS)
CheckWall: aibots.o OsDeps.o CheckWall.c
	${CC} -o../../CheckWall CheckWall.c aibots.o OsDeps.o $(LIBS)
Fire: aibots.o OsDeps.o Fire.c
	${CC} -o../../Fire Fire.c aibots.o OsDeps.o $(LIBS)
UpgradeEdan: aibots.o OsDeps.o UpgradeEdan.c
	${CC} -o../../UpgradeEdan UpgradeEdan.c aibots.o OsDeps.o $(LIBS)
Messager: aibots.o OsDeps.o Messager.c
	${CC} -o../../Messager Messager.c aibots.o OsDeps.o $(LIBS)
UpgradeNost: aibots.o OsDeps.o UpgradeNost.c
	${CC} -o../../UpgradeNost UpgradeNost.c aibots.o OsDeps.o $(LIBS)
Power: aibots.o OsDeps.o Power.c
	${CC} -o../../Power Power.c aibots.o OsDeps.o $(LIBS)
ResourceLover: aibots.o OsDeps.o ResourceLover.c
	${CC} -o../../ResourceLover ResourceLover.c aibots.o OsDeps.o $(LIBS)

aibots.o: aibots.c
	${CC} -o aibots.o -c aibots.c
OsDeps.o: OsDeps.c OsIncl.h
	${CC} -o OsDeps.o -c OsDeps.c
