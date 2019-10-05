
#include <cstdlib>
#include <cmath>
#include <list>
#include <iostream>
#include "QuadTree.h"
#include "intersectionDetectionRoutines.h"

using namespace std;

// QuadtreeNode constructor.
QuadtreeNode::QuadtreeNode(const float x, const float z, const float s)
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
		const float& radius = arrayAsteroids[i].getRadius();
		if (radius > 0.f)
   		{
			
			const float& c_x = arrayAsteroids[i].getCenterX();
			const float& c_z = arrayAsteroids[i].getCenterZ();
			
			if (checkDiscRectangleIntersection( SWCornerX, SWCornerZ, SWCornerX + size, SWCornerZ - size,
					c_x, c_z, radius )
				)
   			{
   				asteroidList.emplace_back(arrayAsteroids[i]); 
				++numVal;
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
		
		const float halfSize = size / 2.f;
		const float cornerHalfSize = SWCornerZ - size / 2.f;
		const float otherCornerHalfSize = SWCornerX + size / 2.f;
		
		SWChild = new QuadtreeNode(SWCornerX, SWCornerZ, halfSize);
		SWChild->setLength(length);
		SWChild->setArray(arr);

		NWChild = new QuadtreeNode(SWCornerX, cornerHalfSize, halfSize);
		NWChild->setLength(length);
		NWChild->setArray(arr);

		NEChild = new QuadtreeNode(otherCornerHalfSize, cornerHalfSize, halfSize);
		NEChild->setLength(length);
		NEChild->setArray(arr);

		SEChild = new QuadtreeNode(otherCornerHalfSize, SWCornerZ, halfSize);
		SEChild->setLength(length);
		SEChild->setArray(arr);

		// can be even further optimized by hashing asteroids that ended up at the very end (1) so that no recalc needs to happen
		SWChild->build(); NWChild->build(); NEChild->build(); SEChild->build(); 
	}	
}

// Recursive routine to draw the asteroids in a square's list if the square is a
// leaf and it intersects the frustum (which is specified by the input parameters);
// if the square is not a leaf, the routine recursively calls itself on its children.
void QuadtreeNode::drawAsteroids(const float& x1, const float& z1, const float& x2, const float& z2, 
					             const float& x3, const float& z3, const float& x4, const float& z4)
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
	        ++asteroidListIterator;
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

void QuadtreeNode::gatherAsteroid(const float& x, const float& z, vector<Asteroid>& arr)
{
	//if(arr[3] == 0.f) // found the point recurse up
	//{
		if(checkDiscRectangleIntersection(SWCornerX, SWCornerZ, SWCornerX+size, SWCornerZ-size, x, z, 5.f))
		{
			if (SWChild == NULL) // Square is leaf.
			{
				if(asteroidList.size() > 0)
				{
					arr.push_back(asteroidList[0]);
					/*
					arr[0] = asteroidList[0].getCenterX();
					arr[1] = asteroidList[0].getCenterY();
					arr[2] = asteroidList[0].getCenterZ();
					arr[3] = asteroidList[0].getRadius();
					*/
				}
			}
			else
			{
				SWChild->gatherAsteroid(x, z, arr);
				NWChild->gatherAsteroid(x, z, arr);
				NEChild->gatherAsteroid(x, z, arr); 
				SEChild->gatherAsteroid(x, z, arr);
			}
		}
	//}
}

// Initialize quadtree by splitting nodes till each leaf node intersects at most one asteroid.
void Quadtree::initialize(const float x, const float z, const float s)
{
   header = new QuadtreeNode(x, z, s);
   header->setLength(length);
   header->setArray(arrayAsteroids);
   header->build();
}

// Routine to draw all the asteroids in the asteroid list of each leaf square that intersects the frustum.
void Quadtree::drawAsteroids(const float& x1, const float& z1, const float& x2, const float& z2, 
					         const float& x3, const float& z3, const float& x4, const float& z4)
{
   header->drawAsteroids(x1, z1, x2, z2, x3, z3, x4, z4); 
}

void Quadtree::gatherAsteroid(const float& x, const float& z, vector<Asteroid>& arr) const
{
	header->gatherAsteroid(x, z, arr);
}
