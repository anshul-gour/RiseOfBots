#include "aibots.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int edanid;
void movetoxy(int ,int );
void movetoxy_ret(int,int);

void movetoxy_ret(int x,int y)
{
	int x0,y0,dx,dy,dir,movesteps;
	GetInfoEdan(edanid,&x0,&y0,NULL,NULL,NULL,NULL,NULL,NULL);
	dx = x - x0;
	dy = y - y0;

	movesteps = (int) (0.5 + sqrt(dx*dx +dy*dy)); 
	dir = (int) (atan2((float)dy,(float)dx)/M_PI*180.0);

	Move(dir,movesteps);
}

void movetoxy(int x,int y)
{
	int ismoving=1,x1,y1;
	do
	{
		movetoxy_ret(x,y);
		while(ismoving)
		{
			GetInfoEdan(edanid,NULL,NULL,NULL,NULL,&ismoving,NULL,NULL,NULL);
		}
		GetInfoEdan(edanid,&x1,&y1,NULL,NULL,NULL,NULL,NULL,NULL);
	}
	while(x1 != x || y1 != y);

}

int main()
{
	int ismoving,mdir;
	edanid = Init("Check Wall");

	while(1)
	{
		if(edanid == 0)
		{
			movetoxy(11,1);
			if(IsWall(-2,2))
				printf("WALL.\n");
		}
		if(edanid == 1)
		{
			mdir = rand()%360;
			ismoving = 1;

			if(Move(mdir,5))
				Stop();

			while(ismoving)
			{
				GetInfoEdan(edanid,NULL,NULL,NULL,NULL,&ismoving,NULL,NULL,NULL);
			}

		}


		if(edanid  == 2)
		{
			mdir = rand()%360;
			ismoving = 1;

			if(Move(mdir,5))
				Stop();

			while(ismoving)
			{
				GetInfoEdan(edanid,NULL,NULL,NULL,NULL,&ismoving,NULL,NULL,NULL);
			}

		}

	}

}

