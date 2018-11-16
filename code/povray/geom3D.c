
/* --- mesh.c --- */

#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>

#include "geom3D.h"

/* -------------------------------- */

int which ( int i, int3 t )
{
  if (i==t[0])
    return 0;
  if (i==t[1])
    return 1;
  assert(i==t[2]);
  return 2;
}

/* -------------------------------- */

void double3_subtract ( double3 t, double3 a, double3 b )
{
  t[0] = a[0] - b[0];
  t[1] = a[1] - b[1];
  t[2] = a[2] - b[2];
}

/* -------------------------------- */

double double3_dot ( double3 a, double3 b )
{
  return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

/* -------------------------------- */

void double3_add ( double3 t, double3 a, double3 b )
{
  t[0] = a[0] + b[0];
  t[1] = a[1] + b[1];
  t[2] = a[2] + b[2];
}

/* -------------------------------- */

void double3_cross ( double3 t, double3 a, double3 b )
{
  t[0] = a[1]*b[2]-a[2]*b[1];
  t[1] = a[2]*b[0]-a[0]*b[2];
  t[2] = a[0]*b[1]-a[1]*b[0];
}

/* -------------------------------- */

void double3_scale ( double3 t, double c, double3 a )
{
  t[0] = c*a[0];
  t[1] = c*a[1];
  t[2] = c*a[2];
}

/* -------------------------------- */

void double3_scale_and_add ( double3 t, double c, double3 a )
{
  t[0] += c*a[0];
  t[1] += c*a[1];
  t[2] += c*a[2];
}

/* -------------------------------- */

void double3_normalize ( double3 p )
{
  double3_scale(p,1/sqrt(double3_dot(p,p)),p);
}

/* ---------------------------------- */

double double3_dist ( double3 p1, double3 p2 )
{
  double3 p;
  double3_subtract(p,p1,p2);
  return sqrt(double3_dot(p,p));
}

/* ---------------------------------- */

double double3_length ( double3 v )
{
  return sqrt(double3_dot(v,v));
}

/* ---------------------------------- */

void double3_copy ( double3 res, double3 a )
{
  memcpy(res,a,sizeof(double3));
}

/* ---------------------------------- */

void double3_max ( double3 res, double3 a, double3 b )
{
  int j;
  for ( j=0; j<3; j++ )
    res[j] = a[j]<b[j] ? b[j] : a[j];
}

/* ---------------------------------- */

void double3_min ( double3 res, double3 a, double3 b )
{
  int j;
  for ( j=0; j<3; j++ )
    res[j] = a[j]>b[j] ? b[j] : a[j];
}

/* ---------------------------------- */

double double3_maxcoord ( double3 a )
{
  double res = a[0]>a[1] ? a[0] : a[1];
  return res>a[2] ? res : a[2];
}
