//reorient.cpp
//James Vanderhyde, 1 May 2007.

#include "TriangleMesh.h"

int main(int argc,char* argv[])
{
    if (argc<3)
    {
	cout << "Usage: " << argv[0] << " <input triangle file> <output triangle file>\n";
	return 1;
    }
    
    int error;
    TriangleMesh m;
    
    error=m.readFile(argv[1]);
    if (error)
    {
	cout << "Error reading file " << argv[1] << '\n';
	return 1;
    }
    //cout << "File read from " << argv[1] << '\n';
    
    m.reorientAllTris();
    
    error=m.writeFile(argv[2]);
    if (error)
    {
	cout << "Error writing file " << argv[2] << '\n';
	return 1;
    }
    //cout << "File written to " << argv[2] << '\n';

    return 0;
}