//topogauss.cpp
//James Vanderhyde, 27 May 2005

#include <fstream.h>

#include "MarchableVolume.h"

#define CLAMPMIN 0.01
#define CLAMP(o,n) (((o)<0.0)?(((n)<-CLAMPMIN)?(n):-CLAMPMIN):(((n)>CLAMPMIN)?(n):CLAMPMIN))

//These are actually erf(1.5*x) so that we hit 3 stddevs after moving 2 voxels away
#define ERF0 0.39894228
#define ERF1 0.129517596
#define ERFROOT2 0.042048207
#define ERFROOT3 0.0136510542
#define NORM 1.78983477


class topogauss
{
protected:
  octree* dataroot;
  MarchableVolume* v;  
  bitc topoinfoPatch,topoinfoThread;
  int vsize[3];

  float* prevOldSlice,* curOldSlice;

  float totalDelta,maxDelta;
  int numChanged,numClamped;
  int numAdded,numCarved;

  float oldd(int x,int y,int z,int zoff);
  int topologyCheck(bitc& topoinfo,int x,int y,int z);
  int topologyCheckOutside(int x,int y,int z);
  int topologyCheckInside(int x,int y,int z);
  float clamp(float origVal,float newVal);
  float clampOrChange(float origVal,float newVal,int x,int y,int z);

public:
  topogauss();
  ~topogauss();
  int readTopoInfoFiles();
  int readOctreeFile(char* filename);
  int writeOctreeFile(char* filename);

  void gaussianFilterOnVolume();
  void printStatistics();
};

topogauss::topogauss()
{
  dataroot=new octree();
  v=new OctreeVolume(dataroot);
  prevOldSlice=NULL;
  curOldSlice=NULL;
}

topogauss::~topogauss()
{
  delete v;
  //dataroot is deleted by OctreeVolume's destructor.
  topoinfoPatch.freespace();
  topoinfoThread.freespace();
}

float topogauss::oldd(int x,int y,int z,int zoff)
{
  switch (zoff)
    {
    case -1:
      return prevOldSlice[x+vsize[0]*y];
    case 0:
      return curOldSlice[x+vsize[0]*y];
    default:
      return v->d(x,y,z+zoff);
    }
}


//We use look-up tables based on the state of the 26 neighbors.
//For each neighbor, we set the bit if the neighbor is
// in the current inside set (negative).
int topogauss::topologyCheck(bitc& topoinfo,int x,int y,int z)
{
  int topoType=0,bit=0;
  int stepSize=1;
  int ox,oy,oz;
  octree* voxel;
  for (oz=-stepSize; oz<=stepSize; oz+=stepSize)
    for (oy=-stepSize; oy<=stepSize; oy+=stepSize)
      for (ox=-stepSize; ox<=stepSize; ox+=stepSize)
	if ((oz!=0) || (oy!=0) || (ox!=0))
	  {
	    if ((z+oz>=0) && (z+oz<vsize[2]) &&
		(y+oy>=0) && (y+oy<vsize[1]) &&
		(x+ox>=0) && (x+ox<vsize[0]))
	      {
		//Out of range is considered positive and therefore not inside.
		if (v->d(x+ox,y+oy,z+oz)<0.0)
		  topoType |= (1<<bit);
	      }
	    bit++;
	  }
  return topoinfo.getbit(topoType);
}

//Returns true if changing the specified voxel from negative to positive
// causes an increase in genus.
int topogauss::topologyCheckOutside(int x,int y,int z)
{
  return topologyCheck(topoinfoPatch,x,y,z);
}

//Returns true if changing the specified voxel from positive to negative
// causes an increase in genus.
int topogauss::topologyCheckInside(int x,int y,int z)
{
  return topologyCheck(topoinfoThread,x,y,z);
}

//Checks whether setting the voxel at (x,y,z) to newVal will cause the voxel to change sign.
// If so, clamps to CLAMPMIN with appropriate sign.
float topogauss::clamp(float origVal,float newVal)
{
  if (origVal<0.0)
    {
      if (newVal<-CLAMPMIN) return newVal;
      else return -CLAMPMIN;
    }
  else
    {
      if (newVal>CLAMPMIN) return newVal;
      else return CLAMPMIN;
    }
}

//Checks whether setting the voxel at (x,y,z) to newVal will cause a topology change.
// If so, clamps to CLAMPMIN with appropriate sign.
float topogauss::clampOrChange(float origVal,float newVal,int x,int y,int z)
{
  if (origVal<0.0)
    {
      if (newVal<-CLAMPMIN) return newVal;
      else 
	{
	  if ((newVal>CLAMPMIN) && (!topologyCheckOutside(x,y,z)))
	    {
	      numAdded++;
	      return newVal;
	    }
	  else 
	    {
	      numClamped++;
	      return -CLAMPMIN;
	    }
	}
    }
  else
    {
      if (newVal>CLAMPMIN) return newVal;
      else
	{
	  if ((newVal<-CLAMPMIN) && (!topologyCheckInside(x,y,z)))
	    {
	      numCarved++;
	      return newVal;
	    }
	  else
	    {
	      numClamped++;
	      return CLAMPMIN;
	    }
	}
    }
}

void topogauss::gaussianFilterOnVolume()
{
  int sliceSize=vsize[1]*vsize[0];
  float* tempSlicePtr;
  int x,y,z,index;
  float val,delta;

  prevOldSlice=new float[sliceSize];
  curOldSlice=new float[sliceSize];

  //Set up statistics
  totalDelta=0;
  maxDelta=0;
  numChanged=0;
  numClamped=0;
  numAdded=0;
  numCarved=0;

  //Set up the initial previous slice
  z=0;
  index=0;
  for (y=0; y<vsize[1]; y++)
    for (x=0; x<vsize[0]; x++)
      prevOldSlice[index++]=v->d(x,y,z);
  
  for (z=1; z<vsize[2]-1; z++)
    {
      //Copy the original data into the currrent slice
      index=0;
      for (y=0; y<vsize[1]; y++)
	for (x=0; x<vsize[0]; x++)
	  curOldSlice[index++]=v->d(x,y,z);

      //Compute the filter and store into the volume
      //We hold the boundary values fixed.
      for (y=1; y<vsize[1]-1; y++)
	{
	  for (x=1; x<vsize[0]-1; x++)
	    {
	      if (!v->getKnownFlag(x,y,z))
		{
		  val =
		    (ERFROOT3*(oldd(x-1,y-1,z,-1)+oldd(x+1,y-1,z,-1)+oldd(x-1,y+1,z,-1)+oldd(x-1,y-1,z,+1)+
			       oldd(x+1,y+1,z,-1)+oldd(x-1,y+1,z,+1)+oldd(x+1,y-1,z,+1)+oldd(x+1,y+1,z,+1))+
		     ERFROOT2*(oldd(x-1,y,z,-1)+oldd(x,y-1,z,-1)+oldd(x+1,y,z,-1)+oldd(x,y-1,z,-1)+
			       oldd(x-1,y,z,0)+oldd(x,y-1,z,0)+oldd(x+1,y,z,0)+oldd(x,y-1,z,0)+
			       oldd(x-1,y,z,+1)+oldd(x,y-1,z,+1)+oldd(x+1,y,z,+1)+oldd(x,y-1,z,+1))+
		     ERF1*(oldd(x-1,y,z,0)+oldd(x,y-1,z,0)+oldd(x,y,z,-1)+oldd(x+1,y,z,0)+oldd(x,y+1,z,0)+oldd(x,y,z,+1))+
		     ERF0*oldd(x,y,z,0))/NORM;
		  val=clampOrChange(v->d(x,y,z),val,x,y,z);
		  delta=fabs(v->d(x,y,z)-val);
		  totalDelta+=delta;
		  numChanged++;
		  if (delta>maxDelta) maxDelta=delta;
		  v->d(x,y,z,val); //commit change
		}
	    }
	}

      //The current slice becomes the previous slice.
      tempSlicePtr=curOldSlice;
      curOldSlice=prevOldSlice;
      prevOldSlice=tempSlicePtr;
    }
  
  delete[] prevOldSlice;
  delete[] curOldSlice;
  prevOldSlice=NULL;
  curOldSlice=NULL;
}

void topogauss::printStatistics()
{
  cout << "Average change: " << totalDelta/numChanged << '\t';
  cout << "Max change: " << maxDelta << '\t';
  cout << "Num clamped: " << numClamped << '\n';
  cout << "  Num carved:  " << numCarved << '\t';
  cout << "  Num added: " << numAdded << '\n';
}

int topogauss::readTopoInfoFiles()
{
  ifstream finp("topoinfo_vertex_patch");
  if (!finp)
    {
      cerr << "Can't open topoinfo_vertex_patch\n";
      return 1;
    }
  topoinfoPatch.read(finp,64*1024*1024); // 2^26 = 64*1024*1024

  ifstream fint("topoinfo_vertex_thread");
  if (!fint)
    {
      cerr << "Can't open topoinfo_vertex_thread\n";
      return 1;
    }
  topoinfoThread.read(fint,64*1024*1024); // 2^26 = 64*1024*1024

  return 0;
}

int topogauss::readOctreeFile(char* filename)
{
  int result=v->readFile(filename);
  int* size=v->getSize();
  vsize[0]=size[0];
  vsize[1]=size[1];
  vsize[2]=size[2];
  return result;
}

int topogauss::writeOctreeFile(char* filename)
{
  return v->writeFile(filename);
}

int main(int argc, char* argv[])
{
  int result=0;
  if (argc<3)
    {
      cout << "Usage:" << argv[0] << " <input volume file> <output volume file> <num iterations>\n";
      return 1;
    }
  
  topogauss tg;

  result=tg.readTopoInfoFiles();
  if (result) return result;

  cout << "Reading file " << argv[1] << "..."; cout.flush();
  result=tg.readOctreeFile(argv[1]);
  if (result) cout << "error!!\n";
  else cout << "done.\n";
  if (result) return result;

  int numIterations=1;
  if (argc>3) numIterations=atoi(argv[3]);

  for (int i=0; i<numIterations; i++)
    {
      tg.gaussianFilterOnVolume();
      tg.printStatistics();
    }

  cout << "Writing file " << argv[2] << " ..."; cout.flush();
  result=tg.writeOctreeFile(argv[2]);
  if (result) cout << "error!\n";
  else cout << "done.\n";
  return result;
}
