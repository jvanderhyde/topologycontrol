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

#include "Vector3D.h"

class Vertex
{
public:
	float x,y,z;
	Vector3D n;
	int label;
	Vertex();
	Vertex(float xx,float yy,float zz);
	Vector3D getLocation();
};

class Triangle
{
public:
	int a,b,c;
	Vector3D n;
	int label;
	Triangle();
	Triangle(int aa,int bb,int cc);
};

class TriangleMesh
{
protected:
	std::vector<Triangle> tris;
	std::vector<Vertex> verts;
	std::vector<int> opposites;
	int numInternalEdges,numExternalEdges;
	
	int triangleHasVertex(int t,int v);
	int findIncidentTriangle(int* degrees,int** incidences,int va,int vb,int notTri=-1);
	
public:
	Vertex getVert(int i);
	Triangle getTri(int i);
	int getOpposite(int tri,int abc);
	void addVert(Vertex v);
	void addTri(Triangle tri);
	void setVertLabel(int i,char label);
	void setTriLabel(int i,char label);
	int getNumVerts();
	int getNumTris();
	int getNumInternalEdges();
	int getNumExternalEdges();
	void eliminateExcessVertices();
	void calculateNormals();
	void computeAdjacencies();
	int readBinaryFile(istream& in);
	int writeBinaryFile(ostream& out);
	int readASCIIFile(istream& in);
	int writeASCIIFile(ostream& out);
	int readFile(char* filename);
	int writeFile(char* filename);
};
