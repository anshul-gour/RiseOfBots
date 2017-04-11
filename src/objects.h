/*
 *
 *  RISE OF BOTS
 *
 *  OBJECTS_H
 *
 *
 */

#ifndef _OBJECTS_H
#define _OBJECTS_H

typedef struct _entity
{
	char id;
	char type;

	unsigned char x,y;
	unsigned char flx,fly;
	int dir;
	char stage;
	char visible;
	char teamid;

	char plGift;
	char giftSlot[3];
} entity;


typedef struct _score
{
	struct _s
	{
		char name[20];
		char lastmsg[11];
		int  sc;
		int life;
		char deactivated;
	} pl[6];
	int numpl;

	struct _c
	{
		int sc;
		int life;
		char active;
	} ct[10];
	int numct;

	int team1score;
	int team2score;
} score;

/* Types */

#define PLAYER 0
#define SCOREBOARD 1
#define CITY 2
#define BOMB 3
#define WALL 99
#define TURBO 91
#define RANGEDBOMB 92
#define GOLDENEYE 93
#define SPEED 94
#define BALLISTO 95
#define SHIELD 96
#define GIFTFOOD 101
#define GIFTTIMBER 102
#define GIFTMETAL 103

#define MAPX 25
#define MAPY 25
#endif /*_OBJECTS_H*/
