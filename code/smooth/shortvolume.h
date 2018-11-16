/*
 *  shortvolume.h
 *  
 *
 *  Created by James Vanderhyde on Wed Apr 7 2004.
 *
 */

#include <iostream.h>
#include <queue>
#include <vector>

#include "bitc.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)<(b))?(b):(a))
#define SGN(x) (((x)<0)?-1:(((x)>0)?1:0))
#define ABS(x) (((x)<0)?-(x):(x))
#define BIGNUM 1e20
#define SMALLNUM 1e-15

class pqitem
{
    friend ostream& operator<< (ostream& out, const pqitem& i);
public:
    short priority;
    int x,y,z;
    pqitem ();
    pqitem ( short p, int xx, int yy, int zz );
    operator short();
};

typedef std::priority_queue < pqitem, std::vector<pqitem>, std::greater<float> > minqueue;
typedef std::priority_queue < pqitem, std::vector<pqitem>, std::less<float> > maxqueue;
typedef std::queue<pqitem> voxelqueue;
typedef std::vector<pqitem> pqvector;
typedef std::vector<int> intvector;


class volume
{
protected:
    int size[3];
    short* data;
	bitc queued;
	bitc carved;
	bitc known; //means the sign (inside/outside) is known
	pqitem minvoxel;
	bitc topoinfo;
    int animationOn;
    int frameNumber;
	intvector patchCentersInside,patchCentersOutside;
public:
    volume();
    ~volume();
    int* getSize();
	int getVoxelIndex(int x,int y,int z);
	void getVoxelLocFromIndex(int index,int* x,int* y,int* z);
    float d(int x,int y,int z);
	short getVoxel(int x,int y,int z);
	void setVoxel(int x,int y,int z,short val);
    int readTopoinfoFile();
    void changeAllSigns();
    int getNeighbors6(int x,int y,int z,int* neighbors,pqitem* neighborsQ=NULL);
    int getNeighbors26(int x,int y,int z,int* neighbors,pqitem* neighborsQ=NULL);
	void addToBoundary(int x,int y,int z,int index,maxqueue& boundary);
    int topologyCheckInside(int x,int y,int z);
    int topologyCheckOutside(int x,int y,int z);
    void carveVoxelInside(int x,int y,int z,int index,maxqueue& boundary,int* neighbors,pqitem* neighborsQ);
    void carveVoxelOutside(int x,int y,int z,int index,maxqueue& boundary,int* neighbors,pqitem* neighborsQ);
	int openFeatureInside(intvector& forLater,maxqueue& boundary,int* neighbors,pqitem* neighborsQ);
	int openFeatureOutside(intvector& forLater,maxqueue& boundary,int* neighbors,pqitem* neighborsQ);
	void constructInitialOuterBoundary(maxqueue& outerBoundary);
	
	void calcDistances();
	void carveSimultaneously(int numFeaturesToOpen=0);
	int guessNumExtraFeatures(int numFeaturesToOpen=0);
	void carveOutside(int numFeaturesToOpen=0);
	void fixVolumeTopology(int sideToKeep);
	void carveInside(int numFeaturesToOpen=0);
	
    void turnOnAnimation();
    void turnOffAnimation();
    void renderVolume(int doItAnyway=0);
    void printVolume(ostream& out);
	int readVFile(istream& in,float isovalue);
	int writeVFile(ostream& out);
	int readV2File(istream& in);
	int writeV2File(ostream& out);
	int readVRIFile(char* filename);
    int readFile(char* filename,float isovalue=0.0);
    int writeFile(char* filename);
};

