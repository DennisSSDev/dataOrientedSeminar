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
#include <vector>
#include "Asteroid.h"
#include "QuadTree.h"

using namespace std;

#pragma comment ( lib, "opengl32.lib" )
#pragma comment ( lib, "glew32.lib" )
#pragma comment ( lib, "glfw3.lib" )


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
static int cone_index = 0;
static int line_index = cone_index + CONE_VERTEX_COUNT;
static int sphere_index = line_index + LINE_VERTEX_COUNT;

// shader stuff
// addition of all rows/cols for asteroid vertices + spaceship vertices + line vertices  
static glm::vec3 points[CONE_VERTEX_COUNT+LINE_VERTEX_COUNT+SPHERE_VERTEX_COUNT*ROWS*COLUMNS];
static GLuint myShaderProgram;
GLuint InitShader(const char* vShaderFile, const char* fShaderFile);
static GLuint myBuffer;

// the asteroids and quad tree from the initial program
static Asteroids asteroids = Asteroids(); // Global array of asteroids.
static QuadTree asteroidsQuadTree = QuadTree(); // Global quadtree.

// function obtained from tutorial at:
// http://www.freemancw.com/2012/06/opengl-cone-function/
// used in drawing a cone
static glm::vec3  perp(const glm::vec3 &v) {
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
static void CreateCone(const glm::vec3 &d, const glm::vec3 &a,
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
}

// function derived from tutorial at:
// http://www.swiftless.com/tutorials/opengl/sphere.html
// change -- make this function be called once and just copy these values over
// to other spheres so that you don't recalculate this every time
static glm::uint CreateSphere(const float& R, const float& H, const float& K, const float& Z, const int& offset) {
	int n = 0;
	double a;
	double b;
	const int space = 30;
	int inc = 0;
	// draw the front half
	for (b = 0; b <= 90 - space; b += space) {
		
		// memo
		const float p4 =  cos((b) / 180 * PI);
		const float p2 = sin((b) / 180 * PI);

		const float p5 = sin((b + space) / 180 * PI);
		const float p6 = cos((b + space) / 180 * PI);

		const float z_Calc1 = R * p4 - Z;
		const float z_Calc2 = R * p6 - Z;
		
		for (a = 0; a <= 360 - space; a += space){
			
			const float p1 = sin((a) / 180 * PI);
			const float p3 = cos((a) / 180 * PI);

			const float p7 = sin((a + space) / 180 * PI);
			const float p8 = cos((a + space) / 180 * PI);

			glm::uint nOffset = n + offset;
		
			
			points[nOffset].x = R * p1 * p2 - H;
			points[nOffset].y = R * p3 * p2 + K;
			points[nOffset++].z = z_Calc1;
			++n;
			++inc;
			
			points[nOffset].x = R * p1 * p5 - H;
			points[nOffset].y = R * p3 * p5 + K;
			points[nOffset++].z = z_Calc2;
			++n;
			++inc;

			points[nOffset].x = R * p7 * p2 - H;
			points[nOffset].y = R * p8 * p2 + K;
			points[nOffset++].z = z_Calc1;
			++n;
			++inc;
			
			points[nOffset].x = R * p7 * p5 - H;
			points[nOffset].y = R * p8 * p5 + K;
			points[nOffset++].z = z_Calc2;
			++n;
			++inc;
		}
	}

	// draw the back half
	// why recalculate? Access the previous vertices
	for (b = 0; b <= 90 - space; b += space)
	{
		for (a = 0; a <= 360 - space; a += space)
		{
			glm::uint nOffset = n + offset;
			glm::uint indexOffset = nOffset - inc;
			glm::uint iterator = 0;

			while(iterator++ < 4)
			{
				points[nOffset].x = points[indexOffset].x;
				points[nOffset].y = points[indexOffset].y;
				points[nOffset++].z = -1 * points[indexOffset].z; // negative but same
				indexOffset++;
				n++;
			}
		}
	}
	return n; // determine how much is the offset so that you can easily copy already calculated values for the sphere
}


// OpenGL window reshape routine.
inline void resize(GLFWwindow* window, int w, int h)
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
    // create memory for each potential asteroid
	
    // create the quad tree for the asteroids
	asteroidsQuadTree.length = COLUMNS*ROWS;

    // create the line for the middle of the screen
    points[line_index].x = 0;
    points[line_index].y = -5;
    points[line_index].z = -6;
    points[line_index + 1].x = 0;
    points[line_index + 1].y = 5;
    points[line_index + 1].z = -6;

    // create the cone for a spaceship
    const glm::vec3 direction( 0, 1, 0 );
    const glm::vec3 apex(0, 10, 0);

	// no need to optimize, has low impact as only 1 cone is ever created
    CreateCone(direction, apex, 10, 5, 10, cone_index);

    // create where the spheres are going in the field   
    int index = sphere_index;
	// calculate the sphere once - reuse the data after
	const glm::uint validSpaces = CreateSphere(SPHERE_SIZE, 0, 0, 0, index);

    // Initialize global arrayAsteroids.
    // 
	// change -- move the if else outside of the loop
	// change -- no need to check for COLUMN evenness every time just calculate it once and add the number if necessary, a lot less code
	// Position the asteroids depending on if there is an even or odd number of columns
    // so that the spacecraft faces the middle of the asteroid field.
	const float odd = (COLUMNS % 2) ? 0 : 15.f; // changed
	for (i = 0; i < COLUMNS; i++)
	{
		for (j = 0; j < ROWS; j++)
		{
			if (rand() % 100 < FILL_PROBABILITY)
			{
				const glm::uint inn = COLUMNS * j + i;
				asteroids.x[inn] = odd + 30.0f * (-COLUMNS / 2.f + j);
				asteroids.y[inn] = 0.f;
				asteroids.z[inn] = -40.0f - 30.0f * i;
				asteroids.rds[inn] = 3.f;
				
				asteroids.r[inn] = rand() % 256;
				asteroids.g[inn] = rand() % 256;
				asteroids.b[inn] = rand() % 256;
				
				asteroids.i[inn] = index;
				
				glm::uint count = 0;
				
				while(count < validSpaces)
				{
					points[count + index] = points[count + sphere_index];
					count++;
				}
	   			index += SPHERE_VERTEX_COUNT;
			}
		}
	}

	// Initialize global asteroidsQuadtree - the root square bounds the entire asteroid field.
	if (ROWS <= COLUMNS) initialSize = (COLUMNS - 1) * 30.0f + 6.0f;
	else initialSize = (ROWS - 1) * 30.0f + 6.0f;

	asteroidsQuadTree.arrayAsteroids = asteroids;
	QuadTreeInitializeSystem(-initialSize / 2.0f, -37.f, initialSize, asteroidsQuadTree);
	
	// initialize the graphics
	glEnable(GL_DEPTH_TEST);
	
	glViewport(0, 0, (GLsizei)WINDOW_X, (GLsizei)WINDOW_Y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 250.0);
	glMatrixMode(GL_MODELVIEW);

	
	GLuint vbo;
	glGenBuffers(1, &vbo);
	myBuffer = vbo;
	glBindBuffer(GL_ARRAY_BUFFER, myBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	myShaderProgram = program;
	glUseProgram(myShaderProgram);

	// Initialize the vertex position attribute from the vertex shader
	GLuint loc = glGetAttribLocation(myShaderProgram, "vPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// enter polygon mode ONCE!
	glPolygonMode(GL_FRONT, GL_LINE);
	glPolygonMode(GL_BACK, GL_LINE);
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
// Utilized QuadTree to reduce the amount of collision checks
// Collision detection is approximate as instead of the spacecraft we use a bounding sphere.
int asteroidCraftCollision(const float& x, const float& z, const float& a)
{
	// Check for collision with up to 4 asteroids by using the quad tree
	vector<Location> astl;
	
	const float x_calc = x - 5.f * sin((PI/180.f) * a); 
	const float z_calc = z - 5 * cos((PI/180.f) * a);
	GatherAsteroidSystem(x_calc, z_calc, asteroidsQuadTree.header, astl /*OUT*/);
	if (!astl.empty())
	{
		for(auto &it : astl)
		{
			if (checkSpheresIntersection(x_calc, 0.f, z_calc, 7.072f, 
		    it.x, it.y, it.z, it.rds))
			{
				return 1;
			}
		}
	}
	return 0;
}

// function taken from glu
void lookAt(
	const float& eyex, 
	const float& eyey, 
	const float& eyez, 
	const float& centerx,
	const float& centery, 
	const float& centerz, 
	const float& upx, 
	const float& upy, 
	const float& upz)
{
	int i;
	glm::vec3 forward, side, up;
	float m[4][4];

	// create identity matrix
	for (i = 0; i < 4; i++) {
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

// Function that loops through every asteroid and draws it
// The Draw calls are the bottleneck of the application
// There is a branch on github that solved the rendering issue:
// https://github.com/DennisSSDev/dataOrientedSeminar/tree/OptimizedRender
static void DrawAllAsteroidsSystem()
{
	for(int i = 0; i < ROWS*COLUMNS; i++)
	{
		if(asteroids.rds[i] > 0.f)
		{
			glPushMatrix();
			
			glTranslatef(asteroids.x[i], asteroids.y[i], asteroids.z[i]);
			glColor3ub(asteroids.r[i], asteroids.g[i], asteroids.b[i]);
			
			glDrawArrays(GL_TRIANGLE_FAN, asteroids.i[i], SPHERE_VERTEX_COUNT);
		

			glPopMatrix();
		}
	}
}

// Draws only 1 asteroid.
// This is used mainly by the QuadTree to reduce the draw calls.
void drawAsteroid(const unsigned int at)
{
	if(asteroids.rds[at] > 0.f)
	{
		glPushMatrix();
			
		glTranslatef(asteroids.x[at], asteroids.y[at], asteroids.z[at]);
		glColor3ub(asteroids.r[at], asteroids.g[at], asteroids.b[at]);
			
		glDrawArrays(GL_TRIANGLE_FAN, asteroids.i[at], SPHERE_VERTEX_COUNT);

		glPopMatrix();
	}
}


// Drawing routine.
void drawScene(void)
{ 
   int i;
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Use the buffer and shader for each circle.
   glUseProgram(myShaderProgram);
   glBindBuffer(GL_ARRAY_BUFFER, myBuffer);

   // Initialize the vertex position attribute from the vertex shader.
    GLuint loc = glGetAttribLocation(myShaderProgram, "vPosition");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Begin left viewport.
   glViewport (0, 0, width / 2.0,  height); 
   glLoadIdentity();

   
   // Fixed camera 
   lookAt(0.f, 10.f, 20.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	if (!isFrustumCulled)
	{
   		DrawAllAsteroidsSystem();
	}
	else
	{
		// Draw only asteroids in leaf squares of the quadtree that intersect the fixed frustum
		// with apex at the origin.
		DrawAsteroidsSystem(-5.f, -5.f, -250.f, -250.f, 250.f, -250.f, 5.f, -5.f, asteroidsQuadTree.header);
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
   glRotatef(-90.f, 1.f, 0.f, 0.f); // To make the spacecraft point down the $z$-axis initially.

   glDrawArrays(GL_TRIANGLE_FAN, cone_index, CONE_VERTEX_COUNT);

   glPopMatrix();
   // End left viewport.
   
	// Begin right viewport.
	//=====================================
	//
	//
   glViewport(width / 2.0, 0, width / 2.0, height);
   glLoadIdentity();

   // draw the line in the middle to separate the two viewports
   glPushMatrix();
   glTranslatef(-6.f, 0.f, 0.f);
   glColor3f(1.f, 1.f, 1.f);
   glLineWidth(2.0);
   glDrawArrays(GL_LINE_STRIP, line_index, LINE_VERTEX_COUNT);
   glLineWidth(1.0);
   glPopMatrix();

   // Locate the camera at the tip of the cone and pointing in the direction of the cone.
	const float sinDegree = sin( (PI/180.0) * angle);
	const float cosDegree = cos( (PI/180.0) * angle);
	lookAt(xVal - 10 * sinDegree, 
	         0.0, 
			 zVal - 10 * cosDegree, 
	         xVal - 11 * sinDegree,
			 0.0,
             zVal - 11 * cosDegree, 
             0.0, 
			 1.0, 
			 0.0);

   if (!isFrustumCulled)
   {
	   DrawAllAsteroidsSystem();
   }
   else
   {
	   // Draw only asteroids in leaf squares of the quadtree that intersect the frustum
	   // "carried" by the spacecraft with apex at its tip and oriented with its axis
	   // along the spacecraft's axis.
	   
		const float sinAngleDeg = sin((PI / 180.0) * (45.0 + angle));
   		const float zCosAngleDeg = cos((PI / 180.0) * (45.0 + angle));
   		const float xSinAngleDeg = sin((PI / 180.0) * (45.0 - angle));
   		const float cosAngleDeg = cos((PI / 180.0) * (45.0 - angle));

   		DrawAsteroidsSystem(xVal - 7.072 * sinAngleDeg,
		   zVal - 7.072 * zCosAngleDeg,
		   xVal - 353.6 * sinAngleDeg,
		   zVal - 353.6 * zCosAngleDeg,
		   xVal + 353.6 * xSinAngleDeg,
		   zVal - 353.6 * cosAngleDeg,
		   xVal + 7.072 * xSinAngleDeg,
		   zVal - 7.072 * cosAngleDeg, asteroidsQuadTree.header);
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
inline void printInteraction(void)
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

