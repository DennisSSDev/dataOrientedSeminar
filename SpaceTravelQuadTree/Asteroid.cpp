
#include <cstdlib>
#include <cmath>
#include <list>
#include <iostream>
#include <GL/glew.h>
#include <GL/glfw3.h>
#include "Asteroid.h"

using namespace std;

// Asteroid default constructor.
Asteroid::Asteroid()
{
   centerX = 0.0;
   centerY = 0.0;
   centerZ = 0.0; 
   radius = 0.0; // Indicates no asteroid exists in the position.
   color[0] = 0;
   color[1] = 0;
   color[2] = 0;
}

// Asteroid constructor.
Asteroid::Asteroid(float x, float y, float z, float r, unsigned char valueR, 
		unsigned char valueG, unsigned char valueB)
{
   centerX = x;
   centerY = y;
   centerZ = z; 
   radius = r;
   color[0] = valueR;
   color[1] = valueG;
   color[2] = valueB;
}

extern void CreateSphere(double R, double H, double K, double Z, int offset);

// Function to draw asteroid.
void Asteroid::draw()
{
   if (radius > 0.0) // If asteroid exists.
   {
      glPushMatrix();
      glTranslatef(centerX, centerY, centerZ);
	  //glScalef(0.0125 * radius, 0.0125 * radius, 0.0125 * radius);
	  glColor3ubv(color);

	  // Turn on wireframe mode
	  glPolygonMode(GL_FRONT, GL_LINE);
	  glPolygonMode(GL_BACK, GL_LINE);

	  // create the sphere and place it in a global array
	  CreateSphere(SPHERE_SIZE, 0, 0, 0, index);

	  // draw sphere
	  glDrawArrays(GL_TRIANGLE_FAN, index, SPHERE_VERTEX_COUNT);

	  // Turn off wireframe mode
	  glPolygonMode(GL_FRONT, GL_FILL);
	  glPolygonMode(GL_BACK, GL_FILL);

      glPopMatrix();
   }
}
