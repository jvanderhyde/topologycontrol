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
	int vertex(int abc);
	int getOneVertex(int notVert1=-1,int notVert2=-1);
};

class TriangleMesh
{
protected:
	std::vector<Triangle> tris;
	std::vector<Vertex> verts;
	std::vector<int> opposites;
	int numInternalEdges,numExternalEdges;
	int numComponents;
	int verbose;
	
	int triangleHasVertex(int t,int v);
	int findIncidentTriangle(int* degrees,int** incidences,int va,int vb,int notTri=-1);
	void traverseAndLabel(int startTri,int label);
	
public:
	TriangleMesh();

	Vertex getVert(int i);
	Triangle getTri(int i);
	int getOpposite(int tri,int abc);
	void addVert(Vertex v);
	void addTri(Triangle tri);
	void setVertLabel(int i,int label);
	void setTriLabel(int i,int label);
	void setVerbose(int p_verbose);
	int getVerbose();
	int getNumVerts();
	int getNumTris();
	int getNumInternalEdges();
	int getNumExternalEdges();
	int getNumComponents();
	void reorientAllTris();
	void eliminateExcessVertices();
	void calculateNormals();
	void computeConnectivity();
	void markBoundaryVertices();
	void labelComponents(); //note: changes tri labels
	void printBettiInfo(ostream& out=cout);
	void eliminateAllButLargestComponent(); //note: changes tri labels
	void eliminateLargestComponent(); //note: changes tri labels
	void eliminateSmallComponents(int threshold); //note: changes tri labels
	void cropBelowPlane(float a,float b,float c,float d);
	int readBMKBinFile(istream& in);
	int readBinaryFile(istream& in);
	int writeBinaryFile(ostream& out);
	int readASCIIFile(istream& in);
	int writeASCIIFile(ostream& out);
	int readFile(char* filename);
	int writeFile(char* filename);
	void clear();
};
