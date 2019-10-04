
#include <cstdlib>
#include <cmath>
#include <list>
#include <iostream>
#include "QuadTree.h"
#include "intersectionDetectionRoutines.h"

using namespace std;

// QuadtreeNode constructor.
QuadtreeNode::QuadtreeNode(float x, float z, float s)
{
   SWCornerX = x; SWCornerZ = z; size = s;
   SWChild = NWChild = NEChild = SEChild = NULL;
   asteroidList.clear();
}

// Return the number of asteroids intersecting the square.

// memoize the values when calculating the distances in order to not recalculate them for other recursive iterations
int QuadtreeNode::numberAsteroidsIntersected()
{
	int numVal = 0;
	// changed the iteration to be column row major
	for (int i = 0; i < length; i++)
	{
		if (arrayAsteroids[i].getRadius() > 0.f)
   		{
   			if ( checkDiscRectangleIntersection( SWCornerX, SWCornerZ, SWCornerX+size, SWCornerZ-size,
					arrayAsteroids[i].getCenterX(), arrayAsteroids[i].getCenterZ(), 
					arrayAsteroids[i].getRadius() )
				)
   			{
   				asteroidList.emplace_back(Asteroid(arrayAsteroids[i])); 
				numVal++;
   			}
   		}
	}
	return numVal;
}

// Recursive routine to split a square that intersects more than one asteroid; if it intersects
// at most one asteroid leave it as a leaf and add the intersecting asteroid, if any, to a local 
// list of asteroids.
void QuadtreeNode::build()
{
	const glm::uint length = this->numberAsteroidsIntersected();
	// clear recalculation happening in the recursion. Memo this
	if ( length > 1 ) 
	{
		Asteroid *arr = asteroidList.data(); // why give the whole thing down to the other branches? only bother with the ones that intersected
		
		SWChild = new QuadtreeNode(SWCornerX, SWCornerZ, size / 2.f);
		SWChild->setLength(length);
		SWChild->setArray(arr);

		NWChild = new QuadtreeNode(SWCornerX, SWCornerZ - size / 2.f, size / 2.f);
		NWChild->setLength(length);
		NWChild->setArray(arr);

		NEChild = new QuadtreeNode(SWCornerX + size / 2.f, SWCornerZ - size / 2.f, size / 2.f);
		NEChild->setLength(length);
		NEChild->setArray(arr);

		SEChild = new QuadtreeNode(SWCornerX + size / 2.f, SWCornerZ, size / 2.f);
		SEChild->setLength(length);
		SEChild->setArray(arr);

		// can be even further optimized by hashing asteroids that ended up at the very end (1) so that no recalc needs to happen
		SWChild->build(); NWChild->build(); NEChild->build(); SEChild->build(); 
	}
}

// Recursive routine to draw the asteroids in a square's list if the square is a
// leaf and it intersects the frustum (which is specified by the input parameters);
// if the square is not a leaf, the routine recursively calls itself on its children.
void QuadtreeNode::drawAsteroids(float x1, float z1, float x2, float z2, 
					             float x3, float z3, float x4, float z4)
{
   // If the square does not intersect the frustum do nothing.
   if ( checkQuadrilateralsIntersection(x1, z1, x2, z2, x3, z3, x4, z4,
								        SWCornerX, SWCornerZ, SWCornerX, SWCornerZ-size,
								        SWCornerX+size, SWCornerZ-size, SWCornerX+size, SWCornerZ) )
   {
      if (SWChild == NULL) // Square is leaf.
	  {
         // Define local iterator to traverse asteroidList and initialize.
         auto asteroidListIterator = asteroidList.begin();

         // Draw all the asteroids in asteroidList.
         while(asteroidListIterator != asteroidList.end())
		 {
            asteroidListIterator->draw();
	        asteroidListIterator++;
		 }	  
	  }
	  else 
	  {
	     SWChild->drawAsteroids(x1, z1, x2, z2, x3, z3, x4, z4); 
		 NWChild->drawAsteroids(x1, z1, x2, z2, x3, z3, x4, z4); 
		 NEChild->drawAsteroids(x1, z1, x2, z2, x3, z3, x4, z4); 
		 SEChild->drawAsteroids(x1, z1, x2, z2, x3, z3, x4, z4);
	  }
   }
}

// Initialize quadtree by splitting nodes till each leaf node intersects at most one asteroid.
void Quadtree::initialize(float x, float z, float s)
{
   header = new QuadtreeNode(x, z, s);
   header->setLength(length);
   header->setArray(arrayAsteroids);
   header->build();
}

// Routine to draw all the asteroids in the asteroid list of each leaf square that intersects the frustum.
void Quadtree::drawAsteroids(float x1, float z1, float x2, float z2, 
					         float x3, float z3, float x4, float z4)
{
   header->drawAsteroids(x1, z1, x2, z2, x3, z3, x4, z4); 
}
