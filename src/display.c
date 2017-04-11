/*
 *
 *  RISE OF BOTS
 *
 *  Display
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "OsIncl.h"
#include "objects.h"
#include "dispcomm.h"

#include <GL/gl.h>
#include <GL/glut.h>

#ifdef WIN32
	extern void close(socket);
#endif

#define drawLine(x1,y1,x2,y2)  glBegin(GL_LINES);  \
   glVertex2f ((x1),(y1)); glVertex2f ((x2),(y2)); glEnd();

static GLint T0 = 0;
static GLint T00 = 0;
static GLint Frames = 0;
static GLint myWindow;

typedef struct _map
{
	char tile[MAPX][MAPY];
} Map;

static entity objs[100];
static int numobjs;
static score scoreBoard;
static Map map;

void init(void);
void display(void);
void drawobj(entity *);
void drawGround(void);
void showstring(char *,float ,float ,float );
void showscores(void);
void extra(void);
void keyboard(unsigned char,int ,int);
void idle(void);
void reshape(int,int);
void special(int,int,int);

static int s;
static int mode,normal,numbering;
static int width,hight;
unsigned long gtime;
static char gdelay = 2;
static int lookPersX ,lookPersY ,lookPersZ,viewX, viewY ,viewZ ;
static GLint lookOrthoX1,lookOrthoX2,lookOrthoY1,lookOrthoY2;
static int extraHelp;
GLuint theBot ,theCity1,theCity2,theCity3,theCity4;
static const GLdouble clipBottom[] = {0.0, 1.0, 0.0, 0.0};
static const GLdouble clipUp[] = {0.0, -1.0, 0.0, 26};
static const GLdouble clipLeft[] = {1.0, 0.0, 0.0, 0.0};
static const GLdouble clipRight[] = {-1.0, 0.0, 0.0, 26};
static const GLfloat colors[][3] = {
				                {1.0, 0.0, 0.0},
				                {0.0, 0.0, 1.0},
				                {0.7, 0.1, 0.1},
				                {0.1, 0.1, 0.7},
				                {0.9, 0.3, 0.3},
				                {0.3, 0.3, 0.9},
				                {1.0, 1.0, 1.0},
				                {0.0, 0.0, 0.0},
				                {0.2, 0.9, 0.5},
				                {0.7, 0.6, 1.0}
					};

void over(void)
{
	printf("dispclient: Communications Closed.\n");
	close(s);
	socketCleanup();
	exit(0);
}

int main(int argc, char ** argv) 
{
	struct sockaddr_in sa;
	int i;
	
	socketSetup();

	printf("AI Bots1 - Display Client started.\n");

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

	printf("dispclient: Loaded Map\n");

	if(recv(s,(char *)&numobjs,sizeof(int),0) <= 0)
		over();

	printf("dispclient: Number of objects -> %d\n\n",numobjs);

	for(i = 0;i < numobjs;i++)
		objs[i].visible = 0;

	printf("dispclient: Initialising Display ...\n");

	gtime = 0;
	lookPersX = lookPersY =5;
	lookPersZ = 1;
	viewX = 5;
	viewY = 10;
	viewZ = 1;
	mode = 0;
	normal =1;
	lookOrthoX1 = lookOrthoY1 =-1;
	lookOrthoX2 = lookOrthoY2 =26;
	extraHelp = -1;

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800,500);
	glutInitWindowPosition(10,10);
	myWindow = glutCreateWindow("AI Bots - Arena");
	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);

	printf("dispclient: Starting simulation ...\n");
	glutMainLoop();

	over();

	return(0);
}

void idle(void)
{
	entity e;
	int n = 3;

	if(send(s,(char *)&gdelay,1,0) <= 0)
		over();

	if(recv(s,(char *)&gtime,sizeof(gtime),0) <= 0)
		over();
	
	if(recv(s,(char *)&n,sizeof(int),0) <= 0)
		over();
	
	
	for(;n > 0;n--)
	{
		if(recv(s,(char *)&e,sizeof(e),0)<=0)
			over();
		
		if(e.type == SCOREBOARD)
		{
			if(recv(s,(char *)&scoreBoard,sizeof(score),0) <= 0)
				over();
		}
		else
			memcpy(&objs[e.id],&e,sizeof(e));
	}

	
	{
		GLint t1 = glutGet(GLUT_ELAPSED_TIME);
		if (t1 - T00 >= 60)
		{
			T00 = t1;
			glutPostRedisplay();
		}
	}
}


void reshape(int w,int h)
{

	glViewport(0,0,(GLsizei) w,(GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	width = w;
	hight = h;
}


void init (void) 
{
	/*bot gl list*/
	theBot = glGenLists(1);
    glNewList(theBot, GL_COMPILE);
	glPushMatrix();
    glutSolidCone(0.2,0.5,15,15);
	glTranslatef(0,0,0.5);
	glutSolidSphere(0.1,15,15);
	glPopMatrix();
    glEndList();

	/*city1 gl list*/
	theCity1 = glGenLists(2);
	glNewList(theCity1, GL_COMPILE);
	glPushMatrix();
	glutSolidCube(0.2);
	glPopMatrix();
    glEndList();

	/*city3 gl list*/
	theCity2 = glGenLists(3);
	glNewList(theCity2, GL_COMPILE);
	glPushMatrix();
	glutSolidCube(0.4);
	glPopMatrix();
    glEndList();

	/*city5 gl list*/
	theCity3 = glGenLists(4);
	glNewList(theCity3, GL_COMPILE);
	glPushMatrix();
	glutSolidCube(0.6);
	glPopMatrix();
    glEndList();

	/*city7 gl list*/
	theCity4 = glGenLists(5);
	glNewList(theCity4, GL_COMPILE);
	glPushMatrix();
	glutSolidCube(0.9);
	glPopMatrix();
    glEndList();


/*  select clearing (background) color       */
    glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);

/*  initialize viewing values  */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key,int x,int y)
{

	switch(key)
	{

	case 'q':
	case 'Q':
		exit(0);break;

	case ' ': mode++; if(mode >= 2) mode = 0;
			  break;

	case 'x': if(mode == 0) {lookOrthoX1--; lookOrthoX2--;} 
			  if(mode == 1) {lookPersX -=1;}
			  break;
	case 'X': if(mode == 0) {lookOrthoX1++; lookOrthoX2++;}
			  if(mode == 1) {lookPersX +=1 ;}
			  break;
	case 'y': if(mode == 0) {lookOrthoY1--; lookOrthoY2--;}
			  if(mode == 1) {lookPersY -=1;}
			  break;
	case 'Y': if(mode == 0) {lookOrthoY1++; lookOrthoY2++;}
			  if(mode == 1) {lookPersY +=1 ;}
			  break;
	case 'z': if(mode == 0) {lookOrthoX1++; lookOrthoX2--;lookOrthoY1++; lookOrthoY2--;}
			  if(mode == 1) {lookPersZ--;}
			  break;
	case 'Z': if(mode == 0) {lookOrthoX1--; lookOrthoX2++;lookOrthoY1--; lookOrthoY2++;}
			  if(mode == 1) {lookPersZ++;}
			  break;

	case 'a': lookPersX -= 2; viewX -=2; break;
	case 'd': lookPersX += 2; viewX +=2; break;
	case 's': lookPersY -= 2; viewY -=2; break;
	case 'w': lookPersY += 2; viewY +=2; break;
	case 'n': numbering = !numbering; break;

	case '+': gdelay--;
				if(gdelay < 1) gdelay = 1; break;
	case '-': gdelay++;
				if(gdelay > 9) gdelay = 9; break; 
	}
	
	idle();
}

void special(int k, int x, int y)
{
  switch (k) {
  case GLUT_KEY_UP:
	extraHelp--;
    	if(extraHelp < -1)
    		extraHelp = -1;
    	break;
  case GLUT_KEY_DOWN:
  	extraHelp++;
   	if(extraHelp > 5)
    		extraHelp = 5;
    	break;
  case GLUT_KEY_LEFT:
    break;
  case GLUT_KEY_RIGHT:
    break;
  default:
    return;
  }
  idle();
}


void display(void)
{
	int i,ns = 0;

	glClear(GL_COLOR_BUFFER_BIT);
	
	glPushMatrix();
	if(mode == 0)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (width <= hight)
			gluOrtho2D(lookOrthoX1, lookOrthoX2, lookOrthoY1, lookOrthoY2 * (GLfloat) hight / (GLfloat) width);
		else
			gluOrtho2D(lookOrthoX1, lookOrthoX2 * (GLfloat) width / (GLfloat) hight, lookOrthoY1, lookOrthoY2);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	else if(mode == 1)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45,1,0,100);
		gluLookAt(lookPersX,lookPersY,lookPersZ,12.5,12.5,0,0,0,1);
		glMatrixMode(GL_MODELVIEW);
	}

	glColor3f(1,1,1);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glBegin(GL_POLYGON);
	glVertex3f(-0.5,-0.5,0);
	glVertex3f(-0.5,25.5,0);
	glVertex3f(25.5,25.5,0);
	glVertex3f(25.5,-0.5,0);
	glEnd();

	/*draw the ground*/
	drawGround();

	for(i = 0;i < numobjs;i++)
		drawobj(&objs[i]);

	glPopMatrix();

	if(mode == 0)
	{
		showscores();
		if(extraHelp > -1)
			extra();
	}

	glutSwapBuffers();
	//glGetError();
}

void drawobj(entity * e)
{
	GLfloat blue[4] ={0.35,0.33,0.30};
	int dir;
	
	if(e->visible == 0 )
		return;

	glPushMatrix();

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	switch(e->type)
	{
	case PLAYER :
		
		glColor3fv(colors[e->id]);

		dir = (float)e->dir;

		glTranslatef((float)e->flx/10.0 ,(float)e->fly/10.0 ,0);
		glRotatef(dir, 0.0, 0.0, 1.0);

		if(mode == 0 || mode == 1)
		{
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			glBegin(GL_POLYGON);
				glVertex3f(0.3,0.0,0.0);
				glVertex3f(-0.3,-0.2,0.0);
				glVertex3f(-0.3,0.2,0.0);
			glEnd();
		}
		if(mode == 1)
			glCallList(theBot);


		switch(objs[e->plGift].type)
		{
			case GIFTFOOD : glColor3f(0,1,1); glutSolidSphere(0.1,10,10); break;
			case GIFTTIMBER : glColor3f(1,1,0); glutSolidSphere(0.1,10,10); break;
			case GIFTMETAL : glColor3f(1,0,1); glutSolidSphere(0.1,10,10); break;
		}

		break;

	case BOMB :
		glColor3fv(colors[e->dir]);

		glTranslatef(e->x + 0.5,e->y + 0.5,0);
		if(mode == 0 || mode == 1)
		{
			glutWireCone(e->stage/1.5f,0,10,4);
		}
		break; 
	case CITY :
		if(e->teamid == 0)
			glColor3fv(colors[0]);
		else
			glColor3fv(colors[1]);

		glTranslatef(e->x + 0.5,e->y + 0.5,0);

		if(mode == 0 || mode == 1)
		{
			switch(e->stage)
			{
			case 1:	glutSolidCube(0.4);
					break;
			case 2: glutSolidCube(0.6);
					break;
			case 3: glutSolidCube(0.7);
					break;
			}
		}
		break;
	case GIFTFOOD :
		glColor3f(0.0,1.0,1.0);
		glTranslatef(e->x + 0.5,e->y + 0.5,0);
			glutWireSphere(0.3,15,2);
		break;
	case GIFTTIMBER : 
		glColor3f(0.25,0.20,0.14);
		glTranslatef(e->x + 0.5,e->y + 0.5,0);

			glLineWidth (5.0);
			drawLine(-.3,-.2,.3,.2);
			glTranslatef(.12,-.12,0);
			drawLine(-.3,-.2,.3,.2);
			glTranslatef(-.24,.24,0);
			drawLine(-.3,-.2,.3,.2);

			glLineWidth (1.0);
		break;
	case GIFTMETAL :
		glColor3f(0.5,0.5,0.5);
		glTranslatef(e->x + 0.5,e->y + 0.5,0);
		glScalef(0.4,0.4,0.4);
		glutWireIcosahedron();
		break;
	}

	glPopMatrix();
}

void drawGround()
{
	int i,j;
	glColor3f(0.3,0.3,0.3);

	for( i = 0; i <= MAPX; i += 1)
	{
		glBegin(GL_LINES);

			// Do the horizontal lines (along the X)
			glVertex3f(0, i, 0);
			glVertex3f(MAPY, i, 0);

			// Do the vertical lines (along the Y)
			glVertex3f(i, 0, 0);
			glVertex3f(i, MAPY, 0);

		// Stop drawing lines
		glEnd();
	}

	glPolygonMode(GL_FRONT,GL_FILL);
	for(i = 0;i < MAPX;i++)
		for(j = 0;j < MAPY;j++)
			if(map.tile[i][j] == WALL)
			{
			   glPushMatrix();
			   glTranslatef(i+0.5,j+0.5,0);
			   glutSolidCube(1.0);
			   glPopMatrix();
			}

}

void showstring(char * s,float x,float y,float z)
{
	int i;
	//glColor3f(1.0,1.0,1.0);
	glRasterPos3f(x,y,z);
	for(i = 0;s[i];i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,s[i]);
}

void extra(void)
{
	char str[256];
	int i = extraHelp;

	if(extraHelp <= 5)
	{
	sprintf(str,"Edan %d ::  \t\t\tId: %d",extraHelp,extraHelp);
	showstring(str,26,12,0);

	if(objs[i].visible)
	{
		sprintf(str,"Name = %s",scoreBoard.pl[i].name);
		showstring(str,29,11,0);

		sprintf(str,"Team = %d",objs[i].teamid);
		showstring(str,29,10,0);

		sprintf(str,"x = %.1f y = %.1f",objs[i].flx/10.0,objs[i].fly/10.0);
		showstring(str,29,9,0);

		sprintf(str,"Dir = %d",objs[i].dir);
		showstring(str,29,8,0);

		if(objs[i+6].visible)
			sprintf(str,"Bomb : %d %d",objs[i+6].x,objs[i+6].y);
		else
			sprintf(str,"Bomb : NULL");
		showstring(str,29,7,0);

		sprintf(str,"Recent Msg Got: %s",scoreBoard.pl[i].lastmsg);
		showstring(str,29,6,0);

		switch(objs[objs[i].plGift].type)
		{
		case GIFTFOOD : sprintf(str,"Resource: Vasa"); break;
		case GIFTTIMBER : sprintf(str,"Resource: Tathar"); break;
		case GIFTMETAL : sprintf(str,"Resource: Malda"); break;
		default : sprintf(str,"Resource: NONE");break;
		}
		showstring(str,29,5,0);

		switch(objs[i].stage)
		{
		case TURBO: sprintf(str,"Upgraded: Turbo"); break;
		case RANGEDBOMB: sprintf(str,"Upgraded: Ranged Bomb"); break;
		case GOLDENEYE: sprintf(str,"Upgraded: Golden Eye"); break;
		default: sprintf(str,"Upgraded: No"); break;
		}
		showstring(str,29,4,0);
	}
	else { if(!scoreBoard.pl[i].deactivated)
	{
		sprintf(str,"DEAD !!");
		showstring(str,29,11,0);
	}
	else
		{
		sprintf(str,"Deactivated");
		showstring(str,29,11,0);
		}
	}
	}

}

void showscores(void)
{
	int i,j,t,gf[3];
	float pos;
	char str[256];

	glColor3f(1,1,1);
	pos = 25;
	sprintf(str,"Gametime  -->  %lu",gtime);
	showstring(str,26,pos,0);

	pos -= 1;
	sprintf(str,"GameSpeed  -->  %dx",10-gdelay);
	showstring(str,26,pos,0);

	///team1
	glColor3fv(colors[0]);
	pos -= 2;
	showstring(scoreBoard.pl[0].name,26,pos,0);  //pos = 22
	sprintf(str,"\tScore : %d",scoreBoard.team1score);
	showstring(str,33,pos,0);
	glColor3f(1,1,1);
	for(i = 0;i < scoreBoard.numpl;i+=2)
	{
		if(objs[i].visible)
		{
			sprintf(str,"Edan %d [Id: %d] \tSc: %4d \tHt : %3d \tRe: ",i/2,i,scoreBoard.pl[i].sc,scoreBoard.pl[i].life);
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
			sprintf(str,"Edan %d [Id: %d] \tSc: %d \t NOW DEAD !!",i/2,i,scoreBoard.pl[i].sc);
		else
			sprintf(str,"Edan %d [Id: %d] \tSc: %d \t DEACTIVATED",i/2,i,scoreBoard.pl[i].sc);

		pos -= 0.7;
		showstring(str,26,pos,0);
	}
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
				sprintf(str,"Nost %d [Id: %d]\tSc: %d \tHt: %d\t Vs: %d\t Ta: %d\t Ma: %d",
					t,i+12,scoreBoard.ct[i].sc,scoreBoard.ct[i].life,gf[0],gf[1],gf[2]);

				pos -= 0.7;
				showstring(str,26,pos,0);
			}
			else
			{
				sprintf(str,"Nost %d [Id: %d]\tSc: %d   DEAD !!",t,i+12,scoreBoard.ct[i].sc);

				pos -= 0.7;
				showstring(str,26,pos,0);
			}
		}		
	}



	///////team 2

	glColor3fv(colors[1]);
	pos -= 1;
	showstring(scoreBoard.pl[1].name,26,pos,0);
	sprintf(str,"\tScore : %d",scoreBoard.team2score);
	showstring(str,33,pos,0);
	glColor3f(1,1,1);
	for(i = 1;i < scoreBoard.numpl;i+=2)
	{
		if(objs[i].visible)
		{
			sprintf(str,"Edan %c [Id: %d] \tSc: %4d \tHt : %3d \tRe:",'a'+i/2,i,scoreBoard.pl[i].sc,scoreBoard.pl[i].life);
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
			sprintf(str,"Edan %c [Id: %d] \tSc: %d \t NOW DEAD !!",'a'+i/2,i,scoreBoard.pl[i].sc);
		else
			sprintf(str,"Edan %c [Id: %d] \tSc: %d \t DEACTIVATED",'a'+i/2,i,scoreBoard.pl[i].sc);

		pos -= 0.7;
		showstring(str,26,pos,0);
	}
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
				sprintf(str,"Nost %d [Id: %d]\tSc: %d \tHt: %d\t Vs: %d\t Ta: %d\t Ma: %d",
					t,i+12,scoreBoard.ct[i].sc,scoreBoard.ct[i].life,gf[0],gf[1],gf[2]);

				pos -= 0.7;
				showstring(str,26,pos,0);
			}
			else
			{
				sprintf(str,"Nost %d [Id: %d]\tSc: %d   DEAD !!",t,i+12,scoreBoard.ct[i].sc);

				pos -= 0.7;
				showstring(str,26,pos,0);
			}
		}
	}


	if(numbering)
	{
		for(i=0;i<6;i++)
		{
			if(!objs[i].visible)
				continue;
			if(objs[i].teamid == 0)
				sprintf(str,"%d",objs[i].id/2);
			else
				sprintf(str,"%c",'a'+objs[i].id/2);
			showstring(str,objs[i].flx/10.0 +1,objs[i].fly/10.0 -1,0);
		}
	}

}

