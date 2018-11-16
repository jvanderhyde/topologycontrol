//transformmesh.cpp
//James Vanderhyde, 2 June 2005

#include "TriangleMesh.h"

int main(int argc, char* argv[])
{
	if (argc<3)
	{
		cout << "Usage: " << argv[0] << " <input triangle file> <output triangle file>\n";
		return 1;
	}
	
	TriangleMesh mesh,mesh1,mesh2,mesh3,mesh4;
	mesh.readFile(argv[1]);
	int oldNumVerts=mesh.getNumVerts();
	Vertex v1(20,20,20),v2(19,20,20),v3(20,19,20);
	Triangle t1(oldNumVerts,oldNumVerts+1,oldNumVerts+2);

	mesh1.readFile(argv[1]);
	mesh1.translate(4,4,-4);
	mesh1.writeFile(argv[2]);

	mesh2.readFile(argv[1]);
	mesh2.translate(4,-4,-4);
	mesh2.writeFile(argv[3]);
	
	mesh3.readFile(argv[1]);
	mesh3.translate(-4,4,-4);
	mesh3.writeFile(argv[4]);

	mesh4.readFile(argv[1]);
	mesh4.translate(-4,-4,-4);
	mesh4.writeFile(argv[5]);
	
	return 0;
}