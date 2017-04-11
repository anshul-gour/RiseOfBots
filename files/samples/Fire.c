#include "aibots.h"
#include <math.h>
#include <stdio.h>

int edanid;
void movetoxy(int ,int );
void movetoxy_ret(int,int);

void movetoxy_ret(int x,int y)
{
	int x0,y0,dx,dy,dir,range;
	GetInfoEdan(edanid,&x0,&y0,NULL,NULL,NULL,NULL,NULL,NULL);
	dx = x - x0;
	dy = y - y0;

	range = (int) sqrt(dx*dx +dy*dy);
	dir = (int) (atan2((float)dy,(float)dx)/M_PI*180.0);

	Move(dir,(int)(range+0.5));
}

void movetoxy(int x,int y)
{
	int ismoving=1,dir = 0,sdir,srange;

	movetoxy_ret(x,y);
	while(ismoving)
	{
		if(ScanEdan(dir,&sdir,&srange,NULL,NULL,NULL))
			Fire(sdir,srange);
		else
			dir += 40;
		GetInfoEdan(edanid,NULL,NULL,NULL,NULL,&ismoving,NULL,NULL,NULL);
	}
	movetoxy_ret(x,y);

}

int main()
{
	int dir,sdir,srange;
	edanid = Init("Fire");

	while(1)
	{
		if(edanid == 0)
		{
			movetoxy(4,9);
			movetoxy(9,2);
			movetoxy(15,2);
			movetoxy(20,9);
			movetoxy(15,15);
			movetoxy(9,15);
		}
		if(edanid == 1)
		{
			movetoxy(15,2);
			movetoxy(20,9);
			movetoxy(15,15);
			movetoxy(9,15);
			movetoxy(4,9);
			movetoxy(9,2);
		}
		if(edanid  == 2)
		{
			movetoxy(12,20);
			if(ScanNost(dir,&sdir,&srange,NULL))
				Fire(sdir,srange);
			else
				dir+=40;
			
		}

	}

}

