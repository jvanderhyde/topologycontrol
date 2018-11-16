//traceseeds.cpp
//James Vanderhyde, 14 March 2007

#include "Volume2DplusT.h"

#include <fstream.h>

int numContourSeeds;
int* contourSeeds=NULL;
int* xmin,* xmax,* ymin,* ymax,* zmin,* zmax;

int* vsize;
int* vorder;

void readSeedList(ifstream& fin,int dim,int num)
{
    int i,seed,winStart,winEnd,x,y,z;
    for (i=0; i<num; i++)
    {
	xmin[i]=0;
	xmax[i]=vsize[0]-1;
	ymin[i]=0;
	ymax[i]=vsize[1]-1;
	zmin[i]=0;
	zmax[i]=vsize[2]-1;
	fin >> seed >> winStart >> winEnd;
	if (dim==0)
	{
	    x=seed%vsize[0];
	    y=(seed/vsize[0])%vsize[1];
	    z=(seed/vsize[0])/vsize[1];
	    zmin[i]=winStart;
	    zmax[i]=winEnd;
	}
	else if (dim==1)
	{
	    y=seed%vsize[1];
	    z=(seed/vsize[1])%vsize[2];
	    x=(seed/vsize[1])/vsize[2];
	    xmin[i]=winStart;
	    xmax[i]=winEnd;
	}
	else
	{
	    z=seed%vsize[2];
	    x=(seed/vsize[2])%vsize[0];
	    y=(seed/vsize[2])/vsize[0];
	    ymin[i]=winStart;
	    ymax[i]=winEnd;
	}
	contourSeeds[i]=(z*vsize[1]+y)*vsize[0]+x;
    }
}

int readSeedFile(char* filename1,int dim)
{
    ifstream fin1(filename1);
    if (!fin1)
    {
	cerr << "Can't find file " << filename1 << "\n";
	return 1;
    }
    
    fin1 >> numContourSeeds;
    
    contourSeeds=new int[numContourSeeds];
    xmin=new int[numContourSeeds];
    xmax=new int[numContourSeeds];
    ymin=new int[numContourSeeds];
    ymax=new int[numContourSeeds];
    zmin=new int[numContourSeeds];
    zmax=new int[numContourSeeds];
    
    readSeedList(fin1,dim,numContourSeeds);
    
    return 0;
}

void swap(int a,int b)
{
    int ts,txn,txx,tyn,tyx,tzn,tzx;
    ts=contourSeeds[a];
    txn=xmin[a];
    txx=xmax[a];
    tyn=ymin[a];
    tyx=ymax[a];
    tzn=zmin[a];
    tzx=zmax[a];
    contourSeeds[a]=contourSeeds[b];
    xmin[a]=xmin[b];
    xmax[a]=xmax[b];
    ymin[a]=ymin[b];
    ymax[a]=ymax[b];
    zmin[a]=zmin[b];
    zmax[a]=zmax[b];
    contourSeeds[b]=ts;
    xmin[b]=txn;
    xmax[b]=txx;
    ymin[b]=tyn;
    ymax[b]=tyx;
    zmin[b]=tzn;
    zmax[b]=tzx;
}

void sortSeeds()
{
    //We want the seeds that are last in the order first, because they are the least constraining.
    for (int i=0; i<numContourSeeds; i++)
	for (int j=0; j<numContourSeeds-i-1; j++)
	{
	    if (vorder[contourSeeds[j]]<vorder[contourSeeds[j+1]]) swap(j,j+1);
	}
}

int main(int argc, char* argv[])
{
  if (argc<4)
  {
      cerr << "Usage: " << argv[0] << " <input volume file> <output .v2 file> <seed file> [<rotated seed file> <rotated seed file>]\n";
      return 1;
  }
    
  int result=0;
  int numSeedFiles=argc-3;
  int i,j,x,y,z;
  int simplifyTopology=0;
  unsigned short threshold=65535;

  Volume2DplusT v(threshold);
  result=v.readFile(argv[1]);
  if (result)
  {
      cerr << "Error reading file " << argv[1] << '\n';
      return result;
  }
  if (simplifyTopology)
  {
      v.readTopoinfoFiles();
      v.fixTopologyStrict();
  }
  vsize=v.getSize();
  vorder=v.getDefaultOrder();
  
  v.clearAllVoxelsQueued();
  for (j=0; j<numSeedFiles; j++)
  {
      result=readSeedFile(argv[3+j],j);
      if (result) return result;
      //cout << j << ' ';
      
      //Should sort seeds so that greatest in voxel order is handled first. Otherwise floodfill may miss some.
      sortSeeds();
      
      for (i=0; i<numContourSeeds; i++)
      {
	  v.clearVoxelsCarvedInRange(xmin[i],xmax[i],ymin[i],ymax[i],zmin[i],zmax[i]);
	  v.markVoxelsBelowVoxelInRange(contourSeeds[i],xmin[i],xmax[i],ymin[i],ymax[i],zmin[i],zmax[i]);
	  v.setQueuedToUnionOfQueuedAndCarvedInRange(xmin[i],xmax[i],ymin[i],ymax[i],zmin[i],zmax[i]);
      }
      
      delete[] contourSeeds;
      delete[] xmin;
      delete[] xmax;
      delete[] ymin;
      delete[] ymax;
      delete[] zmin;
      delete[] zmax;
  }
  v.clearAllVoxelsCarved();
  v.setCarvedToUnionOfQueuedAndCarved();
  v.clearAllVoxelsQueued();
  //cout << '\n';
  
  result=v.saveCarveInfo(argv[2]);
  if (result)
  {
      cerr << "Error writing to file " << argv[2] << '\n';
      return result;
  }
  
  return result;
}
