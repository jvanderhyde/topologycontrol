//Volume3DplusT.h
//James Vanderhyde, 14 May 2007.

#include "bitc.h"
#include "carvecomp4D.h"

#define BIGNUM 1e20

class voxelComparator4D : public std::binary_function<int,int,int>
{
private:
    Volume3DplusT* vol;
    
public:
    voxelComparator4D(Volume3DplusT* p_vol);
    int operator()(int a,int b);
};

class Volume3DplusT
{
protected:
    int size[4];
    unsigned short* data;
    bitc queued;
    bitc carved;
    int numSlices;
    TopoCheck4D* strictTopoCheck;
    TopoCheck4D* comboTopoCheck;
    TopoCheck4D* sliceTopoCheck;
    TopoCheck4D* thickSliceTopoCheck;
    int numNeighborsVertex,numNeighborsFace;
    int** neighborhoodVertex,** neighborhoodFace;
    int numNeighborsSliceVertex,numNeighborsSliceFace;
    int** neighborhoodSliceVertex,** neighborhoodSliceFace;
    int numCarvedRegions;
    carvecomp4D** carvedRegions;
    int* initialOrder;
    int* carvedInsideOrder;
public:
    Volume3DplusT();
    virtual ~Volume3DplusT();
    void setUpNeighborhoods();
    int* getSize();
    int getNumVoxels();
    int getNumSlices();
    int getSliceSize();
    int getVoxelIndex(int x,int y,int z,int t);
    void getVoxelLocFromIndex(int index,int* x,int* y,int* z,int* t);
    float d(int x,int y,int z,int t);
    unsigned short getVoxel(int index);
    unsigned short getVoxel(int x,int y,int z,int t);
    void setVoxel(int x,int y,int z,int t,unsigned short val);
    int voxelCarved(int index);
    void setVoxelCarved(int index,char label=1);
    void clearVoxelCarved(int index);
    int voxelQueued(int index);
    void setVoxelQueued(int index);
    void clearVoxelQueued(int index);
    int getVoxelOrder(int index,int* order=NULL);
    int getVoxelInitialOrder(int index);
    int getVoxelCarvedInsideOrder(int index);
    virtual void readTopoinfoFiles();
    void changeAllSigns();
    void constructInitialBoundaryForSlice(int slice);
    void constructInitialBoundary(int component);
    void invertOrder(int* order);
    void invertSliceOrder(int* order,int slice);
    void invertPermutation(int* order);
    void sortVoxels();
    void checkAndAddNeighborToQueue(pqitem voxel,int component,int neighborIndex);
    void addSliceNeighborsToQueue(pqitem voxel,int component);
    void addNeighborsToQueue(pqitem voxel,int component);
    void carveSimultaneously(unsigned short featureSize,int bySlices=0);
    void fixTopologyBySlices(unsigned short featureSize=32768,int fixDirection=-1);
    void fixTopologyStrict(unsigned short featureSize=32768,int fixDirection=-1);
    void fixTopologyLoadedOrder(char* filename);
    void fixTopology(char* arg);
    void checkUncarvedVoxels(int slice);
    int* getDefaultOrder();
    void getTopoType(bool* topoType,int index,int* order);
    void getTopoTypeInRange(bool* topoType,int index,int* order,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int tmin,int tmax);
    int voxelCriticalInVolume(bool* topoType,int index,int* order=NULL);
    int voxelCriticalInSlice(bool* topoType,int index,int* order=NULL);
    int voxelCriticalInThickSlice(bool* topoType,int index,int leftOrRight,int* order=NULL);
    int countCriticalsInVolume(int* order=NULL);
    int countCriticalsInSlices(int* order=NULL);
    int countCriticalsInSlice(int slice,int* order=NULL);
    int countCriticalsInThickSlices(int* order=NULL);
    int countCriticalsInThickSlice(int slice,int* order=NULL,int augment=0);
    int loadOrder(char* filename,int* order);
    int saveOrder(char* filename,int* order=NULL);
    int readByteFile(char* filename);
    void storeFloatDataIntoShorts(float* floatdata,int numVoxels);
    int readFloatFile(char* filename);
    int readFloat3DFile(char* filename);
    int readFile(char* filename);
    int writeFile(char* filename);
};
