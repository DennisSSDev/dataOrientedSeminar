//////////////////////////////////////////////////////////////////////////////////////
// User-defined constants: 
// ROWS is the number of rows of  asteroids.
// COLUMNS is the number of columns of asteroids.
// FILL_PROBABILITY is the percentage probability that a particular row-column slot
// will be filled with an asteroid.
/////////////////////////////////////////////////////////////////////////////////////

#pragma once

constexpr auto PI = 3.14159265;
constexpr auto ROWS = 100;  // Number of rows of asteroids.;
constexpr auto COLUMNS = 100; // Number of columns of asteroids.;
constexpr auto FILL_PROBABILITY = 100;

constexpr auto SPHERE_VERTEX_COUNT = 288;
constexpr auto SPHERE_SIZE = 5.0f;

struct Asteroids
{
	Asteroids(){}; // compiler complaints 
	// pos
	float x[ROWS*COLUMNS];
	float y[ROWS*COLUMNS];
	float z[ROWS*COLUMNS];
	// index
	float i[ROWS*COLUMNS];
	// radius
	float rds[ROWS*COLUMNS];
	//colors
	unsigned char r[ROWS*COLUMNS];
	unsigned char g[ROWS*COLUMNS];
	unsigned char b[ROWS*COLUMNS];
};
