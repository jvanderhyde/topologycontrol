//convertt.cpp
//James Vanderhyde, 19 May 2004

#include "TriangleMesh.h"

int main(int argc,char** argv)
{
	int result;
	
	if (argc<3)
	{
		cerr << "Usage: " << argv[0] << " <input triangle file> <output triangle file>\n";
		return 1;
	}
	
	TriangleMesh mesh;
	
	result=mesh.readFile(argv[1]);
	if (result) return result;
	result=mesh.writeFile(argv[2]);
	if (result) return result;
	
	return 0;
}
