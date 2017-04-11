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

#define MAPX 25
#define MAPY 25
#define WALL 99

typedef struct _map
{
	char tile[MAPX][MAPY];
} Map;

static Map map;

void display(void);
void dokeys(void);
void mainloop(void);
void showPos(void);
void showstring(char *);

static int edit = 0;
static int putX,putY;
static char fname[20];

void over(void)
{
	clear();
	endwin();
	printf("dispclient: Map closed.\n");
	exit(0);
}

int loadMap(char * fname)
{
	FILE *fr;
	char buf[MAPX+2];
	int i,j;

	fr = fopen(fname,"r");

	if(!fr)
		return 0;
	for(i=0;i<MAPY;i++)
	{
		fgets(buf,MAPX+3,fr);
		for(j=0;j<MAPX;j++)
		{
			if(buf[j] == ' ')
				map.tile[j][i] = -1;
			else if (buf[j] == '=')
				map.tile[j][i] = WALL;
			else if (buf[j]!=10)
			{
				printf("Invalid character : %i in line %i %i\n",buf[j],i,j);
				fclose(fr);
				return 0;
			}
		}
	}

	fclose(fr);

	return 1;
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

int main(int argc, char ** argv)
{
	int i,j;

	printf("AI Bots - Map Editer.\n");

	if(argc != 2)
	{
		printf("USAGE: Map Editer mapfile\n\n");
		return -1;
	}

	sprintf(fname,argv[1]);

	if(!loadMap(fname))
	{
		printf("Error: Invalid map file !!\n");
		return -1;
	}

	printf("dispclient: Loaded Map\n");

	putX = 24;
	putY = 0;
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

	showPos();

	attron(A_BLINK);
	move(MAPX - putY,MAPX - putX);
	attroff(A_BLINK);
	refresh();
}

void dokeys(void)
{
	int key,i,j;
	FILE *fw;

	rewind(stdin);          // Clears the Keyboard Buffer
	key = getch();
	if(key == ERR)
		return;

	switch(key)
	{
		case 'q' : case 'Q' : over();
				break;

		case KEY_LEFT:
				putX++;
				if(putX > 24)
					putX = 24;
				break;
		case KEY_RIGHT:
				putX--;
    				if(putX < 0)
    					putX = 0;
    				break;
		case KEY_UP:
				putY++;
    				if(putY > 24)
    					putY = 24;
    				break;
		case KEY_DOWN:
				putY--;
    				if(putY < 0)
					putY = 0;
    				break;
		case ' ':
				if(map.tile[putX][putY] == WALL)
					map.tile[putX][putY] = -1;
				else
					map.tile[putX][putY] = WALL;
				edit = 1;
				break;
		case 's':
		case 'S':
				fw = fopen(fname,"w");
				for(i=0;i<MAPY;i++)
				{
					for(j=0;j<MAPX;j++)
					{
						if(map.tile[j][i] == -1)
							fprintf(fw," ");
						else if (map.tile[j][i] == WALL)
							fprintf(fw,"=");
					}
					fprintf(fw,"*\n");
				}
				fprintf(fw,"\n");
				fclose(fw);
				edit = 0;
				break;
	}

	display();
}


void showPos(void)
{
	char str[255];
	sprintf(str,"Curser position :: x: %d y: %d",24-putX,putY);
	move(3,27);
	showstring(str);

	if(edit != 0)
	{
		move(5,27);
		showstring("For save the changes press 's'.");
	}

}
