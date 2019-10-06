#ifndef QuadTree_239847
#define QuadTree_239847

#include <vector>
#include "Asteroid.h"
#include "intersectionDetectionRoutines.h"

// func to draw the asteroids (BottleNeck since every sphere is a separate draw call)
extern void drawAsteroid(const unsigned int at);

struct Location
{
	float x;
	float y;
	float z;
	float rds; // radius
	unsigned int index;
};

struct QuadTreeNode
{
	QuadTreeNode(){size = 0;}
	QuadTreeNode(const float x, const float z, const float s)
	{
		SWCornerX = x; SWCornerZ = z; size = s;
		SWChild = NWChild = NEChild = SEChild = nullptr;
	}
	
	QuadTreeNode *SWChild, *NWChild, *NEChild, *SEChild; // Children nodes.
	
	std::vector<Location> asteroidLocations; // global list of asteroid locations to process (reduced every time the tree is subdivided)
	std::vector<Location> nodeAsteroids; // local list of asteroid locations, leaf nodes store 1 item
	
	float SWCornerX, SWCornerZ; // x and z co-ordinates of the SW corner of the square.
	float size; // Side length of square.
};

struct QuadTree
{
	QuadTree(){length = 0;}

	QuadTreeNode header; // starting node of the quad tree
	Asteroids arrayAsteroids; // Global array of asteroids.
	int length;
};


static int NumberAsteroidsIntersectedSystem(QuadTreeNode& node)
{
	int numVal = 0;
	
	const auto& asteroidLocations = node.asteroidLocations;
	auto& nodeAsteroids = node.nodeAsteroids;
	
	const unsigned int& lSize = asteroidLocations.size();
	const float& SWCornerX = node.SWCornerX;
	const float& SWCornerZ = node.SWCornerZ;
	const float& NECornerX = SWCornerX + node.size;
	const float& NECornerZ = SWCornerZ - node.size;
	
	for (unsigned int i = 0; i < lSize; i++)
	{
		const Location& loc = asteroidLocations[i];
		const float& radius = loc.rds;
		if (radius > 0.f)
   		{
			const float& c_x = loc.x;
			const float& c_z = loc.z;
			
			if (checkDiscRectangleIntersection( SWCornerX, SWCornerZ, NECornerX, NECornerZ,
					c_x, c_z, radius )
				)
   			{
   				nodeAsteroids.emplace_back(loc); 
				++numVal;
   			}
   		}
	}
	return numVal;
}

static void BuildSystem(QuadTreeNode& node)
{
	const glm::uint length = NumberAsteroidsIntersectedSystem(node);
	if(length > 1)
	{
		const float& size = node.size; 
		const float& SWCornerZ = node.SWCornerZ;
		const float& SWCornerX = node.SWCornerX;
		
		const float& halfSize = size / 2.f;
		const float& cornerHalfSize = SWCornerZ - halfSize;
		const float& otherCornerHalfSize = SWCornerX + halfSize;
		
		node.SWChild = new QuadTreeNode(SWCornerX, SWCornerZ, halfSize);
		node.SWChild->asteroidLocations = node.nodeAsteroids;
		
		node.NWChild = new QuadTreeNode(SWCornerX, cornerHalfSize, halfSize);
		node.NWChild->asteroidLocations = node.nodeAsteroids;

		node.NEChild = new QuadTreeNode(otherCornerHalfSize, cornerHalfSize, halfSize);
		node.NEChild->asteroidLocations = node.nodeAsteroids;

		node.SEChild = new QuadTreeNode(otherCornerHalfSize, SWCornerZ, halfSize);
		node.SEChild->asteroidLocations = node.nodeAsteroids;

		// no longer need that data, passed down to child
		node.nodeAsteroids.clear();
		node.asteroidLocations.clear();
		
		BuildSystem(*node.SWChild); BuildSystem(*node.NWChild); BuildSystem(*node.NEChild); BuildSystem(*node.SEChild); 
	}
}

static void GatherAsteroidSystem(const float& x, const float& z, const QuadTreeNode& node, vector<Location>& al /*OUT*/)
{
	const float& size = node.size; 
	const float& SWCornerZ = node.SWCornerZ;
	const float& SWCornerX = node.SWCornerX;
		
	const float& corner = SWCornerZ - size;
	const float& otherCorner = SWCornerX + size;
	
	if(checkDiscRectangleIntersection(SWCornerX, SWCornerZ, otherCorner, corner, x, z, 5.f))
	{
		const auto& SWChild = node.SWChild;
		if (SWChild == NULL) // Square is leaf.
		{
			const auto& asteroids = node.nodeAsteroids;
			if(!asteroids.empty())
			{
				al.push_back(asteroids[0]);
			}
		}
		else
		{
			GatherAsteroidSystem(x, z, *SWChild, al);
			GatherAsteroidSystem(x, z, *node.NWChild, al);
			GatherAsteroidSystem(x, z, *node.NEChild, al);
			GatherAsteroidSystem(x, z, *node.SEChild, al);
		}
	}
};

static void DrawAsteroidsSystem(const float& x1, const float& z1, const float& x2, const float& z2,  // Routine to draw all the asteroids in the  
					  const float& x3, const float& z3, const float& x4, const float& z4, const QuadTreeNode& node)
{
	const float& size = node.size; 
	const float& SWCornerZ = node.SWCornerZ;
	const float& SWCornerX = node.SWCornerX;
		
	const float& corner = SWCornerZ - size;
	const float& otherCorner = SWCornerX + size;
	
	const auto& nodeAsteroids = node.nodeAsteroids;
	const auto& SWChild = node.SWChild;
	 // If the square does not intersect the frustum do nothing.
   if ( checkQuadrilateralsIntersection(x1, z1, x2, z2, x3, z3, x4, z4,
								        SWCornerX, SWCornerZ, SWCornerX, corner,
								        otherCorner, corner, otherCorner, SWCornerZ) )
   {
      if (SWChild == NULL) // Square is leaf.
	  {
		 if(!nodeAsteroids.empty())
		 {
			drawAsteroid(nodeAsteroids[0].index);
		 }
         return;
	  }
	  DrawAsteroidsSystem(x1, z1, x2, z2, x3, z3, x4, z4, *SWChild);
	  DrawAsteroidsSystem(x1, z1, x2, z2, x3, z3, x4, z4, *node.NWChild);
	  DrawAsteroidsSystem(x1, z1, x2, z2, x3, z3, x4, z4, *node.NEChild);
	  DrawAsteroidsSystem(x1, z1, x2, z2, x3, z3, x4, z4, *node.SEChild); 
   }
};																						

static void QuadTreeInitializeSystem(const float x, const float z, const float s, QuadTree& quadTree)
{
	quadTree.header = QuadTreeNode(x, z, s);
	vector<Location> asteroidData;
	const unsigned int& length = quadTree.length;
	const auto& globalAsteroids = quadTree.arrayAsteroids;
	asteroidData.resize(length); // preallocate to not waste time resizing
	// grab the neccessary data instead of copying over everything
	for(unsigned int i = 0; i < length; ++i)
	{
		asteroidData[i] = { globalAsteroids.x[i], globalAsteroids.y[i], globalAsteroids.z[i], globalAsteroids.rds[i], i };
	}
	quadTree.header.asteroidLocations = asteroidData;
	BuildSystem(quadTree.header);
}

#endif