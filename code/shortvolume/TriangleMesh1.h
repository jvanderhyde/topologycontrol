/*
 *  TriangleMesh.h
 *  
 *
 *  Created by James Vanderhyde on Thu May 6 2004.
 *
 */

#include <vector>
#include <iostream.h>
#include <fstream.h>

class Triangle
{
public:
	int a,b,c;
	int label;
	Triangle();
	Triangle(int aa,int bb,int cc);
};

class Point3D
{
public:
	float x,y,z;
	int label;
	Point3D();
	Point3D(float xx,float yy,float zz);
};

class TriangleMesh
{
protected:
	std::vector<Triangle> tris;
	std::vector<Point3D> verts;
public:
	Point3D getVert(int i);
	Triangle getTri(int i);
	void addVert(Point3D pt);
	void addTri(Triangle tri);
	void setVertLabel(int i,char label);
	void setTriLabel(int i,char label);
	int getNumVerts();
	int getNumTris();
	void eliminateExcessVertices();
	int readBinaryFile(istream& in);
	int writeBinaryFile(ostream& out);
	int readASCIIFile(istream& in);
	int writeASCIIFile(ostream& out);
	int readFile(char* filename);
	int writeFile(char* filename);
};
