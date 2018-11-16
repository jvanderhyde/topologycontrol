/*
 *  MarchableVolume.h
 *
 *  This is an abstract class for accessing voxel data.
 *  Access is obtained through calls to d(x,y,z).
 *  d(x,y,z) assumes access is desired in a slice-coherent fashion,
 *    so it keeps a cache of two slices--the last slice accessed and
 *    the previous slice with the greater z value. For example, if
 *    processing slices from 0 to size[2]-1 consecutively, the last 
 *    two slices are the ones that are cached. Also, it starts over
 *    at 0 and 1 if called with z=0. Make sure to call with z=0
 *    the first time so the arrays are initialized. (The createVolume
 *    factory method handles this for you.) Also if the data is changed
 *    be sure to call with z=0 to make sure the last slices are swapped
 *    out before saving. Implementations of writeFile should handle this.
 *  d(x,y,z) currently calls d3(x,y,z) for 3 slice access at a time.
 *  
 *
 *  Created by James Vanderhyde on Wed May 5 2004.
 *
 */

//#include "octree.h"
//#include "volfill/OccGridRLE.h"
#include "bitc.h"

#define BIGNUM 1e20

class MarchableVolume
{
 private:
  float* slice1;
  float* slice2;
  float* slice3;
  int z1,z2,z3;
  int modified1,modified2,modified3;
  void swapSlice1(int z);
  void swapSlice2(int z);
  void swapSlice3(int z);
  float d2(int x,int y,int z);
  float d3(int x,int y,int z);
  void d3(int x,int y,int z,float val);
  
 protected:
  int size[3];
  int sliceSize,numVoxels;
  virtual void copySlice(int z,float* slice) = 0;
  virtual void storeSlice(int z,float* slice) = 0;
  
 public:
  MarchableVolume();
  virtual ~MarchableVolume();
  int* getSize();
  float d(int x,int y,int z);
  void d(int x,int y,int z,float val);
  virtual int getKnownFlag(int x,int y,int z);  //warning: this may be slow
  virtual int getCarvedFlag(int x,int y,int z); //warning: this may be slow
  virtual int readFile(char* filename) = 0;
  virtual int writeFile(char* filename) = 0;
  virtual int isFloatingType() = 0;
  static MarchableVolume* createVolume(char* filename);
};

class FloatVolume : public MarchableVolume
{
 protected:
  float* data;
  void copySlice(int z,float* slice);
  void storeSlice(int z,float* slice);
  
 public:
  FloatVolume();
  ~FloatVolume();
  int readFile(char* filename);
  int writeFile(char* filename);
  int isFloatingType();
};

/*class VRIPVolume : public MarchableVolume
{
 protected:
  OccGridRLE *ogSrc;
  void copySlice(int z,float* slice);
  void storeSlice(int z,float* slice);
  
 public:
  VRIPVolume();
  ~VRIPVolume();
  int getKnownFlag(int x,int y,int z);
  int readFile(char* filename);
  int writeFile(char* filename);
  int isFloatingType();
};*/

class Bit2Volume : public MarchableVolume
{
 protected:
  bitc positive;
  bitc known;
  void copySlice(int z,float* slice);
  void storeSlice(int z,float* slice);
  
 public:
  Bit2Volume();
  ~Bit2Volume();
  int getKnownFlag(int x,int y,int z);
  int getCarvedFlag(int x,int y,int z);
  int readFile(char* filename);
  int writeFile(char* filename);
  int isFloatingType();
};

/*class OctreeVolume : public MarchableVolume
{
 protected:
  octree* dataroot;
  int treeHeight;
  void copySlice(int z,float* slice);
  void storeSlice(int z,float* slice);
  
 public:
  OctreeVolume();
  OctreeVolume(octree* rootOfOctree);
  ~OctreeVolume();
  int getKnownFlag(int x,int y,int z);
  int getCarvedFlag(int x,int y,int z);
  int readFile(char* filename);
  int writeFile(char* filename);
  int isFloatingType();
  };*/

class ByteVolume : public MarchableVolume
{
 protected:
  unsigned char* data;
  void copySlice(int z,float* slice);
  void storeSlice(int z,float* slice);
  
 public:
  ByteVolume();
  ~ByteVolume();
  int readFile(char* filename);
  int writeFile(char* filename);
  int isFloatingType();
};

class RawVolume : public MarchableVolume
{
protected:
    unsigned short* data;
    void copySlice(int z,float* slice);
    void storeSlice(int z,float* slice);
    
public:
    RawVolume();
    ~RawVolume();
    int readFile(char* filename);
    int writeFile(char* filename);
    int isFloatingType();
};

