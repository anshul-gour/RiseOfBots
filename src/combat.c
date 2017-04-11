/*
 *  RISE OF BOPTS
 *
 *
 *  Combat Engine.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "OsIncl.h"
#include "dispcomm.h"
#include "objects.h"


/* Some utility functions */

int isreadable(int s)
{
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(s,&rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	return select(s+1,&rfds,NULL,NULL,&tv);
}

/* argh !!!! */

int castint(float f)
{
	if(f>0)
		return (int) (f+0.0001);
	else
		return (int) (f+0.0001 -1);
}

int getrand(int center,int length)
{
	float a;
	int retval;

	a = (float) rand();
	a /= ( (float)RAND_MAX )/2.0 ;
	a -= 1.0;
	a *= (float) length;
	a += (float)center;
	retval = (int) a;

	if(retval > center+length)
		retval = (int)(center + length);
	if(retval < center-length)
		retval = (int)(center - length);


	return retval;
}

/*Programme Structures*/

typedef struct _thing
{
	entity e;
	int needsync;
} thing;

typedef struct _message
{
	char characters[11];
}message;


typedef struct _player
{
	char name[20];
	thing * t;

	char packet[100];
	int plen;
	int socket;
	PID pid;

	int teamid;

	message messages[10];
	int unread;

	int moveSpeed;
	int movedir,movesteps;
	float acx,acy;

	int isactive;
	int isdead;
	float score;

	unsigned long lastscan;

	int energy;
	int shield;
	int adBomb;
} player;

typedef struct _map
{
	char tile[MAPX][MAPY];
} Map;

typedef struct _gift
{
	thing *t;
} Gift;

typedef struct _city
{
	thing * t;
	char isLoaded;
	char teamid;
	float score;
	int cityArea;
} City;

typedef struct _bomb
{
	thing * t;
	int active;            /* 0 if not active, if active goes from 1-2 */
} bomb;

typedef struct _Team
{
	float speed;
	int bombRange;
	int goldenEye;
} Team;

#define MAXBOMBS 1
#define MAXPLAYERS 6
#define MAXCITYS 10
#define MAXGIFTS 8

static Map map;
static score scoreBoard;
static int updateScore;
static player players[MAXPLAYERS];
static bomb bombs[MAXPLAYERS*MAXBOMBS];
static City city[MAXCITYS];
static Team team[2];
static Gift gift[MAXGIFTS*3];
int activeGift[3] = {-1,-1,-1};
int activeGiftDelay[3] = {0,0,0};
int giftCounter[3] = {0,0,0};

int numthings,numplayers,numCity,numGifts;
static thing things[6+6+10+24 + 1];
static int prog1team,prog2team;

unsigned long gtime;
unsigned int gdelay;
time_t  tstart,tend;

#define PLAYERPORT 6670

#define TEAM0X MAPX/2
#define TEAM0Y MAPY/5
#define TEAM1X MAPX/2 + 1
#define TEAM1Y MAPY - MAPY/5
#define LOADRAND 4

#define CITY0X MAPX/2
#define CITY0Y MAPY/5
#define CITY1X MAPX/2 +1
#define CITY1Y MAPY - MAPY/5
#define CITYRAND 3

#define GIFT0X MAPX/2
#define GIFT0Y MAPY/2
#define GIFTRAND 12

#define MAXTIME 12000
#define DEATHDELAY 200   //actually it is 200*3
#define GIFTDELAY 1000
#define MAXLIFEPL 100

#define SCANRANGE 7 
#define SCANDIR 20
#define BOMBRANGE 3
#define MAXBOMBRANGE 6
#define GIFTTHROWCITY 3
#define GIFTTHROW 3
#define MAXSLOT 3

/*functions*/

void hitPlayer(int ,int );
void hitCity(int ,int );
int getrange(int,int);

/*to initailise the things sending to display port*/
void initaliseThings(void)
{
	int i,j;
	numthings = 0;

	for(i=0;i<2;i++)
	{
		team[i].speed = 0.33;
		team[i].bombRange = BOMBRANGE;
		team[i].goldenEye = 0;
	}

	for(i=0;i<MAXPLAYERS;i++)
	{
		things[numthings].needsync = 0;
		things[numthings].e.id = numthings;
		things[numthings].e.type = PLAYER;
		things[numthings].e.visible = 0;

		players[i].t = &things[numthings];

		numthings++;
	}

	for(i = 0;i < MAXPLAYERS;i++)
		for(j = i*MAXBOMBS;j < (i+1)*MAXBOMBS;j++)
		{
			things[numthings].needsync = 0;
			things[numthings].e.id = numthings;
			things[numthings].e.type = BOMB;
			things[numthings].e.visible = 0;
			things[numthings].e.dir = players[i].t->e.id;
			things[numthings].e.stage = 0;
			bombs[j].active = 0;
			bombs[j].t = &things[numthings]; 

			numthings++;
		}

	for(i=0;i<MAXCITYS ;i++)
	{
		things[numthings].needsync = 0;
		things[numthings].e.id = numthings;
		things[numthings].e.type = CITY;
		things[numthings].e.visible = 0;

		city[i].t = &things[numthings];
		city[i].isLoaded = 0;

		numthings++;
	}

	for(i=0;i<MAXGIFTS * 3 ;i++)
	{
		things[numthings].needsync = 0;
		things[numthings].e.id = numthings;
		things[numthings].e.visible = 0;

		gift[i].t = &things[numthings];

		numthings++;
	}

	updateScore = 1;	
	for(i = 0;i < MAXPLAYERS;i++)
	{
		scoreBoard.pl[i].sc = 0;
		scoreBoard.pl[i].life = MAXLIFEPL;
		scoreBoard.pl[i].deactivated = 0;
	}
	scoreBoard.numpl = MAXPLAYERS;

	for(i = 0;i < MAXCITYS;i++)
	{
		scoreBoard.ct[i].sc = 0;
		scoreBoard.ct[i].life = 500;
		scoreBoard.ct[i].active = 0;
	}
	scoreBoard.numct = MAXCITYS;

	scoreBoard.team1score = scoreBoard.team2score = 0;

}

/* Get number of things that need to be synced */

int numsync(void)
{
	int i,retval = 0;

	for(i = 0;i < numthings;i++)
		if(things[i].needsync)
			retval++;
	return retval;
}

/* Get playername which bot sends on init call */

int getplayername(char * name,int s)
{
	int i = 0;

	do
	{
		if(recv(s,&name[i],1,0) <= 0)
			return 0;
		i++;
	} while(name[i-1] != '\0' && i <= 19);

	if(i == 20)
		return 0;
	else
		return 1;
}

void showscores(void)
{
	int i,lt1,lt2,ct;
	float prog1teamScore=0.0,prog2teamScore=0.0;

	printf("\n\nFinal Scores\n============\n\n");

	printf("Prog1team's Score ::\n");

	for(i = 0;i < numplayers;i++)
	{
		if(players[i].teamid == prog1team)
		{
			lt1 = i;
			prog1teamScore += scoreBoard.pl[i].sc;
			printf("PL %d. %s has a final score of %d\n",i/2,players[i].name,scoreBoard.pl[i].sc);
		}
	}
	ct = 0;
	for(i = 0;i < numCity;i++)
	{
		if(city[i].teamid == prog1team)
		{
			lt1 = i;
			prog1teamScore += scoreBoard.ct[i].sc;
			printf("CT %d has a final score of %d\n",ct,scoreBoard.ct[i].sc);
			ct++;
	
		}
	}
	printf("\nThe team of %s has a final score of %.3f.\n\n",players[lt1].name,prog1teamScore);
	printf("Prog2team's Score ::\n");
	for(i = 0;i < numplayers;i++)
	{
		if(players[i].teamid == prog2team)
		{
			lt2 = i;
			prog2teamScore += scoreBoard.pl[i].sc;
			printf("PL %d. %s has a final score of %d\n",i/2,players[i].name,scoreBoard.pl[i].sc);
		}
	}
	ct = 0;
	for(i = 0;i < numCity;i++)
	{
		if(city[i].teamid == prog2team)
		{
			lt2 = i;
			prog2teamScore += scoreBoard.ct[i].sc;
			printf("CT %d has a final score of %d\n",ct,scoreBoard.ct[i].sc);
			ct++;
		}
	}

	printf("\nThe team of %s has a final score of %.3f\n\n",players[lt2].name,prog2teamScore);

	printf("==================================\n\n");
}

/* Cast player scores into int and copy them to scoreboard */
void copyscores(void)
{
	int i;
	scoreBoard.team1score = scoreBoard.team2score = 0;
	for(i = 0;i<MAXPLAYERS;i++)
	{
		scoreBoard.pl[i].sc = (int) players[i].score;
		if(players[i].teamid == 0)
			scoreBoard.team1score += scoreBoard.pl[i].sc;
		else
			scoreBoard.team2score += scoreBoard.pl[i].sc;
	}
	for(i = 0;i<numCity;i++)
	{
		scoreBoard.ct[i].sc = (int) city[i].score;
		if(city[i].teamid == 0)
			scoreBoard.team1score += scoreBoard.ct[i].sc;
		else
			scoreBoard.team2score += scoreBoard.ct[i].sc;
	}
}

int addMap(char * fname) 
{

	FILE * fp;
	char buf[MAPX+2];
	int i,j;

	fp = fopen(fname,"rt");

	if(!fp)
		return 0;

	for(i=0;i<MAPY;i++) 
	{
		fgets(buf,MAPX+3,fp);
		for(j=0;j<MAPX;j++) 
		{
			if(buf[j] == ' ')
				map.tile[j][i] = -1;
			else if (buf[j] == '=')
				map.tile[j][i] = WALL;
			else if (buf[j]!=10)
			{
				printf("Invalid character : %i in line %i\n",buf[j],i+1);
				fclose(fp);
				return 0;
			}
		}
	}

	fclose(fp);

	return 1;
}
/*change the value at any time on map*/

int changeMap(int x,int y,int value)
{
	if(x>MAPX || x<0 || y>MAPY || y<0)
		return (0);
	map.tile[x][y] = value;
	return (1);
}

/*check the tile for wall, city,pl,gf return 0 if non empty.*/

int checkTile(int x,int y)
{
	int j;

	if(x<0 || x>=MAPX || y<0 || y>=MAPY || map.tile[x][y] == WALL)
		return (0);

	for(j = 0;j < numCity;j++)
		if(city[j].t->e.x == x &&	city[j].t->e.y == y && city[j].t->e.visible)
			return (0);
		
	for(j = 0;j < numplayers;j++)
		if(players[j].t->e.x == x && players[j].t->e.y == y && players[j].t->e.visible)
			return (0);

	for(j=0;j<numGifts;j++)
		if(gift[j].t->e.x == x && gift[j].t->e.y == y && gift[j].t->e.visible == 1 )
			return (0);

	return (1);
}

/* Launch Bomb */

char launchbomb(int i,int dir,int range)
{
	int j,x,y;
	float ang;

	if(range < 0)
		return 0;

	if(range > team[players[i].teamid].bombRange)
		range = team[players[i].teamid].bombRange;

	for(j = i*MAXBOMBS;j < (i+1)*MAXBOMBS;j++)
		if(bombs[j].active == 0)
			break;

	if(j == (i+1)*MAXBOMBS)
		return 0;

	dir %= 360;
	ang = ((float)dir)/180.0*M_PI;

	x = castint( 0.5+( (float)players[i].t->e.x + ((float) range)* cos(ang)) );
	y = castint( 0.5+( (float)players[i].t->e.y + ((float) range)* sin(ang)) );

	if(x < 0 || y < 0 || x >= MAPX || y >= MAPY)
		return 0;

	bombs[j].active = 1;
	bombs[j].t->needsync = 1;
	bombs[j].t->e.visible = 1;
	bombs[j].t->e.stage = 1;         // it is the activeness of bomb.
	bombs[j].t->e.x = x;
	bombs[j].t->e.y = y;

	return 1;
}
/*do the bombs*/

void dobombs(void)
{
	int i,j;
	static int hits[] = {2,1};

	for(i = 0;i < MAXPLAYERS*MAXBOMBS;i++)
		if(bombs[i].active)
		{
			for(j = 0;j < numplayers;j++)
			{
				if(players[j].shield > 0)
						continue;

				if( sqrt( (players[j].t->e.x - bombs[i].t->e.x)
					*(players[j].t->e.x - bombs[i].t->e.x)
					+(players[j].t->e.y - bombs[i].t->e.y)
					*(players[j].t->e.y - bombs[i].t->e.y)
					) < (float) bombs[i].active)
				{
					if(players[i].adBomb > 0)
						hitPlayer(j,2*hits[bombs[i].active-1]);
					else
						hitPlayer(j,hits[bombs[i].active-1]);
					updateScore =1;
					if(players[i/MAXBOMBS].teamid != players[j].teamid)
						players[i/MAXBOMBS].score += (float)hits[bombs[i].active-1];
							//GOOD SHOT!
					else
						players[i/MAXBOMBS].score -= (float)hits[bombs[i].active-1];
							// FRATRICIDE
				}
			}

			for(j=0;j<numCity;j++)
				if(city[j].t->e.visible == 1)
				{
					if( sqrt( (city[j].t->e.x - bombs[i].t->e.x)
						*(city[j].t->e.x - bombs[i].t->e.x)
						+(city[j].t->e.y - bombs[i].t->e.y)
						*(city[j].t->e.y - bombs[i].t->e.y)
						) < (float) bombs[i].active)
					{
						if(players[i].adBomb > 0)
							hitCity(j,2*hits[bombs[i].active-1]);
						else
							hitCity(j,hits[bombs[i].active-1]);
					}	
				}


			bombs[i].active++;
			if(bombs[i].active > 2)
			{
				bombs[i].active = 0;
				bombs[i].t->e.visible = 0;
			}

			bombs[i].t->e.stage = bombs[i].active;   // the dir is activeness of bomb (radious)
			bombs[i].t->needsync = 1;
		}
}

/*hit player*/
void hitPlayer(int i,int damage)
{
	int lifeDec,temp;
	lifeDec = damage;

	if(scoreBoard.pl[i].life > lifeDec)
		scoreBoard.pl[i].life -= lifeDec;
	else
	{
		/*check for gift  that he has*/
		if(players[i].t->e.plGift != -1)
		{
			temp =players[i].t->e.plGift;

			things[temp].e.visible =1;
			things[temp].e.x = players[i].t->e.x;
			things[temp].e.y = players[i].t->e.y;
			things[temp].needsync = 1;
		}

		/* Make Player Re-Op */

		scoreBoard.pl[i].life = MAXLIFEPL;
		players[i].score  -= 10;
		players[i].acx = (float)(MAPX+1);
		players[i].acy = (float)(MAPY+1);
		players[i].t->e.x = (MAPX+1);
		players[i].t->e.y = (MAPY+1);
		players[i].t->e.dir = 0;
		players[i].t->needsync = 1;
		players[i].moveSpeed = 0;
		players[i].t->e.plGift = -1;

		players[i].isdead = DEATHDELAY;
		players[i].t->e.visible = 0;
	}
	updateScore = 1;
}

/* Deactivate player and close socket */

void deactivate(int i)
{
	players[i].isactive = 0;
	close(players[i].socket);
	printf("combat: Closing connection with player %s\n",players[i].name);
	scoreBoard.pl[i].deactivated = 1;
	players[i].score -= 240.0;
	hitPlayer(i,100);
	updateScore = 1;
}

/* Ask all players to start */

void startall()
{
	int i;
	char ch;

	for(i = 0;i < numplayers;i++)
		if(players[i].isactive)
		{
			ch = i/2;
			if(send(players[i].socket,&ch,1,0) <= 0)
				deactivate(i);
		}
}

/*loading a player a sp location*/
void loadPlayer(int s,int x,int y,PID pid,int teamid)
{
	players[numplayers].t->needsync = 1;
	players[numplayers].t->e.x = x;
	players[numplayers].t->e.y = y;
	players[numplayers].t->e.flx = x * 10;
	players[numplayers].t->e.fly = y * 10;
	players[numplayers].t->e.dir = 0;
	players[numplayers].t->e.visible = 1;
	players[numplayers].teamid = players[numplayers].t->e.teamid = teamid;
	players[numplayers].score = 0.0;
	players[numplayers].t->e.plGift = -1;
	players[numplayers].t->e.stage = 0;
	players[numplayers].t->e.giftSlot[0] = 0;
	players[numplayers].t->e.giftSlot[1] = 0;
	players[numplayers].t->e.giftSlot[2] = 0;
	
	players[numplayers].acx = (float)x;
	players[numplayers].acy = (float)y;
	players[numplayers].moveSpeed = 0;
	players[numplayers].socket = s;
	players[numplayers].pid = pid;
	players[numplayers].plen = 0;
	players[numplayers].unread = 0;
	players[numplayers].lastscan = 0;

	players[numplayers].shield = 0;
	players[numplayers].adBomb = 0;
	players[numplayers].energy = 0;

	if(getplayername(players[numplayers].name,s))
	{
		players[numplayers].isactive = 1;
		strcpy(scoreBoard.pl[numplayers].name,players[numplayers].name);
		printf("combat: Player %s has been loaded.\n",players[numplayers].name);
	}
	else
	{
		players[numplayers].isactive = 0;
		scoreBoard.pl[numplayers].deactivated = 1;
		players[numplayers].t->e.visible = 0;
		strcpy(players[numplayers].name,"Inactive");
		printf("combat: Player not responding as expected. Inactivated.\n");
	}

	numplayers++;
}

/*loading a Gift*/
int loadGift(int type)
{
	int x,y,i,j,k,loop=0;

	if(numGifts >= 24)
		return(-1);

	do{
		i = 0;
		j = 0;
		k = 0;
		x = getrand(GIFT0X,GIFTRAND);
		y = getrand(GIFT0Y,GIFTRAND);

		if(( (x-12)*(x-12) - 12*(y-15) < 0) || ( (x-12)*(x-12) + 12*(y-9) < 0 ))
			i++;

		if(checkTile(x,y) == 0)
			i++;

		for(j=0;j<numGifts;j++)
		{
			if(gift[j].t->e.visible)
			{
				gift[numGifts].t->e.x = x;
				gift[numGifts].t->e.y = y;
				if(getrange(gift[numGifts].t->e.id ,gift[j].t->e.id)<4)
					k++;
			}
		}
		for(j=0;j<numCity;j++)
		{
			if(city[j].t->e.visible)
			{
				gift[numGifts].t->e.x = x;
				gift[numGifts].t->e.y = y;
				if(getrange(gift[numGifts].t->e.id ,city[j].t->e.id)<4)
					k++;
			}
		}

		loop++;
		if(loop>20 && i==0)
			break;

	}while(i>0 || k>0);


	gift[numGifts].t->needsync = 1;
	gift[numGifts].t->e.x = x;
	gift[numGifts].t->e.y = y;
	gift[numGifts].t->e.visible = 1;
	gift[numGifts].t->e.teamid = -1;
	gift[numGifts].t->e.type = type;

	numGifts++;

	return(gift[numGifts-1].t->e.id);
}

int putGift(int id,int x,int y)
{
	int t;
	if(x<0 || x>MAPX-1 || y<0 || y>MAPY-1)
		return (0);
	if(map.tile[x][y] != -1)
		return (0);
	for(t=0;t<numGifts;t++)
		if(gift[t].t->e.visible == 1 && gift[t].t->e.x == x && gift[t].t->e.y == y)
			return (0);

	things[id].needsync = 1;
	things[id].e.x = x;
	things[id].e.y = y;
	things[id].e.visible = 1;

	return (1);
}

void doGift(void)
{
	int i;

	for(i=0;i<3;i++)
	{
		if(activeGift[i] == -1 && activeGiftDelay[i] == 0 && giftCounter[i] < MAXGIFTS)
		{
			activeGift[i] = loadGift(101+i);
			giftCounter[i]++;
		}
	}

	for(i=0;i<3;i++)
	{
		if(activeGiftDelay[i] > 0)
			activeGiftDelay[i]--;
	}

	for(i=0;i<3;i++)
	{
		if(activeGift[i] != -1)
			if(things[activeGift[i]].e.teamid != -1)
			{
				activeGift[i] = -1;
				activeGiftDelay[i] = GIFTDELAY;
			}
	}

}

void scatterGifts(int i)
{
	int x,y,t;
	for(t=0;t<MAXSLOT;t++)
		if(city[i].t->e.giftSlot[t] != -1)
		{
			do{
			x=getrand(city[i].t->e.x,2);
			y=getrand(city[i].t->e.x,2);
			}
			while(!putGift(city[i].t->e.giftSlot[t],x,y));

			city[i].t->e.giftSlot[t] = -1;
			city[i].t->needsync = 1;
		}
}

int calculateCityArea(int i)
{
	int ret,k,l,x,y,value;
	ret = 0;

	x = city[i].t->e.x;
	y = city[i].t->e.y;
	value = city[numCity].t->e.teamid+1;
	
	switch(city[i].t->e.stage)
	{
	case 1: ret = 0; break;
	case 2: ret = 10; break;
	case 3: ret = 25; break;
	}

	return (ret);
}

void hitCity(int i,int damage)
{
	if(scoreBoard.ct[i].life >= 50 + damage)
		scoreBoard.ct[i].life -= damage;
	else
	{
		if(scoreBoard.ct[i].life > damage)           //plunder before city die & have <50life
		{
			scatterGifts(i);
			scoreBoard.ct[i].life -= damage;
		}
		else if(scoreBoard.ct[i].life <= damage)
		{
			scatterGifts(i);
			city[i].t->e.visible = 0;
			city[i].isLoaded  = 0;
			city[i].t->needsync = 1;
			changeMap(city[i].t->e.x,city[i].t->e.y, -1);
			city[i].score -= 200;
		}
	}


		if(scoreBoard.ct[i].life <= 500 && city[i].t->e.stage != 1)
		{
			city[i].t->e.stage = 1;
			city[i].cityArea = calculateCityArea(i);
			city[i].t->needsync = 1;
		}
		else { if(scoreBoard.ct[i].life > 500 && scoreBoard.ct[i].life <= 1000 && city[i].t->e.stage != 2)
		{
			city[i].t->e.stage = 2;
			city[i].cityArea = calculateCityArea(i);
			city[i].t->needsync = 1;
		}
		else { if(scoreBoard.ct[i].life > 1000 && city[i].t->e.stage != 3)
		{
			city[i].t->e.stage = 3;
			city[i].cityArea = calculateCityArea(i);
			city[i].t->needsync = 1;
		} } }
	updateScore = 1;
}

void doCity(void)
{
	int i,team0=0,team1=0;
	for(i=0;i<numCity;i++)
	{
		if(city[i].t->e.visible == 0)
			continue;

		if(city[i].teamid == 0)
			team0++;
		if(city[i].teamid == 1)
			team1++;


		if(city[i].t->e.stage == 2)
		{
			scoreBoard.ct[i].life--;
			updateScore = 1;
		}

		if(city[i].t->e.stage == 3)
		{
			scoreBoard.ct[i].life -= 2;
			updateScore = 1;
		}

		if(gtime%12 == 2 && city[i].t->e.stage == 1 && scoreBoard.ct[i].life < 50)
		{
			scoreBoard.ct[i].life++;
			city[i].score -= 2;
			updateScore = 1;
		}
		
		if(scoreBoard.ct[i].life <= 500 && city[i].t->e.stage != 1)
		{
			city[i].t->e.stage = 1;
			city[i].cityArea = calculateCityArea(i);
			city[i].t->needsync = 1;
		}
		else { if(scoreBoard.ct[i].life > 500 && scoreBoard.ct[i].life <= 1000 && city[i].t->e.stage != 2)
		{
			city[i].t->e.stage = 2;
			city[i].cityArea = calculateCityArea(i);
			city[i].t->needsync = 1;
		}
		else { if(scoreBoard.ct[i].life > 1000 && city[i].t->e.stage != 3)
		{
			city[i].t->e.stage = 3;
			city[i].cityArea = calculateCityArea(i);
			city[i].t->needsync = 1;
		} } }

		/*city points */
		city[i].score += ((float)city[i].cityArea /50.0) ;
		updateScore = 1;

	}

	if(team0 == 0)
	{
		for(i=0;i<numplayers;i++)
			if(players[i].teamid == 0 && players[i].isactive != 0)
				deactivate(i);
	}
	
	if(team1 == 0)
	{
		for(i=0;i<numplayers;i++)
			if(players[i].teamid == 1 && players[i].isactive != 0)
				deactivate(i);
	}
}


/*loading City*/
int loadCity(int x,int y,int teamid,char stage)
{
	int t;
	
	if(x<0 || x>=MAPX-1 || y<0 || y>=MAPY-1 )
		return (0);

	if(map.tile[x][y] >= 0)
		return (0);
	for(t=0;t<numGifts;t++)
		if(gift[t].t->e.visible == 1 && gift[t].t->e.x == x && gift[t].t->e.y == y)
			return (0);

	city[numCity].t->needsync = 1;
	city[numCity].t->e.stage = stage;
	city[numCity].t->e.x = x;
	city[numCity].t->e.y = y;
	city[numCity].t->e.visible = 1;
	scoreBoard.ct[numCity].active = 1;
	city[numCity].teamid = city[numCity].t->e.teamid = teamid;
	for(t=0;t<MAXSLOT;t++)
		city[numCity].t->e.giftSlot[t] = -1;
	city[numCity].isLoaded = 1;
	city[numCity].cityArea = calculateCityArea(numCity);
	changeMap(x,y,city[numCity].t->e.id);

	numCity++;
	return (1);
}

/* Finds difference between two directions */

int diffdir(int a,int b)
{
	int diff,i;

	a %= 360;
	b %= 360;

	if(a > b)
	{
		diff = a;
		a = b;
		b = diff;
		i = -1;
	}
	else
		i = 1;

	diff = b-a;

	b -= 360;

	if(a - b < diff)
		return i*(b-a);
	else
		return i*diff;
}

/* Get Direction */

int getdir(int i,int j,int *rdir)
{
	float x1,y1,x2,y2,ang;
	int dir;

	if((things[i].e.visible != 1) || (things[j].e.visible != 1))
		return(0);

	x1 = (float)things[i].e.x;
	x2 = (float)things[j].e.x;
	y1 = (float)things[i].e.y;
	y2 = (float)things[j].e.y;
	

	y2 -= y1;
	x2 -= x1;

	if(x2 == 0)
	{
		if(y2 > 0)
			dir = 90;
		else
			dir = 270;
	}
	else if(y2 == 0)
	{
		if(x2 > 0)
			dir = 0;
		else
			dir = 180;
	}
	else
	{
		ang = atan(fabs(y2/x2));
		dir = castint( ( 180.0/M_PI*ang) );

		if(y2 > 0 && x2 < 0)
			dir = 180 - dir;
		else if(y2 < 0 && x2 < 0)
			dir = 180 + dir;
		else if(y2 < 0 && x2 > 0)
			dir = 360 - dir;
	}

	*rdir = dir;
	return (1);

}

/* Gets distance b/w two things */

int getrange(int i,int j)
{
	return castint(0.5 + sqrt( (things[i].e.x-things[j].e.x)
			*(things[i].e.x-things[j].e.x)
			+(things[i].e.y-things[j].e.y)
			*(things[i].e.y-things[j].e.y)) );
}

/* Is there a wall in that scan ?? */
int nowall(int x1, int y1, int x2, int y2, int range)
{
	int i;
		float delx,dely,x,y;

		delx = (float) (x2 - x1);
		dely = (float) (y2 - y1);

		delx /= (float) (2*range);
		dely /= (float) (2*range);

		x = (float)x1;
		y = (float)y1;

		for(i = 0;i < 2*range;i++)
		{
			if(map.tile[castint(x)][castint(y)] == WALL)
				return 0;

			x += delx;
			y += dely;

	}

		return 1;

}

/* Do a scan for a player */

int doPscan(int dir,int i,int *scanDir,char *scanBotid,int *scanRange)
{
	int j,tdir,trange,pl,mintheta,theta,minrange;

	pl = -1;
	minrange = -1;
	mintheta = SCANDIR;

	for(j = 0;j < numplayers;j++)
		if(i != j)
		{
			if(!getdir(players[i].t->e.id ,players[j].t->e.id,&tdir))
				continue;
			trange = getrange(players[i].t->e.id ,players[j].t->e.id);
			theta = abs(diffdir(tdir,dir));


			if(theta < mintheta || trange == 0)
			{
				if(minrange == -1 || trange < SCANRANGE)
					if(nowall(players[i].t->e.x,players[i].t->e.y,
						players[j].t->e.x,players[j].t->e.y,trange) && 
							players[i].teamid != players[j].teamid )
					{
						mintheta = theta;
						*(scanDir) = tdir;
						minrange = trange;
						pl = j;
					}
			}
			else if(theta == mintheta)
			{
				if(trange < minrange)
				{
					if(nowall(players[i].t->e.x,players[i].t->e.y,
						players[j].t->e.x,players[j].t->e.y,trange) && 
							players[i].teamid != players[j].teamid )
					{
						mintheta = theta;
						*(scanDir) = tdir;
						minrange = trange;
						pl = j;
					}
				}

			}
		}

	if(minrange > SCANRANGE) 
	{
		minrange = -1;
		pl = -1;
		*(scanDir) = -1;
	}

	*(scanRange) = minrange;
	*(scanBotid) = pl;

	if(minrange == -1)
		return (0);
	else
	{
		players[pl].lastscan = gtime;
		return(1);
	}
}

/* Do a scan for a city */

int doCscan(int dir,int i,int *scanDir,char *scanCityid,int *scanRange)
{
	int j,tdir,trange,ct,mintheta,theta,minrange;

	ct = -1;
	minrange = -1;
	mintheta = SCANDIR;

	for(j = 0;j < numCity;j++)
		{
			if(!getdir(players[i].t->e.id ,city[j].t->e.id,&tdir))
				continue;
			trange = getrange(players[i].t->e.id ,city[j].t->e.id);
			theta = abs(diffdir(tdir,dir));


			if(theta < mintheta || trange == 0)
			{
				if(minrange == -1 || trange < SCANRANGE)
					if(nowall(players[i].t->e.x,players[i].t->e.y,
						city[j].t->e.x,city[j].t->e.y,trange) && 
							players[i].teamid != city[j].teamid )
					{
						mintheta = theta;
						*(scanDir) = tdir;
						minrange = trange;
						ct = j;
					}
			}
			else if(theta == mintheta)
			{
				if(trange < minrange)
				{
					if(nowall(players[i].t->e.x,players[i].t->e.y,
						city[j].t->e.x,city[j].t->e.y,trange) && 
							players[i].teamid != city[j].teamid )
					{
						mintheta = theta;
						*(scanDir) = tdir;
						minrange = trange;
						ct = j;
					}
				}

			}
		}

	if(minrange > SCANRANGE) 
	{
		minrange = -1;
		ct = -1;
		*(scanDir) = -1;
	}

	*(scanRange) = minrange;
	*(scanCityid) = ct;

	if(minrange == -1)
		return (0);
	else
	{
		return(1);
	}
}

/* Do a scan for a gift */

int doGscan(int dir,int i,int *scanDir,char *scanType,int *scanRange)
{
	int j,tdir,trange,pl,mintheta,theta,minrange;

	pl = -1;
	minrange = -1;
	mintheta = SCANDIR;

	for(j = 0;j < numplayers;j++)
		{
			if(!getdir(players[i].t->e.id ,gift[j].t->e.id,&tdir))
				continue;
			trange = getrange(players[i].t->e.id ,gift[j].t->e.id);
			theta = abs(diffdir(tdir,dir));


			if(theta < mintheta || trange == 0)
			{
				if(minrange == -1 || trange < SCANRANGE)
					if(nowall(players[i].t->e.x,players[i].t->e.y,
						gift[j].t->e.x,gift[j].t->e.y,trange))
					{
						mintheta = theta;
						*(scanDir) = tdir;
						*(scanType) = gift[j].t->e.type;
						minrange = trange;
					}
			}
			else if(theta == mintheta)
			{
				if(trange < minrange)
				{
					if(nowall(players[i].t->e.x,players[i].t->e.y,
						gift[j].t->e.x,gift[j].t->e.y,trange))
					{
						mintheta = theta;
						*(scanDir) = tdir;
						minrange = trange;
						*(scanType) = gift[j].t->e.type;
					}
				}

			}
		}

	if(minrange > SCANRANGE) 
	{
		minrange = -1;
		*(scanType) = -1;
		*(scanDir) = -1;
	}

	*(scanRange) = minrange;

	if(minrange == -1)
		return (0);
	else
		return(1);
}

/* Move Player */

void moveplayer(int i)
{
	int j,newdir;
	int x,y,cx,cy,cflx,cfly;
	float newX,newY,shift;

	if(!players[i].isactive)
		return;

	if(players[i].isdead > 0)
	{
		players[i].isdead--;
		if(players[i].isdead == 0)
		{
			players[i].t->needsync = players[i].t->e.visible = 1;
			do
			{
				if(players[i].teamid == 0)
				{
					x = getrand(TEAM0X,LOADRAND);
					y = getrand(TEAM0Y,LOADRAND);
				}
				else
				{
					x = getrand(TEAM1X,LOADRAND);
					y = getrand(TEAM1Y,LOADRAND);
				}
			}
			while(!checkTile(x,y));

			players[i].t->e.x = x;
			players[i].t->e.y = y;
			players[i].acx = (float) players[i].t->e.x;
			players[i].acy = (float) players[i].t->e.y;
			players[i].t->e.flx = 10 * players[i].t->e.x;
			players[i].t->e.fly = 10 * players[i].t->e.y;

		}	
		return;
	}
	
	if(players[i].movesteps == 0)
		players[i].moveSpeed = 0;

	if(players[i].moveSpeed == 1)
	{
		if(players[i].energy > 0 || team[players[i].teamid].speed > 0.5)
			shift = 2;
		else
			shift = 1;

		newdir = diffdir(players[i].t->e.dir,players[i].movedir);

		if(newdir != 0)
		{
			if(abs(newdir) < shift*20)
				players[i].t->e.dir = players[i].movedir;
			else if(newdir > 0)
				players[i].t->e.dir += shift*20;
			else
				players[i].t->e.dir -= shift*20;
		}
		else
		{
			/*moving with 0.33 block per 3 gametime.*/
			if(players[i].t->e.plGift != -1)
			{
				newX = players[i].acx + 0.75 * shift * 0.33 * cos( (float)players[i].movedir/180.0*M_PI);
				newY = players[i].acy + 0.75 * shift * 0.33 * sin( (float)players[i].movedir/180.0*M_PI);
			}
			else
			{
				newX = players[i].acx + shift * 0.33 * cos( (float)players[i].movedir/180.0*M_PI);
				newY = players[i].acy + shift * 0.33 * sin( (float)players[i].movedir/180.0*M_PI);
			}

			cx = castint( newX );
			cy = castint( newY );
			cflx = castint( newX * 10 );
			cfly = castint( newY * 10 );

			/*check for wall*/
			if(!(newX>=0 && newX<MAPX  && newY>=0 && newY<MAPY && map.tile[cx][cy] != WALL))
			{
				players[i].moveSpeed = 0 ;
				players[i].score -=1;
				hitPlayer(i,1);
				return;
			}

			/* check for city & players.*/
			for(j = 0;j < numCity;j++)
				if(city[j].t->e.x == cx &&	city[j].t->e.y == cy && city[j].t->e.visible)
				{
					players[i].moveSpeed =  0;
					hitPlayer(i,2);
					players[i].score -= 1;
					return;

				}
		
			for(j = 0;j < numplayers;j++)
				if(i != j)
					if(players[j].t->e.x == cx && players[j].t->e.y == cy)
						break;
					if(j < numplayers)
					{
						players[i].moveSpeed =  0;
						players[i].score -= 1;
						updateScore = 1;
						hitPlayer(i,2);
						hitPlayer(j,2);
						return;
					}
			for(j=0;j<numGifts;j++)
				if(gift[j].t->e.visible == 1 )
					if(gift[j].t->e.x == cx && gift[j].t->e.y == cy)
					{
						if(players[i].t->e.plGift == -1)
						{
							players[i].t->e.plGift = gift[j].t->e.id;
							players[i].t->needsync =1;

							gift[j].t->needsync = 1;
							gift[j].t->e.visible = 0;
							if(gift[j].t->e.teamid != players[i].teamid)
								players[i].score +=100;
							gift[j].t->e.teamid = players[i].teamid ;
						}
						else 
						{
							players[i].moveSpeed = 0 ;
							hitPlayer(i,1);
							return;
						}
					}
			players[i].acx = newX;
			players[i].acy = newY;
			players[i].t->e.x = cx;
			players[i].t->e.y = cy;
			players[i].t->e.flx = cflx;
			players[i].t->e.fly = cfly;
			players[i].movesteps--;
		}
		players[i].t->needsync = 1;
	}
}


/* Do Player stuff  comunicate with player client*/

void doplayer(int i)
{
	int j,n,x,y,t;
	int dir;
	char type,gftype[MAXSLOT];

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	struct _gPLinfo
	{
		int x,y;
		char gift,stage;
		int ismoving,life;
		unsigned long lastscan,time;
		int unrd;
		char ch;
	} gPLinfo;

	struct _gPLOinfo
	{
		int x,y;
		char gift;
		int life;
		int unrd;
		char ch;
	} gPLOinfo;

	struct _gCTinfo
	{
		int x,y,life;
		char haveFood,haveTimber,haveMetal;
		unsigned long time;
		int unrd;
		char ch;
	} gCTinfo;

	struct _scPinfo
	{
		int dir;
		int range;
		char botid;
		char gift;
		int life;
		int unrd;
		char ch;
	} scPinfo;

	struct _scCinfo
	{
		int dir;
		int range;
		char cityid;
		int unrd;
		char ch;
	} scCinfo;

	struct _scGinfo
	{
		int dir;
		int range;
		char type;
		int unrd;
		char ch;
	} scGinfo;

	struct _giftinfo
	{
		int x;
		int y;
		int unrd;
		char ch;
	} giftinfo;

	struct _inmsg
	{
		int index;
		message msg;
		int unrd;
	}inmsg;


	if(players[i].shield > 0)
		players[i].shield--;
	else
		players[i].t->e.giftSlot[0] = 0;
						/*shield for person not hitted upto gtime 1000*/
	if(players[i].adBomb > 0)
		players[i].adBomb--;
	else
		players[i].t->e.giftSlot[1] = 0;
						/*player can use funtion see opp x,y,health*/
	if(players[i].energy > 0)
		players[i].energy--;
	else
		players[i].t->e.giftSlot[2] = 0;
						/*player can run with double speed for a time 1000*/

	if(!players[i].isactive)
		return;

	if(players[i].isdead > 0)
		return;

	/* Get function packet */
	if(isreadable(players[i].socket))	
	{
		if(players[i].plen == 0)
		{
			if(recv(players[i].socket,players[i].packet,1,0) <= 0)
			{
				deactivate(i);
				return;
			}
			players[i].plen++;
		}

		if(isreadable(players[i].socket))
		{
			n = recv(players[i].socket,&players[i].packet[players[i].plen],
				players[i].packet[0] - players[i].plen,0);

			if(n<=0)
			{
				deactivate(i);
				return;
			}

			players[i].plen += n;
		}
	}

	/*got stuff from client now do it*/

	if(players[i].packet[0] == players[i].plen && players[i].plen != 0)
	{
		switch(players[i].packet[1])
		{

			/*move player */
			case 'M':	j = 2;

					players[i].moveSpeed = 1;
					players[i].movedir = *( (int*) &players[i].packet[j]);

					if(players[i].teamid == 1)
						players[i].movedir -=180;

					players[i].movedir %= 360;
					j += sizeof(int);
					players[i].movesteps = *( (int*) &players[i].packet[j]);

					if(players[i].energy > 0 || team[players[i].teamid].speed > 0.5)
						players[i].movesteps = (int)((float)(players[i].movesteps)/0.66);
					else
						players[i].movesteps = (int)((float)(players[i].movesteps)/0.33);

					if(players[i].t->e.plGift != -1)
						players[i].movesteps = (int)((float)(players[i].movesteps)/0.75);

					n = players[i].movesteps;

					if(n > 2)
						n = 2;
					pckt.ch = !(nowall(players[i].t->e.x,players[i].t->e.y,
							(int)(players[i].acx+ n*0.33*cos( (float)players[i].movedir/180.0*M_PI)),
							(int)(players[i].acy+ n*0.33*cos( (float)players[i].movedir/180.0*M_PI)),n));

					pckt.unrd = players[i].unread;
					/* send the no. of unread messages*/
					if(send(players[i].socket,(const char *)&pckt,
								sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/* stop() */
			case 'S':	players[i].moveSpeed = 0;
					players[i].acx = (float) players[i].t->e.x;
					players[i].acy = (float) players[i].t->e.y;

					pckt.ch = 1;
					pckt.unrd = players[i].unread;
					/* send the no. of unread messages*/
					if(send(players[i].socket,(const char *)&pckt,
								sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;


			/* getinfoPL(..) */
			case 'G':
					j = *( (int*) &players[i].packet[2]);
					j = j*2 + players[i].teamid;
					gPLinfo.ch = 0;

					if(j >= 0 && j < MAXPLAYERS)
					{
						if(players[j].t->e.visible)
						{
							gPLinfo.x = players[j].t->e.x;
							gPLinfo.y = players[j].t->e.y;
							if(players[j].teamid == 1)
							{
								gPLinfo.x = MAPX-1 - gPLinfo.x;
								gPLinfo.y = MAPY-1 - gPLinfo.y;
							}

							if(players[j].t->e.plGift == -1)
								gPLinfo.gift = 0;
							else
								gPLinfo.gift = things[players[j].t->e.plGift].e.type;

							gPLinfo.stage = players[j].t->e.stage;

							gPLinfo.ismoving = players[j].moveSpeed;
							gPLinfo.time = gtime;
							gPLinfo.life = scoreBoard.pl[j].life;
							gPLinfo.lastscan = players[j].lastscan;

							gPLinfo.unrd = players[i].unread;
							gPLinfo.ch = 1;
						}
					}

					if(gPLinfo.ch == 0)
					{
						gPLinfo.x = -1;
						gPLinfo.y = -1;
						gPLinfo.gift = -1;
						gPLinfo.stage = -1;
						gPLinfo.ismoving = -1;
						gPLinfo.time = gtime;
						gPLinfo.life = -1;
						gPLinfo.lastscan = 0;

						gPLinfo.unrd = players[i].unread;
					}

					if(send(players[i].socket,(const char *)&gPLinfo,sizeof(gPLinfo),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/* getinfoPLO(..) */
			case 'g':
					j = *( (int*) &players[i].packet[2]);
					j = j*2 + (players[i].teamid+1)%2;
					gPLOinfo.ch = 0;

					if(team[players[i].teamid].goldenEye == 1)
					{

						if(j >= 0 && j < MAXPLAYERS)
						{
							if(players[j].t->e.visible)
							{
								gPLOinfo.x = players[j].t->e.x;
								gPLOinfo.y = players[j].t->e.y;
								gPLOinfo.life = scoreBoard.pl[j].life;
							}

							if(players[i].teamid == 1)
							{
								gPLOinfo.x = MAPX-1 - gPLOinfo.x;
								gPLOinfo.y = MAPY-1 - gPLOinfo.y;
							}

							if(players[j].t->e.plGift == -1)
								gPLOinfo.gift = -1;
							else
								gPLOinfo.gift = things[players[j].t->e.plGift].e.type;

							gPLOinfo.unrd = players[i].unread;
							gPLOinfo.ch = 1;
						}
					}

					if(gPLOinfo.ch == 0)
					{
						gPLOinfo.x = -1;
						gPLOinfo.y = -1;
						gPLOinfo.gift = -1;
						gPLOinfo.life = -1;
						gPLOinfo.unrd = players[i].unread;
					}

					if(send(players[i].socket,(const char *)&gPLOinfo,sizeof(gPLOinfo),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/* getinfoCT(..) */
			case 'H':
					j = *( (int*) &players[i].packet[2]);
					n = -1;

					for(t=0;t<numCity;t++)
					{
						if(players[i].teamid == city[t].teamid)
						{
							n++;
							if(n == j)
							{
								j = t;
								break;
							}
						}
					}
					gCTinfo.ch = 0;

					if(j >= 0 && j < MAXCITYS)
					{
						if(city[j].t->e.visible == 1)
						{
							gCTinfo.x = city[j].t->e.x;
							gCTinfo.y = city[j].t->e.y;

							if(players[i].teamid == 1)
							{
								gCTinfo.x = MAPX-1 - gCTinfo.x;
								gCTinfo.y = MAPY-1 - gCTinfo.y;
							}

							gCTinfo.haveFood = 0;
							gCTinfo.haveTimber = 0;
							gCTinfo.haveMetal = 0;
							for(t=0;t<MAXSLOT;t++)
							{
								if(city[j].t->e.giftSlot[t] != -1)
								{
									switch(things[city[j].t->e.giftSlot[t]].e.type)
									{
									case GIFTFOOD: gCTinfo.haveFood += 1;break;
									case GIFTTIMBER: gCTinfo.haveTimber += 1;break;
									case GIFTMETAL: gCTinfo.haveMetal += 1;break;
									}
								}
							}
							gCTinfo.time = gtime;
							gCTinfo.life = scoreBoard.ct[j].life;

							gCTinfo.unrd = players[i].unread; 
							gCTinfo.ch = 1;
						}
					}

					if(gCTinfo.ch == 0)
					{
						gCTinfo.x = -1;
						gCTinfo.y = -1;
						gCTinfo.haveFood = -1;
						gCTinfo.haveTimber = -1;
						gCTinfo.haveMetal = -1;
						gCTinfo.time = gtime;
						gCTinfo.life = -1;

						gCTinfo.unrd = players[i].unread;

					}

					if(send(players[i].socket,(const char *)&gCTinfo,sizeof(gCTinfo),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

				/*active gift's position*/
			case 'V':
					type = *((int*)&players[i].packet[2]);
					
					if(activeGift[type - GIFTFOOD] != -1)
					{
						giftinfo.x = things[activeGift[type - GIFTFOOD]].e.x;
						giftinfo.y = things[activeGift[type - GIFTFOOD]].e.y;
						giftinfo.ch = 1;
						giftinfo.unrd = players[i].unread;

						if(players[i].teamid == 1)
						{
							giftinfo.x = MAPX -1 - giftinfo.x;
							giftinfo.y = MAPY -1 - giftinfo.y;
						}
					}
					else
					{
						giftinfo.x = -1;
						giftinfo.y = -1;
						giftinfo.ch = 0;
						giftinfo.unrd = players[i].unread;
					}

					if(send(players[i].socket,(const char *)&giftinfo,sizeof(giftinfo),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/*scan player*/
			case 'x':
					dir = *((int*)&players[i].packet[2]);
					if(players[i].teamid == 1)
					{
						dir -= 180;
						dir %= 360;
					}

					if(doPscan(dir,i,&(scPinfo.dir),&(scPinfo.botid),&(scPinfo.range)))
					{
						scPinfo.ch = 1;
						scPinfo.life = scoreBoard.pl[scPinfo.botid].life;
						if(players[scPinfo.botid].t->e.plGift != -1)
							scPinfo.gift = things[players[scPinfo.botid].t->e.plGift].e.type;
						else
							scPinfo.gift = -1;
					}
					else
					{
						scPinfo.ch = 0;
						scPinfo.life = -1;
						scPinfo.gift = -1;
					}

					if(players[i].teamid == 1)
					{
						scPinfo.dir -= 180;
						scPinfo.dir %= 360;
					}

					scPinfo.botid = scPinfo.botid/2;

					scPinfo.unrd = players[i].unread;	

					if(send(players[i].socket,(const char *)&scPinfo,sizeof(scPinfo),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/*scan City*/
			case 'y':
					dir = *((int*)&players[i].packet[2]);

					if(players[i].teamid == 1)
					{
						dir -= 180;
						dir %= 360;
					}

					if(doCscan(dir,i,&(scCinfo.dir),&(scCinfo.cityid),&(scCinfo.range)))
						scCinfo.ch = 1;
					else
						scCinfo.ch = 0;

					if(players[i].teamid == 1)
					{
						scCinfo.dir -= 180;
						scCinfo.dir %= 360;
					}

					j = -1;
					for(t=0;t<scCinfo.cityid;t++)
						if(players[i].teamid != city[t].teamid)
							j++;
					scCinfo.cityid = j;

					scCinfo.unrd = players[i].unread;	

					if(send(players[i].socket,(const char *)&scCinfo,sizeof(scCinfo),0) <= 0)
					{
						deactivate(i);
						return;
					}

					break;

			/*scan Gift*/
			case 'z':
					dir = *((int*)&players[i].packet[2]);

					if(players[i].teamid == 1)
					{
						dir -= 180;
						dir %= 360;
					}

					if(doGscan(dir,i,&(scGinfo.dir),&(scGinfo.type),&(scGinfo.range)))
						scGinfo.ch = 1;
					else
						scGinfo.ch = 0;

					if(players[i].teamid == 1)
					{
						scGinfo.dir -= 180;
						scGinfo.dir %= 360;
					}

					scGinfo.unrd = players[i].unread;	

					if(send(players[i].socket,(const char *)&scGinfo,sizeof(scGinfo),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/* scan wall in arround 5 * 5 block */
			case 'A':
					x = *((int*)&players[i].packet[2]);
					y = *((int*)&players[i].packet[2 + sizeof(int)]);

					pckt.ch = 1;

					if(players[i].teamid == 1)
					{
						x*=-1;
						y*=-1;
					}

					if(x<3 && x>-3 && y<3 && y>-3)
					{
						x = players[i].t->e.x + x;
						y = players[i].t->e.y + y;

						if(x>0 && x<MAPX-1 && y>0 && y<MAPY-1)
						{
							if(map.tile[x][y] != WALL)
								pckt.ch = 0;
						}
					}

					pckt.unrd = players[i].unread;

					if(send(players[i].socket,(const char *)&pckt,
								sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}

					break;


			/*put Gift on dir,range*/
			case 'L': j=2;
					x = *( (int*) &players[i].packet[j]);    //x is dir
					j += sizeof(int);
					y = *( (int*) &players[i].packet[j]);    //y is range

					pckt.ch = 0;

					if(players[i].teamid == 1)
						{
							x -= 180;
							x %= 360;
						}						
						
					if(players[i].t->e.plGift != -1 && y <= GIFTTHROW)
					{
						if(putGift(players[i].t->e.plGift,
							players[i].t->e.x + (int)(y * cos( (float)x/180.0*M_PI)),
							players[i].t->e.y + (int)(y * sin( (float)x/180.0*M_PI))  ))
						{
							players[i].t->e.plGift = -1;
							players[i].t->needsync = 1;
							pckt.ch = 1;
						}
					}

					pckt.unrd = players[i].unread;

					if(send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/*pass gift to other pl*/ 
			case 'P':
					n = *( (int*) &players[i].packet[2]);
					n = n*2 + players[i].teamid;
					pckt.ch = 0;

					if(n >= 0 && n < MAXPLAYERS)
						if(getrange(players[i].t->e.id ,players[n].t->e.id) <= GIFTTHROW && players[n].isdead == 0)
							if(players[i].t->e.plGift != -1 && players[n].t->e.plGift == -1)
							{
								players[n].t->e.plGift = players[i].t->e.plGift;
								players[i].t->e.plGift = -1;
								pckt.ch = 1;
								players[n].t->needsync  = 1;
							}

					pckt.unrd = players[i].unread;
					if( send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/*pass gift to CITY*/ 
			case 'Q':
					n = *( (int*) &players[i].packet[2]);
					j = -1;
					for(t=0;t<numCity;t++)
					{
						if(players[i].teamid == city[t].teamid)
						{
							j++;
							if(n == j)
							{
								n = t;
								break;
							}
						}
					}
					pckt.ch = 0;

					if(n >= 0 && n < numCity)
						if(getrange(players[i].t->e.id,city[n].t->e.id) <= GIFTTHROW && city[n].t->e.visible)
							if(players[i].t->e.plGift != -1 && scoreBoard.ct[i].life >= 50)
							{
								for(t=0;t<MAXSLOT;t++)
								{
									if(city[n].t->e.giftSlot[t] == -1)
									{
										city[n].t->e.giftSlot[t] = players[i].t->e.plGift;
										city[n].t->needsync = 1;
										players[i].t->e.plGift = -1;
										players[i].t->needsync = 1;

										pckt.ch = 1;
										break;
									}
								}	
							}

					pckt.unrd = players[i].unread;
					if( send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;
					
			/*get gift from CITY*/ 
			case 'O':
					n = *( (int*) &players[i].packet[2]);
					j = -1;
					for(t=0;t<numCity;t++)
					{
						if(players[i].teamid == city[t].teamid)
						{
							j++;
							if(n == j)
							{
								n = t;
								break;
							}
						}
					}
					pckt.ch = 0;
					type = players[i].packet[2 + sizeof(int)];

					if(n >= 0 && n < numCity && city[n].t->e.visible && getrange(players[i].t->e.id,city[n].t->e.id) <= GIFTTHROWCITY )
					{
						for(t=0;t<MAXSLOT;t++)
							if(city[n].t->e.giftSlot[t] != -1 && things[city[n].t->e.giftSlot[t]].e.type == type)
							{
								players[i].t->e.plGift = city[n].t->e.giftSlot[t];
								players[i].t->needsync = 1;
								city[n].t->e.giftSlot[t] = -1;
								city[n].t->needsync = 1;
								pckt.ch = 1;
								break;
							}
					}

					pckt.unrd = players[i].unread;
					if( send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/*make new city*/
			case 'N':	j = 2;
					n = *( (int*) &players[i].packet[j]);   //n is cityid
					j += sizeof(int);
					x = *( (int*) &players[i].packet[j]);   // x is position rel to pl.x
					j += sizeof(int);
					y = *( (int*) &players[i].packet[j]);    //y  is pos rel to pl.y

					j = -1;
					for(t=0;t<numCity;t++)
					{
						if(players[i].teamid == city[t].teamid)
						{
							j++;
							if(n == j)
							{
								n = t;
								break;
							}
						}
					}

					if(players[i].teamid == 1)
					{
						x *= -1;
						y *= -1;
					}

					pckt.ch = 0;

					t = 0;
					if(city[n].t->e.visible == 1)
					{
						for(t=0;t<MAXSLOT;t++)
						{
							if(city[n].t->e.giftSlot[t] != -1)
								 gftype[t] = things[city[n].t->e.giftSlot[t]].e.type;
							else							
								 break;
						}
					}

					if(!(t<MAXSLOT || gftype[0]==gftype[1] || gftype[1]==gftype[2] || gftype[2]==gftype[0]))
					{
						if((x == 1 || x == -1) && (y == 1 || y == -1))
						{
							if(loadCity(players[i].t->e.x + x,players[i].t->e.y + y ,players[i].teamid,1))
							{
								for(t=0;t<MAXSLOT;t++)
									city[n].t->e.giftSlot[t] = -1;
								city[n].t->needsync = 1;
								players[i].score += 300;
								updateScore = 1;
								pckt.ch = 1;
							}
						}
					}

					pckt.unrd = players[i].unread;
					if(send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/*upgrade bot*/
			case 'U':	j = 2;
					n = *( (int*) &players[i].packet[j]);
					j = -1;
					for(t=0;t<numCity;t++)
					{
						if(players[i].teamid == city[t].teamid)
						{
							j++;
							if(n == j)
							{
								n = t;
								break;
							}
						}
					}
					pckt.ch = 0;

					t = 0;
					if(city[n].t->e.visible == 1)
					{
						for(t=0;t<MAXSLOT;t++)
						{
							if(city[n].t->e.giftSlot[t] != -1)
								 gftype[t] = things[city[n].t->e.giftSlot[t]].e.type;
							else							
								 break;
						}
					}

					if(t==MAXSLOT && gftype[0] == gftype[1] && gftype[1] == gftype[2])
					{
						if(players[i].t->e.stage == 0)
						{
							city[i].score += 300;
							updateScore = 1;

							switch(gftype[0])
							{
							case GIFTFOOD:
								team[players[i].teamid].speed = 0.66;
								players[i].t->e.stage = TURBO;
								players[(i+2)%6].t->e.stage = TURBO;
								players[(i+4)%6].t->e.stage = TURBO;
								break;

							case GIFTTIMBER:
								team[players[i].teamid].bombRange = MAXBOMBRANGE;
								players[i].t->e.stage = RANGEDBOMB;
								players[(i+2)%6].t->e.stage = RANGEDBOMB;
								players[(i+4)%6].t->e.stage = RANGEDBOMB;
								break;

							case GIFTMETAL: 
								team[players[i].teamid].goldenEye = 1;
								players[i].t->e.stage = GOLDENEYE;
								players[(i+2)%6].t->e.stage = GOLDENEYE;
								players[(i+4)%6].t->e.stage = GOLDENEYE;
								break;				
							}
							
							for(t=0;t<MAXSLOT;t++)
								city[n].t->e.giftSlot[t] = -1;
							city[n].t->needsync = 1;
							pckt.ch = 1;
						}
					}
				
					pckt.unrd = players[i].unread;
					if(send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/*upgrade bot for 1000 gtime*/
			case 'u':	j = 2;
					n = *( (int*) &players[i].packet[j]);
					j += sizeof(int);
					type = players[i].packet[j];  //which gift type

					j = -1;
					for(t=0;t<numCity;t++)
					{
						if(players[i].teamid == city[t].teamid)
						{
							j++;
							if(n == j)
							{
								n = t;
								break;
							}
						}
					}
					pckt.ch = 0;

					if(city[n].t->e.visible == 1)
					{
						for(t=0;t<MAXSLOT;t++)
						{
							if(city[n].t->e.giftSlot[t] != -1 && things[city[n].t->e.giftSlot[t]].e.type == type)
							{
									
								if(scoreBoard.pl[i].life <= 50)
									scoreBoard.pl[i].life += 50;
								else
									scoreBoard.pl[i].life = 100;

								switch(type)
								{
								case GIFTFOOD:
										players[i].energy = 800;
									    players[i].t->e.giftSlot[0] = 1;
										players[i].t->needsync = 1;
										break;
								case GIFTTIMBER:
										players[i].adBomb = 800;
									    players[i].t->e.giftSlot[1] = 1;
										players[i].t->needsync = 1;
										break;
								case GIFTMETAL:
										players[i].shield = 800;
									    players[i].t->e.giftSlot[2] = 1;
										players[i].t->needsync = 1;
										break;
								}

								city[n].t->e.giftSlot[t] = -1;
								city[n].t->needsync = 1;
								pckt.ch = 1;
								break;
							}
						}
					}
				
					pckt.unrd = players[i].unread;
					if(send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}

					break;

			/*chack For players power*/
			case 'e': 
					type = players[i].packet[2];

					pckt.ch = 0;

					switch(type)
					{
					case SPEED: if(players[i].energy > 0) pckt.ch = 1; break;
					case BALLISTO: if(players[i].adBomb > 0) pckt.ch = 1; break;
					case SHIELD: if(players[i].shield > 0) pckt.ch = 1; break;
					}

					pckt.unrd = players[i].unread;
					if(send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}

					break;

			/*upgrade city*/
			case 'W':	j = 2;
					n = *( (int*) &players[i].packet[j]);
					j += sizeof(int);
					type = players[i].packet[j];  //which gift type

					j = -1;
					for(t=0;t<numCity;t++)
					{
						if(players[i].teamid == city[t].teamid)
						{
							j++;
							if(n == j)
							{
								n = t;
								break;
							}
						}
					}
					pckt.ch = 0;

					if(city[n].t->e.visible == 1)
					{
						for(t=0;t<MAXSLOT;t++)
						{
							if(city[n].t->e.giftSlot[t] != -1 && things[city[n].t->e.giftSlot[t]].e.type == type)
							{
								scoreBoard.ct[n].life += 500;
								updateScore = 1;
								city[n].t->e.giftSlot[t] = -1;
								city[n].t->needsync = 1;
								pckt.ch = 1;
								break;
							}
						}
					}
				
					pckt.unrd = players[i].unread;
					if(send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}

					break;
							
			/* fire(int dir,int range) */
			case 'F' :	j = 2;
					x = *( (int*) &players[i].packet[j]);   // x= dir
					j += sizeof(int);
					y = *( (int*) &players[i].packet[j]);   // y = range

					if(players[i].teamid == 1)
					{
						x -= 180;
						x %= 360;
					}

					pckt.ch = launchbomb(i,x,y);
					pckt.unrd = players[i].unread;

					if(send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/* void broadcast(char *m) */
			case 'B':
					for(j=0;j<numplayers;j++)
					{
						if(j != i && players[j].teamid == players[i].teamid	&& players[j].unread <10)
						{
							strcpy(players[j].messages[players[j].unread].characters, &players[i].packet[2]);
							strcpy( scoreBoard.pl[j].lastmsg,players[j].messages[players[j].unread].characters);
							updateScore = 1;
							players[j].unread++;
						}
					}

					if(send(players[i].socket,(const char *)&players[i].unread,
								sizeof(int),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/* int msgsend(int playerindex, char *m) */
			case 'D':
					j = *( (int*) &players[i].packet[2]);
					j = j*2 + players[i].teamid;

					n = 2 + sizeof(int);
					if(j>=0 && j<numplayers)
					{
						if(players[j].unread <= 10)
						{
							strcpy( players[j].messages[ players[j].unread ].characters,&players[i].packet[n] );
							strcpy( scoreBoard.pl[j].lastmsg,players[j].messages[players[j].unread].characters);
							updateScore = 1;
							players[j].unread++;
							pckt.ch = 1;
						}
					}
					else
						pckt.ch = 0;

					pckt.unrd=players[i].unread;
					if( send(players[i].socket,(const char *)&pckt,sizeof(pckt),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;

			/* int msgrecv(char *m) */
			case 'R':
					j = players[i].unread;
					if(j != 0)
					{
						inmsg.index = j - 1;
						inmsg.msg = players[i].messages[j - 1];
						players[i].unread--;
					}
					else
						inmsg.index = -1;

					inmsg.unrd = players[i].unread;
					if(send(players[i].socket,(const char *)&inmsg,
								sizeof(inmsg),0) <= 0)
					{
						deactivate(i);
						return;
					}
					break;
		}
		players[i].plen = 0;
	}
}

/*THE MAIN PROG*/

int main(int argc,char ** argv)
{

	int lfd,fd;
	PID pid, dispPid;
	int i,x,y,cityx,cityy,n,teamid;
	char ch;
	entity e;
	struct sockaddr_in addr;
	char cmdline[128];
	char *temp;

	socketSetup();

	numthings = numplayers = numCity = numGifts = 0;

	srand(time(NULL));

	printf("AI-Wars Combat Engine started\n");

	gdelay=2;

	if(argc != 4)
	{
		printf("USAGE: combat map team1 team2 \n");
		return -1;
	}

	prog1team = rand() % 2;
	printf("prog1team=%d\n",prog1team);
	prog2team = 1 - prog1team;
	printf("prog2team=%d\n",prog2team);

	/*initalise map*/
	if(addMap(argv[1]))
	{
		printf("Using map::");
		puts(argv[1]);
	}
	else
	{
		printf("Error Loading Map.\n");
		exit(0);
	}

	initaliseThings();

	numplayers = numCity = numGifts = 0;

	/*loading 2 inital city*/
	for(i = 0;i < 2;i++)
	{

		if(i < 1)
		{
			do{
			cityx=getrand(CITY0X,CITYRAND);
			cityy=getrand(CITY0Y,CITYRAND);
			}while(map.tile[cityx][cityy] >= 0);
			teamid=0;
		}
		else
		{
			do{
			cityx=getrand(CITY1X,CITYRAND);
			cityy=getrand(CITY1Y,CITYRAND);
			}while(map.tile[cityx][cityy] >= 0);
			teamid=1;
		}

		loadCity(cityx,cityy,teamid,1);
	}


	/* Start Players */

	lfd = socket(AF_INET,SOCK_STREAM,0);
	if(lfd < 0)
	{
		printf("combat: SOCKET error.\n");
		return -1;
	}

	i = 1;
	setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,(const char *)&i,sizeof(i));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PLAYERPORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(lfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0)
	{
		printf("combat: Could not BIND to player port.\n");
		return -1;
	}

	listen(lfd,11);

	if(prog1team == 1)
	{
		temp = argv[2];
		argv[2] = argv[3];
		argv[3] = temp;
	}

	for(i = 0;i < MAXPLAYERS;i++)
	{
		if(i%2==0)
			strcpy(cmdline, argv[2]);
		else
			strcpy(cmdline, argv[3]);

		if(i%2==0)
		{
			do{x=getrand(TEAM0X,LOADRAND);
			y=getrand(TEAM0Y,LOADRAND);}while(map.tile[x][y] >= 0);
			teamid = 0;
		}
		else
		{
			do{x=getrand(TEAM1X,LOADRAND);
			y=getrand(TEAM1Y,LOADRAND);}while(map.tile[x][y] >= 0);
			teamid = 1;
		}


        printf("combat: Spawning %s \n",cmdline);
        pid = createFork(cmdline, lfd);

        delay(1000000);
        if(!isreadable(lfd))
        {
            printf("combat: Player did not connect back. Terminating player!\n");
            terminateFork(pid);
			loadPlayer(-1,-1,-1,-1,teamid);
        }
        else
        {
            fd = accept(lfd,NULL,NULL);
			printf("combat: Loading player of team %d  at (%d,%d) ...\n",teamid,x,y);
			loadPlayer(fd,x,y,pid,teamid);
		}
	}

	close(lfd);


	/* Setup Display Client */

	lfd = socket(AF_INET,SOCK_STREAM,0);
	if(lfd < 0)
	{
		printf("combat: SOCKET error.\n");
		return -1;
	}

	i = 1;
	setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,(const char *)&i,sizeof(int));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(DISPORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(lfd, (struct sockaddr *) &addr,sizeof(struct sockaddr_in)) < 0)
	{
		printf("combat: Could not bind to display port.\n");
		return -1;
	}

	listen(lfd,10);
	printf("combat: Spawning display client\n");

	dispPid = createFork("display", 0);

	delay(3000000);
	if(!isreadable(lfd))
	{
		printf("combat: Display client did not connect back\n");
		printf("combat: Terminating all processes\n");
		terminateFork(dispPid);

		for(i = 0;i < numplayers;i++)
		{
			close(players[i].socket);
			if(players[i].pid != -1)
				terminateFork(players[i].pid);
		}

		socketCleanup();
		printf("combat: Combat server stopping\n");

		return (1);
	}


	fd = accept(lfd,NULL,NULL);
	close(lfd);

	printf("combat: Display client has successfully connected back.\n");

	printf("combat: Sending map to display client ...\n");
	if(send(fd,(const char *)&map,sizeof(Map),0) <= 0)
			return -1;

	printf("combat: Sending number of objects to display client ...\n");
	send(fd,(const char *)&numthings,sizeof(int),0);

	printf("combat: Allowing all players to start ...\n");
	startall();

	/* Main Loop */
	tstart =time(NULL);
	printf("combat: Starting main loop ...\n");
	for(gtime = 0;gtime < MAXTIME;gtime++)
	{
        delay(10000*gdelay);

		/* Process player commands and moves */
		for(i = 0;i < numplayers;i++)
			doplayer(i);

		if(gtime % 3 == 2)
		{
			for(i = 0;i < numplayers;i++)
				moveplayer(i);

			dobombs();
			doCity();
		}

		doGift();

		copyscores();

		/* Sync objects with display server */
		if(isreadable(fd))
		{
			n = numsync()+updateScore;
			if(recv(fd,&ch,1,0) < 1)
				break;
			if(ch>0 && ch<=9)
				gdelay = ch;
			if(send(fd,(const char *)&gtime,sizeof(gtime),0) <= 0)
				break;
			if(send(fd,(const char *)&n,sizeof(int),0) <= 0)
				break;
			if(updateScore)
			{
				copyscores();
				e.id = 0;
				e.type = SCOREBOARD;
				if(send(fd,(const char *)&e,sizeof(e),0) <= 0)
					break;
				if(send(fd,(const char *)&scoreBoard,sizeof(score),0) <= 0)
					break;
				updateScore = 0;
			}
			for(i = 0;i < numthings;i++)
				if(things[i].needsync)
				{

					if(send(fd,(const char *)&things[i].e,sizeof(entity),0) <= 0)
						break;
					things[i].needsync = 0;
				}
			if(i != numthings)
				break;
		}
	}

	tend=time(NULL);

	printf("combat: Combat over - doing cleanup ...\n");
	close(fd);

	for(i = 0;i < numplayers;i++)
	{
		close(players[i].socket);
		if(players[i].pid != -1)
			terminateFork(players[i].pid);
	}
	terminateFork(dispPid);

	delay(1000000);
	showscores();
	printf("Game lasted for %d seconds\n",(int)difftime(tend,tstart));
	printf("combat: Combat server stopping !\n");

	socketCleanup();
	return(0);
}
