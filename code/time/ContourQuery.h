//ContourQuery.h
//James Vanderhyde, 6 February 2007.

#include "Volume2DplusT.h"

#include <vector>

class ContourQuery
{
protected:
    Volume2DplusT* vol;
    std::vector<int> queryResult;
    int windowStart,windowLength;
    ContourTree windowJT,windowST,windowCT;
    int cavitiesComputed,eulerComputed,rootChi;
    
    void sortedListIntersection(std::vector<int>& v1,std::vector<int>& v2,std::vector<int>& v);
    
public:
    int questionVoxel;
    void checkQuestionVoxel();
    
    ContourQuery(Volume2DplusT* v);
    ~ContourQuery();
    
    int getNumResults();
    void getResults(int* results);
    ContourTree* getWindowTree();
    ContourTree* getWindowCT();
    int containsVoxel(int index);
    
    void computeBoundarySliceOverlaps();
    void computeEulerCh();
    void computeCavities();
    
    void defineWindow(int startSlice,int numSlices,char* filenameJT=NULL,char* filenameST=NULL,char* filenameCT=NULL);
    void calcCompsBelowIsovalueOf(int voxel);
    void calcCompsWithBoundarySliceOverlapInRange(int min,int max);
    void keepCompsWithBoundarySliceOverlapInRange(int min,int max);
    void keepCompsWithSliceOverlapInRange(int slice,int min,int max);
    void keepCompsWithSliceOverlapInBoundaryRange(int slice);
    void keepSmallComps(int maxSize);
    void keepCompsBelowIsovalue(int voxel);
    void keepOnlyContainingSublevelSets(int voxel);
    void keepOnlyContainingSublevelSets();
    void keepOnlyContainedSublevelSets();
    void keepWindowInterior();
    void keepCompsWithSmallNumberOfCavities(int max);
    void keepCompsWithEulerChInRange(int min,int max);
    void keepCompsWithSmallNumberOfHandles(int max);
    
    int findVoxelPathInTree(int voxel,std::vector<int>& path);
    void printTreeBelow(int critVoxel);
    
    void printResult();
    
};

