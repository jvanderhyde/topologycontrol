//repairvol.cpp
//James Vanderhyde, 24 January 2005

#include <iostream.h>
#include <fstream.h>
#include "MarchableVolume.h"
#define SGN(x) (((x)<0)?-1:(((x)>0)?1:0))

int readBitFile(char* filename, bitc& positive, bitc& flags)
{
	if (filename)
	{
		ifstream fin(filename);
		if (fin)
		{
			int size[3];
			fin.read((char*)size,sizeof(size));
			int numVoxels=size[2]*size[1]*size[0],sliceSize=size[1]*size[0];
			positive.setbit(numVoxels-1); positive.resetbit(numVoxels-1);
			positive.read(fin,numVoxels);
			flags.setbit(numVoxels-1); flags.resetbit(numVoxels-1);
			flags.read(fin,numVoxels);
			fin.close();
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
	return 0;
}

int decideSide(MarchableVolume* mv, bitc& positive, bitc& carved)
{
	int* vsize;
	vsize=mv->getSize();
	int x,y,z,index=0;
	float val;
	int numPos=0,numNeg=0;
	for (z=0; z<vsize[2]; z++)
		for (y=0; y<vsize[1]; y++)
			for (x=0; x<vsize[0]; x++)
			{
				if (!carved.getbit(index))
				{
					if (positive.getbit(index)) numPos++;
					if (!positive.getbit(index)) numNeg++;
				}
				index++;
			}
	if (numPos>numNeg) return -1;
	else return 1;
}

void processVolume(MarchableVolume* mv, bitc& positive, bitc& known, bitc& carved,int side)
{
	int* vsize;
	vsize=mv->getSize();
	int numVoxels=vsize[2]*vsize[1]*vsize[0],sliceSize=vsize[1]*vsize[0];
	int x,y,z,index=0;
	float val;
	int last=-1;
	for (z=0; z<vsize[2]; z++)
	{		
		for (y=0; y<vsize[1]; y++)
		{
			for (x=0; x<vsize[0]; x++)
			{
				val=mv->d(x,y,z);
				if ((!carved.getbit(index)) && ((2*positive.getbit(index)-1==side) || (!known.getbit(index))))
				{
					//uncarved on side (or unknown) put slightly on the other side
					val=0.5*(-side);
					if (index>last) last=index;
				}
				mv->d(x,y,z,val);
				index++;
			}
		}
	}
}

int main(int argc,char* argv[])
{
    if (argc<=5)
    {
        cerr << "Usage: " << argv[0] << " <input volume file> <output volume file> <known .v2 file> <carved .v2 file> inside|outside|decide\n";
        return 1;
    }
	
	int error=0;
	int side=0;
	if (!strncmp(argv[5],"in",2)) side=-1;
	if (!strncmp(argv[5],"out",3)) side=1;
	MarchableVolume* mv=MarchableVolume::createVolume(argv[1]);
	bitc positive,known,carved;
	error=readBitFile(argv[3],positive,known);
	if (error)
	{
		cerr << "Error reading known info file: " << argv[3] << '\n';
		return error;
	}
	error=readBitFile(argv[4],positive,carved);
	if (error)
	{
		cerr << "Error reading carved info file: " << argv[4] << '\n';
		return error;
	}
	
	if (side==0) side=decideSide(mv,positive,carved);
	processVolume(mv,positive,known,carved,side);
	
	cout << "Writing file " << argv[2] << "..."; cout.flush();
    error=mv->writeFile(argv[2]);
	if (error)
	{
		cerr << "Error writing file " << argv[2] << '\n';
		return error;
	}
	cout << "done.\n";
	return error;
}
