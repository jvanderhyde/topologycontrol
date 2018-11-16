//gauss.cpp
//James Vanderhyde, 25 May 2005

#include <fstream.h>
#include <math.h>

#include "MarchableVolume.h"

#define CLAMPMIN 0.1
#define CLAMP(o,n) ((o<0.0)?((n<-CLAMPMIN)?n:-CLAMPMIN):((n>CLAMPMIN)?n:CLAMPMIN))

//These are actually erf(1.5*x) so that we hit 3 stddevs after moving 2 voxels away
#define ERF0 0.39894228
#define ERF1 0.129517596
#define ERFROOT2 0.042048207
#define ERFROOT3 0.0136510542
#define NORM 1.78983477

float clamp(float origVal,float newVal)
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

void gaussianFilterOnVolume(MarchableVolume* v)
{
  int* vsize=v->getSize();
  int sliceSize=vsize[1]*vsize[0];
  float* prevSlice=new float[sliceSize];
  float* curSlice=new float[sliceSize];
  float* tempSlicePtr;
  int x,y,z,index;
  float val,delta,totalDelta=0,maxDelta=0;
  int numChanged=0;

  //Set up the initial previous slice
  z=0;
  index=0;
  for (y=0; y<vsize[1]; y++)
    for (x=0; x<vsize[0]; x++)
      prevSlice[index++]=v->d(x,y,z);
  
  for (z=1; z<vsize[2]-1; z++)
    {
      //Compute the filter and store into current slice
      //We hold the boundary values fixed.
      index=0;
      y=0;
      for (x=0; x<vsize[0]; x++) curSlice[index++]=v->d(x,y,z);
      for (y=1; y<vsize[1]-1; y++)
	{
	  x=0;
	  curSlice[index++]=v->d(x,y,z);
	  for (x=1; x<vsize[0]-1; x++)
	    {
	      if (!v->getKnownFlag(x,y,z))
		{
		  val =
		    (ERFROOT3*(v->d(x-1,y-1,z-1)+v->d(x+1,y-1,z-1)+v->d(x-1,y+1,z-1)+v->d(x-1,y-1,z+1)+
			       v->d(x+1,y+1,z-1)+v->d(x-1,y+1,z+1)+v->d(x+1,y-1,z+1)+v->d(x+1,y+1,z+1))+
		     ERFROOT2*(v->d(x-1,y,z-1)+v->d(x,y-1,z-1)+v->d(x+1,y,z-1)+v->d(x,y-1,z-1)+
			       v->d(x-1,y,z)+v->d(x,y-1,z)+v->d(x+1,y,z)+v->d(x,y-1,z)+
			       v->d(x-1,y,z+1)+v->d(x,y-1,z+1)+v->d(x+1,y,z+1)+v->d(x,y-1,z+1))+
		     ERF1*(v->d(x-1,y,z)+v->d(x,y-1,z)+v->d(x,y,z-1)+v->d(x+1,y,z)+v->d(x,y+1,z)+v->d(x,y,z+1))+
		     ERF0*v->d(x,y,z))/NORM;
		  curSlice[index]=CLAMP(v->d(x,y,z),val);
		  delta=fabs(curSlice[index]-v->d(x,y,z));
		  totalDelta+=delta;
		  numChanged++;
		  if (delta>maxDelta) maxDelta=delta;
		}
	      else curSlice[index]=v->d(x,y,z);
	      index++;
	    }
	  curSlice[index++]=v->d(x,y,z);
	}
      for (x=0; x<vsize[0]; x++) curSlice[index++]=v->d(x,y,z);
      
      //We're done with the previous slice's data, so copy it into the volume
      index=0;
      for (y=0; y<vsize[1]; y++)
	for (x=0; x<vsize[0]; x++)
	  v->d(x,y,z-1,prevSlice[index++]);
      
      //The current slice becomes the previous slice.
      tempSlicePtr=curSlice;
      curSlice=prevSlice;
      prevSlice=tempSlicePtr;
    }
  
  //Save the last previous slice
  index=0;
  for (y=0; y<vsize[1]; y++)
    for (x=0; x<vsize[0]; x++)
      v->d(x,y,z-1,prevSlice[index++]);
  
  cout << "Average change: " << totalDelta/numChanged << "  ";
  cout << "Max change: " << maxDelta << '\n';
  
  delete[] prevSlice;
  delete[] curSlice;
}

int main(int argc, char* argv[])
{
  int result=0;
  if (argc<3)
    {
      cout << "Usage:" << argv[0] << " <input volume file> <output volume file>\n";
      return 1;
    }
  
  MarchableVolume* v=MarchableVolume::createVolume(argv[1]);
  
  if (v==NULL) return 1;

  for (int i=0; i<50; i++)
    gaussianFilterOnVolume(v);

  cout << "Writing file " << argv[2] << " ..."; cout.flush();
  result=v->writeFile(argv[2]);
  if (result) cout << "error!\n";
  else cout << "done.\n";
  return result;
}
