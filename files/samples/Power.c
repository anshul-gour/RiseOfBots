#include "aibots.h"
#include <math.h>
#include <stdio.h>

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
	int resX,resY,nx,ny,ex,ey,ismoving,vasa,tathar,malda;
	char res;
	int dir=0,sdir,srange;
	edanid = Init("Special Powers");

	while(1)
	{
		if(edanid == 0)
		{
			LocateResource(VASA,&resX,&resY);
			if(resX != -1)
				movetoxy(resX,resY);
			GetInfoEdan(edanid,NULL,NULL,&res,NULL,NULL,NULL,NULL,NULL);
			if(res != -1)
			{
				GetInfoNost(0,&nx,&ny,NULL,NULL,NULL,NULL,NULL);
				movetoxy(nx,ny+2);
				if(PassResourceToNost(0))
					printf("resouce passed to city.\n");
				else
					printf("resource can not passed.\n");
			}

			GetInfoNost(0,&nx,&ny,&vasa,NULL,NULL,NULL,NULL);
			if(vasa != 0)
				PowerEdan(0,VASA);
			if(HavePower(SPEED))
				printf("I have The SPEED.\n");

		}
		if(edanid == 1)
		{
			LocateResource(TATHAR,&resX,&resY);
			if(resX != -1)
				movetoxy(resX,resY);
			GetInfoEdan(edanid,NULL,NULL,&res,NULL,NULL,NULL,NULL,NULL);
			if(res != -1)
			{
				GetInfoNost(0,&nx,&ny,NULL,NULL,NULL,NULL,NULL);
				movetoxy(nx-1,ny+2);
				if(PassResourceToNost(0))
					printf("resouce passed to city.\n");
				else
					printf("resource can not passed.\n");
			}

			GetInfoNost(0,&nx,&ny,NULL,&tathar,NULL,NULL,NULL);
			if(tathar != 0)
				PowerEdan(0,TATHAR);
			if(HavePower(BALLISTO))
				printf("I have The BALLISTO.\n");

		}
		if(edanid  == 2)
		{
			LocateResource(MALDA,&resX,&resY);
			if(resX != -1)
				movetoxy(resX,resY);
			GetInfoEdan(edanid,NULL,NULL,&res,NULL,NULL,NULL,NULL,NULL);
			if(res != -1)
			{
				GetInfoNost(0,&nx,&ny,NULL,NULL,NULL,NULL,NULL);
				movetoxy(nx+1,ny+2);
				if(PassResourceToNost(0))
					printf("resouce passed to city.\n");
				else
					printf("resource can not passed.\n");
			}
		
			GetInfoNost(0,&nx,&ny,NULL,NULL,&malda,NULL,NULL);
			if(malda != 0)
				PowerEdan(0,MALDA);
			if(HavePower(SHIELD))
				printf("I have The SHIELD.\n");
			

		}

	}

}

