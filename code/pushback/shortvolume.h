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

class pqitem
{
    friend ostream& operator<< (ostream& out, const pqitem& i);
	friend class pqitemcomparator;
public:
    short priority;
    int x,y,z;
    pqitem ();
    pqitem ( short p, int xx, int yy, int zz );
    operator short();
};

class pqitemlesscomparator
{
public :
    int operator()(const pqitem &a, const pqitem &b);
};

class pqitemgreatercomparator
{
public :
    int operator()(const pqitem &a, const pqitem &b);
};

//typedef std::priority_queue < pqitem, std::vector<pqitem>, std::greater<short> > minqueue;
//typedef std::priority_queue < pqitem, std::vector<pqitem>, std::less<short> > maxqueue;
typedef std::priority_queue < pqitem, std::vector<pqitem>, pqitemgreatercomparator > minqueue;
typedef std::priority_queue < pqitem, std::vector<pqitem>, pqitemlesscomparator > maxqueue;
typedef std::queue<pqitem> voxelqueue;
typedef std::vector<pqitem> pqvector;
typedef std::vector<int> intvector;


class volume
{
protected:
    int size[3];
    short* data;
	int* initialOrder;
	int* carvedInsideOrder;
	int* carvedOutsideOrder;
	int* parentInside;
	int* parentOutside;
	int* lifeSpans;
	bitc queued;
	bitc carved;
	bitc known; //means the sign (inside/outside) is known
	pqitem minvoxel;
	bitc topoinfo;
    int animationOn;
    int frameNumber;
	void mergesort(int start,int end);
	void selectionsort(int start,int end);
public:
	volume();
    volume(int sizex,int sizey,int sizez);
    ~volume();
    int* getSize();
	int getVoxelIndex(int x,int y,int z);
	void getVoxelLocFromIndex(int index,int* x,int* y,int* z);
    float d(int x,int y,int z);
	float d(int index);
	short getVoxel(int x,int y,int z);
	void setVoxel(int x,int y,int z,short val);
	int getKnown(int index);
	void setKnown(int index, int val);
    int readTopoinfoFile();
    void changeAllSigns();
	short findMinimum();
	void addToAll(short val);
	
	int getInitialOrderAt(int index);
	int getCarvedInsideOrderAt(int index);
	int getCarvedOutsideOrderAt(int index);
	int getParentInsideAt(int index);
	int getParentOutsideAt(int index);
	int getLifeSpansAt(int index);
	
    int getNeighbors6(int x,int y,int z,int* neighbors,pqitem* neighborsQ=NULL);
    int getNeighbors26(int x,int y,int z,int* neighbors,pqitem* neighborsQ=NULL);
	void addToBoundaryInside(int x,int y,int z,int index,minqueue& boundary,short adjustment=0);
	void addToBoundaryOutside(int x,int y,int z,int index,maxqueue& boundary,short adjustment=0);
    int topologyCheckInside(int x,int y,int z);
    int topologyCheckOutside(int x,int y,int z);
    void carveVoxelInside(int x,int y,int z,int index,minqueue& boundary,int* neighbors,pqitem* neighborsQ);
    void carveVoxelOutside(int x,int y,int z,int index,maxqueue& boundary,int* neighbors,pqitem* neighborsQ);
	void constructInitialOuterBoundary(maxqueue& outerBoundary);
	void invertPermutation(int* order);
	int isInitiallyCritical(int index);
	int isCriticalInside(int* order,int index);
	int isCriticalOutside(int* order,int index);
	int countCriticalsInside(int* order);
	int countCriticalsOutside(int* order);
	int computeLifeSpan(int* parent,int index);
	
	void calcDistances();
	void sortVoxels();
	void carveSimultaneously(int numFeaturesToOpen=0);
	void carveFromInside(int lifeSpanThreshold=-1,int calcLifeSpans=1);
	void carveFromOutside(int lifeSpanThreshold=-1,int calcLifeSpans=1);
	void countCriticals();
	void printLifeSpans(int minLifeSpan=0);
	void fixVolumeTopology(int sideToKeep);
	
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

