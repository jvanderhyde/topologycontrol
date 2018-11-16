//convert.cpp
//James Vanderhyde, 15 February 2007.

#include "MarchableVolume.h"

#include <fstream.h>

int main(int argc, char* argv[])
{
    if (argc<3)
    {
	cerr << "Usage: " << argv[0] << " <input volume file> <output .v file>\n";
	return 1;
    }
    
    MarchableVolume* v=MarchableVolume::createVolume(argv[1]);
    if (!v) return 1;
    
    ofstream fout(argv[2]);
    if (!fout)
    {
	cerr << "Error writing to file " << argv[2] << "\n";
	return 1;
    }
    
    cout << "Writing to file " << argv[2] << " ... "; cout.flush();
    int* vsize=v->getSize();
    fout.write((char*)vsize,3*sizeof(int));
    
    float* slice=new float[vsize[0]*vsize[1]];
    int x,y,z,index;
    for (z=0; z<vsize[2]; z++)
    {
	for (y=0; y<vsize[1]; y++) for (x=0; x<vsize[0]; x++)
	    slice[x+y*vsize[0]]=v->d(x,y,z);
	fout.write((char*)slice,vsize[0]*vsize[1]*sizeof(float));
    }
    
    fout.close();
    cout << "done.\n";

    return 0;
}
