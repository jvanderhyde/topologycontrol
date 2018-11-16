//Volume2DplusT.h
//James Vanderhyde, 28 September 2006.

#include "shortvolume.h"

class Volume2DplusT : public volume
{
protected:
    int numSlices;
    TopoCheck* thickSliceTopoCheck;
    int numNeighborsSliceVertex,numNeighborsSliceFace;
    int** neighborhoodSliceVertex,** neighborhoodSliceFace;
    int numNeighborsThickLeftVertex,numNeighborsThickLeftFace;
    int** neighborhoodThickLeftVertex,** neighborhoodThickLeftFace;
    int numNeighborsThickRightVertex,numNeighborsThickRightFace;
    int** neighborhoodThickRightVertex,** neighborhoodThickRightFace;
    ContourTree* sliceJoinTrees,* thickSliceJoinTrees;
    ContourTree* sliceSplitTrees,* thickSliceSplitTrees;
    ContourTree* sliceTrees,* thickSliceTrees;
    
public:
    Volume2DplusT(unsigned short p_featureSize=65535,int p_fixStyle=0);
    virtual ~Volume2DplusT();
    void setUpSliceNeighborhoods();
    int getNumSlices();
    int getSliceSize();
    virtual void readTopoinfoFiles();
    void constructInitialInnerBoundaryForSlice(int slice);
    int fixTopologyBySlices();
    virtual int fixTopologyStrict(int useUnionFind=0);
    virtual TopoCheck* getTopoCheck();
    int getSliceVoxelsInOrder(int slice,int* data,int* order=NULL);
    int voxelCriticalInSlice(int index,int* order=NULL);
    int voxelCriticalInVolume(int index,int* order=NULL);
    int voxelCriticalInThickSlice(int index,int leftOrRight,int* order=NULL);
    int countCriticalsInVolume(int* order=NULL);
    int countCriticalsInSlices(int* order=NULL);
    int countCriticalsInSlice(int slice,int* order=NULL);
    int countCriticalsInThickSlices(int* order=NULL);
    int countCriticalsInThickSlice(int slice,int* order=NULL,int augment=0);
    void buildJoinTreeForSlice(int* order,int slice,ContourTree& jt);
    void buildSplitTreeForSlice(int* order,int slice,ContourTree& st);
    void buildJoinTreeForThickSlice(int* order,int sliceLeft,ContourTree& jt,int augment=0,std::vector<int>* nodes=NULL);
    void buildSplitTreeForThickSlice(int* order,int sliceLeft,ContourTree& st,int augment=0,std::vector<int>* nodes=NULL);
    void buildTrees(int* order,int augment);
    void mergeJoinTrees(int* order,int sliceLeft,int sliceRight,ContourTree& jt,std::vector<int>* nodes);
    void mergeSplitTrees(int* order,int sliceLeft,int sliceRight,ContourTree& st,std::vector<int>* nodes);
    int simpleTrack(int prevSlice,int curSlice,int numSeeds,int* seeds,unsigned short isovalue);
    int trackContour(int prevSlice,int curSlice,int numSeeds,int* seeds);
    int getJoinTreeForSlice(int slice,int* critList,int* numCrit,int* edgeList=NULL,int simplified=0);
    int getSplitTreeForSlice(int slice,int* critList,int* numCrit,int* edgeList=NULL,int simplified=0);
    int getContourTreeForSlice(int slice,int* critList,int* numCrit,int* edgeList=NULL,int simplified=0);
    int getContourTreeForThickSlice(int sliceLeft,int* critList,int* numCrit,int* edgeList=NULL,int* edgeLabels=NULL,int simplified=0,int augment=0);
    void getContourTreesForSlices(int** critList,int** numCrit,int* numEdges,int** edgeList,int simplified=0);
    void getContourTreesForThickSlices(int** critList,int** numCrit,int* numEdges,int** edgeList,int simplified=0);
    int markVoxelsBelowVoxelInSlices(int v,int firstSlice,int lastSlice,int* order=NULL);
    virtual void createFromMarchableVolume(MarchableVolume* v);

};

