//carvecomp.cpp
//James Vanderhyde, 9 November 2005

#include "shortvolume.h"

carvecomp::carvecomp(char p_label,int p_increasing,float p_isovalue,TopoCheck* p_topoinfo,volume* p_vol,int* p_carvedOrder)
{
  label=p_label;
  increasing=p_increasing;
  isovalue=p_isovalue;
  topoinfo=p_topoinfo;
  vol=p_vol;
  carvedOrder=p_carvedOrder;
  neighbors=new int[27];
  neighborsQ=new pqitem[27];
  numCarved=0;
  if (increasing)
    boundary=new minQueueWrapper();
  else
    boundary=new maxQueueWrapper();
}

carvecomp::~carvecomp()
{
  delete[] neighbors;
  delete[] neighborsQ;
  delete boundary;
}

char carvecomp::getLabel()
{
  return label;
}

int carvecomp::getNumCarved()
{
  return numCarved;
}

//Returns true if adding the specified voxel changes the topology.
// We use look-up tables based on the state of the 26 neighbors.
// For each neighbor, we set the bit if the neighbor is
//  in the current carved set.
int carvecomp::topologyCheck(int index)
{
  //In this case, a neighbor is in the current inside set
  // if it is not carved or it is negative.
  int topoType=0,bit=0;
  int x,y,z,ox,oy,oz;
  int sz=0,ez=0;
  int neighborIndex;
  int* vsize=vol->getSize();
  vol->getVoxelLocFromIndex(index,&x,&y,&z);
  if (topoinfo->getDimension()>=3)
  {
      sz=-1;
      ez=1;
  }
  for (oz=sz; oz<=ez; oz++)
    for (oy=-1; oy<=1; oy++)
      for (ox=-1; ox<=1; ox++)
	if ((oz!=0) || (oy!=0) || (ox!=0))
	  {
	    if ((z+oz>=0) && (z+oz<vsize[2]) &&
		(y+oy>=0) && (y+oy<vsize[1]) &&
		(x+ox>=0) && (x+ox<vsize[0]))
	      {
		neighborIndex=vol->getVoxelIndex(x+ox,y+oy,z+oz);
		//If the voxel is carved in this component, we set the bit.
		if (vol->voxelCarved(neighborIndex))
		  topoType |= (1<<bit);
	      }
	    //Out of range is not part of this component, so we do not set the bit.
	    bit++;
	  }
  return topoinfo->topologyChange(topoType);
}

void carvecomp::addToBoundary(pqitem voxel)
{
  boundary->push(voxel);
}

void carvecomp::addDelayedToBoundary(pqitem voxel)
{
  if (voxel.subdelay<numCarved)
    boundary->push(pqitem(voxel.index,numCarved,voxel.value));
  else
    boundary->push(pqitem(voxel.index,numCarved,voxel.value+((increasing)?1:-1)));
}

int carvecomp::pastIsovalue(int index)
{
  if (increasing)
    {
      return (vol->getData()[index] > isovalue);
    }
  else
    {
      return (vol->getData()[index] < isovalue);
    }
}

unsigned short carvecomp::valueWithinIsovalue()
{
  if (increasing)
    {
      return (unsigned short)isovalue;
    }
  else
    {
      return (unsigned short)isovalue;
    }
}

void carvecomp::carveVoxel(int index)
{
  carveVoxel(pqitem(index,0,vol->getVoxel(index)));
}

void carvecomp::carveVoxel(pqitem voxel)
{
  int n,num;
  float val,val2;
  int x,y,z;
  int index=voxel.index;

  vol->setVoxelCarved(index,label);
  carvedOrder[index]=numCarved;
  numCarved++;
}

int carvecomp::peekNextVoxel()
{
  if (boundary->empty()) return -BIGNUM;
  return boundary->top().value;
}

pqitem carvecomp::getNextVoxel()
{
  pqitem top=boundary->top();
  boundary->pop();
  vol->clearVoxelQueued(top.index);
  return top;
}

