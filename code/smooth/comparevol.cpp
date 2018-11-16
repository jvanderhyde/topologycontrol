//comparevol.cpp
//James Vanderhyde, 8 March 2005

#include <iostream.h>
#include <fstream.h>

#include "MarchableVolume.h"

int main(int argc, char* argv[])
{
  if (argc<=2)
    {
      cerr << "Usage: " << argv[0] << " <volume 1> <volume 2>\n";
      return 1;
    }

  MarchableVolume* v1,* v2;
  v1=MarchableVolume::createVolume(argv[1]);
  v2=MarchableVolume::createVolume(argv[2]);

  if (!v1)
    {
      cerr << "Problem reading file " << argv[1] << '\n';
      return 2;
    }
  if (!v2)
    {
      cerr << "Problem reading file " << argv[2] << '\n';
      return 2;
    }

  int* vsize1,* vsize2;
  vsize1=v1->getSize();
  vsize2=v2->getSize();
  if (vsize1[0] != vsize2[0])
    cout << "x dimension not the same\n";
  if (vsize1[1] != vsize2[1])
    cout << "y dimension not the same\n";
  if (vsize1[2] != vsize2[2])
    cout << "z dimension not the same\n";
  if ((vsize1[0] != vsize2[0]) || (vsize1[1] != vsize2[1]) || (vsize1[2] != vsize2[2]))
    return 0;

  int* vsize=vsize1;

  /*vsize=new int[3];
  vsize[0]=vsize[1]=vsize[2]=0;
  ifstream fin(argv[1]);
  octree* dataroot=new octree();
  dataroot->readDF(fin,vsize,0,0,0);
  int treeHeight=ceillogbase2(MAX(MAX(vsize[0],vsize[1]),vsize[2]));*/
  
  int x,y,z;
  int numDiffs=0;
  for (z=0; z<vsize[2]; z++) for (y=0; y<vsize[1]; y++) for (x=0; x<vsize[0]; x++)
    {
      if (v1->d(x,y,z) != v2->d(x,y,z))
	{
	  numDiffs++;
	  if (numDiffs<=100)
	    { 
	      cout << "(" << x << "," << y << "," << z << "): v1=" << v1->d(x,y,z) << ", v2=" << v2->d(x,y,z);
	      //cout << " octree depth=" << dataroot->levelOfDeepestAncestor(treeHeight,x,y,z);
	      cout << '\n';
	    }
	}
    }
  if (numDiffs>100) cout << "etc.\n";
  if (numDiffs==0) cout << "Volumes are exactly the same!\n";

  return 0;
}
