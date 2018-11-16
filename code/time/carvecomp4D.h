//carvecomp4D.h
//James Vanderhyde, 16 May 2007

#include "TopoCheck4D.h"
#include "priorityQueue.h"

class Volume3DplusT;

class carvecomp4D
{
 protected:
  char label;
  int increasing;
  priorityQueueWrapper* boundary;
  int numCarved;

  int* neighbors;
  pqitem* neighborsQ;
  bool neighborhood[80];

  TopoCheck4D* topoinfo;
  Volume3DplusT* vol;
  int* carvedOrder;

 public:
  carvecomp4D(char p_label,int p_increasing,TopoCheck4D* p_topoinfo,Volume3DplusT* p_vol,int* p_carvedOrder);
  ~carvecomp4D();
  char getLabel();
  int getNumCarved();
  int topologyCheck(int index,int questionVoxel=-1);
  void addToBoundary(pqitem voxel);
  void addDelayedToBoundary(pqitem voxel);
  void carveVoxel(int index);
  void carveVoxel(pqitem voxel);
  int peekNextVoxel();
  pqitem getNextVoxel();
};

