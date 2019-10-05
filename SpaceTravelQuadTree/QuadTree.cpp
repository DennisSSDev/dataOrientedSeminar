
#include <cstdlib>
#include "QuadTree.h"
#include "intersectionDetectionRoutines.h"

using namespace std;

// QuadtreeNode constructor.
QuadtreeNode::QuadtreeNode(const float x, const float z, const float s)
{
   SWCornerX = x; SWCornerZ = z; size = s;
   SWChild = NWChild = NEChild = SEChild = NULL;
}

// Return the number of asteroids intersecting the square.

// memoize the values when calculating the distances in order to not recalculate them for other recursive iterations
int QuadtreeNode::numberAsteroidsIntersected()
{
	int numVal = 0;
	// changed the iteration to be column row major
	const unsigned int lSize = asteroidLocations.l.size();
	for (int i = 0; i < lSize; i++)
	{
		const Location& loc = asteroidLocations.l[i];
		const float& radius = loc.rds;
		if (radius > 0.f)
   		{
			
			const float& c_x = loc.x;
			const float& c_z = loc.z;
			
			if (checkDiscRectangleIntersection( SWCornerX, SWCornerZ, SWCornerX + size, SWCornerZ - size,
					c_x, c_z, radius )
				)
   			{
   				nodeAsteroids.l.emplace_back(loc); 
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
		
		const float halfSize = size / 2.f;
		const float cornerHalfSize = SWCornerZ - size / 2.f;
		const float otherCornerHalfSize = SWCornerX + size / 2.f;
		
		SWChild = new QuadtreeNode(SWCornerX, SWCornerZ, halfSize);
		SWChild->setArray(nodeAsteroids);

		NWChild = new QuadtreeNode(SWCornerX, cornerHalfSize, halfSize);
		NWChild->setArray(nodeAsteroids);

		NEChild = new QuadtreeNode(otherCornerHalfSize, cornerHalfSize, halfSize);
		NEChild->setArray(nodeAsteroids);

		SEChild = new QuadtreeNode(otherCornerHalfSize, SWCornerZ, halfSize);
		SEChild->setArray(nodeAsteroids);

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
		 if(nodeAsteroids.l.size() > 0)
		 {
			drawAsteroid(nodeAsteroids.l[0].index);
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

void QuadtreeNode::gatherAsteroid(const float& x, const float& z, AsteroidLocations& al)
{
	if(checkDiscRectangleIntersection(SWCornerX, SWCornerZ, SWCornerX+size, SWCornerZ-size, x, z, 5.f))
	{
		if (SWChild == NULL) // Square is leaf.
		{
			if(nodeAsteroids.l.size() > 0)
			{
				al.l.push_back(nodeAsteroids.l[0]);
			}
		}
		else
		{
			SWChild->gatherAsteroid(x, z, al);
			NWChild->gatherAsteroid(x, z, al);
			NEChild->gatherAsteroid(x, z, al); 
			SEChild->gatherAsteroid(x, z, al);
		}
	}
}

// Initialize quadtree by splitting nodes till each leaf node intersects at most one asteroid.
void Quadtree::initialize(const float x, const float z, const float s)
{
	header = new QuadtreeNode(x, z, s);
	AsteroidLocations al;
	al.l.resize(length);
	for(unsigned int i = 0; i < length; ++i)
	{
		al.l[i] = { arrayAsteroids.x[i], arrayAsteroids.y[i], arrayAsteroids.z[i], arrayAsteroids.rds[i], i };
	}
	header->setArray(al);
	header->build();
}

// Routine to draw all the asteroids in the asteroid list of each leaf square that intersects the frustum.
void Quadtree::drawAsteroids(const float& x1, const float& z1, const float& x2, const float& z2, 
					         const float& x3, const float& z3, const float& x4, const float& z4)
{
   header->drawAsteroids(x1, z1, x2, z2, x3, z3, x4, z4); 
}

void Quadtree::gatherAsteroid(const float& x, const float& z, AsteroidLocations& al) const
{
	header->gatherAsteroid(x, z, al);
}
