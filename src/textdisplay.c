/*
 *
 *  RISE OF BOTS
 *
 *  OBJECTS_H
 *
 *
 */

#include <string.h>
#include <curses.h>
#include <stdlib.h>

#include "OsIncl.h"
#include "dispcomm.h"
#include "objects.h"


typedef struct _map
{
	char tile[MAPX][MAPY];
} Map;

static entity objs[100];
static int numobjs;
static score scoreBoard;
static Map map;
//static int fmode;

void display(void);
void drawobj(entity *);
void drawGround(void);
void dokeys(void);
void idle(void);
void mainloop(void);
/*extra() ,,, specialkeys()*/

static int s;
static int numbering;
static char gdelay = 1;
unsigned long gtime;


int maxx,maxy,sx,sy;
int staty;

void over(void)
{
        clear();
	close(s);
	endwin();
	socketCleanup();
	exit(0);
}

void drawchar(int y,int x,char ch)
{
	int ax,ay;

	ax = MAPX - x + 1;
	ay = MAPY - y + 1;

	mvaddch(ay,ax,ch);
}

void showstring(char * s)
{
	while(*s)
	{
		addch(*s);
		s++;
	}
}

void showscores(void)
{
	int i,gf[3],t,j;
	float pos;
        char str[256];

#ifdef WIN32
        int teamscore[2]={0,0};
#endif


	color_set(4,NULL);
	sprintf(str,"GameTime: %lu",gtime);
	move(2,28);
	showstring(str);

	sprintf(str,"GameSpeed: %d",10-gdelay);
	move(4,28);
	showstring(str);


	pos = 4;
	///team1
	color_set(6,NULL);
	pos += 2;
	move(pos,28);
	showstring(scoreBoard.pl[0].name);
	sprintf(str," Score : %d",scoreBoard.team1score);
	move(pos,44);
	showstring(str);

	pos += 1;
	color_set(4,NULL);
	for(i = 0;i < scoreBoard.numpl;i+=2)
	{
		if(objs[i].visible)
		{
			sprintf(str,"Edan %d [Id: %d]  Sc: %4d  Ht : %3d  Re: ",i/2,i,scoreBoard.pl[i].sc,scoreBoard.pl[i].life);
			if(objs[i].plGift == -1)
				strcat(str,"None");
			else
			{
				switch(objs[objs[i].plGift].type)
				{
				case GIFTFOOD: strcat(str,"Vasa");break;
				case GIFTTIMBER: strcat(str,"Tathar");break;
				case GIFTMETAL: strcat(str,"Malda");break;
				}
			}
		}

		else if(!scoreBoard.pl[i].deactivated)
			sprintf(str,"Edan %d [Id: %d]  Sc: %d   NOW DEAD !!",i/2,i,scoreBoard.pl[i].sc);
		else
			sprintf(str,"Edan %d [Id: %d]  Sc: %d   DEACTIVATED",i/2,i,scoreBoard.pl[i].sc);

		pos += 1;
		move(pos,28);
		showstring(str);
	}
	pos += 1;
	t = 0;
	for(i = 0;i < scoreBoard.numct;i++)
	{
		if(objs[i+12].teamid == 0 && scoreBoard.ct[i].active == 1)
		{
			gf[0] = gf[1] = gf[2] = 0;
			for(j=0;j<3;j++)
			{
				if(objs[i+12].giftSlot[j] == -1)
					;
				else
					gf[objs[objs[i+12].giftSlot[j]].type - GIFTFOOD]++;
			}

			t++;
			if(objs[i+12].visible == 1)
			{
				sprintf(str,"Nost %d [Id: %d] Sc: %d  Ht: %d  Vs: %d Ta: %d Ma: %d",t,i+12,scoreBoard.ct[i].sc,scoreBoard.ct[i].life,gf[0],gf[1],gf[2]);

				pos += 1;
				move(pos,28);
				showstring(str);
			}
			else
			{
				sprintf(str,"Nost %d [Id: %d] Sc: %d   DEAD !!",t,i+12,scoreBoard.ct[i].sc);

				pos += 1;
				move(pos,28);
				showstring(str);
			}
		}
	}

	///team 2

	color_set(1,NULL);
	pos += 3;
	move(pos,28);
	showstring(scoreBoard.pl[1].name);
	sprintf(str," Score : %d",scoreBoard.team2score);
	move(pos,44);
	showstring(str);

	pos += 1;
	color_set(4,NULL);
	for(i = 1;i < scoreBoard.numpl;i+=2)
	{
		if(objs[i].visible)
		{
			sprintf(str,"Edan %c [Id: %d]  Sc: %4d  Ht : %3d  Re:",'a'+i/2,i,scoreBoard.pl[i].sc,scoreBoard.pl[i].life);
			if(objs[i].plGift == -1)
				strcat(str,"None");
			else
			{
				switch(objs[objs[i].plGift].type)
				{
				case GIFTFOOD: strcat(str,"Vasa");break;
				case GIFTTIMBER: strcat(str,"Tathar");break;
				case GIFTMETAL: strcat(str,"Malda");break;
				}
			}
		}
		else if(!scoreBoard.pl[i].deactivated)
			sprintf(str,"Edan %c [Id: %d]  Sc: %d   NOW DEAD !!",'a'+i/2,i,scoreBoard.pl[i].sc);
		else
			sprintf(str,"Edan %c [Id: %d]  Sc: %d   DEACTIVATED",'a'+i/2,i,scoreBoard.pl[i].sc);

		pos += 1;
		move(pos,28);
		showstring(str);
	}
	pos += 1;
	t = 0;
	for(i = 0;i < scoreBoard.numct;i++)
	{
		if(objs[i+12].teamid == 1 && scoreBoard.ct[i].active == 1)
		{
			gf[0] = gf[1] = gf[2] = 0;
			for(j=0;j<3;j++)
			{
				if(objs[i+12].giftSlot[j] != -1)
					gf[objs[objs[i+12].giftSlot[j]].type - GIFTFOOD]++;
			}

			t++;
			if(objs[i+12].visible == 1)
			{
				sprintf(str,"Nost %d [Id: %d] Sc: %d  Ht: %d  Vs: %d Ta: %d Ma: %d",t,i+12,scoreBoard.ct[i].sc,scoreBoard.ct[i].life,gf[0],gf[1],gf[2]);

				pos += 1;
				move(pos,28);
				showstring(str);
			}
			else
			{
				sprintf(str,"Nost %d [Id: %d] Sc: %d   DEAD !!",t,i,scoreBoard.ct[i].sc);

				pos += 1;
				move(pos,28);
				showstring(str);
			}
		}
	}

}


int main(int argc, char ** argv)
{
	int i,j,numpl;
	struct sockaddr_in sa;

	socketSetup();

	printf("AI Bots - Display Client started.\n");
	gtime = 0;
	//fmode = -2;

	if(argc > 2)
	{
		printf("USAGE: dispclient ipaddress\n\n");
		return -1;
	}

	if(argc < 2)
		sa.sin_addr.s_addr = inet_addr("127.0.0.1");
	else
	{
		sa.sin_addr.s_addr = inet_addr(argv[1]);
		if(sa.sin_addr.s_addr == INADDR_NONE)
		{
			printf("dispclient: Invalid address.\n");
			return -1;
		}
	}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(DISPORT);

	s = socket(AF_INET,SOCK_STREAM,0);
	if(s < 0)
	{
		printf("dispclient: Socket Error !!\n");
		return -1;
	}

	if(connect(s, (struct sockaddr *) &sa, sizeof(sa)) < 0)
	{
		printf("dispclient: Unable to connect !!\n");
		return -1;
	}

	if(recv(s,(char *)&map,sizeof(Map),0) <= 0)
		over();

	printf("dispclient: Loaded Map.\n");

	if(recv(s, (char *)&numobjs,sizeof(int),0) <= 0)
		over();

	printf("dispclient: Number of objects -> %d\n",numobjs);

	for(i = 0;i < numobjs;i++)
		objs[i].visible = 0;

	initscr();
	start_color();

	init_pair(1,COLOR_BLUE,COLOR_BLACK);
	init_pair(2,COLOR_YELLOW,COLOR_BLACK);
	init_pair(3,COLOR_RED,COLOR_BLACK);
	init_pair(4,COLOR_WHITE,COLOR_BLACK);
	init_pair(5,COLOR_GREEN,COLOR_BLACK);
	init_pair(6,COLOR_MAGENTA,COLOR_BLACK);


	noecho();
	nodelay(stdscr,TRUE);
	raw();
	keypad(stdscr,TRUE);
	getmaxyx(stdscr,maxy,maxx);
	sx = sy = 0;
	staty = 0;
	mainloop();
	over();
	return(0);
}


void mainloop(void)
{
	display();

	while(1)
	{
		dokeys();
		idle();
	}
}

void display(void)
{
	int i,j,nppl;

	clear();

	color_set(4,NULL);

	for(i = 0;i < MAPX+2;i++)
		drawchar(0,i,'$');
	for(i = 1;i < MAPX+1;i++)
		drawchar(i,0,'$');
	for(i = 0;i < MAPX+2;i++)
		drawchar(26,i,'$');
	for(i = 1;i < MAPX+1;i++)
		drawchar(i,26,'$');

	for(i = 0;i < MAPX;i++)
		for(j = 0;j < MAPY;j++)
			if(map.tile[i][j] == WALL)
				drawchar(j+1,i+1,'$');

	for(i = numobjs-1;i >= 0;i--)
		drawobj(&objs[i]);

	showscores();
	refresh();
}

void idle(void)
{
	entity e;
	int n;

	if(send(s, (const char *)&gdelay,1,0) <= 0)
		over();

	if(recv(s, (char *)&gtime,sizeof(gtime),0) <= 0)
		over();

	if(recv(s, (char *)&n,sizeof(int),0) <= 0)
		over();

	for(;n > 0;n--)
	{
		if(recv(s, (char *)&e,sizeof(e),0)<=0)
			over();

		if(e.type == SCOREBOARD)
		{
			if(recv(s, (char *)&scoreBoard,sizeof(score),0) <= 0)
				over();
#ifdef WIN32
                        staty = 2;
#else
                        staty = scoreBoard.numpl+1;
#endif
		}
		else
			memcpy(&objs[e.id],&e,sizeof(e));
	}

	display();
}


void dokeys(void)
{
	int key;

	rewind(stdin);          // Clears the Keyboard Buffer
	key = getch();
	if(key == ERR)
		return;

	switch(key)
	{
		case 'q' : case 'Q' : over();
				break;

		case KEY_RIGHT:	sx--;
				if(sx < maxx-MAPX)
					sx = maxx-MAPX;
				break;
		case KEY_LEFT:	sx++;
				if(sx >= 0)
					sx = 0;
				break;
		case KEY_UP:	sy++;
				if(sy >= 0)
					sy = -1;
				break;
		case KEY_DOWN: sy--;
			if(sy < maxy-MAPY-staty-1)
				sy = maxy-MAPY-staty-1;
			break;
		case 'n': numbering = !numbering; break;
		case '+': gdelay--;
				if(gdelay < 1) gdelay = 1; break;
		case '-': gdelay++;
				if(gdelay > 9) gdelay = 9; break;


		/*case 'm' : case 'M' : fmode = -1;
				break;
		case 'n' : case 'N' : fmode = -2;
				break;*/
	}

	/*if(key >= 'A' && key < 'A'+scoreBoard.numpl)
			fmode = key - 'A';
	if(key >= 'a' && key < 'a'+scoreBoard.numpl)
			fmode = key - 'a';*/
}


void drawobj(entity * e)
{
	int cid;

	if(!e->visible)
		return;

	switch(e->type)
	{
	case PLAYER :
		if(e->teamid == 0)
		{
			color_set(6,NULL);
			if(e->plGift != -1)
				attron(A_UNDERLINE);
			if(e->stage > 0)
				attron(A_BLINK);
			drawchar(e->y+1,e->x+1,'A'+ e->id/2);
			attroff(A_UNDERLINE | A_BLINK);
		}
		else
		{
			color_set(1,NULL);
			if(e->plGift != -1)
				attron(A_UNDERLINE);
			if(e->stage > 0)
				attron(A_BLINK);
			drawchar(e->y+1,e->x+1,'1'+ e->id/2);
			attroff(A_UNDERLINE | A_BLINK);
		}
		break;
	case BOMB :
		{int xx,yy;char ch;

		if(e->dir%2 == 0)
			ch = 'a'+ e->dir/2;
		else
			ch = '1'+ e->dir/2;

		color_set(3,NULL);

		if(e->stage == 1)
			drawchar(e->y+1,e->x+1,ch);
		else
			for(xx = -1;xx <= 1;xx++)
				for(yy = -1;yy <= 1;yy++)
					drawchar(e->y+yy+1,e->x+xx+1,ch);
		break;}
	case CITY :
		if(e->teamid == 0)
			color_set(6,NULL);
		else
			color_set(1,NULL);

		drawchar(e->y+1,e->x+1,'*');
		break;
	case GIFTFOOD :
		color_set(5,NULL);
		drawchar(e->y+1,e->x+1,'^');
		break;
	case GIFTTIMBER :
		color_set(5,NULL);
		drawchar(e->y+1,e->x+1,'<');
		break;
	case GIFTMETAL :
		color_set(5,NULL);
		drawchar(e->y+1,e->x+1,'>');
		break;
	}
}
