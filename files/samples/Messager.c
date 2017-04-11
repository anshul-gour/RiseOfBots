#include "aibots.h"
#include <math.h>
#include <stdio.h>

int edanid;
void movetoxy(int ,int );
void movetoxy_ret(int,int);

void movetoxy_ret(int x,int y)
{
	int x0,y0,dx,dy;
	float dir,range;
	GetInfoEdan(edanid,&x0,&y0,NULL,NULL,NULL,NULL,NULL,NULL);
	dx = x - x0;
	dy = y - y0;

	range = sqrt(dx*dx +dy*dy);
	dir = atan2((float)dy,(float)dx)/M_PI*180.0;

	Move((int)dir,(int)(range+ 0.5));
}

void movetoxy(int x,int y)
{
	int ismoving=1,x1,y1;

	movetoxy_ret(x,y);
	while(ismoving)
	{
		GetInfoEdan(edanid,&x1,&y1,NULL,NULL,&ismoving,NULL,NULL,NULL);
	}
	movetoxy_ret(x,y);
}

int main()
{
	int unrdmsg,resX,resY;
	char m[11],resource;

	edanid = Init("Messager");

	while(1)
	{
		unrdmsg = NewMsgCount();
		if(unrdmsg !=0)
		{
			MsgRecv(m);
			printf("mesg got :: ");
			puts(m);
		}

		GetInfoEdan(edanid,NULL,NULL,&resource,NULL,NULL,NULL,NULL,NULL);
		if(resource != -1)
		{
			switch(resource)
			{
			case VASA:
					printf("got vasa.\n");
					BroadCast("vasa");
					PutResource(270,3);
					break;
			case TATHAR:
					printf("got tathar.\n");
					BroadCast("tathar");
					PutResource(270,3);
					break;
			case MALDA:
					printf("got malda.\n");
					BroadCast("malda");
					PutResource(270,3);
					break;
			}
		}

		if(edanid == 0)
		{
			movetoxy(4,9);
			LocateResource(VASA,&resX,&resY);
			if(resX != -1)
				movetoxy(resX,resY);
		}
		if(edanid == 1)
		{
			movetoxy(15,2);
			if(LocateResource(TATHAR,&resX,&resY))
				movetoxy(resX,resY);
		}
		if(edanid  == 2)
		{
			movetoxy(15,15);
			if(LocateResource(MALDA,&resX,&resY))
				movetoxy(resX,resY);
		}

	}

}

