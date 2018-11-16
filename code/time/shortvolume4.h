//shortvolume.h
//James Vanderhyde, 3 November 2005

#include <iostream.h>

#include "bitc.h"
#include "carvecomp.h"
#include "MarchableVolume.h"
#include "ContourTree.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)<(b))?(b):(a))
#define SGN(x) (((x)<0)?-1:(((x)>0)?1:0))
#define ABS(x) (((x)<0)?-(x):(x))
#define BIGNUM 1e20
#define SMALLNUM 1e-15

class voxelComparator : public std::binary_function<int,int,int>
{
 private:
  volume* vol;
  
 public:
  voxelComparator(volume* p_vol);
  int operator()(int a,int b);
};

class volume
{
    friend istream& operator>> (istream& in, volume& v);
    friend ostream& operator<< (ostream& out, const volume& v);
protected:
    int size[3];
    unsigned short* data;
    bitc queued;
    bitc carved;
    int* criticalParent;
    TopoCheck* noTopoCheck;
    TopoCheck* strictTopoCheck;
    TopoCheck* sliceTopoCheck;
    TopoCheck* comboTopoCheck;
    int numNeighborsVertex,numNeighborsFace;
    int** neighborhoodVertex,** neighborhoodFace;
    int animationOn;
    int frameNumber;
    int numCarvedRegions;
    carvecomp** carvedRegions;
    int alreadyCarvedNegative;
    int numCarvedNegative;
    unsigned short featureSize;
    int useStrictTopologyCheck;
    int printCarveDetails;
    pqvector* innerForLater;
    pqvector* outerForLater;
    int fixStyle;
    float isovalue;
    int* carvedInsideOrder;
    int* carvedSlicesOrder;
    int* initialOrder;
    int tempStructure;
    ContourTree fullJoinTree;
public:
    volume(unsigned short p_featureSize=65535,int p_fixStyle=0);
    virtual ~volume();
    void setUpNeighborhoods();
    int* getSize();
    int getNumVoxels();
    void setFeatureSize(unsigned short p_featureSize);
    int getVoxelIndex(int x,int y,int z);
    void getVoxelLocFromIndex(int index,int* x,int* y,int* z);
    float d(int x,int y,int z);
    unsigned short getVoxel(int index);
    unsigned short getVoxel(int x,int y,int z);
    void setVoxel(int x,int y,int z,unsigned short val);
    int voxelKnown(int index);
    int voxelCarved(int index);
    void setVoxelCarved(int index);
    void setVoxelCarved(int index,char label);
    void clearVoxelCarved(int index);
    int voxelQueued(int index);
    void setVoxelQueued(int index);
    void clearVoxelQueued(int index);
    int getVoxelOrder(int index,int* order=NULL);
    unsigned short* getData();
    int getCriticalParent(int index);
    int traceLowerCriticalAncestor(int index);
    void calcCricitalParentsByTracing();
    int findCriticalAncestor(int index);
    int findCriticalAncestorOfNeighbor(int index,int numNeighbors,int** neighborhood);
    void joinCriticalPoints(int index,int numNeighbors,int** neighborhood);
    virtual void readTopoinfoFiles();
    int dataShouldBeNegated();
    void changeAllSigns();
    int voxelOnIsosurface(int x,int y,int z);
    void constructInitialOuterBoundary(int component);
    void constructInitialInnerBoundary(int component);
    int getNeighbors6(int x,int y,int z,int* neighbors);
    int getNeighbors26(int x,int y,int z,int* neighbors);
    int getNeighbors124(int x,int y,int z,int* neighbors);
    int topologyCheckOutside(int x,int y,int z,unsigned char* topoinfo);
    int topologyCheckInside(int x,int y,int z,unsigned char* topoinfo);
    void invertPermutation(int* order);
    void sortVoxels();
    virtual void addNeighborsToQueue(pqitem voxel,int component);
    void carveSimultaneously();
    void fixVolumeByComponent(int comp);
    virtual int fixTopologyStrict(int useUnionFind=0);
    int* getDefaultOrder();
    virtual TopoCheck* getTopoCheck();
    int voxelCritical(int index,int* order=NULL);
    int countCriticals(int* order);
    int countInsideCriticals();
    void buildJoinTreeInside(int* edgeList=NULL);
    int getArraysFromCT(ContourTree& ct,int* critList,int* numCrit,int* edgeList,int* edgeLabels,int simplified);
    int getContourTree(int* critList,int* numCrit,int* edgeList=NULL,int simplified=0);
    void buildJoinTreeForInsideOrder(int* order,ContourTree& jt);
    void buildSplitTreeForInsideOrder(int* order,ContourTree& st);
    void buildFullJoinTree(int* order=NULL);
    void labelFullJoinTree(int isovalue);
    ContourTree* getFullJoinTree();
    int voxelBelowJTEdge(int v,int node);
    void turnOnAnimation();
    void turnOffAnimation();
    void renderVolume(int component);
    void printVolume(ostream& out,int* order=NULL);
    void printRegion(ostream& out,int startx,int starty,int startz,int lenx,int leny,int lenz);
    virtual void createFromMarchableVolume(MarchableVolume* v);
    int readFile(char* filename);
    int writeFile(char* filename);
};

