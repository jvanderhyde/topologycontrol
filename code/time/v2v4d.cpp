//v2v4D.cpp
//James Vanderhyde, 30 May 2007.

#include <iostream.h>
#include <fstream.h>
#include <assert.h>

int main(int argc, char* argv[])
{
    if (argc<3)
    {
	cout << "Usage: " << argv[0] << " <output .v4d file> <input .v file> ... \n";
	return 1;
    }
    
    int vsize[3];
    int numSlices=argc-2;
    ifstream fin1(argv[2]);
    if (!fin1)
    {
	cerr << "Error reading file " << argv[2] << "\n";
	return 1;
    }
    fin1.read((char*)vsize,sizeof(vsize));
    int numVoxels=vsize[0]*vsize[1]*vsize[2];
    char* data=new char[numVoxels*sizeof(float)];
    fin1.read(data,numVoxels*sizeof(float));
    fin1.close();
    
    ofstream fout(argv[1]);
    if (!fout)
    {
	cerr << "Error writing to file " << argv[1] << "\n";
	return 1;
    }
    fout.write((char*)vsize,sizeof(vsize));
    fout.write((char*)&numSlices,sizeof(int));
    fout.write(data,numVoxels*sizeof(float));
    
    for (int slice=1; slice<numSlices; slice++)
    {
	int size[3];
	ifstream fin(argv[2+slice]);
	if (!fin)
	{
	    cerr << "Error reading file " << argv[2+slice] << "\n";
	    fout.close();
	    return 1;
	}
	fin.read((char*)size,sizeof(size));
	if ((size[0]!=vsize[0]) || (size[1]!=vsize[1]) || (size[1]!=vsize[1]))
	{
	    cerr << "File " << argv[2+slice] << " does not have the correct dimensions\n";
	    fout.close();
	    return 1;
	}
	fin.read(data,numVoxels*sizeof(float));
	fin.close();
	fout.write(data,numVoxels*sizeof(float));
    }
    fout.close();
    
    delete[] data;
    return 0;
}
