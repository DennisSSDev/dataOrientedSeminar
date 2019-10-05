#ifndef QuadTree_239847
#define QuadTree_239847

#include <vector>
#include "Asteroid.h"

using namespace std;

// Quadtree node class.
extern void drawAsteroid(const unsigned int at);

struct Location
{
	float x;
	float y;
	float z;
	float rds; // radius
	unsigned int index;
};

struct AsteroidLocations
{
	vector<Location> l;
};

class QuadtreeNode
{
public:
	QuadtreeNode(float x, float z, float s);
	int numberAsteroidsIntersected(); // Return the number of asteroids intersecting the square.

	void build(); // Recursive routine to split a square that intersects more than one asteroid; 
				  // if it intersects at most one asteroid leave it as a leaf and add the intersecting 
                  // asteroid, if any, to a local list of asteroids.

	void drawAsteroids(const float& x1, const float& z1, const float& x2, const float& z2,   // Recursive routine to draw the asteroids
					  const float& x3, const float& z3, const float& x4, const float& z4);  // in a square's list if the square is a
																							// leaf and it intersects the frustum (which
																							// is specified by the input parameters); 
																							// if the square is not a leaf, the routine
																							// recursively calls itself on its children.
	void gatherAsteroid(const float& x, const float& z, AsteroidLocations& al /*OUT*/);
	void setArray(AsteroidLocations al) { this->asteroidLocations = al; }

private: 
	float SWCornerX, SWCornerZ; // x and z co-ordinates of the SW corner of the square.
	float size; // Side length of square.
	QuadtreeNode *SWChild, *NWChild, *NEChild, *SEChild; // Children nodes.
	AsteroidLocations asteroidLocations; // global list of asteroid locations
	AsteroidLocations nodeAsteroids; // local list of asteroid locations
	friend class Quadtree;
};

// Quadtree class.
class Quadtree
{
public:
   Quadtree() { header = NULL; } // Constructor.
   void initialize(float x, float z, float s); // Initialize quadtree by splitting nodes
                                               // till each leaf node intersects at
                                               // most one asteroid.

   void drawAsteroids(const float& x1, const float& z1, const float& x2, const float& z2,  // Routine to draw all the asteroids in the  
					  const float& x3, const float& z3, const float& x4, const float& z4); // asteroid list of each leaf square that
																						   // intersects the frustum.
   void gatherAsteroid(const float& x, const float& z, AsteroidLocations& al) const;
   void setLength(int length) { this->length = length; }
   void setArray(const Asteroids& arrayAsteroids)
   {
	   this->arrayAsteroids = arrayAsteroids;
   }

private:
	QuadtreeNode *header;
	int length;
	Asteroids arrayAsteroids; // Global array of asteroids.
};


#endif