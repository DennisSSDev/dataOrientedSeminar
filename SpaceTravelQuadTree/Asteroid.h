
#ifndef Asteroid_13389437
#define Asteroid_13389437
#define PI 3.14159265
#define ROWS 100  // Number of rows of asteroids.
#define COLUMNS 100 // Number of columns of asteroids.

#define SPHERE_VERTEX_COUNT 288
#define SPHERE_SIZE 5.0f

struct Asteroids
{
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

#endif
