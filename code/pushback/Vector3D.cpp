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

void times(Matrix m1,Matrix m2,Matrix r)
{
	r[0]=m1[0]*m2[0]+m1[4]*m2[1]+m1[8]*m2[2]+m1[12]*m2[3];
	r[1]=m1[1]*m2[0]+m1[5]*m2[1]+m1[9]*m2[2]+m1[13]*m2[3];
	r[2]=m1[2]*m2[0]+m1[6]*m2[1]+m1[10]*m2[2]+m1[14]*m2[3];
	r[3]=m1[3]*m2[0]+m1[7]*m2[1]+m1[11]*m2[2]+m1[15]*m2[3];
	r[4]=m1[0]*m2[4]+m1[4]*m2[5]+m1[8]*m2[6]+m1[12]*m2[7];
	r[5]=m1[1]*m2[4]+m1[5]*m2[5]+m1[9]*m2[6]+m1[13]*m2[7];
	r[6]=m1[2]*m2[4]+m1[6]*m2[5]+m1[10]*m2[6]+m1[14]*m2[7];
	r[7]=m1[3]*m2[4]+m1[7]*m2[5]+m1[11]*m2[6]+m1[15]*m2[7];
	r[8]=m1[0]*m2[8]+m1[4]*m2[9]+m1[8]*m2[10]+m1[12]*m2[11];
	r[9]=m1[1]*m2[8]+m1[5]*m2[9]+m1[9]*m2[10]+m1[13]*m2[11];
	r[10]=m1[2]*m2[8]+m1[6]*m2[9]+m1[10]*m2[10]+m1[14]*m2[11];
	r[11]=m1[3]*m2[8]+m1[7]*m2[9]+m1[11]*m2[10]+m1[15]*m2[11];
	r[12]=m1[0]*m2[12]+m1[4]*m2[13]+m1[8]*m2[14]+m1[12]*m2[15];
	r[13]=m1[1]*m2[12]+m1[5]*m2[13]+m1[9]*m2[14]+m1[13]*m2[15];
	r[14]=m1[2]*m2[12]+m1[6]*m2[13]+m1[10]*m2[14]+m1[14]*m2[15];
	r[15]=m1[3]*m2[12]+m1[7]*m2[13]+m1[11]*m2[14]+m1[15]*m2[15];
}

void copy(Matrix m,Matrix r)
{
	for (int i=0; i<16; i++) r[i]=m[i];
}

