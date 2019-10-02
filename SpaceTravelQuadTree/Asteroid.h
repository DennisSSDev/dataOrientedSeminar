
#ifndef Asteroid_13389437
#define Asteroid_13389437
#define PI 3.14159265
#include <glm/glm.hpp>

#define SPHERE_VERTEX_COUNT 288
#define SPHERE_SIZE 5.0f

// Asteroid class.
class Asteroid
{
public:
   Asteroid();
   Asteroid(float x, float y, float z, float r, unsigned char valueR, 
	    	unsigned char valueG, unsigned char valueB);
   float getCenterX() { return centerX; }
   float getCenterY() { return centerY; }
   float getCenterZ() { return centerZ; }
   float getRadius()  { return radius; }
   void draw();
   void setIndex(int i) { index = i; }
private:
   float centerX, centerY, centerZ, radius;
   unsigned char color[3];
   int index; // index into the single array holding all vertices
};

#endif