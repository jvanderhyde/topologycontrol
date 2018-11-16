/*
 *  shortvolume.cpp
 *  
 *
 *  Created by James Vanderhyde on Wed Jun 18 2003.
 *
 */

#include <string.h>
#include <stdio.h>
#include <fstream.h>
#include <algorithm>
#include <math.h>

#include <assert.h>
#include <time.h>

#include "shortvolume.h"

#define BIT(b,n) (!!((n)&(1<<(b))))

voxelComparator::voxelComparator(volume* p_vol) : vol(p_vol) {}

int voxelComparator::operator()(int a,int b)
{
  unsigned short vala,valb;
  vala=vol->getVoxel(a);
  valb=vol->getVoxel(b);
  if (vala != valb) return (vala<valb);
  else return (a<b);
}

volume::volume(unsigned short p_featureSize,int p_fixStyle)
{
    data=NULL;
    criticalParent=NULL;
    size[0]=size[1]=size[2]=0;
    noTopoCheck=NULL;
    strictTopoCheck=NULL;
    sliceTopoCheck=NULL;
    comboTopoCheck=NULL;
    useStrictTopologyCheck=1;
    printCarveDetails=0;
    animationOn=0;
    frameNumber=0;
    numCarvedRegions=0;
    carvedRegions=NULL;
    alreadyCarvedNegative=0;
    featureSize=p_featureSize; //default infinity
    innerForLater=new pqvector();
    outerForLater=new pqvector();
    fixStyle=p_fixStyle; //default 0; that is, program decides whether to fix inside or outside.
    isovalue=-1.0;
    initialOrder=NULL;
    carvedInsideOrder=NULL;
    carvedSlicesOrder=NULL;
    tempStructure=0;
    setUpNeighborhoods();
}

volume::volume(unsigned short* vdata,int* vsize)
{
    data=vdata;
    criticalParent=NULL;
    size[0]=vsize[0];
    size[1]=vsize[1];
    size[2]=vsize[2];
    noTopoCheck=NULL;
    strictTopoCheck=NULL;
    sliceTopoCheck=NULL;
    comboTopoCheck=NULL;
    useStrictTopologyCheck=1;
    printCarveDetails=0;
    animationOn=0;
    frameNumber=0;
    numCarvedRegions=0;
    carvedRegions=NULL;
    alreadyCarvedNegative=0;
    featureSize=0;
    innerForLater=new pqvector();
    outerForLater=new pqvector();
    fixStyle=0;
    isovalue=-1.0;
    initialOrder=NULL;
    carvedInsideOrder=NULL;
    carvedSlicesOrder=NULL;
    tempStructure=0;
    setUpNeighborhoods();
}

volume::~volume()
{
    int i;
    if (data) if (!tempStructure) delete[] data;
    if (criticalParent) delete[] criticalParent;
    if (carvedRegions)
      {
	for (i=0; i<numCarvedRegions; i++) delete carvedRegions[i];
	delete[] carvedRegions;
      }
    innerForLater->clear();
    delete innerForLater;
    outerForLater->clear();
    delete outerForLater;
    if (noTopoCheck) delete noTopoCheck;
    if (strictTopoCheck) delete strictTopoCheck;
    if (sliceTopoCheck) delete sliceTopoCheck;
    if (comboTopoCheck) delete comboTopoCheck;
    if (initialOrder) delete[] initialOrder;
    if (carvedInsideOrder) delete[] carvedInsideOrder;
    if (carvedSlicesOrder) delete[] carvedSlicesOrder;
    for (i=0; i<numNeighborsVertex; i++) delete[] neighborhoodVertex[i];
    delete[] neighborhoodVertex;
    for (i=0; i<numNeighborsFace; i++) delete[] neighborhoodFace[i];
    delete[] neighborhoodFace;
}

void volume::setUpNeighborhoods()
{
    int i,j,x,y,z;
    numNeighborsVertex=26;
    neighborhoodVertex=new int*[numNeighborsVertex];
    for (i=0; i<numNeighborsVertex; i++)
    {
	neighborhoodVertex[i]=new int[3];
    }
    i=0;
    for (z=-1; z<=1; z++) for (y=-1; y<=1; y++) for (x=-1; x<=1; x++) if ((z!=0) || (y!=0) || (x!=0))
    {
	neighborhoodVertex[i][0]=x;
	neighborhoodVertex[i][1]=y;
	neighborhoodVertex[i][2]=z;
	i++;
    }
    numNeighborsFace=6;
    neighborhoodFace=new int*[numNeighborsFace];
    for (i=0; i<numNeighborsFace; i++)
    {
	neighborhoodFace[i]=new int[3];
	for (j=0; j<3; j++) neighborhoodFace[i][j]=0;
    }
    neighborhoodFace[0][2]=-1;
    neighborhoodFace[1][1]=-1;
    neighborhoodFace[2][0]=-1;
    neighborhoodFace[3][0]=1;
    neighborhoodFace[4][1]=1;
    neighborhoodFace[5][2]=1;
}

int* volume::getSize()
{
    return size;
}

int volume::getNumVoxels()
{
  return size[2]*size[1]*size[0];
}

void volume::setFeatureSize(unsigned short p_featureSize)
{
    featureSize=p_featureSize;
}

int volume::getVoxelIndex(int x,int y,int z)
{
    return (z*size[1]+y)*size[0]+x;
}

void volume::getVoxelLocFromIndex(int index,int* x,int* y,int* z)
{
  *x=index%size[0];
  *y=(index/size[0])%size[1];
  *z=(index/size[0])/size[1];
}

//does bounds checking and subtracts isovalue
float volume::d(int x,int y,int z)
{
  if ((x<0) || (y<0) || (z<0) || (x>=size[0]) || (y>=size[1]) || (z>=size[2])) return BIGNUM;
  return data[getVoxelIndex(x,y,z)]-isovalue;
}

unsigned short* volume::getData()
{
  return data;
}

unsigned short volume::getVoxel(int index)
{
  return data[index];
}

unsigned short volume::getVoxel(int x,int y,int z)
{
  return data[getVoxelIndex(x,y,z)];
}

void volume::setVoxel(int x,int y,int z,unsigned short val)
{
  data[getVoxelIndex(x,y,z)]=val;
}

int volume::voxelKnown(int index)
{
  //if (known.hasdata()) return known.getbit(index);
  return 1;
}

int volume::voxelCarved(int index)
{
  return carved.getbit(index);
}

void volume::setVoxelCarved(int index)
{
    carved.setbit(index);
}

void volume::setVoxelCarved(int index,char label)
{
    setVoxelCarved(index);
}

void volume::clearVoxelCarved(int index)
{
    carved.resetbit(index);
}

void volume::clearAllVoxelsCarved()
{
    carved.setbit(size[2]*size[1]*size[0]-1);
    for (int index=0; index<size[2]*size[1]*size[0]; index++) carved.resetbit(index);
}

void volume::clearVoxelsCarvedInRange(int xmin,int xmax,int ymin,int ymax,int zmin,int zmax)
{
    int x,y,z;
    for (z=zmin; z<=zmax; z++) for (y=ymin; y<=ymax; y++) for (x=xmin; x<=xmax; x++)
    {
	carved.resetbit(getVoxelIndex(x,y,z));
    }
}

int volume::voxelQueued(int index)
{
  return queued.getbit(index);
}

void volume::setVoxelQueued(int index)
{
  queued.setbit(index);
}

void volume::clearVoxelQueued(int index)
{
  queued.resetbit(index);
}

void volume::clearAllVoxelsQueued()
{
    queued.setbit(size[2]*size[1]*size[0]-1);
    for (int index=0; index<size[2]*size[1]*size[0]; index++) queued.resetbit(index);
}

void volume::setQueuedToUnionOfQueuedAndCarved()
{
    for (int index=0; index<size[2]*size[1]*size[0]; index++)
	if (voxelCarved(index)) setVoxelQueued(index);
}

void volume::setQueuedToUnionOfQueuedAndCarvedInRange(int xmin,int xmax,int ymin,int ymax,int zmin,int zmax)
{
    int x,y,z;
    for (z=zmin; z<=zmax; z++) for (y=ymin; y<=ymax; y++) for (x=xmin; x<=xmax; x++)
    {
	if (voxelCarved(getVoxelIndex(x,y,z))) setVoxelQueued(getVoxelIndex(x,y,z));
    }
}

void volume::setCarvedToUnionOfQueuedAndCarved()
{
    for (int index=0; index<size[2]*size[1]*size[0]; index++)
	if (voxelQueued(index)) setVoxelCarved(index);
}

int volume::getVoxelOrder(int index,int* order)
{
    if (order==NULL) order=getDefaultOrder();
    return order[index];
}

int volume::getCriticalParent(int index)
{
    return criticalParent[index];
}

int* volume::getCriticalParents()
{
    return criticalParent;
}

int volume::traceToCriticalAncestor(int index)
{
    if (criticalParent[index] != -1) return criticalParent[index];
    
    int x,y,z,i,ox,oy,oz;
    int neighborIndex,minNeighbor=index;
    
    getVoxelLocFromIndex(index,&x,&y,&z);
    for (i=0; i<numNeighborsVertex; i++)
    {
	ox=neighborhoodVertex[i][0]; oy=neighborhoodVertex[i][1]; oz=neighborhoodVertex[i][2]; 
	if ((z+oz>=0) && (z+oz<size[2]) &&
	    (y+oy>=0) && (y+oy<size[1]) &&
	    (x+ox>=0) && (x+ox<size[0]))
	{
	    neighborIndex=getVoxelIndex(x+ox,y+oy,z+oz);
	    if ((data[neighborIndex]<data[minNeighbor]) ||
		((data[neighborIndex]==data[minNeighbor]) && (neighborIndex<minNeighbor)))
		minNeighbor=neighborIndex;
	}
    }
    assert(minNeighbor!=index);
    
    return criticalParent[index]=traceToCriticalAncestor(minNeighbor);
}

int volume::getMinNeighbor(int index,int* order)
{
    return getMinNeighborInRange(index,0,size[0]-1,0,size[1]-1,0,size[2]-1,order);
}

int volume::getMinNeighborInRange(int index,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int* order)
{
    if (!order) order=getDefaultOrder();
    int x,y,z,i,ox,oy,oz;
    int neighborIndex,minNeighbor;
    
    minNeighbor=index;
    getVoxelLocFromIndex(index,&x,&y,&z);
    for (i=0; i<numNeighborsVertex; i++)
    {
	ox=neighborhoodVertex[i][0]; oy=neighborhoodVertex[i][1]; oz=neighborhoodVertex[i][2]; 
	if ((z+oz>=zmin) && (z+oz<=zmax) &&
	    (y+oy>=ymin) && (y+oy<=ymax) &&
	    (x+ox>=xmin) && (x+ox<=xmax))
	{
	    neighborIndex=getVoxelIndex(x+ox,y+oy,z+oz);
	    if (order[neighborIndex]<order[minNeighbor])
		minNeighbor=neighborIndex;
	}
    }
    return minNeighbor;
    
}

int volume::traceToLocalMinInRange(int index,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int* order)
{
    if (!order) order=getDefaultOrder();
    
    int minNeighbor=index;
    
    do
    {
	index=minNeighbor;
	minNeighbor=getMinNeighborInRange(index,xmin,xmax,ymin,ymax,zmin,zmax,order);
    } while (minNeighbor!=index);
    
    return index;
}

void volume::calcCricitalParentsByTracing(ContourTree& jt)
{
    int index;
    for (index=0; index<size[2]*size[1]*size[0]; index++) criticalParent[index]=-1;
    
    //store critical parent info from tree
    jt.setCriticalParentsJT(criticalParent);
    
    //critical nodes not in tree have themselves as critical parent
    
    for (index=0; index<size[2]*size[1]*size[0]; index++)
    {
	traceToCriticalAncestor(index);
	//fix critical parent to be highest ancestor below voxel value
    }
}

int volume::findAndUpdateCriticalAncestor(int index)
{
    if (criticalParent[index]==-1) return -1;
    if (criticalParent[index]==index) return index;
    return criticalParent[index]=findAndUpdateCriticalAncestor(criticalParent[index]);
}

int volume::findCriticalAncestor(int index)
{
    int anc=index;
    while ((anc!=-1) && (anc!=criticalParent[anc])) anc=criticalParent[anc];
    return anc;
}

int volume::findCriticalAncestorOfNeighbor(int index,int numNeighbors,int** neighborhood)
{
    int x,y,z,i,ox,oy,oz;
    int na,nindex,np;
    getVoxelLocFromIndex(index,&x,&y,&z);
    for (i=0; i<numNeighbors; i++)
    {
	ox=neighborhood[i][0]; oy=neighborhood[i][1]; oz=neighborhood[i][2]; 
	if ((z+oz>=0) && (z+oz<size[2]) &&
	    (y+oy>=0) && (y+oy<size[1]) &&
	    (x+ox>=0) && (x+ox<size[0]))
	    nindex=getVoxelIndex(x+ox,y+oy,z+oz);
	else
	    nindex=size[0]*size[1]*size[2];
	np=criticalParent[nindex];
	if (np != -1) 
	{
	    na=findAndUpdateCriticalAncestor(np);
	    if (na != -1) return na;
	}
    }
    return -1;
}

void volume::joinOrSplitCriticalPoints(int index,int numNeighbors,int** neighborhood,ContourTree* ct,int* numChildrenRef,int* children)
{
    int questionVoxel1=-1,questionVoxel2=-1,questionVoxel3=-1;
    int x,y,z,i,ox,oy,oz;
    int na,nindex,np;
    int numDistinctComps=0;
    int numChildren=0;
    getVoxelLocFromIndex(index,&x,&y,&z);
    for (i=0; i<numNeighbors; i++)
    {
	ox=neighborhood[i][0]; oy=neighborhood[i][1]; oz=neighborhood[i][2]; 
	if ((z+oz>=0) && (z+oz<size[2]) &&
	    (y+oy>=0) && (y+oy<size[1]) &&
	    (x+ox>=0) && (x+ox<size[0]))
	    nindex=getVoxelIndex(x+ox,y+oy,z+oz);
	else
	    nindex=size[0]*size[1]*size[2];
	np=criticalParent[nindex];
	if (np != -1) 
	{
	    na=findAndUpdateCriticalAncestor(np);
	    //na=findAndUpdateCriticalAncestor(nindex);
	    
	    if ((na != -1) && (na != index))
	    {
		if (criticalParent[na]==na)
		{
		    //neighbor's ancestor is a critical point that needs to be joined
		    if (ct) children[numChildren++]=na;
		}
		criticalParent[na]=index;
		numDistinctComps++;
	    }
	}
    }
    *numChildrenRef=numChildren;
    //Now if numDistinctComps==1, we know this critical point changed the number of handles (by 1 or more),
    //and if numDistinctComps>=2, we know this critical point decreased the number of components,
    // but we can't tell if the number of handles changed.
}

void volume::joinCriticalPoints(int index,int numNeighbors,int** neighborhood,ContourTree* ct)
{
    int numChildren=0;
    int* children=NULL;
    if (ct) children=new int[numNeighbors];
    joinOrSplitCriticalPoints(index,numNeighbors,neighborhood,ct,&numChildren,children);
    if (ct) ct->addNewJoinNode(index,children,numChildren);
    if (ct) delete[] children;
}

void volume::splitCriticalPoints(int index,int numNeighbors,int** neighborhood,ContourTree* ct)
{
    int numChildren=0;
    int* children=NULL;
    if (ct) children=new int[numNeighbors];
    joinOrSplitCriticalPoints(index,numNeighbors,neighborhood,ct,&numChildren,children);
    if (ct) ct->addNewSplitNode(index,children,numChildren);
    if (ct) delete[] children;
}

void volume::readTopoinfoFiles()
{
  noTopoCheck=new TopoCheckNever();
  strictTopoCheck=new TopoCheckStrict();
  sliceTopoCheck=new TopoCheck2DVertex();
  comboTopoCheck=new TopoCheck2D3DCombo();
}

//Takes a guess at whether all the signs need to be flipped
int volume::dataShouldBeNegated()
{
  unsigned short min=65535,max=0;
  float borderAverage=0.0;
  int i;
  int x,y,z;
  for (i=0; i<size[2]*size[1]*size[0]; i++)
    {
      if (data[i]<min) min=data[i];
      if (data[i]>max) max=data[i];
    }
  for (y=0; y<size[1]; y++) for (x=0; x<size[0]; x++)
    {
      borderAverage+=data[getVoxelIndex(x,y,0)];
      borderAverage+=data[getVoxelIndex(x,y,size[2]-1)];
    }
  for (z=0; z<size[2]; z++) for (y=0; y<size[1]; y++)
    {
      borderAverage+=data[getVoxelIndex(0,y,z)];
      borderAverage+=data[getVoxelIndex(size[0]-1,y,z)];
    }
  for (z=0; z<size[2]; z++) for (x=0; x<size[0]; x++)
    {
      borderAverage+=data[getVoxelIndex(x,0,z)];
      borderAverage+=data[getVoxelIndex(x,size[1]-1,z)];
    }
  borderAverage/=2*(size[1]*size[0]+size[2]*size[1]+size[2]*size[0]);
  cout << "Border stats: " << min << ',' << borderAverage << ',' << max << '\n';

  /*int* checks=new int[size[2]*size[1]*size[0]];
  for (i=0; i<size[2]*size[1]*size[0]; i++) checks[i]=0;
  for (i=0; i<size[2]*size[1]*size[0]; i++) checks[data[i]]=1;
  int allChecked=1;
  for (i=0; i<size[2]*size[1]*size[0]; i++)
    if (checks[i]==0) 
      {
	//cout << "Missed check #" << i << '\n';
	allChecked=0;
      }
  if (allChecked) cout << "Every value in range 0 to " << size[2]*size[1]*size[0]-1 << " is taken on.\n";
  delete[] checks;*/

  return (borderAverage-min < max-borderAverage);
}

//Swaps the sign on every voxel in the volume
//Rotates around midway through max and min values
void volume::changeAllSigns()
{
    cout << "Negating data.\n";
    unsigned short min=65535,max=0;
    for (int i=0; i<size[2]*size[1]*size[0]; i++)
    {
	if (data[i]<min) min=data[i];
	if (data[i]>max) max=data[i];
    }
    for (int i=0; i<size[2]*size[1]*size[0]; i++)
	data[i]=max+min-data[i];
}

//Returns true if the voxel at the given coordinates has a neighbor of known opposite sign
int volume::voxelOnIsosurface(int x,int y,int z)
{
  int index=getVoxelIndex(x,y,z);
  if (!voxelKnown(index)) return 0;
  int s=SGN(data[index]-isovalue);
  if (s==0) return 1;
  if ((s==-1) && ((x-1<0) || (y-1<0) || (z-1<0) || 
		  (x+1>=size[0]) || (y+1>=size[1]) || (z+1>=size[2])))
    return 1;
  int neighbors[27];
  int numNeighbors=getNeighbors26(x,y,z,neighbors);
  for (int i=0; i<numNeighbors; i++)
    if ((voxelKnown(neighbors[i])) && (SGN(data[neighbors[i]]-isovalue) != s)) return 1;
  return 0;
}

//Constructs the starting outer boundary for the given level
void volume::constructInitialOuterBoundary(int component)
{
  /*  int x,y,z,index;
  for (z=0; z<1; z++)
    for (y=0; y<size[1]; y++)
      for (x=0; x<size[0]; x++)
	if (d(x,y,z)>0)
	  carvedRegions[component]->addToBoundary(index);
  for (z=size[2]-1; z<size[2]; z++)
    for (y=0; y<size[1]; y++)
      for (x=0; x<size[0]; x++)
	if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
	  addToBoundary(x,y,z,index,outerBoundary);
  for (z=1; z<size[2]-1; z++)
    for (y=0; y<1; y++)
      for (x=1; x<size[0]; x++)
	if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
	  addToBoundary(x,y,z,index,outerBoundary);
  for (z=1; z<size[2]-1; z++)
    for (y=size[1]-1; y<size[1]; y++)
      for (x=0; x<size[0]-1; x++)
	if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
	  addToBoundary(x,y,z,index,outerBoundary);
  for (z=1; z<size[2]-1; z++)
    for (y=0; y<size[1]-1; y++)
      for (x=0; x<1; x++)
	if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
	  addToBoundary(x,y,z,index,outerBoundary);
  for (z=1; z<size[2]-1; z++)
    for (y=1; y<size[1]; y++)
      for (x=size[0]-1; x<size[0]; x++)
	if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
	addToBoundary(x,y,z,index,outerBoundary);*/
}

//Puts all the voxels into a priority queue
void volume::constructInitialInnerBoundary(int component)
{
  int index;
  int numVoxels=getNumVoxels();
  int voxelIndex;
  int neighbors[27];
  int i,n;
  int localmin;
  
  cout << "Constructing initial boundary..."; cout.flush();
  for (index=0; index<numVoxels; index++)
    {
      setVoxelQueued(index);
      carvedRegions[component]->addToBoundary(pqitem(index,numVoxels+1,data[index]));
    }
  cout << "done.\n";
}

//Returns the number of in-range neighbors around the given voxel.
// The neighbors are stored in neighbors.
int volume::getNeighbors26(int x,int y,int z,int* neighbors)
{
  int n=0;
  int ox,oy,oz;
  for (oz=-1; oz<=1; oz++) if ((z+oz>=0) && (z+oz<size[2]))
    for (oy=-1; oy<=1; oy++) if ((y+oy>=0) && (y+oy<size[1]))
      for (ox=-1; ox<=1; ox++) if ((x+ox>=0) && (x+ox<size[0]))
	if ((oz!=0) || (oy!=0) || (ox!=0))
	  neighbors[n++]=getVoxelIndex(x+ox,y+oy,z+oz);
  return n;
}

//Returns the number of non-null, in-range neighbors around the given voxel.
// The neighbors are stored in neighbors.
int volume::getNeighbors124(int x,int y,int z,int* neighbors)
{
  int n=0;
  int ox,oy,oz;
  for (oz=-2; oz<=2; oz++) if ((z+oz>=0) && (z+oz<size[2]))
    for (oy=-2; oy<=2; oy++) if ((y+oy>=0) && (y+oy<size[1]))
      for (ox=-2; ox<=2; ox++) if ((x+ox>=0) && (x+ox<size[0]))
	if ((oz!=0) || (oy!=0) || (ox!=0))
	  neighbors[n++]=getVoxelIndex(x+ox,y+oy,z+oz);
  return n;
}

//Returns the number of non-null, in-range neighbors around the given voxel.
// The neighbors are stored in neighbors.
int volume::getNeighbors6(int x,int y,int z,int* neighbors)
{
  int n=0;
  if (x-1>=0)      neighbors[n++]=getVoxelIndex(x-1,y,z);
  if (x<size[0]-1) neighbors[n++]=getVoxelIndex(x+1,y,z);
  if (y-1>=0)      neighbors[n++]=getVoxelIndex(x,y-1,z);
  if (y<size[1]-1) neighbors[n++]=getVoxelIndex(x,y+1,z);
  if (z-1>=0)      neighbors[n++]=getVoxelIndex(x,y,z-1);
  if (z<size[2]-1) neighbors[n++]=getVoxelIndex(x,y,z+1);
  return n;
}

//Inverts a permutation so that we can use the result of sort the way we want,
// an index into the sorted order.
void volume::invertPermutation(int* order)
{
  int i,n=size[2]*size[1]*size[0];
  int* temp=new int[n];
  for (i=0; i<n; i++) temp[order[i]]=i;
  for (i=0; i<n; i++) order[i]=temp[i];
  delete[] temp;
}

//Uses stdlib quicksort to sort the voxels.
void volume::sortVoxels()
{
  if (!initialOrder) initialOrder=new int[size[0]*size[1]*size[2]];
  for (int i=0; i<size[0]*size[1]*size[2]; i++)
    {
      initialOrder[i]=i;
    }
  voxelComparator vc(this);

  //sort voxels
  cout << "Sorting voxels..."; cout.flush();
  sort(initialOrder,initialOrder+getNumVoxels(),vc);
  invertPermutation(initialOrder);
  cout << "done.\n";
}

void volume::addNeighborsToQueue(pqitem voxel,int component)
{
    int x,y,z,ox,oy,oz;
    for (int i=0; i<numNeighborsVertex; i++)
    {
	getVoxelLocFromIndex(voxel.index,&x,&y,&z);
	ox=neighborhoodVertex[i][0]; oy=neighborhoodVertex[i][1]; oz=neighborhoodVertex[i][2]; 
	if ((z+oz>=0) && (z+oz<size[2]) &&
	    (y+oy>=0) && (y+oy<size[1]) &&
	    (x+ox>=0) && (x+ox<size[0]))
	{
	    int neighborIndex=getVoxelIndex(x+ox,y+oy,z+oz);
	    if ((!voxelCarved(neighborIndex)) && (!voxelQueued(neighborIndex)))
	    {
		//This must be a delayed voxel, because we added all the voxels to the queue to begin with.
		//We set its queued value to the current carving value, voxel.value.
		pqitem neighbor(neighborIndex,carvedRegions[component]->getNumCarved(),voxel.value);
		carvedRegions[component]->addToBoundary(neighbor);
		setVoxelQueued(neighborIndex);
	    }
	}
    }
}

//Carves without changing topology.
void volume::carveSimultaneously()
{
  int lastNumCarved=-1;
  int questionVoxel=-1;
  cout << "Carving"; cout.flush();
  
  //find first voxel to carve
  int maxPriority=-131072;
  int maxi=-1;
  //  pqitem maxVoxel(-1,0,-1,1,getNumVoxels());
  for (int i=0; i<numCarvedRegions; i++)
    {
      int p=carvedRegions[i]->peekNextVoxel();
      if (p>maxPriority)
	{
	  maxPriority=p;
	  maxi=i;
	}
    }
  while (maxi>-1)
    {
      if ((carvedRegions[maxi]->getNumCarved() % 5000 == 0) && (carvedRegions[maxi]->getNumCarved() != lastNumCarved))
	{
	  //cout << "." << carvedRegions[maxi]->getNumCarved(); cout.flush();
	  cout << "."; cout.flush();
	  lastNumCarved=carvedRegions[maxi]->getNumCarved();
	}
      if (carvedRegions[maxi]->getNumCarved() > 5000)
	{
	  if (carvedRegions[maxi]->getNumCarved() % 100 == 0)
	    {
	      //cout << "." << carvedRegions[maxi]->getNumCarved(); cout.flush();
	    }
	}
      //if (carvedRegions[maxi]->getNumCarved() >= 9781) cout << carvedRegions[maxi]->getNumCarved() << ":";

      //pop voxel from top of appropriate priority queue
      pqitem voxel=carvedRegions[maxi]->getNextVoxel();
      //if (carvedRegions[maxi]->getNumCarved() >= 500000) cout << "v(" << voxel.index << "," << voxel.value << "," << data[voxel.index] << ")";
      if (voxel.index==questionVoxel) cout << "v(" << voxel.index << "," << voxel.value << "," << data[voxel.index] << ")";
      if (voxel.index==questionVoxel) cout << "(" << carvedRegions[maxi]->getNumCarved() << ")";
      
      if (!voxelCarved(voxel.index))
      {
	  //check whether removing this voxel changes the topology
	  if (!carvedRegions[maxi]->topologyCheck(voxel.index))
	  {
	      if (voxel.index==questionVoxel) cout << "a";
	      //If it doesn't, carve this voxel.
	      carvedRegions[maxi]->carveVoxel(voxel);
	      //Update the voxel value to reflect carving order.
	      data[voxel.index]=MAX(MIN(voxel.value,65535),0);
	      //Add neighbors to the queue
	      addNeighborsToQueue(voxel,maxi);
	      //Update contour tree
	      if (criticalParent) 
	      {
		  //Record for contour tree (join tree) as a regular point.
		  criticalParent[voxel.index]=findCriticalAncestorOfNeighbor(voxel.index,numNeighborsVertex,neighborhoodVertex);
	      }
	      //if (maxi==1) if (carvedRegions[maxi]->getNumCarved()%1000==0) renderVolume(1);
	  }
	  else
	  {
	      //If it does, add it back to the queue with adjusted priority,
	      // unless this is a persistent topology change,
	      // in which case we carve this voxel anyway.
	      if ((ABS(voxel.value-data[voxel.index])>=featureSize) && 
		  (voxel.subdelay==getNumVoxels()+1))
	      {
		  if (voxel.index==questionVoxel) cout << "b";
		  //Carve the voxel anyway.
		  carvedRegions[maxi]->carveVoxel(voxel);
		  //Update the voxel value to reflect carving order.
		  data[voxel.index]=MAX(MIN(voxel.value,65535),0);
		  //Add neighbors to the queue
		  addNeighborsToQueue(voxel,maxi);
		  //Update contour tree
		  if (criticalParent) 
		  {
		      //Add to contour tree as a critical point.
		      criticalParent[voxel.index]=voxel.index;
		      //If this is a join, update critical points joining here.
		      joinCriticalPoints(voxel.index,numNeighborsVertex,neighborhoodVertex);
		  }
	      }
	      else
	      {
		  if (voxel.index==questionVoxel) cout << "c";
		  //Add back to the queue with delayed priority.
		  carvedRegions[maxi]->addDelayedToBoundary(voxel);
	      }
	  }
      }
      
      //find next voxel to carve
      maxPriority=-131072;
      maxi=-1;
      for (int i=0; i<numCarvedRegions; i++)
	{
	  int p=carvedRegions[i]->peekNextVoxel();
	  if (p>maxPriority)
	    {
	      maxPriority=p;
	      maxi=i;
	    }
	}
    }
  
  cout << "done.\n";
  for (int i=0; i<numCarvedRegions; i++)
    {
      cout << " " << carvedRegions[i]->getNumCarved() << " carved from component " << i << ".\n";
    }
}

void volume::fixVolumeByComponent(int comp)
{
  //Any voxels that are past the isovalue but not carved are set to the isovalue. ???
  int x,y,z;
  int voxelIndex;
  int numChanged=0;
  
  cout << "Fixing volume..."; cout.flush();
  for (z=0; z<size[2]; z++)
    for (y=0; y<size[1]; y++)
      for (x=0; x<size[0]; x++)
	{
	  voxelIndex=getVoxelIndex(x,y,z);
	  if (voxelCarved(voxelIndex) == carvedRegions[1]->getLabel())
	    {
	      numChanged++;
	      data[voxelIndex]=4000;
	    }
	  else if (voxelCarved(voxelIndex) == carvedRegions[0]->getLabel())
	    {
	      numChanged++;
	      data[voxelIndex]=0;
	    }
	  else
	    {
	      data[voxelIndex]+=100;
	    }
	}
 
  //cout << " " << numChanged << " voxels changed to " << carvedRegions[comp]->valueWithinIsovalue() << ".\n";
}

int volume::fixTopologyStrict(int useUnionFind)
{
  //set up arrays to hold the carving order
  if (!carvedInsideOrder) carvedInsideOrder=new int[size[0]*size[1]*size[2]];
  for (int i=0; i<size[0]*size[1]*size[2]; i++) carvedInsideOrder[i]=-1;
  if (useUnionFind)
  {
      if (!criticalParent) criticalParent=new int[size[0]*size[1]*size[2]];
      for (int i=0; i<size[0]*size[1]*size[2]; i++) criticalParent[i]=-1;
  }

  //set up carve component boundaries
  if (carvedRegions) 
    {
      for (int i=0; i<numCarvedRegions; i++) delete carvedRegions[i];
      delete[] carvedRegions;
    }
  numCarvedRegions=1;
  carvedRegions=new carvecomp*[numCarvedRegions];
  carvedRegions[0]=new carvecomp(1,1,BIGNUM,getTopoCheck(),this,carvedInsideOrder);
  constructInitialInnerBoundary(0);

  //carve the first voxel (global min)
  pqitem voxel=carvedRegions[0]->getNextVoxel();
  carvedRegions[0]->carveVoxel(voxel);
  if (criticalParent) criticalParent[voxel.index]=voxel.index;

  //carve simultaneously
  sortVoxels();
  //countCriticals(initialOrder);
  //printVolume(cout,initialOrder);
  carveSimultaneously();
  
  //build contour tree
  ContourTree jt,st;
  cout << "Join tree\n";
  buildJoinTreeForInsideOrder(carvedInsideOrder,jt);
  jt.printJT(cout,1);
  cout << "\n";
  cout << "Split tree\n";
  buildSplitTreeForInsideOrder(carvedInsideOrder,st);
  st.printST(cout,1);
  cout << "\n";
  cout << "Contour tree\n";
  ContourTree ct(jt,st);
  ct.simplify();
  ct.print(cout);
  cout << "\n";
  
  return countCriticals(carvedInsideOrder);
}

int volume::readCarvedOrder(char* filename)
{
    cout << "Reading carved order info from file " << filename << "..."; cout.flush();
    if (!carvedInsideOrder) carvedInsideOrder=new int[size[0]*size[1]*size[2]];
    int result;
    result=loadOrder(filename,carvedInsideOrder);
    if (result)
    {
	cout << "error!\n";
	delete[] carvedInsideOrder;
	carvedInsideOrder=NULL;
    }
    cout << "done.\n";
    /*//Why is the carving order backwards???
    for (int i=0; i<getNumVoxels()/2; i++)
    {
	int temp=carvedInsideOrder[i];
	carvedInsideOrder[i]=carvedInsideOrder[getNumVoxels()-1-i];
	carvedInsideOrder[getNumVoxels()-1-i]=temp;
    }*/
    return result;
}

int* volume::getDefaultOrder()
{
    if (carvedInsideOrder) return carvedInsideOrder;
    if (carvedSlicesOrder) return carvedSlicesOrder;
    else 
    {
	if (!initialOrder) sortVoxels();
	return initialOrder;
    }
}

int volume::findLastVoxelBelowIsovalue(int isovalue,int* order)
{
    if (!order) order=getDefaultOrder();
    int index;
    int last=-1;
    int max=-1;
    for (index=0; index<getNumVoxels(); index++)
    {
	if ((max<=order[index]) && (data[index]<=isovalue)) 
	{
	    //cout << index << "[" << order[index] << "]" << "(" << data[index] << ")\t";
	    last=index;
	    max=order[index];
	}
    }
    return last;
}

TopoCheck* volume::getTopoCheck()
{
    return strictTopoCheck;
}

int volume::getTopoType(int index,int* order)
{
    return getTopoTypeInRange(index,order,0,size[0]-1,0,size[1]-1,0,size[2]-1);
}

int volume::getTopoTypeInRange(int index,int* order,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax)
{
    int topoType=0,bit=0;
    int x,y,z,ox,oy,oz;
    int neighborIndex;
    
    getVoxelLocFromIndex(index,&x,&y,&z);
    for (oz=-1; oz<=1; oz++)
	for (oy=-1; oy<=1; oy++)
	    for (ox=-1; ox<=1; ox++)
		if ((oz!=0) || (oy!=0) || (ox!=0))
		{
		    if ((z+oz>=zmin) && (z+oz<=zmax) &&
			(y+oy>=ymin) && (y+oy<=ymax) &&
			(x+ox>=xmin) && (x+ox<=xmax))
		    {
			neighborIndex=getVoxelIndex(x+ox,y+oy,z+oz);
			//If the neighbor is before the current voxel, we set the bit.
			if (order[neighborIndex]<order[index])
			    topoType |= (1<<bit);
		    }
		    else
		    {
			//Out of range is considered large positive, so we do not set the bit, because we're increasing.
			//topoType |= (1<<bit);
		    }
		    bit++;
		}
    return topoType;
}

int volume::voxelCritical(int index,int* order)
{
  if (!order) order=getDefaultOrder();
  if (getTopoCheck()->topologyChange(getTopoType(index,order))) return 1;
  else return 0;
}

int volume::countCriticals(int* order)
{
  int index;
  int numCritical=0;
  
  cout << "Counting critical points..."; cout.flush();
  for (index=0; index<size[2]*size[1]*size[0]; index++)
    {
      if (voxelCritical(index,order))
	{
	  numCritical++;
	  /*int x,y,z;
	  getVoxelLocFromIndex(index,&x,&y,&z);
	  cout << "Voxel " << index << " (" << x << "," << y << "," << z << ")\n";
	  if ((z-1>=0) && (z+1<size[2]) &&
	      (y-1>=0) && (y+1<size[1]) &&
	      (x-1>=0) && (x+1<size[0]))
	      printRegion(cout,x-1,y-1,z-1,3,3,3);*/
	}
    }
  cout << "done: " << numCritical << "\n";
  return numCritical;
}

int volume::countInsideCriticals()
{
    return countCriticals(carvedInsideOrder);
}

int fixForVertexConnectivity(int neighborhood)
{
    //faces set incident edges
    if (BIT( 4,neighborhood)) neighborhood |= 0x00000aa; //(1<<1)|(1<<3)|(1<<5)|(1<<7)
    if (BIT(10,neighborhood)) neighborhood |= 0x0040a02; //(1<<1)|(1<<9)|(1<<11)|(1<<18)
    if (BIT(12,neighborhood)) neighborhood |= 0x0104208; //(1<<3)|(1<<9)|(1<<14)|(1<<20)
    if (BIT(13,neighborhood)) neighborhood |= 0x0410820; //(1<<5)|(1<<11)|(1<<16)|(1<<22)
    if (BIT(15,neighborhood)) neighborhood |= 0x1014080; //(1<<7)|(1<<14)|(1<<16)|(1<<24)
    if (BIT(21,neighborhood)) neighborhood |= 0x1540000; //(1<<18)|(1<<20)|(1<<22)|(1<<24)
    
    //edges set incident vertices
    if (BIT( 1,neighborhood)) neighborhood |= 0x0000005; //(1<<0)|(1<<2)
    if (BIT( 3,neighborhood)) neighborhood |= 0x0000041; //(1<<0)|(1<<6)
    if (BIT( 5,neighborhood)) neighborhood |= 0x0000104; //(1<<2)|(1<<8)
    if (BIT( 7,neighborhood)) neighborhood |= 0x0000140; //(1<<6)|(1<<8)
    if (BIT( 9,neighborhood)) neighborhood |= 0x0020001; //(1<<0)|(1<<17)
    if (BIT(11,neighborhood)) neighborhood |= 0x0080004; //(1<<2)|(1<<19)
    if (BIT(14,neighborhood)) neighborhood |= 0x0800040; //(1<<6)|(1<<23)
    if (BIT(16,neighborhood)) neighborhood |= 0x2000100; //(1<<8)|(1<<25)
    if (BIT(18,neighborhood)) neighborhood |= 0x00a0000; //(1<<17)|(1<<19)
    if (BIT(20,neighborhood)) neighborhood |= 0x0820000; //(1<<17)|(1<<23)
    if (BIT(22,neighborhood)) neighborhood |= 0x2080000; //(1<<19)|(1<<25)
    if (BIT(24,neighborhood)) neighborhood |= 0x2800000; //(1<<23)|(1<<25)
    
    return neighborhood;
}

int countEuler(int neighborhood)
{
    int chi=0;
    
    //faces: bits 4,10,12,13,15,21
    if (BIT( 4,neighborhood)) chi++;
    if (BIT(10,neighborhood)) chi++;
    if (BIT(12,neighborhood)) chi++;
    if (BIT(13,neighborhood)) chi++;
    if (BIT(15,neighborhood)) chi++;
    if (BIT(21,neighborhood)) chi++;
    
    //edges: bits 1,3,5,7, 9,11,14,16, 18,20,22,24
    if (BIT( 1,neighborhood)) chi--;
    if (BIT( 3,neighborhood)) chi--;
    if (BIT( 5,neighborhood)) chi--;
    if (BIT( 7,neighborhood)) chi--;
    if (BIT( 9,neighborhood)) chi--;
    if (BIT(11,neighborhood)) chi--;
    if (BIT(14,neighborhood)) chi--;
    if (BIT(16,neighborhood)) chi--;
    if (BIT(18,neighborhood)) chi--;
    if (BIT(20,neighborhood)) chi--;
    if (BIT(22,neighborhood)) chi--;
    if (BIT(24,neighborhood)) chi--;
    
    //vertices: bits 0,2,6,8, 17,19,23,25
    if (BIT( 0,neighborhood)) chi++;
    if (BIT( 2,neighborhood)) chi++;
    if (BIT( 6,neighborhood)) chi++;
    if (BIT( 8,neighborhood)) chi++;
    if (BIT(17,neighborhood)) chi++;
    if (BIT(19,neighborhood)) chi++;
    if (BIT(23,neighborhood)) chi++;
    if (BIT(25,neighborhood)) chi++;
    
    return chi;
}

int volume::getBdEulerChBelowVoxelInRange(int index,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int* order)
{
    int topoType;
    if (!order) order=getDefaultOrder();
    topoType=fixForVertexConnectivity(getTopoTypeInRange(index,order,xmin,xmax,ymin,ymax,zmin,zmax));
    return countEuler(topoType);
}

int volume::getBdEulerChAboveVoxel(int index,int* order)
{
    int topoType;
    if (!order) order=getDefaultOrder();
    topoType=fixForVertexConnectivity(getTopoType(index,order));
    return countEuler((~topoType)&0x3ffffff);
}

//Constructs join tree from criticalParent data defined by carving process.
//Stores result in edgeList and prints result.
void volume::buildJoinTreeInside(int* edgeList)
{
    int index;
    int numCritical=0;
    
    ContourTree jt;
    for (index=0; index<size[2]*size[1]*size[0]; index++)
    {
	if (voxelCritical(index,carvedInsideOrder))
	{
	    if (edgeList)
	    {
		edgeList[2*numCritical]=index;
		edgeList[2*numCritical+1]=criticalParent[index];
	    }
	    jt.addJoinNode(index,criticalParent[index]);
	    numCritical++;
	}
    }
    jt.findJoinRoot();
    //jt.getUpperEdgeList(edgeList);
    jt.print(cout);
}

//Helper for getContourTree function
int volume::getArraysFromCT(ContourTree& ct,int* critList,int* numCrit,int* edgeList,int* edgeLabels,int simplified)
{
    numCrit[0]=ct.getPositiveCriticals(critList);
    numCrit[1]=ct.getNegativeCriticals(critList+numCrit[0]);
    numCrit[2]=ct.getNeutralCriticals(critList+numCrit[0]+numCrit[1]);
    numCrit[3]=ct.getComplexCriticals(critList+numCrit[0]+numCrit[1]+numCrit[2]);
    if (simplified) ct.simplify(); //doesn't handle edge labels correctly
    if (edgeList) ct.getUpperEdgeList(edgeList,edgeLabels);
    return ct.getNumNodes()-1;
}

//Constructs contour tree from carvedInsideOrder and stores result in given arrays.
int volume::getContourTree(int* critList,int* numCrit,int* edgeList,int simplified)
{
    ContourTree jt,st;
    buildJoinTreeForInsideOrder(carvedInsideOrder,jt);
    buildSplitTreeForInsideOrder(carvedInsideOrder,st);
    ContourTree ct(jt,st);
    return getArraysFromCT(ct,critList,numCrit,edgeList,NULL,simplified);
}

//Constructs join tree by processing data in order given. Result stored in jt.
void volume::buildJoinTreeForInsideOrder(int* order,ContourTree& jt)
{
    cout << "Constructing full join tree";
    int i,index;
    if (!criticalParent) criticalParent=new int[size[0]*size[1]*size[2]+1];
    for (i=0; i<size[0]*size[1]*size[2]; i++) criticalParent[i]=-1;
    criticalParent[i]=-1; //an extra point for the global max at infinity
    
    //set up processing order
    int* process=new int[size[0]*size[1]*size[2]];
    for (i=0; i<size[0]*size[1]*size[2]; i++) process[order[i]]=i;
    
    //Compute structure and build tree bottom-up
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	if (i%(size[0]*size[1])==0){ cout << "."; cout.flush(); }
	index=process[i];
	if (voxelCritical(index,order))
	{
	    criticalParent[index]=index;
	    joinCriticalPoints(index,numNeighborsVertex,neighborhoodVertex,&jt);
	}
	else
	{
	    criticalParent[index]=findCriticalAncestorOfNeighbor(index,numNeighborsVertex,neighborhoodVertex);
	}
    }
    
    //Finish tree
    index=size[2]*size[1]*size[0];
    std::vector<int> rootChildren;
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	if (criticalParent[i]==i)
	{
	    criticalParent[i]=index;
	    rootChildren.push_back(i);
	}
	assert(criticalParent[i]!=-1);
    }
    int* rootChildrenArray=new int[rootChildren.size()];
    int numRootChildren=0;
    for (std::vector<int>::iterator it=rootChildren.begin(); it!=rootChildren.end(); it++)
	rootChildrenArray[numRootChildren++]=*it;
    jt.addNewJoinNode(index,rootChildrenArray,rootChildren.size());
    delete[] rootChildrenArray;
    jt.findJoinRoot();
    
    //Fix critical parent array
    jt.setCriticalParentsJT(criticalParent);
    
    cout << "done: " << jt.getNumNodes() << " nodes. Join root is " << jt.getJoinRootIndex() << ".\n";
    
    delete[] process;
}

//Constructs split tree by processing data in order given. Result stored in st.
void volume::buildSplitTreeForInsideOrder(int* order,ContourTree& st)
{
    int i,index;
    if (criticalParent) delete[] criticalParent;
    criticalParent=new int[size[0]*size[1]*size[2]+1];
    for (i=0; i<size[0]*size[1]*size[2]; i++) criticalParent[i]=-1;
    criticalParent[i]=i; //an extra point for the global max at infinity
    
    //set up processing order
    int* process=new int[size[0]*size[1]*size[2]];
    for (i=0; i<size[0]*size[1]*size[2]; i++) process[order[i]]=i;
    
    //Compute structure bottom-up, leaves are at the end of the order
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	index=process[size[2]*size[1]*size[0]-1-i];
	if (voxelCritical(index,order))
	{
	    criticalParent[index]=index;
	    joinCriticalPoints(index,numNeighborsFace,neighborhoodFace);
	}
	else
	{
	    criticalParent[index]=findCriticalAncestorOfNeighbor(index,numNeighborsFace,neighborhoodFace);
	    assert(criticalParent[index]!=-1);
	    //if (criticalParent[index]==-1) criticalParent[index]=size[0]*size[1]*size[2];
	}
    }
    
    //Build tree top-down
    for (i=0; i<size[2]*size[1]*size[0]; i++)
    {
	index=process[i];
	if (voxelCritical(index,order))
	{
	    st.addNewSplitNode(index,criticalParent[index]);
	}
    }
    st.addNewSplitNode(i,criticalParent[i]);
    st.findSplitRoot();
    
    delete[] process;
}

void volume::buildFullJoinTree(int* order)
{
    if (!order) order=getDefaultOrder();
    fullJoinTree.clear();
    buildJoinTreeForInsideOrder(order,fullJoinTree);
}

void volume::labelFullJoinTree(int voxel)
{
    int* order=getDefaultOrder();
    fullJoinTree.calcJTIsovalueAncestors(findLastVoxelBelowIsovalue(isovalue,order),order);
}

ContourTree* volume::getFullJoinTree()
{
    return &fullJoinTree;
}

int volume::voxelBelowJTEdge(int v,int node)
{
    return (fullJoinTree.getNodeAncestor(criticalParent[v])==node);
}

int volume::markVoxelsBelowVoxelInRange(int v,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int* order)
{
    int questionVoxel=-1;
    if (!order) order=getDefaultOrder();
    //cout << v << "(" << data[v] << ")" << "[" << order[v] << "]\n";
    LCLAMP(xmin,0); GCLAMP(xmax,size[0]-1);
    LCLAMP(ymin,0); GCLAMP(ymax,size[1]-1);
    LCLAMP(zmin,0); GCLAMP(zmax,size[2]-1);
    int numMarked=1;
    int index,neighborIndex;
    int x,y,z,i,ox,oy,oz;
    std::queue<int> q;
    q.push(v);
    carved.setbit(v);
    while (!q.empty())
    {
	index=q.front();
	q.pop();
	if (index==questionVoxel)
	{
	    cout << "\tVoxel " << index << " popped when flooding from voxel " << v << "\n";
	}
	getVoxelLocFromIndex(index,&x,&y,&z);
	for (i=0; i<numNeighborsVertex; i++)
	{
	    ox=neighborhoodVertex[i][0]; oy=neighborhoodVertex[i][1]; oz=neighborhoodVertex[i][2]; 
	    if ((z+oz>=zmin) && (z+oz<=zmax) &&
		(y+oy>=ymin) && (y+oy<=ymax) &&
		(x+ox>=xmin) && (x+ox<=xmax))
	    {
		neighborIndex=getVoxelIndex(x+ox,y+oy,z+oz);
		if ((order[neighborIndex]<=order[v]) && (!carved.getbit(neighborIndex)))
		{
		    carved.setbit(neighborIndex);
		    numMarked++;
		    q.push(neighborIndex);
		}
	    }
	}
    }
    //cout << "(" << numMarked << " voxels marked) ";
    return numMarked;
}

int volume::markVoxelsBelowVoxel(int v,int* order)
{
    return markVoxelsBelowVoxelInRange(v,0,size[0]-1,0,size[1]-1,0,size[2]-1,order);
}

void volume::createHistogram()
{
    int hist[65536];
    int h,i;
    for (h=0; h<65536; h++) hist[h]=0;
    for (i=0; i<getNumVoxels(); i++) hist[data[i]]++;
    for (h=0; h<65536; h++) cout << hist[h] << '\n';
}

void volume::turnOnAnimation()
{
    animationOn=1;
}

void volume::turnOffAnimation()
{
    animationOn=0;
}

//Hook for rendering the volume at every step for animation purposes
void volume::renderVolume(int component)
{
  frameNumber++;
  
  int x,y,z,index,voxelIndex;
  int slicesize=size[0]*size[1];
  float* vdata=new float[slicesize];
  //cout << "Level " << level << ' ' << size[0] << 'x' << size[1] << 'x' << size[2] << '\n';
  
  int renderStyle=0;
  if (component>0) renderStyle=1;
  
  cout << "Rendering frame " << frameNumber << ""; cout.flush();
  char filename[20];
  sprintf(filename,"frames/frame_%.4d.v",frameNumber);
  ofstream fout(filename);
  
  if (fout)
    {
      fout.write((char*)size,sizeof(size));
      for (z=0; z<size[2]; z++)
        {
	  cout << '.'; cout.flush();
	  index=0;
	  for (y=0; y<size[1]; y++)
	    for (x=0; x<size[0]; x++)
	      {
		voxelIndex=getVoxelIndex(x,y,z);
		if (renderStyle==0)
		  {
		    if (voxelCarved(voxelIndex)>0)
		      vdata[index]=-1.0;
		    else
		      vdata[index]=1.0;
		  }
		else if (renderStyle==1)
		  {
		    if (voxelCarved(voxelIndex)==carvedRegions[component]->getLabel())
		      vdata[index]=-1.0;
		    else
		      vdata[index]=1.0;
		  }
		index++;
	      }
	  fout.write((char*)vdata,slicesize*sizeof(float));
        }
      fout.close();
      cout << "done." << '\n';
    }
  else cout << "output to file \"" << filename << "\" failed!" << '\n';
  
  delete[] vdata;
}

float trunc(float x)
{
    return floor(1000*x)/1000.0;
}

void volume::printVolume(ostream& out,int* order)
{
  int x,y,z;
  int index;
  
  for (x=0; x<size[0]; x++)
    {
      out << "----" << '\t';
    }
  out << '\n';
  
  for (z=0; z<size[2]; z++)
    {
      for (y=0; y<size[1]; y++)
        {
	  for (x=0; x<size[0]; x++)
            {
	      if (order)
		{
		  out << order[getVoxelIndex(x,y,z)];
		}
	      else
		{
		  index=getVoxelIndex(x,y,z);
		  if (voxelCarved(index)) out << 'x';
		  else if (voxelQueued(index)) out << '|';
		  else out << ' ';
		  out << data[index];
		}
	      out << '\t';
	    }
	  out << '\n';
        }
      out << '\n';
    }
}

void volume::printRegion(ostream& out,int startx,int starty,int startz,int lenx,int leny,int lenz)
{
  int x,y,z;
  int index;
  unsigned short val;
  for (z=0; z<lenz; z++)
    {
      for (y=0; y<leny; y++)
        {
	  for (x=0; x<lenx; x++)
            {
	      index=getVoxelIndex(x+startx,y+starty,z+startz);
	      if (voxelCarved(index)) out << 'x';
	      else if (voxelQueued(index)) out << '|';
	      else out << ' ';
	      val=data[index];
	      if (!voxelKnown(index)) out << '?';
	      else out << ' ';
	      out << val+index/(float)getNumVoxels();
	      out << '\t';
            }
	  out << '\n';
        }
      out << '\n';
    }
}

int volume::loadOrder(char* filename,int* order)
{
    ifstream fin(filename);
    int dontclobbersize[3];
    if (fin)
    {
	fin.read((char*)dontclobbersize,sizeof(dontclobbersize));
	fin.read((char*)order,getNumVoxels()*sizeof(int));
	fin.close();
	return 0;
    }
    else
    {
	cerr << "Error reading from file " << filename << " !\n";
	return 1;
    }
    
}

int volume::saveOrder(char* filename,int* order)
{
    if (!order) order=getDefaultOrder();
    ofstream fout(filename);
    if (fout)
    {
	fout.write((char*)size,sizeof(size));
	fout.write((char*)order,getNumVoxels()*sizeof(int));
	fout.close();
	return 0;
    }
    else
    {
	cerr << "Error saving to file " << filename << " !\n";
	return 1;
    }
}

int volume::loadCarveInfo(char* filename)
{
    ifstream fin(filename);
    if (!fin) return 1;
    int dontclobbersize[3];
    fin.read((char*)dontclobbersize,sizeof(dontclobbersize));
    carved.read(fin,getNumVoxels());
    carved.read(fin,getNumVoxels());
    fin.close();
    return 0;
}

int volume::saveCarveInfo(char* filename)
{
    ofstream fout(filename);
    if (!fout) return 1;
    fout.write((char*)size,sizeof(size));
    carved.write(fout,getNumVoxels());
    carved.write(fout,getNumVoxels());
    fout.close();
    return 0;
}

int volume::readFile(char* filename)
{
  MarchableVolume* v=MarchableVolume::createVolume(filename);
  if (v)
    {
      createFromMarchableVolume(v);
      delete v;
      return 0;
    }
  else
    {
      return 1;
    }
}

void volume::createFromMarchableVolume(MarchableVolume* v)
{
  size[0]=v->getSize()[0];
  size[1]=v->getSize()[1];
  size[2]=v->getSize()[2];
  if (data) delete[] data;
  int numVoxels=size[2]*size[1]*size[0];
  data=new unsigned short[numVoxels];
  int x,y,z,index=0;
  float min=BIGNUM,max=-BIGNUM;
  for (z=0; z<size[2]; z++) for (y=0; y<size[1]; y++) for (x=0; x<size[0]; x++)
    {
      if (v->d(x,y,z)<min) min=v->d(x,y,z);
      if (v->d(x,y,z)>max) max=v->d(x,y,z);
    }
  cout << "Min: " << min << "  Max: " << max << '\n';

  unsigned short maxshort=65535-featureSize;
  if (maxshort==0) maxshort=65535;
  //maxshort=32767; //This only loses 1 bit at worst, and it leaves plenty of room for the featureSize.
  for (z=0; z<size[2]; z++)
    for (y=0; y<size[1]; y++)
      for (x=0; x<size[0]; x++)
      {
	  if (v->isFloatingType())
	  {
	      if ((128<=max-min) && (max-min<maxshort))
		  data[index++]=(unsigned short)floor(v->d(x,y,z)-min+0.5);
	      else
	      {
		  data[index++]=(unsigned short)floor(maxshort*(v->d(x,y,z)-min)/(max-min)+0.5);
		  //unsigned short s=(unsigned short)floor(65535*(v->d(x,y,z)-min)/(max-min)+0.5);
		  //if (s>2*9200) data[index++]=65535;
		  //else data[index++]=(unsigned short)floor((65535.0/(2*9200))*65535*(v->d(x,y,z)-min)/(max-min)+0.5);
	      }
	  }
	  else
	      data[index++]=(unsigned short)floor(v->d(x,y,z)+0.5);
      }
  queued.setbit(numVoxels-1); queued.resetbit(numVoxels-1);
  carved.setbit(numVoxels-1); carved.resetbit(numVoxels-1);
}

int volume::writeFile(char* filename)
{
  ofstream fout(filename);
  if (fout)
    {
      fout.write((char*)size,sizeof(size));
      
      int sliceSize=size[1]*size[0];
      float* slice=new float[sliceSize];
      int x,y,z,voxelIndex=0,index;
      int* order=getDefaultOrder();
      
      for (z=0; z<size[2]; z++)
	{
	  index=0;
	  for (y=0; y<size[1]; y++)
	    for (x=0; x<size[0]; x++)
	      {
		//slice[index]=(float)carvedInsideOrder[voxelIndex];
		//if (voxelIndex<100) cout << carvedInsideOrder[voxelIndex] << ' ';
		//slice[index]=(float)order[voxelIndex];
		slice[index]=(float)data[voxelIndex] + carvedInsideOrder[voxelIndex]/(2*(float)getNumVoxels());
		index++;
		voxelIndex++;
	      }
	  fout.write((char*)slice,sliceSize*sizeof(float));
	}
      fout.close();
      
      delete[] slice;
      return 0;
    }
  else
    {
      cerr << "Error saving to file " << filename << " !\n";
      return 1;
    }
}

istream& operator>> (istream& in, volume& v)
{
    in.read((char*)v.size,sizeof(v.size));
    if (v.data) delete[] v.data;
    v.data=new unsigned short[v.size[2]*v.size[1]*v.size[0]];
    in.read((char*)v.data,v.size[2]*v.size[1]*v.size[0]*sizeof(unsigned short));
    return in;
}

ostream& operator<< (ostream& out, const volume& v)
{
    out.write((char*)v.size,sizeof(v.size));
    out.write((char*)v.data,v.size[2]*v.size[1]*v.size[0]*sizeof(unsigned short));
    return out;
}


#ifdef FIXTOP
int main(int argc,char* argv[])
{
  if (argc<=2)
    {
      cerr << "Usage: " << argv[0] << " <input .v file> <output .v file> [threshold]\n";
      return 1;
    }

  unsigned short threshold=65535;
  if (argc>3)
    {
      threshold=atoi(argv[3]);
    }
  
  int result;
  
  volume v(threshold);
  
  v.readTopoinfoFiles();
  
  result=v.readFile(argv[1]);
  if (result) return result;
  
  cout << v.getSize()[0] << 'x' << v.getSize()[1] << 'x' << v.getSize()[2] << '=';
  cout << v.getSize()[0]*v.getSize()[1]*v.getSize()[2] << '\n';

  int signsNeedToBeChanged=v.dataShouldBeNegated();
  if (signsNeedToBeChanged) v.changeAllSigns();
  clock_t starttime = clock();
  v.fixTopologyStrict();
  clock_t endtime = clock();
  if (signsNeedToBeChanged) v.changeAllSigns();
  cout << "Topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";
  
  cout << "Writing to file " << argv[2] << " ..."; cout.flush();
  result=v.writeFile(argv[2]);
  if (result)
  {
      cout << "error!\n";
      return result;
  }
  else cout << "done.\n";
  
  return 0;
}
#endif

#ifdef COMPARE
int main(int argc,char* argv[])
{
  if (argc<=1)
    {
      cerr << "Usage: " << argv[0] << " <input .v file> [threshold]\n";
      return 1;
    }

  unsigned short threshold=65535;
  if (argc>2)
    {
      threshold=atoi(argv[2]);
    }
  
  int result;
  
  volume v(threshold);
  
  v.readTopoinfoFiles();
  
  result=v.readFile(argv[1]);
  if (result) return result;
  
  cout << v.getSize()[0] << 'x' << v.getSize()[1] << 'x' << v.getSize()[2] << '=';
  cout << v.getSize()[0]*v.getSize()[1]*v.getSize()[2] << '\n';

  int signsNeedToBeChanged=v.dataShouldBeNegated();
  clock_t starttime,endtime;

  if (signsNeedToBeChanged) v.changeAllSigns();
  starttime = clock();
  v.fixTopologyBySlices();
  endtime = clock();
  cout << "Slices topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n\n";
  
  result=v.readFile(argv[1]);
  if (result) return result;
  
  if (signsNeedToBeChanged) v.changeAllSigns();
  starttime = clock();
  v.fixTopologyCombo();
  endtime = clock();
  cout << "Combo  topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n\n";
  
  result=v.readFile(argv[1]);
  if (result) return result;
  
  if (signsNeedToBeChanged) v.changeAllSigns();
  starttime = clock();
  v.fixTopologyStrict();
  endtime = clock();
  cout << "Strict topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n\n";
  
  return 0;
}
#endif

