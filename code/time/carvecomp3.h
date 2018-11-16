//carvecomp.h
//James Vanderhyde, 9 November 2005

#include "TopoCheck.h"
#include "priorityQueue.h"

class volume;

class carvecomp
{
 protected:
  char label;
  int increasing;
  float isovalue;
  priorityQueueWrapper* boundary;
  int numCarved;

  int* neighbors;
  pqitem* neighborsQ;

  TopoCheck* topoinfo;
  volume* vol;
  int* carvedOrder;

 public:
  carvecomp(char p_label,int p_increasing,float p_isovalue,TopoCheck* p_topoinfo,volume* p_vol,int* p_carvedOrder);
  ~carvecomp();
  char getLabel();
  int getNumCarved();
  int topologyCheck(int index);
  void addToBoundary(pqitem voxel);
  void addDelayedToBoundary(pqitem voxel);
  int pastIsovalue(int index);
  unsigned short valueWithinIsovalue();
  void carveVoxel(int index);
  void carveVoxel(pqitem voxel);
  int peekNextVoxel();
  pqitem getNextVoxel();
};

