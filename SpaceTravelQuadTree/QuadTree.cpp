
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
int QuadtreeNode::numberAsteroidsIntersected()
{
   int numVal = 0;
   int i, j;
   for (j=0; j<cols; j++)
      for (i=0; i<rows; i++)
	     if (arrayAsteroids[i][j].getRadius() > 0.0) 
	        if ( checkDiscRectangleIntersection( SWCornerX, SWCornerZ, SWCornerX+size, SWCornerZ-size,
                 arrayAsteroids[i][j].getCenterX(), arrayAsteroids[i][j].getCenterZ(), 
				 arrayAsteroids[i][j].getRadius() )
			   )
		       numVal++;
   return numVal;
}

// Add the asteoroids intersecting the square to a local list of asteroids.
void QuadtreeNode::addIntersectingAsteroidsToList()
{
   int i, j;
   for (j=0; j<cols; j++)
      for (i=0; i<rows; i++)
	     if (arrayAsteroids[i][j].getRadius() > 0.0) 
	        if ( checkDiscRectangleIntersection( SWCornerX, SWCornerZ, SWCornerX+size, SWCornerZ-size, 
				 arrayAsteroids[i][j].getCenterX(), arrayAsteroids[i][j].getCenterZ(), 
				 arrayAsteroids[i][j].getRadius() )
			   )
		       asteroidList.push_back( Asteroid(arrayAsteroids[i][j]) );
}

// Recursive routine to split a square that intersects more than one asteroid; if it intersects
// at most one asteroid leave it as a leaf and add the intersecting asteroid, if any, to a local 
// list of asteroids.
void QuadtreeNode::build()
{
   if ( this->numberAsteroidsIntersected() <= 1 ) this->addIntersectingAsteroidsToList();
   else
   {
      SWChild = new QuadtreeNode(SWCornerX, SWCornerZ, size/2.0);
	  SWChild->setRowsCols(rows, cols);
	  SWChild->setArray(arrayAsteroids);

      NWChild = new QuadtreeNode(SWCornerX, SWCornerZ - size/2.0, size/2.0);
	  NWChild->setRowsCols(rows, cols);
	  NWChild->setArray(arrayAsteroids);

      NEChild = new QuadtreeNode(SWCornerX + size/2.0, SWCornerZ - size/2.0, size/2.0);
	  NEChild->setRowsCols(rows, cols);
	  NEChild->setArray(arrayAsteroids);

      SEChild = new QuadtreeNode(SWCornerX + size/2.0, SWCornerZ, size/2.0);
	  SEChild->setRowsCols(rows, cols);
	  SEChild->setArray(arrayAsteroids);

	  
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
         list<Asteroid>::iterator asteroidListIterator;
         asteroidListIterator = asteroidList.begin();

         // Draw all the asteroids in asteroidList.
         while(asteroidListIterator != asteroidList.end() )
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
   header->setRowsCols(rows, cols);
   header->setArray(arrayAsteroids);
   header->build();
}

// Routine to draw all the asteroids in the asteroid list of each leaf square that intersects the frustum.
void Quadtree::drawAsteroids(float x1, float z1, float x2, float z2, 
					         float x3, float z3, float x4, float z4)
{
   header->drawAsteroids(x1, z1, x2, z2, x3, z3, x4, z4); 
}
