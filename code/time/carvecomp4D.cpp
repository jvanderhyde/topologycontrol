//carvecomp4D.cpp
//James Vanderhyde, 16 May 2005

#include "Volume3DplusT.h"

carvecomp4D::carvecomp4D(char p_label,int p_increasing,TopoCheck4D* p_topoinfo,Volume3DplusT* p_vol,int* p_carvedOrder)
{
  label=p_label;
  increasing=p_increasing;
  topoinfo=p_topoinfo;
  vol=p_vol;
  carvedOrder=p_carvedOrder;
  neighbors=new int[81];
  neighborsQ=new pqitem[81];
  numCarved=0;
  if (increasing)
    boundary=new minQueueWrapper();
  else
    boundary=new maxQueueWrapper();
}

carvecomp4D::~carvecomp4D()
{
  delete[] neighbors;
  delete[] neighborsQ;
  delete boundary;
}

char carvecomp4D::getLabel()
{
  return label;
}

int carvecomp4D::getNumCarved()
{
  return numCarved;
}

//Returns true if carving the specified voxel changes the topology.
// We decide based on the state of the 80 neighbors.
// For each neighbor, we set the bit if the neighbor is
//  in the current inside set.
int carvecomp4D::topologyCheck(int index,int questionVoxel)
{
  //In this case, a neighbor is in the current inside set
  // if we're increasing and it is carved
  // or we're decreasing and it is not carved.
  int bit=0;
  int x,y,z,t,ox,oy,oz,ot;
  int neighborIndex;
  int* vsize=vol->getSize();
  vol->getVoxelLocFromIndex(index,&x,&y,&z,&t);
  for (ot=-1; ot<=1; ot++)
   for (oz=-1; oz<=1; oz++)
    for (oy=-1; oy<=1; oy++)
      for (ox=-1; ox<=1; ox++)
	if ((ot!=0) || (oz!=0) || (oy!=0) || (ox!=0))
	  {
	    if ((t+ot>=0) && (t+ot<vsize[3]) &&
		(z+oz>=0) && (z+oz<vsize[2]) &&
		(y+oy>=0) && (y+oy<vsize[1]) &&
		(x+ox>=0) && (x+ox<vsize[0]))
	      {
		//First assume voxel is not carved.
		if (increasing) neighborhood[bit]=false;
		else neighborhood[bit]=true;
		neighborIndex=vol->getVoxelIndex(x+ox,y+oy,z+oz,t+ot);
		//If the voxel is carved in this component, we flip the bit.
		if (vol->voxelCarved(neighborIndex))
		  neighborhood[bit]=!neighborhood[bit];
	      }
	    else
	      {
	 	//Out of range is not part of this component, so we do not set the bit.
		neighborhood[bit]=false;
	      }
	    
	    bit++;
	  }
	    
  if (index==questionVoxel)
  {
      cout << index << "(" << x << "," << y << "," << z << ";" << t << "):" << vol->getVoxel(index) << "\n";
      topoinfo->printNeighborhood(neighborhood);
  }
  return topoinfo->topologyChange(neighborhood);
}

void carvecomp4D::addToBoundary(pqitem voxel)
{
  boundary->push(voxel);
}

void carvecomp4D::addDelayedToBoundary(pqitem voxel)
{
  //We're using voxel.subdelay==vol->getNumVoxels()+1 to indicate that the voxel is one of the
  // originals put on the queue, not one of the pushed neighbors.
  //Any other subdelay value is a pushed neighbor. If the subdelay is less than numCarved,
  // we may yet be able to carve this voxel at this value so we update the subdelay and put it back in the queue.
  // Otherwise we just throw the voxel away.
  if (voxel.subdelay==vol->getNumVoxels()+1)
    boundary->push(pqitem(voxel.index,voxel.subdelay,voxel.value+((increasing)?1:-1)));
  else if (voxel.subdelay<numCarved)
    boundary->push(pqitem(voxel.index,numCarved,voxel.value));
}

void carvecomp4D::carveVoxel(int index)
{
  carveVoxel(pqitem(index,0,vol->getVoxel(index)));
}

void carvecomp4D::carveVoxel(pqitem voxel)
{
  int n,num;
  float val,val2;
  int x,y,z;
  int index=voxel.index;

  vol->setVoxelCarved(index,label);
  carvedOrder[index]=numCarved;
  numCarved++;

  /*int questionOrder=1865;
  int numVoxels=vol->getNumVoxels();
  if ((-2<=numVoxels-1-numCarved-questionOrder) && (numVoxels-1-numCarved-questionOrder<=2))
  {
      cout << "order=" << numCarved-1 << " ";
      cout << (topologyCheck(index,index)?"change ":"   no  ");
  }*/
}

int carvecomp4D::peekNextVoxel()
{
  if (boundary->empty()) return -1;
  return boundary->top().value;
}

pqitem carvecomp4D::getNextVoxel()
{
  pqitem top=boundary->top();
  boundary->pop();
  vol->clearVoxelQueued(top.index);
  return top;
}

