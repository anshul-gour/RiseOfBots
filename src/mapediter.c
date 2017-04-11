/*
 *
 *  RISE OF BOTS
 *
 *  Map Editer
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>

#ifdef WIN32
	extern void close(socket);
#endif

#define MAPX 25
#define MAPY 25
#define WALL 99

typedef struct _map
{
	char tile[MAPX][MAPY];
} Map;

static Map map;

void init(void);
void display(void);
void drawGround(void);
void showstring(char *,float ,float ,float );
void keyboard(unsigned char,int ,int);
void reshape(int,int);
void special(int,int,int);
void showPos(void);

static int mode,edit;
static int putX,putY;
static char fname[20];
static int width,hight;
static int lookPersX ,lookPersY ,lookPersZ,viewX, viewY ,viewZ ;
static GLint lookOrthoX1,lookOrthoX2,lookOrthoY1,lookOrthoY2;
static const GLfloat colors[][3] = {
				                {1.0, 0.0, 0.0},
				                {0.0, 0.0, 1.0},
				                {0.7, 0.1, 0.1},
				                {0.1, 0.1, 0.7},
				                {0.9, 0.3, 0.3},
				                {0.3, 0.3, 0.9}
					};

void over(void)
{
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


int main(int argc, char ** argv) 
{
	printf("AI Bots1 - Map Editer.\n");

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

	lookPersX = lookPersY =5;
	lookPersZ = 1;
	viewX = 5;
	viewY = 10;
	viewZ = 1;
	mode = 0;
	edit = 0;
	lookOrthoX1 = lookOrthoY1 =-1;
	lookOrthoX2 = lookOrthoY2 =26;

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(500,520);
	glutInitWindowPosition(10,10);
	glutCreateWindow("AI Bots - Map Editer");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);

	glutMainLoop();

	over();

	return(0);
}

void reshape(int w,int h)
{

	glViewport(0,0,(GLsizei) w,(GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	width = w;
	hight = h;

	display();
}


void init (void) 
{

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
	int i,j;
	FILE *fw;

	switch(key)
	{

	case 'q':
	case 'Q':
		over();break;

	//case ' ': mode++; if(mode >= 2) mode = 0;break;

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
	case ' ': edit = 1;
			  if(map.tile[putX][putY] == WALL)
				  map.tile[putX][putY] = -1;
			  else
				  map.tile[putX][putY] = WALL;
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

void special(int k, int x, int y)
{
  switch (k) {
  case GLUT_KEY_UP:
	putY++;
    	if(putY > 24)
    		putY = 24;
    	break;
  case GLUT_KEY_DOWN:
  	putY--;
   	if(putY < 0)
    		putY = 0;
    	break;
  case GLUT_KEY_LEFT:
    	putX--;
    	if(putX < 0)
    		putX = 0;
    	break;
  case GLUT_KEY_RIGHT:
   	putX++;
    	if(putX > 24)
    		putX = 24;
    	break;
  default:
    return;
  }
  display();
}


void display(void)
{
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

	glColor3f(1,1,1);
	glTranslatef(putX+0.5,putY+0.5,0);
	glutWireCube(1.0);

	glPopMatrix();

	if(mode == 0) showPos();

	glutSwapBuffers();
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
	glColor3f(1.0,1.0,1.0);
	glRasterPos3f(x,y,z);
	for(i = 0;s[i];i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,s[i]);
}

void showPos(void)
{
	char str[255];
	sprintf(str,"Curser position :: x: %d y: %d",putX,putY);
	showstring(str,3,26,0);
	if(edit != 0)
	{
		showstring("For save the changes press 's'.",15,26,0);
	}
}
