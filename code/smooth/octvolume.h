/*
 *  octvolume.h
 *  
 *
 *  Created by James Vanderhyde on Wed Jun 18 2003.
 *
 */

#include "octree.h"

class volume
{
    friend istream& operator>> (istream& in, volume& v);
    friend ostream& operator<< (ostream& out, const volume& v);
protected:
    int size[3];
    octree* dataroot;
    int treeHeight;
    int highestResolutionLevel;
    unsigned char* topoinfoPatch;
    unsigned char* topoinfoThread;
    int animationOn;
    int frameNumber;
    maxqueue* innerBoundary;
    maxqueue* outerBoundary;
    int alreadyCarvedNegative;
    int numCarvedNegative;
    int numFeatures;
    int useStrictTopologyCheck;
    int printCarveDetails;
    pqvector* innerForLater;
    pqvector* outerForLater;
    int fixStyle;
    float* extractedSlice1;
    float* extractedSlice2;
    int extractedSlice1z,extractedSlice2z;
    int extractedSlice1Age,extractedSlice2Age;
    octree** space27;
public:
    volume(int features=0,int p_fixStyle=0,int resolution=100);
    ~volume();
    int* getSize();
    octree* getDataroot();
    int getTreeHeight();
    octree* getVoxel(int level,int x,int y,int z);
    octree* getDeepestVoxel(int x,int y,int z);
    void deleteVoxel(int level,int x,int y,int z);
    float d(int x,int y,int z);
    int readTopoinfoFiles();
    void extractSlice(int slice,octree** buf);
    void extractSlice(int slice,float* buf);
    void extractSliceLevels(int slice,int* buf);
    void changeAllSigns();
    void resetUnknowns(int level);
    int voxelOnIsosurface(int level,int x,int y,int z,octree* givenVoxel=NULL);
    void uncarveLayer(int level);
    void fillInLevel(int level);
    void divideLevel(int level);
    void checkLevel(int level);
    void boundaryOutsideCheck(int level,octree* supervoxel,int x,int y,int z,int nx,int ny,int nz,int outOfBounds);
    void constructBoundaryOutside(int level);
    void constructBoundaryInside(int level);
    void printBoundary(int level,ostream& out);
    int getNeighbors6(int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ);
    int getNeighbors27(int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ=NULL,int deepest=0);
    int getNeighbors26(int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ=NULL,int deepest=0);
    int getNeighbors124(int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ);
    void calcDistances(int level);
    int topologyCheckOutside(int level,int x,int y,int z,unsigned char* topoinfo);
    int topologyCheckInside(int level,int x,int y,int z,unsigned char* topoinfo);
    void carveVoxelOutside(octree* voxel,int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ);
    void carveVoxelInside(octree* voxel,int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ);
    void carveSimultaneousLowRes(int level);
    void carveSimultaneousHighRes(int level);
    int openFeatureOutside();
    int openFeatureInside();
    int changeInsideOrOutside(int level);
    void fixVolumeOutside(int level);
    void fixVolumeInside(int level);
    void fixVolumeBoth(int level);
    void fixVolumeInsideCarved(int level);
    void addNeighborsOfUncarved(int level);
    void fixTopologyOutside();
    void fixTopologyInside();
    void fixTopologyBoth();
    void fixTopology();
    void turnOnAnimation();
    void turnOffAnimation();
    void renderVolume();
    void renderVolume(int level);
    void printVolume(int level,ostream& out);
    void printRegion(int level,ostream& out,int startx,int starty,int startz,int lenx,int leny,int lenz);
    int readFile(char* filename);
    int writeFile(char* filename);
};

