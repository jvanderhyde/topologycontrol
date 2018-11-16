//combinedv2.cpp
//James Vanderhyde, 17 Apr 2007.

#include "bitc.h"

#include <fstream.h>

int main(int argc, char* argv[])
{
    if (argc<5)
    {
	cerr << "Usage: " << argv[0] << " <r0 .v2 file> <r1 .v2 file> <r2 .v2 file> <output .v2 file>\n";
	return 1;
    }
    
    bitc r0,r1,r2,result;
    
    int vsize[3],vsize1[3],vsize2[3];
    int numVoxels;

    cout << "Reading input files..."; cout.flush();
    ifstream fin0(argv[1]);
    if (!fin0)
    {
	cerr << "Error reading file " << argv[1] << "\n";
	return 1;
    }
    ifstream fin1(argv[2]);
    if (!fin1)
    {
	cerr << "Error reading file " << argv[2] << "\n";
	return 1;
    }
    ifstream fin2(argv[3]);
    if (!fin2)
    {
	cerr << "Error reading file " << argv[3] << "\n";
	return 1;
    }
    fin0.read((char*)vsize,sizeof(vsize));
    numVoxels=vsize[2]*vsize[1]*vsize[0];
    r0.read(fin0,numVoxels);
    r0.read(fin0,numVoxels);
    fin0.close();
    fin1.read((char*)vsize1,sizeof(vsize1));
    if (numVoxels != vsize1[2]*vsize1[1]*vsize1[0])
    {
	cerr << "Number of voxels different in file 0 and file 1.\n";
	return 2;
    }
    r1.read(fin1,numVoxels);
    r1.read(fin1,numVoxels);
    fin1.close();
    fin2.read((char*)vsize2,sizeof(vsize2));
    if (numVoxels != vsize2[2]*vsize2[1]*vsize2[0])
    {
	cerr << "Number of voxels different in file 0 and file 2.\n";
	return 2;
    }
    r2.read(fin2,numVoxels);
    r2.read(fin2,numVoxels);
    fin2.close();
    cout << "done.\n";
    
    result.setbit(numVoxels-1); result.resetbit(numVoxels-1);
    
    int x,y,z;
    int index0, index1, index2;
    for (z=0; z<vsize[2]; z++) for (y=0; y<vsize[1]; y++) for (x=0; x<vsize[0]; x++)
    {
	index0=(z*vsize[1]+y)*vsize[0]+x;
	index1=(x*vsize[2]+z)*vsize[1]+y;
	index2=(y*vsize[0]+x)*vsize[2]+z;
	if ((r0.getbit(index0)) || (r1.getbit(index1)) || (r2.getbit(index2)))
	    result.setbit(index0);
	else
	    result.resetbit(index0);
    }
    
    cout << "Writing to file " << argv[4] << "..."; cout.flush();
    ofstream fout(argv[4]);
    if (!fout)
    {
	cerr << "Error writing to file " << argv[4] << "\n";
	return 1;
    }
    fout.write((char*)vsize,sizeof(vsize));
    result.write(fout,numVoxels);
    result.write(fout,numVoxels);
    fout.close();
    cout << "done.\n";
    return 0;
    
}
