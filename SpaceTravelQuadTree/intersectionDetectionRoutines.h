#ifndef intersectionDetectionRoutines_2394
#define intersectionDetectionRoutines_2394

#include <cstdlib>
#include <cmath>
#include <list>
#include <iostream>

#define PI 3.14159265

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////     
// intersectionDetectionRoutines.cpp
//
// Routines are written to check for intersection between two co-planar straight line segments,
// between two coplanar quadrilaterals, and a coplanar disc and axis-aligned rectangle. 
// Required sub-routines are written as well.
//
// Sumanta Guha.
///////////////////////////////////////////////////////////////////////////////////////////////     

// Return determinant of a 2x2 matrix with elements input in row-major order.
float det2(float a11, float a12, float a21, float a22);


// Return determinant of a 3x3 matrix with elements input in row-major order.
float det3(float a11, float a12, float a13, float a21, float a22,
	float a23, float a31, float a32, float a33);


// Given three collinear points (x1,y1) and (x2,y2) and (x3,y3) return 0 if (x3,y3) lies
// in the segment joining (x1,y1) and (x2,y2), -1 if it lies on one side, and 1 if on the other.
int checkPointWRTSegment(float x1, float y1, float x2, float y2, float x3, float y3);


// Return 1 if the segment joining (x1,y1) and (x2,y2) intersects the 
// segment joining (x3,y3) and (x4,y4), otherwise return 0.
int checkSegmentsIntersection(float x1, float y1, float x2, float y2,
	float x3, float y3, float x4, float y4);


// Return 1 if the point (x5,y5) lies in the quadrilateral with vertices at (x1,y1), (x2,y2), (x3,y3) 
// and (x4,y4), otherwise return 0.
int checkPointInQuadrilateral(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	float x5, float y5);

// Return 1 if the quadrilateral with  vertices (x1,y1), (x2,y2), (x3,y3) and (x4,y4) 
// intersects the quadrilateral with vertices at (x5,y5), (x6,y6), (x7,y7) and (x8,y8) 
// (both assumed not self-intersecting), otherwise return 0.
int checkQuadrilateralsIntersection(float x1, float y1, float x2, float y2,
	float x3, float y3, float x4, float y4,
	float x5, float y5, float x6, float y6,
	float x7, float y7, float x8, float y8);


// Return 1 if the axes-parallel rectangle with diagonally opposite corners at (x1,y1) and (x2,y2)
// intersects the disc centered (x3,y3) of radius r, otherwise return 0.
int checkDiscRectangleIntersection(float x1, float y1, float x2, float y2, float x3, float y3, float r);


#endif