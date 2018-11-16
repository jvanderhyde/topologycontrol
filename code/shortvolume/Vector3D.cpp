//Vector3D.cpp
//James Vanderhyde, 16 October 2004

#include <math.h>
#include <iostream.h>

#include "Vector3D.h"

Vector3D::Vector3D() : x(0.0), y(0.0), z(0.0)
{
}

Vector3D::Vector3D(float xx,float yy,float zz) : x(xx), y(yy), z(zz)
{
}

ostream& operator<< (ostream& out, const Vector3D& p)
{
    out << '(' << p.x << ',' << p.y << ',' << p.z << ')';
    return out;
}

float dot(Vector3D p1,Vector3D p2)
{
	return p1.x*p2.x+p1.y*p2.y+p1.z*p2.z;
}

Vector3D cross(Vector3D p1,Vector3D p2)
{
	Vector3D r;
	r.x=p1.y*p2.z-p2.y*p1.z;
	r.y=p1.z*p2.x-p2.z*p1.x;
	r.z=p1.x*p2.y-p2.x*p1.y;
	return r;
}

Vector3D plus(Vector3D p1,Vector3D p2)
{
	Vector3D r;
	r.x=p1.x+p2.x;
	r.y=p1.y+p2.y;
	r.z=p1.z+p2.z;
	return r;
}

Vector3D minus(Vector3D p1,Vector3D p2)
{
	Vector3D r;
	r.x=p1.x-p2.x;
	r.y=p1.y-p2.y;
	r.z=p1.z-p2.z;
	return r;
}

Vector3D times(Vector3D p,double c)
{
	Vector3D r;
	r.x=c*p.x;
	r.y=c*p.y;
	r.z=c*p.z;
	return r;
}

float length(Vector3D p)
{
	return sqrt(p.x*p.x+p.y*p.y+p.z*p.z);
}

Vector3D normalized(Vector3D p)
{
	return times(p,1/length(p));
}

