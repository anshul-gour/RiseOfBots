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
	char res,upgrade;
	int dir=0,sdir,srange;
	edanid = Init("Upgrade Nost");

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

			GetInfoNost(0,&nx,&ny,&vasa,&tathar,&malda,NULL,NULL);
			if(vasa == 1 && tathar == 1 && malda == 1)
				if(MakeNost(0,1,1))
					printf("Ho Hu Hu made new Nost.\nHo Hu Hu made new Nost.\nHo Hu Hu made new Nost.\n");
				else
					printf("kjfgkj hhg khdkgh kh.\n");

			if(vasa >1) UpgradeNost(0,VASA);
			if(tathar >1) UpgradeNost(0,TATHAR);
			if(malda >1) UpgradeNost(0,MALDA);

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
		}
	}
}

