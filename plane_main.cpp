#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#define GLUT_DISABLE_ATEXIT_HACK 
#include <GL/glut.h>
#include <gl/gl.h>			// Header File For The OpenGL32 Library
#include <gl/GLAUX.H>		// Header File For The Glaux Library

#include <list> 

float WIDTH = 800.0;
float HEIGHT = 600.0;

GLuint texture[3]; 

float planeX = 0.0,planeY = 0.0;

int bulletStart = -1, bulletEnd = -1;
//float bullet[10][2] = {0};
struct bullet
{
	float x;
	float y;
};

struct enemy
{
	float x;
	float y;
};

std::list<bullet> bulletList;
std::list<bullet>::iterator it;

std::list<enemy> enemyList;
std::list<enemy>::iterator enemyIt;

bool mousestate = false;

AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		printf("read image success!\n");
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;										// If Load Failed Return NULL
}


bool load_resource()
{
	bool status = false;

	AUX_RGBImageRec *textureImage[3];
	memset(textureImage,0,sizeof(void*)*1);

	if ((textureImage[0]=LoadBMP("plane.bmp")) && (textureImage[1]=LoadBMP("bullet.bmp")) 
		&& (textureImage[2] = LoadBMP("enemy.bmp")))
	{
		status = true;
		int i;
		glGenTextures(3,&texture[0]);

		for (i = 0; i<3; i++)
		{
			glBindTexture(GL_TEXTURE_2D, texture[i]);

			glTexImage2D(GL_TEXTURE_2D, 0, 3, textureImage[i]->sizeX, textureImage[i]->sizeY, 0, 
				GL_RGB, GL_UNSIGNED_BYTE, textureImage[i]->data);

			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		}

		for (i = 0; i<3; i++)
		{
			if (textureImage[i])
			{	
				if (textureImage[i]->data)
				{
					free(textureImage[i]);
				}
			}
		}
	}
	return status;
}

void convertCoordinate(int& x, int& y)
{
	x = (x-400)/400.0 * 5;
	y =-(y-300)/300.0 * 5;
}

void init(void)
{   
	glClearColor (1.0, 1.0, 1.0, 1.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);

	srand(unsigned(time(NULL)));

	if (!load_resource())
		printf("cannot load image!\n");
}

void displayBullets()
{
	it = bulletList.begin();

	while(it !=bulletList.end())
	{
		//	if (bullet[i][0])
		glPushMatrix();

		glColor3f(1.0,1.0,1.0);
		glTranslatef(it->x,it->y,0.0);
		
		glBindTexture(GL_TEXTURE_2D,texture[1]);
		glBegin(GL_QUADS);

		glTexCoord2f(0.0f,0.0f); glVertex2f(-0.1,-0.1);
		glTexCoord2f(1.0f,0.0f); glVertex2f( 0.1,-0.1);
		glTexCoord2f(1.0f,1.0f); glVertex2f( 0.1, 0.1);
		glTexCoord2f(0.0f,1.0f); glVertex2f(-0.1, 0.1);

		glEnd();
		glPopMatrix();

		it++;
	}
}

void displayEnemy()
{
	enemyIt = enemyList.begin();
	while(enemyIt != enemyList.end())
	{
		glPushMatrix();

		glTranslatef(enemyIt->x,enemyIt->y,0);
		glBindTexture(GL_TEXTURE_2D,texture[2]);

		glColor3f(1.0,1.0,1.0);
		glBegin(GL_QUADS);
		glTexCoord2d(0.0,0.0); glVertex2f(-0.2,-0.2);
		glTexCoord2d(1.0,0.0); glVertex2f( 0.2,-0.2);
		glTexCoord2d(1.0,1.0); glVertex2f( 0.2, 0.2);
		glTexCoord2d(0.0,1.0); glVertex2f(-0.2, 0.2);

		glEnd();

		glPopMatrix();

		enemyIt++;
	}
}

void updateBullet()
{
	it = bulletList.begin();

	while(it !=bulletList.end())
	{
		it->y +=0.4;
		it++;
	}
	if (mousestate)
	{
		bullet tempbullet;
		tempbullet.x = planeX;
		tempbullet.y = planeY + 0.5;
		bulletList.push_back(tempbullet);
	}
	it = bulletList.begin();
	while(it!=bulletList.end())
	{
		if (it->y>5.0)
			it = bulletList.erase(it);
		else
			it++;
	}
//	printf("bullet count = %d\n",bulletList.size());

}

void updateEnemy()
{
	enemyIt = enemyList.begin();

	if ((enemyList.size()<5) && (rand()%5<3))
	{
		enemy tempEnemy;
		tempEnemy.x = rand()%10-5.0;
		tempEnemy.y = 5.0;

		enemyList.push_back(tempEnemy);
	}

	while(enemyIt!= enemyList.end())
	{
		enemyIt->y -= 0.1;
		enemyIt++;
	}

	// delete the bullet out of screen
	enemyIt = enemyList.begin();
	while (enemyIt != enemyList.end())
	{
		if (enemyIt->y < -5)
			enemyIt = enemyList.erase(enemyIt);
		else
			enemyIt ++;
	}
}

// process the collision between the bullet and enemy and spaceship 
void processCollision()
{
	enemyIt = enemyList.begin();
	
	while(enemyIt != enemyList.end())
	{
		it = bulletList.begin();
		bool click = false;
		while (it != bulletList.end())
		{
			/*
			1.check y position between enemy and bullet 
			  whether the distance is less than the total edge of enemy box and bullet box
			2.check x
			*/
			if (abs(enemyIt->y-it->y)<0.40)
			{
				if (abs(enemyIt->x-it->x)<0.30)
				{
					click = true;
					break;
				}
			}
			it++;
		}
		if (click)
		{
			bulletList.erase(it);
			enemyIt = enemyList.erase(enemyIt);
		}
		else
			enemyIt++;
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);

	//
	glColor3f(0.0,1.0,0.0);
	glBegin(GL_LINES);

	glVertex2f(-5.0,0.0);
	glVertex2f( 5.0,0.0);
	glVertex2f( 0.0,5.0);
	glVertex2f( 0.0,-5.0);

	glEnd();
	// draw bullet 
	displayBullets();
	
	// draw enemy
	displayEnemy();

	// draw plane
	glPushMatrix();
	glLoadIdentity();
	glColor3f(1.0,1.0,1.0);
	glTranslatef(planeX, planeY, 0.0);
	glScalef(0.5f,0.5f,0.5f);
	glBindTexture(GL_TEXTURE_2D,texture[0]);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f,0.0f); glVertex2f(-1.0f,-1.0f);
	glTexCoord2f(0.0f,1.0f); glVertex2f(-1.0f,1.0f);
	glTexCoord2f(1.0f,1.0f); glVertex2f(1.0f,1.0f);
	glTexCoord2f(1.0f,0.0f); glVertex2f(1.0f,-1.0f);
	glEnd();
	glPopMatrix();

	

	glutSwapBuffers();
	glFlush();
}

void mouseClick(int btn, int state, int x, int y)
{
	switch(btn)
	{
	case GLUT_LEFT_BUTTON:
		/* when start to click left mouse button , 
		 iterate the bullet list to change the bullets' position by plane
		*/
		{
			switch(state)
			{
			case GLUT_DOWN:
				mousestate = true;
			//	printf("plane position = %d %d\n",x,y);
				
				break;
			case GLUT_UP:
				mousestate = false;
				break;
			}
		}
		break;
	}
}

void processMousePassiveMotion(int x, int y)
{
	float halfWidth = WIDTH/2, halfHeight = HEIGHT/2;
	planeX = (x-halfWidth)/halfWidth * 5;
	planeY =-(y-halfHeight)/halfHeight * 5;
//	printf("plane pos = %f %f\n",planeX, planeY);
	glutPostRedisplay();
}
void idleFunc()
{
//	updateBullet();

	glutPostRedisplay();
}
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	WIDTH = w;
	HEIGHT = h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// ortho z position must be in range of -1.0 to 1.0.
	gluOrtho2D(-5, 5, -5, 5);

	//  gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}
void Timer(int value)
{
//	printf("time = %d\n",time++);

	int i;
	
	processCollision();

	updateBullet();
	updateEnemy();
	

	glutTimerFunc(100,Timer,0);
}

void keyboard (unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutIdleFunc(idleFunc);

	glutTimerFunc(100, Timer, 0);

	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	glutMouseFunc(mouseClick);
	glutMotionFunc(processMousePassiveMotion);
	
	glutMainLoop();
	return 0; 
}
