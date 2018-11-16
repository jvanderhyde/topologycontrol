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
#define LCLAMP(x,y) (((x)<(y))?(x=y):(x))
#define GCLAMP(x,y) (((x)>(y))?(x=y):(x))
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
    volume(unsigned short* vdata,int* vsize);
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
    void clearVoxelsCarvedInRange(int xmin,int xmax,int ymin,int ymax,int zmin,int zmax);
    void clearAllVoxelsCarved();
    int voxelQueued(int index);
    void setVoxelQueued(int index);
    void clearVoxelQueued(int index);
    void clearAllVoxelsQueued();
    void setQueuedToUnionOfQueuedAndCarved();
    void setQueuedToUnionOfQueuedAndCarvedInRange(int xmin,int xmax,int ymin,int ymax,int zmin,int zmax);
    void setCarvedToUnionOfQueuedAndCarved();
    int getVoxelOrder(int index,int* order=NULL);
    int voxelBelowVoxel(int index,int x,int y,int z,int* order);
    unsigned short* getData();
    int getCriticalParent(int index);
    int* getCriticalParents();
    int traceToCriticalAncestor(int index);
    int getMinNeighbor(int index,int* order=NULL);
    int getMinNeighborInRange(int index,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int* order=NULL);
    int traceToLocalMinInRange(int index,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int* order=NULL);
    void calcCricitalParentsByTracing(ContourTree& jt);
    int findAndUpdateCriticalAncestor(int index);
    int findCriticalAncestor(int index);
    int findCriticalAncestorOfNeighbor(int index,int numNeighbors,int** neighborhood);
    void joinOrSplitCriticalPoints(int index,int numNeighbors,int** neighborhood,ContourTree* ct=NULL,int* numChildrenRef=NULL,int* children=NULL);
    void joinCriticalPoints(int index,int numNeighbors,int** neighborhood,ContourTree* ct=NULL);
    void splitCriticalPoints(int index,int numNeighbors,int** neighborhood,ContourTree* ct=NULL);
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
    int readCarvedOrder(char* filename);
    int* getDefaultOrder();
    int findLastVoxelBelowIsovalue(int isovalue,int* order=NULL);
    virtual TopoCheck* getTopoCheck();
    int getTopoType(int index,int* order);
    int getTopoTypeInRange(int index,int* order,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax);
    int voxelCritical(int index,int* order=NULL);
    int countCriticals(int* order);
    int countInsideCriticals();
    int getBdEulerChBelowVoxelInRange(int index,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int* order=NULL);
    int getBdEulerChAboveVoxel(int index,int* order=NULL);
    void buildJoinTreeInside(int* edgeList=NULL);
    int getArraysFromCT(ContourTree& ct,int* critList,int* numCrit,int* edgeList,int* edgeLabels,int simplified);
    int getContourTree(int* critList,int* numCrit,int* edgeList=NULL,int simplified=0);
    void buildJoinTreeForInsideOrder(int* order,ContourTree& jt);
    void buildSplitTreeForInsideOrder(int* order,ContourTree& st);
    void buildFullJoinTree(int* order=NULL);
    void labelFullJoinTree(int voxel);
    ContourTree* getFullJoinTree();
    int voxelBelowJTEdge(int v,int node);
    int markVoxelsBelowVoxelInRange(int v,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int* order=NULL);
    int markVoxelsBelowVoxel(int v,int* order=NULL); //both use carved bit array to mark (flood fill)
    void createHistogram();
    void turnOnAnimation();
    void turnOffAnimation();
    void renderVolume(int component);
    void printVolume(ostream& out,int* order=NULL);
    void printRegion(ostream& out,int startx,int starty,int startz,int lenx,int leny,int lenz);
    int loadOrder(char* filename,int* order);
    int saveOrder(char* filename,int* order=NULL);
    int loadCarveInfo(char* filename);
    int saveCarveInfo(char* filename);
    virtual void createFromMarchableVolume(MarchableVolume* v);
    int readFile(char* filename);
    int writeFile(char* filename);
};

