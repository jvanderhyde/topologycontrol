
/* --- geom3D.h --- */

#include <stdio.h>
#include <math.h>

#ifndef __GEOM3D_H

#define __GEOM3D_H

/* -------------------------------- */

typedef double double3[3];

typedef int int3[3];

/* -------------------------------- */

extern double double3_maxcoord ( double3 a );

extern void double3_max ( double3 res, double3 a, double3 b );

extern void double3_min ( double3 res, double3 a, double3 b );

extern void double3_subtract ( double3 t, double3 a, double3 b );

extern double double3_dot ( double3 a, double3 b );

extern void double3_add ( double3 t, double3 a, double3 b );

extern void double3_scale ( double3 t, double c, double3 a );

extern void double3_cross ( double3 t, double3 a, double3 b );

extern void double3_normalize ( double3 p );

extern void double3_scale_and_add ( double3 t, double c, double3 a );

extern int which ( int i, int3 t );

extern double double3_dist ( double3 p1, double3 p2 );

extern double double3_length ( double3 v );

extern void double3_copy ( double3 res, double3 a );

/* -------------------------------- */

#endif
