//////////////////////////////////////////////////////////////////////////////////////         
// spaceTravelFrustumCulled.cpp
//
// This program is based on spaceTravel.cpp with an added frustum culling option.
// It draws a conical spacecraft that can travel and an array of fixed spherical 
// asteroids. The view in the left viewport is from a fixed camera; the view in 
// the right viewport is from the spacecraft.There is approximate collision detection.  
// Frustum culling is implemented by means of a quadtree data structure.
// 
// COMPILE NOTE: File intersectionDetectionRoutines.cpp must be in the same folder.
// EXECUTION NOTE: If ROWS and COLUMNS are large the quadtree takes time to build so
//                 the display may take several seconds to come up.
//
// User-defined constants: 
// ROWS is the number of rows of  asteroids.
// COLUMNS is the number of columns of asteroids.
// FILL_PROBABILITY is the percentage probability that a particular row-column slot
// will be filled with an asteroid.
//
// Interaction:
// Press the left/right arrow keys to turn the craft.
// Press the up/down arrow keys to move the craft.
// Press space to toggle between frustum culling enabled and disabled.
// 
// Sumanta Guha.
// C/C++ version: Jessica Bayliss
////////////////////////////////////////////////////////////////////////////////////// 
#include <malloc.h>
#include <cstdlib>
#include <ctime> 
#include <cmath>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <glm/glm.hpp>
#include <list>
#include <vector>
#include "intersectionDetectionRoutines.h"
#include "Asteroid.h"
#include "QuadTree.h"

using namespace std;

#pragma comment ( lib, "opengl32.lib" )
#pragma comment ( lib, "glew32.lib" )
#pragma comment ( lib, "glfw3.lib" )

#define ROWS 100  // Number of rows of asteroids.
#define COLUMNS 100 // Number of columns of asteroids.
#define FILL_PROBABILITY 100 // Percentage probability that a particular row-column slot will be 
                             // filled with an asteroid. It should be an integer between 0 and 100.
#define WINDOW_X 1600
#define WINDOW_Y 800

// Globals.
static int width, height; // Size of the OpenGL window.
static float angle = 0.0; // Angle of the spacecraft.
static float xVal = 0, zVal = 0; // Co-ordinates of the spacecraft.
static int isFrustumCulled = 0;
static int isCollision = 0; // Is there collision between the spacecraft and an asteroid?


// vertex counting for where everything goes in the global array
// fixed number of vertices for cone and sphere
#define CONE_VERTEX_COUNT 12
#define LINE_VERTEX_COUNT 2
// #define SPHERE_VERTEX_COUNT 288 // moved to Asteroids.h because asteroids draw themselves

// initial indices where data starts getting drawn for different data types
int cone_index = 0;
int line_index = cone_index + CONE_VERTEX_COUNT;
int sphere_index = line_index + LINE_VERTEX_COUNT;

// shader stuff
glm::vec3 points[CONE_VERTEX_COUNT+LINE_VERTEX_COUNT+SPHERE_VERTEX_COUNT*ROWS*COLUMNS]; // addition of all rows/cols for asteroid vertices + spaceship vertices + line vertices  
GLuint  myShaderProgram;
GLuint InitShader(const char* vShaderFile, const char* fShaderFile);
GLuint	myBuffer;

// the asteroids and quad tree from the initial program
Asteroid **arrayAsteroids; // Global array of asteroids.
Quadtree asteroidsQuadtree; // Global quadtree.

//static long font = (long)GLUT_BITMAP_8_BY_13; // Font selection.
// Routine to draw a bitmap character string.
// DOES NOT WORK WITHOUT GLUT
void writeBitmapString(void *font, char *string)
{  
   char *c;
//   for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
} 

// function obtained from tutorial at:
// http://www.freemancw.com/2012/06/opengl-cone-function/
// used in drawing a cone
glm::vec3  perp(const glm::vec3 &v) {
	float min = fabs(v.x);
	glm::vec3 cardinalAxis( 1, 0, 0 );

	if (fabs(v.y) < min) {
		min = fabs(v.y);
		cardinalAxis = glm::vec3( 0, 1, 0);
	}

	if (fabs(v.z) < min) {
		cardinalAxis = glm::vec3(0, 0, 1);
	}

	return glm::cross(v, cardinalAxis);
}

// function derived from tutorial at:
// http://www.freemancw.com/2012/06/opengl-cone-function/
void CreateCone(const glm::vec3 &d, const glm::vec3 &a,
	const float h, const float rd, const int n, int offset) {
	glm::vec3 c;
	c.x = a.x + (-d.x * h);
	c.y = a.y + (-d.y * h);
	c.z = a.z + (-d.z * h);
	glm::vec3 e0 = perp(d);
	glm::vec3 e1 = glm::cross(e0, d);
	float angInc = 360.0 / n * (PI / 180.0f);

	// calculate points around directrix
	vector <glm::vec3> pts;
	for (int i = 0; i < n; ++i) {
		float rad = angInc * i;
		glm::vec3 p = c + (((e0 * cos(rad)) + (e1 * sin(rad))) * rd);
		pts.push_back(p);
	}

	// draw cone top
	int i = 0;
	int o = offset + i;
	points[o].x = a.x;
	points[o].y = a.y;
	points[o].z = a.z;

	for (i = 1; i < n+1; ++i) {
		o = i + offset;
		points[o].x = pts[i - 1].x;
		points[o].y = pts[i - 1].y;
		points[o].z = pts[i - 1].z;
	}

	o = i + offset;
	points[o].x = pts[0].x;
	points[o].y = pts[0].y;
	points[o].z = pts[0].z;

	// original tutorial has cone bottom
	// not necessary when cone is a spaceship!
}

// function derived from tutorial at:
// http://www.swiftless.com/tutorials/opengl/sphere.html
void CreateSphere(double R, double H, double K, double Z, int offset) {
	int n;
	double a;
	double b;
	n = 0;
	const int space = 30;

	// draw the front half
	for (b = 0; b <= 90 - space; b += space){
		for (a = 0; a <= 360 - space; a += space){

			points[n + offset].x = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
			points[n + offset].y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
			points[n + offset].z = R * cos((b) / 180 * PI) - Z;
			n++;

			points[n + offset].x = R * sin((a) / 180 * PI) * sin((b + space) / 180 * PI) - H;
			points[n + offset].y = R * cos((a) / 180 * PI) * sin((b + space) / 180 * PI) + K;
			points[n + offset].z = R * cos((b + space) / 180 * PI) - Z;
			n++;
			points[n + offset].x = R * sin((a + space) / 180 * PI) * sin((b) / 180 * PI) - H;
			points[n + offset].y = R * cos((a + space) / 180 * PI) * sin((b) / 180 * PI) + K;
			points[n + offset].z = R * cos((b) / 180 * PI) - Z;
			n++;
			points[n + offset].x = R * sin((a + space) / 180 * PI) * sin((b + space) / 180 * PI) - H;
			points[n + offset].y = R * cos((a + space) / 180 * PI) * sin((b + space) / 180 * PI) + K;
			points[n + offset].z = R * cos((b + space) / 180 * PI) - Z;
			n++;
		}
	}

	// draw the back half
	for (b = 0; b <= 90 - space; b += space){
		for (a = 0; a <= 360 - space; a += space){

			points[n + offset].x = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
			points[n + offset].y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
			points[n + offset].z = -1 * (R * cos((b) / 180 * PI)) - Z;
			n++;

			points[n + offset].x = R * sin((a) / 180 * PI) * sin((b + space) / 180 * PI) - H;
			points[n + offset].y = R * cos((a) / 180 * PI) * sin((b + space) / 180 * PI) + K;
			points[n + offset].z = -1 * (R * cos((b + space) / 180 * PI)) - Z;
			n++;
			points[n + offset].x = R * sin((a + space) / 180 * PI) * sin((b) / 180 * PI) - H;
			points[n + offset].y = R * cos((a + space) / 180 * PI) * sin((b) / 180 * PI) + K;
			points[n + offset].z = -1 * (R * cos((b) / 180 * PI)) - Z;
			n++;
			points[n + offset].x = R * sin((a + space) / 180 * PI) * sin((b + space) / 180 * PI) - H;
			points[n + offset].y = R * cos((a + space) / 180 * PI) * sin((b + space) / 180 * PI) + K;
			points[n + offset].z = -1 * (R * cos((b + space) / 180 * PI)) - Z;
			n++;
		}
	}

}


// OpenGL window reshape routine.
void resize(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 250.0);
	glMatrixMode(GL_MODELVIEW);

	// Pass the size of the OpenGL window.
	width = w;
	height = h;
}

// Initialization routine.
void setup(void) 
{
   int i, j;
   float initialSize;
   // create meory for each potential asteroid
   arrayAsteroids = new Asteroid *[ROWS];
   for (int i = 0; i < ROWS; i++) {
	   arrayAsteroids[i] = new Asteroid[COLUMNS];
   }

   // create the quad tree for the asteroids
   asteroidsQuadtree.setRowsCols(ROWS, COLUMNS);
   asteroidsQuadtree.setArray(arrayAsteroids);

   // create the line for the middle of the screen
   points[line_index].x = 0;
   points[line_index].y = -5;
   points[line_index].z = -6;
   points[line_index + 1].x = 0;
   points[line_index + 1].y = 5;
   points[line_index + 1].z = -6;

   // create the cone for a spaceship
   glm::vec3 direction( 0, 1, 0 );
   glm::vec3 apex(0, 10, 0);
   CreateCone(direction, apex, 10, 5, 10, cone_index);

   // create where the spheres are going in the field   
   int index = sphere_index;
   // Initialize global arrayAsteroids.
   for (j=0; j<COLUMNS; j++)
      for (i=0; i<ROWS; i++)
		  if (rand() % 100 < FILL_PROBABILITY)
			  // If rand()%100 >= FILL_PROBABILITY the default constructor asteroid remains in the slot which
			  // indicates that there is no asteroid there because the default's radius is 0.
		  {
	   // Position the asteroids depending on if there is an even or odd number of columns
	   // so that the spacecraft faces the middle of the asteroid field.
	   if (COLUMNS % 2) // Odd number of columns. 
	   {
		   arrayAsteroids[i][j] = Asteroid(30.0*(-COLUMNS / 2 + j), 0.0, -40.0 - 30.0*i, 3.0,
			   rand() % 256, rand() % 256, rand() % 256);
		   arrayAsteroids[i][j].setIndex(index);
		   CreateSphere(SPHERE_SIZE, 0, 0, 0, index);
		   index += SPHERE_VERTEX_COUNT;
	   }
	   else // Even number of columns. 
	   {
		   arrayAsteroids[i][j] = Asteroid(15.0 + 30.0*(-COLUMNS / 2 + j), 0.0, -40.0 - 30.0*i, 3.0,
			   rand() % 256, rand() % 256, rand() % 256);
		   arrayAsteroids[i][j].setIndex(index);
		   CreateSphere(SPHERE_SIZE, 0, 0, 0, index);
		   index += SPHERE_VERTEX_COUNT;
	   }
		  }

   // Initialize global asteroidsQuadtree - the root square bounds the entire asteroid field.
   if (ROWS <= COLUMNS) initialSize = (COLUMNS - 1)*30.0 + 6.0;
   else initialSize = (ROWS - 1)*30.0 + 6.0;
   asteroidsQuadtree.initialize( -initialSize/2.0, -37.0, initialSize );
   
   // initialize the graphics
   glEnable(GL_DEPTH_TEST);
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glViewport(0, 0, (GLsizei)WINDOW_X, (GLsizei)WINDOW_Y);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 250.0);
   glMatrixMode(GL_MODELVIEW);

   // Create a vertex array object
   GLuint vao;
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);

   // Create and initialize a buffer object for each circle
   GLuint aBuffer;
   glGenBuffers(1, &aBuffer);
   myBuffer = aBuffer;
   glBindBuffer(GL_ARRAY_BUFFER, myBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

   // Load shaders and use the resulting shader program
   GLuint program = InitShader("vshader.glsl", "fshader.glsl");
   myShaderProgram = program;
   glUseProgram(myShaderProgram);

   // Initialize the vertex position attribute from the vertex shader
   GLuint loc = glGetAttribLocation(myShaderProgram, "vPosition");
   glEnableVertexAttribArray(loc);
   glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

}

// Function to check if two spheres centered at (x1,y1,z1) and (x2,y2,z2) with
// radius r1 and r2 intersect.
int checkSpheresIntersection(float x1, float y1, float z1, float r1, 
						       float x2, float y2, float z2, float r2)
{
   return ( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2) <= (r1+r2)*(r1+r2) );
}

// Function to check if the spacecraft collides with an asteroid when the center of the base
// of the craft is at (x, 0, z) and it is aligned at an angle a to to the -z direction.
// Collision detection is approximate as instead of the spacecraft we use a bounding sphere.
int asteroidCraftCollision( float x, float z, float a)
{
   int i,j;

   // Check for collision with each asteroid.
   for (j=0; j<COLUMNS; j++)
      for (i=0; i<ROWS; i++)
		 if (arrayAsteroids[i][j].getRadius() > 0 ) // If asteroid exists.
            if ( checkSpheresIntersection( x - 5 * sin( (PI/180.0) * a), 0.0, 
		         z - 5 * cos( (PI/180.0) * a), 7.072, 
		         arrayAsteroids[i][j].getCenterX(), arrayAsteroids[i][j].getCenterY(), 
		         arrayAsteroids[i][j].getCenterZ(), arrayAsteroids[i][j].getRadius() ) )
		       return 1;
   return 0;
}

// function taken from glu
void lookAt(float eyex, float eyey, float eyez, float centerx,
float centery, float centerz, float upx, float upy, float upz)
{
	int i;
	glm::vec3 forward, side, up;
	float m[4][4];

	// create identity matrix
	for (int i = 0; i < 4; i++) {
		m[i][0] = 0;
		m[i][1] = 0;
		m[i][2] = 0;
		m[i][3] = 0;
	}

	m[0][0] = 1.0f;
	m[1][1] = 1.0f;
	m[2][2] = 1.0f;
	m[3][3] = 1.0f;

	// glu code for lookat
	forward.x = centerx - eyex;
	forward.y = centery - eyey;
	forward.z = centerz - eyez;

	up.x = upx;
	up.y = upy;
	up.z = upz;

	forward = glm::normalize(forward);

	// Side = forward x up 
	side = glm::cross(forward, up);
	side = glm::normalize(side);

	// Recompute up as: up = side x forward 
	up = glm::cross(side, forward);

	m[0][0] = side[0];
	m[1][0] = side[1];
	m[2][0] = side[2];

	m[0][1] = up[0];
	m[1][1] = up[1];
	m[2][1] = up[2];

	m[0][2] = -forward[0];
	m[1][2] = -forward[1];
	m[2][2] = -forward[2];

	glMultMatrixf((const GLfloat *)m[0]);
	glTranslated(-eyex, -eyey, -eyez);
}


// Drawing routine.
void drawScene(void)
{ 
   int i, j;
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Use the buffer and shader for each circle.
   glUseProgram(myShaderProgram);
   glBindBuffer(GL_ARRAY_BUFFER, myBuffer);

   // Initialize the vertex position attribute from the vertex shader.
   GLuint loc = glGetAttribLocation(myShaderProgram, "vPosition");
   glEnableVertexAttribArray(loc);
   glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
   // copy over global points array in case anything has changed
   // very slow when points is just HUGE
   glBindBuffer(GL_ARRAY_BUFFER, myBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

   // Begin left viewport.
   glViewport (0, 0, width/2.0,  height); 
   glLoadIdentity();
   
   // Write text in isolated (i.e., before gluLookAt) translate block.
   // DOES NOT WORK WITHOUT GLUT 
   //glPushMatrix();
   //glColor3f(1.0, 1.0, 1.0);
   //glRasterPos3f(5.0, 25.0, -30.0);
   //if (isFrustumCulled) writeBitmapString((void*)font, "Frustum culling on!");
   //else writeBitmapString((void*)font, "Frustum culling off!");
   //glColor3f(1.0, 0.0, 0.0);
   //glRasterPos3f(-28.0, 25.0, -30.0);
   //if (isCollision) writeBitmapString((void*)font, "Cannot - will crash!");
   //glPopMatrix();
   
   // Fixed camera 
   lookAt(0.0, 10.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

   if (!isFrustumCulled)
   {
	   // Draw all the asteroids in arrayAsteroids.
	   for (j = 0; j<COLUMNS; j++)
         for (i=0; i<ROWS; i++)
		   arrayAsteroids[i][j].draw();
   }
   else
   {
	   // Draw only asteroids in leaf squares of the quadtree that intersect the fixed frustum
	   // with apex at the origin.
	   asteroidsQuadtree.drawAsteroids(-5.0, -5.0, -250.0, -250.0, 250.0, -250.0, 5.0, -5.0);
   }

   // off is white spaceship and on it red
   if (isFrustumCulled)
	glColor3f(1.0, 0.0, 0.0);
   else 
	glColor3f(1.0, 1.0, 1.0);

   // spacecraft moves and so we translate/rotate according to the movement
   glPushMatrix();
   glTranslatef(xVal, 0, zVal);
   glRotatef(angle, 0.0, 1.0, 0.0);

   glPushMatrix();
   glRotatef(-90.0, 1.0, 0.0, 0.0); // To make the spacecraft point down the $z$-axis initially.

   // Turn on wireframe mode
   glPolygonMode(GL_FRONT, GL_LINE);
   glPolygonMode(GL_BACK, GL_LINE);
   glDrawArrays(GL_TRIANGLE_FAN, cone_index, CONE_VERTEX_COUNT);
   // Turn off wireframe mode
   glPolygonMode(GL_FRONT, GL_FILL);
   glPolygonMode(GL_BACK, GL_FILL);
   glPopMatrix();
   // End left viewport.
   
   // Begin right viewport.
   glViewport(width/2.0, 0, width/2.0, height);
   glLoadIdentity();

   // Write text in isolated (i.e., before gluLookAt) translate block.
   // DOES NOT WORK WITHOUT GLUT
   //glPushMatrix();
   //glColor3f(1.0, 1.0, 1.0);
   //glRasterPos3f(5.0, 25.0, -30.0);
   //if (isFrustumCulled)  writeBitmapString((void*)font, "Frustum culling on.");
   //else writeBitmapString((void*)font, "Frustum culling off.");
   //glColor3f(1.0, 0.0, 0.0);
   //glRasterPos3f(-28.0, 25.0, -30.0);
   //if (isCollision)  writeBitmapString((void*)font, "Cannot - will crash!");
   //glPopMatrix();

   // draw the line in the middle to separate the two viewports
   glPushMatrix();
   glTranslatef(-6, 0, 0);
   glColor3f(1.0, 1.0, 1.0);
   glLineWidth(2.0);
   glDrawArrays(GL_LINE_STRIP, line_index, LINE_VERTEX_COUNT);
   glLineWidth(1.0);
   glPopMatrix();

   // Locate the camera at the tip of the cone and pointing in the direction of the cone.
   lookAt(xVal - 10 * sin( (PI/180.0) * angle), 
	         0.0, 
			 zVal - 10 * cos( (PI/180.0) * angle), 
	         xVal - 11 * sin( (PI/180.0) * angle),
			 0.0,
             zVal - 11 * cos( (PI/180.0) * angle), 
             0.0, 
			 1.0, 
			 0.0);

   if (!isFrustumCulled)
   {
	   // Draw all the asteroids in arrayAsteroids.
	   for (j = 0; j<COLUMNS; j++)
         for (i=0; i<ROWS; i++)
            arrayAsteroids[i][j].draw();
   }
   else
   {
	   // Draw only asteroids in leaf squares of the quadtree that intersect the frustum
	   // "carried" by the spacecraft with apex at its tip and oriented with its axis
	   // along the spacecraft's axis.
	   asteroidsQuadtree.drawAsteroids(xVal - 7.072 * sin((PI / 180.0) * (45.0 + angle)),
		   zVal - 7.072 * cos((PI / 180.0) * (45.0 + angle)),
		   xVal - 353.6 * sin((PI / 180.0) * (45.0 + angle)),
		   zVal - 353.6 * cos((PI / 180.0) * (45.0 + angle)),
		   xVal + 353.6 * sin((PI / 180.0) * (45.0 - angle)),
		   zVal - 353.6 * cos((PI / 180.0) * (45.0 - angle)),
		   xVal + 7.072 * sin((PI / 180.0) * (45.0 - angle)),
		   zVal - 7.072 * cos((PI / 180.0) * (45.0 - angle))
		   );
   }
   // End right viewport.

}

void keyInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	float tempxVal = xVal, tempzVal = zVal, tempAngle = angle;
	switch (key) {
      case GLFW_KEY_ESCAPE:
		exit(0);
		break;
	  case GLFW_KEY_SPACE:
		// only want this to get called once and so call when key
		// is released
		if (action == GLFW_RELEASE) {
			  isFrustumCulled = 1 - isFrustumCulled;
		}
		break;
	  case GLFW_KEY_LEFT: 
		tempAngle = angle + 5.0;
		break;
	  case GLFW_KEY_RIGHT: 
		tempAngle = angle - 5.0;
		break;
	  case GLFW_KEY_UP:
		tempxVal = xVal - sin(angle * PI / 180.0);
		tempzVal = zVal - cos(angle * PI / 180.0);
		break;
	  case GLFW_KEY_DOWN:
		tempxVal = xVal + sin(angle * PI / 180.0);
		tempzVal = zVal + cos(angle * PI / 180.0);
		break;
	  default:
		break;
   }

  // Angle correction.
  if (tempAngle > 360.0) tempAngle -= 360.0;
  if (tempAngle < 0.0) tempAngle += 360.0;

  // Move spacecraft to next position only if there will not be collision with an asteroid.
  if (!asteroidCraftCollision(tempxVal, tempzVal, tempAngle))
  {
	  isCollision = 0;
	  xVal = tempxVal;
	  zVal = tempzVal;
	  angle = tempAngle;
  }
  else isCollision = 1;

}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
   cout << "ALERT: The OpenGL window may take a while to come up because" << endl
		<< "of the time to build the quadtree!" << endl
		<<  endl;
   cout << "Interaction:" << endl;
   cout << "Press the left/right arrow keys to turn the craft." << endl
        << "Press the up/down arrow keys to move the craft." << endl
		<< "Press space to toggle between frustum culling enabled and disabled." << endl;
}

// Main routine.
int main(int argc, char **argv) 
{

	srand((unsigned)time(0));
	printInteraction();

	// set up the window
	GLFWwindow* window;

	// Initialize the library 
	if (!glfwInit())
		return -1;

	// Create a windowed mode window and its OpenGL context 
	window = glfwCreateWindow(WINDOW_X, WINDOW_Y, "spaceTravelFrustumCulled.cpp", nullptr, nullptr);
	resize(window, WINDOW_X, WINDOW_Y);
	glfwSetWindowSizeCallback(window, resize);
	glfwSetKeyCallback(window, keyInput);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Make the window's context current 
	glfwMakeContextCurrent(window);

	// Init GLEW 
	glewInit();

	// init the graphics and rest of the app
	setup();

	// run!
	while (!glfwWindowShouldClose(window))
	{
		drawScene();
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;

}

