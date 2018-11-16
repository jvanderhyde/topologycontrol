//createvolume.cpp
//James Vanderhyde, 14 March 2005

#include <iostream.h>

#include "shortvolume.h"
#include "Vector3D.h"

#define SGN(x) (((x)<0)?-1:(((x)>0)?1:0))
#define ABS(x) (((x)<0)?-(x):(x))

int getNeighbors6(volume& v,int* vsize,int x,int y,int z,int* neighbors)
{
    int n=0;
    if (x-1>=0)       neighbors[n++]=v.getVoxelIndex(x-1,y,z);
    if (x+1<vsize[0]) neighbors[n++]=v.getVoxelIndex(x+1,y,z);
    if (y-1>=0)       neighbors[n++]=v.getVoxelIndex(x,y-1,z);
    if (y+1<vsize[1]) neighbors[n++]=v.getVoxelIndex(x,y+1,z);
    if (z-1>=0)       neighbors[n++]=v.getVoxelIndex(x,y,z-1);
    if (z+1<vsize[2]) neighbors[n++]=v.getVoxelIndex(x,y,z+1);
    return n;
}

void calcDistances(volume& v)
{
    cout << "Calculating distance function..."; cout.flush();
	
	minqueue q;
	int x,y,z;
    int n,num;
    int neighbors[6];
    int index;
    pqitem top;
	short signedDistance;
	int* vsize=v.getSize();
	
	for (index=0; index<vsize[2]*vsize[1]*vsize[0]; index++)
	{
		if (v.getKnown(index))
		{
			v.getVoxelLocFromIndex(index,&x,&y,&z);
			q.push(pqitem(v.getVoxel(x,y,z),x,y,z));
		}
	}
	
    while (!q.empty())
    {
        //pop voxel from queue
        top=q.top();
        q.pop();
		signedDistance=v.getVoxel(top.x,top.y,top.z);
        
        //push neighbors onto queue
        num=getNeighbors6(v,vsize,top.x,top.y,top.z,neighbors);
        for (n=0; n<num; n++)
        {
            if (!v.getKnown(neighbors[n]))
            {
				v.getVoxelLocFromIndex(neighbors[n],&x,&y,&z);
				v.setVoxel(x,y,z,SGN(signedDistance)*(ABS(signedDistance)+1));
                v.setKnown(neighbors[n],1);
                q.push(pqitem(v.getVoxel(x,y,z),x,y,z));
            }
        }
    }
	
    cout << "done.\n";
}

int main(int argc, char* argv[])
{
	if (argc<=1)
	{
		cerr << "Usage: " << argv[0] << " <output volume file>\n";
		return 1;
	}
	
	volume v(20,10,20);
	int* vsize=v.getSize();
	int numVoxels=vsize[2]*vsize[1]*vsize[0];
	int index,x,y,z;
	
	for (index=0; index<numVoxels; index++)
		v.setKnown(index,0);
	
	int rad1=9;
	for (x=0; x<rad1; x++)
	{
		y=5;
		z=x;
		v.setVoxel(10-(rad1-1-x),y,10-z,1);
		v.setVoxel(10-(rad1-1-x),y,10+z,1);
		v.setVoxel(10+x,y,10-(rad1-1-z),1);
		v.setVoxel(10+x,y,10+(rad1-1-z),1);
		v.setKnown(v.getVoxelIndex(10-(rad1-1-x),y,10-z),1);
		v.setKnown(v.getVoxelIndex(10-(rad1-1-x),y,10+z),1);
		v.setKnown(v.getVoxelIndex(10+x,y,10-(rad1-1-z)),1);
		v.setKnown(v.getVoxelIndex(10+x,y,10+(rad1-1-z)),1);
	}
	
	calcDistances(v);
	
	z=14;
	x=10;
	y=5;
	v.setVoxel(x,y,z,4);
	v.setKnown(v.getVoxelIndex(x,y,z),1);

	/*x=28;
	z=20;
	y=10;
	v.setVoxel(x,y,z,10);
	v.setKnown(v.getVoxelIndex(x,y,z),1);*/
	
	v.writeFile(argv[1]);
	
}