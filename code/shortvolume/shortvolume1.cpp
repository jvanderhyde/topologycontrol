/*
 *  shortvolume.cpp
 *  
 *
 *  Created by James Vanderhyde on Wed Apr 7 2004.
 *
 */

#include <string.h>
#include <stdio.h>
#include <fstream.h>
//#include <iostream.h>

#include <assert.h>
#include <time.h>

#include "shortvolume.h"

#include "volfill/OccGridRLE.h"

pqitem::pqitem () : priority(0),x(0),y(0),z(0) {}
pqitem::pqitem ( short p, int xx, int yy, int zz ) : priority(p),x(xx),y(yy),z(zz) {}

pqitem::operator short()
{
    return priority;
}

ostream& operator<< (ostream& out, const pqitem& i)
{
    out << '(' << i.x << ',' << i.y << ',' << i.z << ')' << ':' << i.priority;
    return out;
}

int floorlogbase2(int x)
{
    int r=0;
    while (x>>++r > 0) ;
    return r-1;
}

int ceillogbase2(int x)
{
    int r=floorlogbase2(x);
    if (x-(1<<r) > 0) r++;
    return r;
}

volume::volume()
{
    data=NULL;
    size[0]=size[1]=size[2]=0;
	minvoxel.priority=0;
    animationOn=0;
    frameNumber=0;
}

volume::~volume()
{
    if (data) delete[] data;
	queued.freespace();
	carved.freespace();
}

int* volume::getSize()
{
    return size;
}

int volume::getVoxelIndex(int x,int y,int z)
{
    return (z*size[1]+y)*size[0]+x;
}

void volume::getVoxelLocFromIndex(int index,int* x,int* y,int* z)
{
	*x=index%size[0];
	*y=(index/size[0])%size[1];
	*z=(index/size[0])/size[1];
}

float volume::d(int x,int y,int z)
{
    if ((x<0) || (y<0) || (z<0) || (x>=size[0]) || (y>=size[1]) || (z>=size[2])) return BIGNUM;
	return data[getVoxelIndex(x,y,z)];
}

short volume::getVoxel(int x,int y,int z)
{
	return data[getVoxelIndex(x,y,z)];
}

void volume::setVoxel(int x,int y,int z,short val)
{
	data[getVoxelIndex(x,y,z)]=val;
}

int volume::readTopoinfoFile()
{
    ifstream fin("topoinfo_vertex");
    if (!fin)
    {
        cerr << "Can't open topoinfo_vertex\n";
        return 1;
    }
	topoinfo.read(fin,1024*1024*64); // 2^26 = 1024*1024*64
    return 0;
}

//Swaps the sign on every voxel in the volume
void volume::changeAllSigns()
{
    for (int i=0; i<size[2]*size[1]*size[0]; i++)
		data[i]=-data[i];
}

//Returns the number of non-null, in-range neighbors around the given voxel.
// The neighbors are stored in both neighborsQ and neighbors.
int volume::getNeighbors26(int x,int y,int z,int* neighbors,pqitem* neighborsQ)
{
    int n=0;
    int ox,oy,oz;
    for (oz=-1; oz<=1; oz++)
        if ((z+oz>=0) && (z+oz<size[2]))
            for (oy=-1; oy<=1; oy++)
                if ((y+oy>=0) && (y+oy<size[1]))
                    for (ox=-1; ox<=1; ox++)
                        if ((x+ox>=0) && (x+ox<size[0]))
                            if ((oz!=0) || (oy!=0) || (ox!=0))
                            {
                                neighbors[n]=getVoxelIndex(x+ox,y+oy,z+oz);
								if (neighborsQ) neighborsQ[n]=pqitem(data[neighbors[n]],x+ox,y+oy,z+oz);
								n++;
                            }
    return n;
}

//Returns the number of non-null, in-range neighbors around the given voxel.
// The neighbors are stored in both neighborsQ and neighbors.
int volume::getNeighbors6(int x,int y,int z,int* neighbors,pqitem* neighborsQ)
{
    int n=0;
    if (x-1>=0)
    {
		neighbors[n]=getVoxelIndex(x-1,y,z);
		if (neighborsQ) neighborsQ[n]=pqitem(data[neighbors[n]],x-1,y,z);
		n++;
    }
    if (x<size[0]-1)
    {
		neighbors[n]=getVoxelIndex(x+1,y,z);
		if (neighborsQ) neighborsQ[n]=pqitem(data[neighbors[n]],x+1,y,z);
		n++;
    }
    if (y-1>=0)
    {
		neighbors[n]=getVoxelIndex(x,y-1,z);
		if (neighborsQ) neighborsQ[n]=pqitem(data[neighbors[n]],x,y-1,z);
		n++;
    }
    if (y<size[1]-1)
    {
		neighbors[n]=getVoxelIndex(x,y+1,z);
		if (neighborsQ) neighborsQ[n]=pqitem(data[neighbors[n]],x,y+1,z);
		n++;
    }
    if (z-1>=0)
    {
		neighbors[n]=getVoxelIndex(x,y,z-1);
		if (neighborsQ) neighborsQ[n]=pqitem(data[neighbors[n]],x,y,z-1);
		n++;
    }
    if (z<size[2]-1)
    {
		neighbors[n]=getVoxelIndex(x,y,z+1);
		if (neighborsQ) neighborsQ[n]=pqitem(data[neighbors[n]],x,y,z+1);
		n++;
    }
    return n;
}

void volume::addToBoundary(int x,int y,int z,int index,maxqueue& boundary)
{
	boundary.push(pqitem(ABS(data[index]),x,y,z));
	queued.setbit(index);
}

//Returns true if removing the specified voxel changes the topology.
// We use a look-up table based on the state of the 26 neighbors.
// For each neighbor, we set the bit if the neighbor is not carved.
int volume::topologyCheckInside(int x,int y,int z)
{
    //In this case, positive voxels must be considered not carved.
    int topoType=0,bit=0;
    int ox,oy,oz;
    int voxelIndex;
    for (oz=-1; oz<=1; oz++)
        for (oy=-1; oy<=1; oy++)
            for (ox=-1; ox<=1; ox++)
                if ((oz!=0) || (oy!=0) || (ox!=0))
                {
                    if ((z+oz>=0) && (z+oz<size[2]) &&
                        (y+oy>=0) && (y+oy<size[1]) &&
                        (x+ox>=0) && (x+ox<size[0]))
                    {
                        voxelIndex=getVoxelIndex(x+ox,y+oy,z+oz);
                        //If the voxel is not carved (or if it is positive), we set the bit.
                        if ((!carved.getbit(voxelIndex)) || ((known.getbit(voxelIndex)) && (data[voxelIndex]>0)))
                            topoType |= (1<<bit);
                    }
                    else topoType |= (1<<bit); //out of range is considered positive, so we set the bit
                    bit++;
                }
	topoType ^= ((1<<26)-1); //inside is dual to outside
    return !!(topoinfo.getbit(topoType));
}

int volume::topologyCheckOutside(int x,int y,int z)
{
    //In this case, negative voxels must be considered not carved.
    int topoType=0,bit=0;
    int ox,oy,oz;
    int voxelIndex;
    for (oz=-1; oz<=1; oz++)
        for (oy=-1; oy<=1; oy++)
            for (ox=-1; ox<=1; ox++)
                if ((oz!=0) || (oy!=0) || (ox!=0))
                {
                    if ((z+oz>=0) && (z+oz<size[2]) &&
                        (y+oy>=0) && (y+oy<size[1]) &&
                        (x+ox>=0) && (x+ox<size[0]))
                    {
                        voxelIndex=getVoxelIndex(x+ox,y+oy,z+oz);
                        //If the voxel is not carved (or if it is negative), we set the bit.
                        if ((!carved.getbit(voxelIndex)) || ((known.getbit(voxelIndex)) && (data[voxelIndex]<0)))
                            topoType |= (1<<bit);
                    }
                    //out of range is considered carved, so we do not set the bit
                    bit++;
                }
	return !!(topoinfo.getbit(topoType));
}

void volume::carveVoxelInside(int x,int y,int z,int index,maxqueue& boundary,int* neighbors,pqitem* neighborsQ)
{
    int n,num;
    short val;
	
	//After we carve, we know the sign of the voxel.
	carved.setbit(index);
    known.setbit(index);
    val=data[index];
	data[index]=-ABS(val); //negative

	//check the neighbors
    num=getNeighbors26(x,y,z,neighbors,neighborsQ);
    for (n=0; n<num; n++)
    {
		//If the neighbor is not carved, not in the queue, and negative or unknown,
        if ((!carved.getbit(neighbors[n])) &&
            (!queued.getbit(neighbors[n]))  &&
            ((data[neighbors[n]] < 0) || (!known.getbit(neighbors[n]))))
        {
            //then add this neighbor to priority queue.
			addToBoundary(neighborsQ[n].x,neighborsQ[n].y,neighborsQ[n].z,
						  neighbors[n],boundary);
			//This actually sort of determines the sign, so we're going to set known right now.
			/*known.setbit(neighbors[n]);
			val=data[neighbors[n]];
			data[neighbors[n]]=-ABS(val); //negative*/
        }
    }

    if (animationOn) renderVolume();
}

void volume::carveVoxelOutside(int x,int y,int z,int index,maxqueue& boundary,int* neighbors,pqitem* neighborsQ)
{
    int n,num;
    short val;
	
	//After we carve, we know the sign of the voxel.
	carved.setbit(index);
    known.setbit(index); 
    val=data[index];
	data[index]=ABS(val); //positive
	
	//check the neighbors
    num=getNeighbors26(x,y,z,neighbors,neighborsQ);
    for (n=0; n<num; n++)
    {
		//If the neighbor is not carved, not in the queue, and positive or unknown,
        if ((!carved.getbit(neighbors[n])) &&
            (!queued.getbit(neighbors[n]))  &&
            ((data[neighbors[n]] > 0) || (!known.getbit(neighbors[n]))))
        {
            //then add this neighbor to priority queue.
			addToBoundary(neighborsQ[n].x,neighborsQ[n].y,neighborsQ[n].z,
						  getVoxelIndex(neighborsQ[n].x,neighborsQ[n].y,neighborsQ[n].z),
						  boundary);
			//This actually sort of determines the sign, so we're going to set known right now.
			/*known.setbit(neighbors[n]);
			val=data[neighbors[n]];
			data[neighbors[n]]=ABS(val); //positive*/
        }
    }
	
    if (animationOn) renderVolume();
}



int volume::openFeatureInside(intvector& forLater,maxqueue& boundary,int* neighbors,pqitem* neighborsQ)
{
	int x,y,z,i,index,maxindex=-1;
	short val;
	pqitem top;
	top.priority=0;
	int numNotCarvedNeg=0;
	
    //Find the max uncarved topology-changing voxel
	/*for (i=0; i<forLater.size(); i++)
	{
		index=forLater[i];
		val=ABS(data[index]);
		if (!carved.getbit(index))
		{
			getVoxelLocFromIndex(index,&x,&y,&z);
			cout << pqitem(data[index],x,y,z) << '\n';
		}
		if ((!carved.getbit(index)) && (val>top.priority))
		{
			getVoxelLocFromIndex(index,&top.x,&top.y,&top.z);
			top.priority=val;
			maxindex=index;
		}
	}
	cout << '\n';*/
	
    //Find the global min uncarved known negative voxel
	for (index=0; index<size[2]*size[1]*size[0]; index++)
	{
		val=ABS(data[index]);
		if ((!carved.getbit(index)) && (data[index]<0))
		{
			getVoxelLocFromIndex(index,&x,&y,&z);
			numNotCarvedNeg++;
			//cout << pqitem(data[index],x,y,z) << '\n';
		}
		if ((!carved.getbit(index)) && (val>top.priority) /*&& (known.getbit(index))*/ && (data[index]<0))
		{
			getVoxelLocFromIndex(index,&top.x,&top.y,&top.z);
			top.priority=val;
			maxindex=index;
		}
	}
	
	//Carve it
	if (maxindex>-1)
	{
		cout << "Max uncarved inside voxel: " << top << "\n";
		//cout << "Total uncarved inside voxels: " << numNotCarvedNeg << '\n';
		carveVoxelInside(top.x,top.y,top.z,maxindex,boundary,neighbors,neighborsQ);
		//cout << '\n';
		return 1;
	}
	else
	{
		cout << "No feature to open from inside.\n";
		return 0;
	}
			
}

int volume::openFeatureOutside(intvector& forLater,maxqueue& boundary,int* neighbors,pqitem* neighborsQ)
{
	int x,y,z,i,index,maxindex=-1;
	short val;
	pqitem top;
	top.priority=0;
	int numNotCarvedPos=0;
	
    //Find the max uncarved topology-changing voxel
	/*for (i=0; i<forLater.size(); i++)
	{
		index=forLater[i];
		val=ABS(data[index]);
		if (!carved.getbit(index))
		{
			getVoxelLocFromIndex(index,&x,&y,&z);
			cout << pqitem(data[index],x,y,z) << '\n';
		}
		if ((!carved.getbit(index)) && (val>top.priority))
		{
			getVoxelLocFromIndex(index,&top.x,&top.y,&top.z);
			top.priority=val;
			maxindex=index;
		}
	}
	cout << '\n';*/
	
    //Find the global max uncarved known positive voxel
	for (index=0; index<size[2]*size[1]*size[0]; index++)
	{
		val=ABS(data[index]);
		if ((!carved.getbit(index)) && (data[index]>0))
		{
			getVoxelLocFromIndex(index,&x,&y,&z);
			numNotCarvedPos++;
			//cout << pqitem(data[index],x,y,z) << '\n';
		}
		if ((!carved.getbit(index)) && (val>top.priority) /*&& (known.getbit(index))*/ && (data[index]>0))
		{
			getVoxelLocFromIndex(index,&top.x,&top.y,&top.z);
			top.priority=val;
			maxindex=index;
		}
	}
	
	//Carve it
	if (maxindex>-1)
	{
		cout << "Max uncarved outside voxel: " << top << "\n";
		//cout << "Total uncarved outside voxels: " << numNotCarvedPos << '\n';
		carveVoxelOutside(top.x,top.y,top.z,maxindex,boundary,neighbors,neighborsQ);
		//cout << '\n';
		return 1;
	}
	else
	{
		cout << "No feature to open from outside.\n";
		return 0;
	}
	
}

void volume::calcDistances()
{
    cout << "Calculating distance function..."; cout.flush();
	
	std::queue<int> q;
	int x,y,z;
    int n,num;
    int neighbors[6];
    int index;
	int onIsosurface;
    int top;
	short curdistance=0;
	int layerSize=0,newLayerSize=0;
	short signedDistance;

	for (index=0; index<size[2]*size[1]*size[0]; index++)
	{
		if (known.getbit(index))
		{
			getVoxelLocFromIndex(index,&x,&y,&z);
			onIsosurface=0;
			num=getNeighbors6(x,y,z,neighbors);
			if ((data[index]<0) && 
				((x==0) || (y==0) || (z==0) || (x==size[0]-1) || (y==size[1]-1) || (z==size[2]-1)))
				onIsosurface=1;
			if (data[index]<0)
				for (n=0; n<num; n++)
					if ((known.getbit(neighbors[n])) && (data[neighbors[n]]>0))
						onIsosurface=1;
			if (data[index]>0)
				for (n=0; n<num; n++)
					if ((known.getbit(neighbors[n])) && (data[neighbors[n]]<0))
						onIsosurface=1;
			if (onIsosurface)
			{	
				q.push(index);
				queued.setbit(index);
				newLayerSize++;
			}
		}
   }
	
    while (!q.empty())
    {
		//keep track of how many at the current distance are pushed
		if (layerSize==0)
		{
			layerSize=newLayerSize;
			newLayerSize=0;
			curdistance++;
		}
		
        //pop voxel from queue
        top=q.front();
        q.pop();
        index=top;
		getVoxelLocFromIndex(index,&x,&y,&z);
		signedDistance=SGN(data[index])*curdistance;
		if ((signedDistance<minvoxel.priority) && (known.getbit(index)))
		{
			//record minimum signed voxel for future use (inside carving starting point)
			minvoxel=pqitem(signedDistance,x,y,z);
		}
		
		//record calculated distance in voxel data, leaving sign unchanged if the sign is known
		if (known.getbit(index)) data[index]=signedDistance;
		else data[index]=curdistance;
		layerSize--;
        
        //push neighbors onto queue
        num=getNeighbors6(x,y,z,neighbors);
        for (n=0; n<num; n++)
        {
            if (!queued.getbit(neighbors[n]))
            {
                q.push(neighbors[n]);
                queued.setbit(neighbors[n]);
				newLayerSize++;
            }
        }
    }
	
	queued.clearall();
    cout << "done.\n";
}

//Add all positive or unknown voxels to boundary from 6 faces of bounding volume
void volume::constructInitialOuterBoundary(maxqueue& outerBoundary)
{
	int x,y,z,index;
	for (z=0; z<1; z++)
		for (y=0; y<size[1]; y++)
			for (x=0; x<size[0]; x++)
				if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
					addToBoundary(x,y,z,index,outerBoundary);
	for (z=size[2]-1; z<size[2]; z++)
		for (y=0; y<size[1]; y++)
			for (x=0; x<size[0]; x++)
				if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
					addToBoundary(x,y,z,index,outerBoundary);
	for (z=1; z<size[2]-1; z++)
		for (y=0; y<1; y++)
			for (x=1; x<size[0]; x++)
				if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
					addToBoundary(x,y,z,index,outerBoundary);
	for (z=1; z<size[2]-1; z++)
		for (y=size[1]-1; y<size[1]; y++)
			for (x=0; x<size[0]-1; x++)
				if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
					addToBoundary(x,y,z,index,outerBoundary);
	for (z=1; z<size[2]-1; z++)
		for (y=0; y<size[1]-1; y++)
			for (x=0; x<1; x++)
				if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
					addToBoundary(x,y,z,index,outerBoundary);
	for (z=1; z<size[2]-1; z++)
		for (y=1; y<size[1]; y++)
			for (x=size[0]-1; x<size[0]; x++)
				if ((data[index=getVoxelIndex(x,y,z)]>0) || (!known.getbit(index)))
					addToBoundary(x,y,z,index,outerBoundary);
}

void volume::carveSimultaneously(int numFeaturesToOpen)
{
    cout << "Carving... "; cout.flush();
	
	maxqueue innerBoundary,outerBoundary;
    pqitem top,toppos,topneg;
	int voxelIndex;
	int neighbors[26];
    pqitem neighborsQ[26];
    int numCarvedNeg=0,numCarvedPos=0;
    int useNegative;
	int numFeatures=-1; //set up to be corrected when entering the while loop
	int numFeaturesOpened=0;
	intvector innerForLater,outerForLater;
	int anyUncarvedVoxelsLeft=1;
	
	carved.clearall();
	
	//set up outer boundary
	constructInitialOuterBoundary(outerBoundary);
	//printVolume(cout);
	
	//carve minimum voxel to start inner boundary
	//pqitem startvoxel(-1,192,140,200);
	pqitem startvoxel(-1,size[0]/2,size[1]/2,size[2]/2);
	//pqitem startvoxel(-1,minvoxel.x,minvoxel.y,minvoxel.z);
	voxelIndex=getVoxelIndex(startvoxel.x,startvoxel.y,startvoxel.z);
	if ((known.getbit(voxelIndex)) && (data[voxelIndex]>0))
		cout << "Carving from inside starting at a known positive voxel!!\n";
	carveVoxelInside(startvoxel.x,startvoxel.y,startvoxel.z,voxelIndex,innerBoundary,neighbors,neighborsQ);
	//cout << "Starting voxel: (" << startvoxel.x << ',' << startvoxel.y << ',' << startvoxel.z << ")... "; cout.flush();
	numCarvedNeg++;
	//printVolume(cout);
	if (animationOn) cout << '\n';

	//Loop over the desired number of features.
	// When the check is made, numFeatures is one less than the actual number of 
	// topology-altering operations that have been performed, but it's incremented
	// immediately upon entering the loop.
	// This is set up so that carving proceeds until no voxel can be carved without
	// changing topology.
	// So the meaning of the variable numFeatures changes throughout the execution of 
	// the loop. This is annoying, but necessary to avoid duplicating code.
	while ((numFeatures<numFeaturesToOpen) && (anyUncarvedVoxelsLeft))
	{
		numFeatures++;
		
		//carve without changing topology
		while ((!outerBoundary.empty()) || (!innerBoundary.empty())) //pq not empty
		{
			//pop supervoxel from top of appropriate priority queue
			if (outerBoundary.empty())
			{
				useNegative=1;
				top=innerBoundary.top();
				innerBoundary.pop();
			}
			else if (innerBoundary.empty())
			{
				useNegative=0;
				top=outerBoundary.top();
				outerBoundary.pop();
			}
			else
			{
				toppos=outerBoundary.top();
				topneg=innerBoundary.top();
				if (topneg>toppos)
				{
					useNegative=1;
					top=topneg;
					innerBoundary.pop();
				}
				else
				{
					useNegative=0;
					top=toppos;
					outerBoundary.pop();
				}
			}
			voxelIndex=getVoxelIndex(top.x,top.y,top.z);
			queued.resetbit(voxelIndex);
			
			//check whether removing this voxel changes the topology
			if (useNegative)
			{
				if (!topologyCheckInside(top.x,top.y,top.z))
				{
					//If it doesn't, carve this voxel.
					carveVoxelInside(top.x,top.y,top.z,voxelIndex,innerBoundary,neighbors,neighborsQ);
					numCarvedNeg++;
					//printVolume(cout);
				}
				else
				{
					//Otherwise, save it for opening features later.
					innerForLater.push_back(voxelIndex);
					//cout << "can't carve\n";
				}
			}
			else
			{
				if (!topologyCheckOutside(top.x,top.y,top.z))
				{
					//If it doesn't, carve this voxel.
					carveVoxelOutside(top.x,top.y,top.z,voxelIndex,outerBoundary,neighbors,neighborsQ);
					numCarvedPos++;
					//printVolume(cout);
				}
				else
				{
					//Otherwise, save it for opening features later.
					outerForLater.push_back(voxelIndex);
					//cout << "can't carve\n";
				}
			}
		}
		
		//open a new feature, if desired
		if (numFeatures<numFeaturesToOpen)
		{
			if (animationOn) renderVolume(1);
			//cout << numCarvedNeg << " carved so far from inside, ";
			//cout << numCarvedPos << " carved so far from outside.\n";
			int openedInside,openedOutside;
			openedInside=openFeatureInside(innerForLater,innerBoundary,neighbors,neighborsQ);
			openedOutside=openFeatureOutside(outerForLater,outerBoundary,neighbors,neighborsQ);
			numCarvedNeg+=openedInside;
			numCarvedPos+=openedOutside;
			if (openedInside) numFeaturesOpened++;
			if (openedOutside) numFeaturesOpened++;
			if (openedInside+openedOutside==0) anyUncarvedVoxelsLeft=0;
		}
		
	}
	
    cout << numCarvedNeg << " carved from inside, ";
    cout << numCarvedPos << " carved from outside.\n";
	if (numFeaturesToOpen>0) cout << numFeaturesOpened/2 << " topology-altering operations performed.\n";
}

void volume::fixVolumeTopology(int sideToKeep)
{
    cout << "Fixing volume..."; cout.flush();

	int side;
	int index;
    int numChanged=0;

	//Determine which patches to keep if not specified by user
	if (sideToKeep==0)
	{
		//Check the uncarved, known voxels to see whether there are more negative or positive.
		int numPositive=0,numNegative=0;
		for (index=0; index<size[0]*size[1]*size[2]; index++)
			if ((!carved.getbit(index)) && (known.getbit(index)))
			{
				if (data[index]>0) numPositive++;
				if (data[index]<0) numNegative++;
			}
		if (numPositive>numNegative) side=-1;
		else side=1;
	}
	else side=SGN(sideToKeep);
	
    //Any voxels that are not carved are made the opposite sign of sideToKeep.
	for (index=0; index<size[0]*size[1]*size[2]; index++)
		if (!carved.getbit(index))
		{
			if (SGN(data[index])==side) numChanged++;
			data[index]=-side;
		}

	if (side>0) cout << " " << numChanged << " positive voxels changed to negative.\n";
	if (side<0) cout << " " << numChanged << " negative voxels changed to positive.\n";
}


void volume::turnOnAnimation()
{
    animationOn=1;
}

void volume::turnOffAnimation()
{
    animationOn=0;
}

//Hook for rendering the volume at every step for animation purposes
void volume::renderVolume(int doItAnyway)
{
    frameNumber++;
    if ((!doItAnyway) && (frameNumber%10000000!=0)) return;
    
    //int x,y,z,voxelIndex,index;
    //int coords[3];
    //coords[0]=size[0]; coords[1]=size[1]; coords[2]=size[2]; 
    //int slicesize=coords[0]*coords[1];
    //float* vdata=new float[slicesize];
    
    cout << "Rendering frame " << frameNumber << "...";
    char filename[30];
    sprintf(filename,"frames/frame_%.9d.v2",frameNumber);
    ofstream fout(filename);
    
    if (fout)
    {
        /*fout.write((char*)coords,sizeof(coords));
		voxelIndex=0;
        for (z=0; z<coords[2]; z++)
        {
            index=0;
            for (y=0; y<coords[1]; y++)
                for (x=0; x<coords[0]; x++)
                {
					if (carved.getbit(voxelIndex))
						vdata[index]=1.0;
					else
						vdata[index]=-1.0;
                    index++;
					voxelIndex++;
                }
            fout.write((char*)vdata,slicesize*sizeof(float));
        }*/
		writeV2File(fout);
        fout.close();
        cout << "done." << '\n';
    }
    else cout << "output to file \"" << filename << "\" failed!" << '\n';

    //delete[] vdata;
}

void volume::printVolume(ostream& out)
{
    int x,y,z;
    int maxcoord=1<<ceillogbase2(MAX(MAX(size[0],size[1]),size[2]));
    int voxelIndex;

    for (x=0; x<size[0]; x++)
    {
		//out << '-';
        out << "----" << '\t';
    }
    out << '\n';

	voxelIndex=0;
        for (y=0; y<size[1]; y++)
        {
    for (z=0; z<size[2]; z++)
    {
            for (x=0; x<size[0]; x++)
            {
				voxelIndex=getVoxelIndex(x,y,z);
				if ((queued.getbit(voxelIndex)) && (data[voxelIndex]>0)) out << '|';
				else if ((queued.getbit(voxelIndex)) && (data[voxelIndex]<0)) out << '/';
				else if ((carved.getbit(voxelIndex)) && (data[voxelIndex]>0)) out << 'o';
				else if ((carved.getbit(voxelIndex)) && (data[voxelIndex]<0)) out << 'x';
				else if (data[voxelIndex]>0) out << ' ';
				else if (data[voxelIndex]<0) out << '.';
				else out << '?';
				//out << data[voxelIndex];
				//if (carved.getbit(voxelIndex)) out << '*';
                //out << '\t';
                //voxelIndex++;
            }
            out << '\t';
        }
        out << '\n';
    }
}

int volume::readVFile(istream& in,float isovalue)
{
    in.read((char*)size,sizeof(size));
	int sliceSize=size[1]*size[0],numVoxels=size[2]*sliceSize;
	data=new short[numVoxels];
	queued.setbit(numVoxels-1); queued.resetbit(numVoxels-1);
	carved.setbit(numVoxels-1); carved.resetbit(numVoxels-1);
	known.setbit(numVoxels-1); known.resetbit(numVoxels-1);

	float* slice=new float[sliceSize];
	float val;
	int x,y,z,voxelIndex=0,index;
	for (z=0; z<size[2]; z++)
	{
		index=0;
		in.read((char*)slice,sliceSize*sizeof(float));
		for (y=0; y<size[1]; y++) for (x=0; x<size[0]; x++)
		{				
			val=slice[index++]-isovalue;
			if (val!=0.0) known.setbit(voxelIndex);
			data[voxelIndex++]=SGN(val);
		}
	}

	delete[] slice;
	return 0;
}

int volume::writeVFile(ostream& out)
{
    out.write((char*)size,sizeof(size));
	
	int sliceSize=size[1]*size[0];
	float* slice=new float[sliceSize];
	int x,y,z,voxelIndex=0,index;
	
	for (z=0; z<size[2]; z++)
	{
		index=0;
		for (y=0; y<size[1]; y++)
			for (x=0; x<size[0]; x++)
				slice[index++]=(float)data[voxelIndex++];
		out.write((char*)slice,sliceSize*sizeof(float));
	}

	delete[] slice;
	return 0;
}

//Loads 2 bits per voxel.
// The first set of bits tells whether the voxel is inside (0) or outside (1).
// The second set of bits is copied into the known flags.
int volume::readV2File(istream& in)
{
    in.read((char*)size,sizeof(size));
	int numVoxels=size[2]*size[1]*size[0];
	data=new short[numVoxels];
	queued.setbit(numVoxels-1); queued.resetbit(numVoxels-1);
	carved.setbit(numVoxels-1); carved.resetbit(numVoxels-1);
	known.setbit(numVoxels-1); known.resetbit(numVoxels-1);

	bitc positive(numVoxels);
	positive.read(in,numVoxels);
	known.read(in,numVoxels);
    cout << "processing..."; cout.flush();
	
	for (int index=0; index<numVoxels; index++)
		data[index]=(known.getbit(index)?(positive.getbit(index)?1:-1):0);
		
	return 0;
}

//Saves 2 bits per voxel.
// Each bit in the first set indicates the voxel is positive (or unknown).
// Each bit in the second set indicates the voxel has been carved.
int volume::writeV2File(ostream& out)
{
    out.write((char*)size,sizeof(size));
	
	int numVoxels=size[2]*size[1]*size[0];
	bitc positive(numVoxels);
	for (int index=0; index<numVoxels; index++)
		if (!((known.getbit(index)) && (data[index]<0)))
			positive.setbit(index);
	
	positive.write(out,numVoxels);
	carved.write(out,numVoxels);
	
	return 0;
}

int volume::readVRIFile(char* filename)
{
#ifdef _OCC_GRID_RLE_
    //read file into an OccGridRLE
    OccGridRLE *ogSrc;
    ogSrc = new OccGridRLE(1,1,1, CHUNK_SIZE);
    if (!ogSrc->read(filename))
    {
        delete ogSrc;
        return 1;
    }
    OccElement* occSlice;
	
    size[0]=ogSrc->xdim;
    size[1]=ogSrc->ydim;
    size[2]=ogSrc->zdim;
	int sliceSize=size[1]*size[0],numVoxels=size[2]*sliceSize;
	data=new short[numVoxels];
	queued.setbit(numVoxels-1); queued.resetbit(numVoxels-1);
	carved.setbit(numVoxels-1); carved.resetbit(numVoxels-1);
	known.setbit(numVoxels-1); known.resetbit(numVoxels-1);
	
	float val;
	int x,y,z,voxelIndex=0,index;
    int junk;
    float pa,pb,pc,pd; pa=pb=pc=0; pd=-1; //for trimming below a plane of junk data
	
	if (!strstr(filename,"david-head-1mm-carve.vri"))
		pa=0.21; pb=0.0; pc=1.0; pd=0.19;
	
    //loop through the slices
    cout << "processing..."; cout.flush();
	for (z=0; z<size[2]; z++)
	{
        //read one slice of data from OccGridRLE and copy into data
        //cout << "."; cout.flush();
		occSlice=ogSrc->getSlice("z",z+1,&junk,&junk);

        index=0;
		for (y=0; y<size[1]; y++) for (x=0; x<size[0]; x++)
		{				
			if ((pa*x)/size[0]+(pb*y)/size[1]+(pc*z)/size[2]<pd) val=BIGNUM;
			else val=32767.5-(float)occSlice[index].value;
			//else val=(float)occSlice[index].value-32767.5;
			data[voxelIndex]=SGN(val);
			if (occSlice[index].totalWeight>0)
				known.setbit(voxelIndex);
			index++;
			voxelIndex++;
		}
	}

	delete ogSrc;
	return 0;
#endif
    cerr << ".vri format not supported. Recompile using volfill code.\n";
	return 2;
}

int volume::readFile(char* filename,float isovalue)
{
    ifstream fin(filename);
    if (!fin)
    {
        cerr << "Cannot open " << filename << " for reading.\n";
        return 1;
    }
    cout << "Reading " << filename << "..."; cout.flush();
	
	if (data) delete[] data;
	queued.freespace();
	carved.freespace();
	known.freespace();
	
    int result=0;
    char* suffix=strrchr(filename,'.');
    if (!strcmp(suffix,".v"))
    {
        //Read voxel file
		result=readVFile(fin,isovalue);
    }
    else if (!strcmp(suffix,".vri"))
    {
        //Read vrip (Michaelangelo format) file
        result=readVRIFile(filename);
    }
    else if (!strcmp(suffix,".v2"))
    {
        //Read voxel file with 2 bits per voxel
		result=readV2File(fin);
    }
    else
    {
		//unsupported format
		result=2;
    }
    fin.close();
	carved.copy(known); //copy known over into carved for use when converting files to .v2 format
    if (result) cout << "error!!\n";
	else cout << "done.\n";
	if (result==2) cerr << "File " << filename << " of unsupported format.\n";
    return result;
}

int volume::writeFile(char* filename)
{
    ofstream fout(filename);
    if (!fout)
    {
        cerr << "Cannot open " << filename << " for writing.\n";
        return 1;
    }
	
	int result=0;
    char* suffix=strrchr(filename,'.');
    if (!strcmp(suffix,".v"))
    {
        //Write voxel file
        cout << "Writing " << size[2] << " slices to " << filename << "..."; cout.flush();
		result=writeVFile(fout);
        if (result) cout << "error!!\n";
		else cout << "done.\n";
    }
    else if (!strcmp(suffix,".v2"))
    {
        //Write voxel file with 2 bits per voxel
        cout << "Writing " << size[2] << " slices to " << filename << "..."; cout.flush();
		result=writeV2File(fout);
        if (result) cout << "error!!\n";
		else cout << "done.\n";
    }
    else
    {
		//unsupported format
		result=2;
    }
    fout.close();
	if (result==2) cerr << "File " << filename << " of unsupported format.\n";
    return result;
}

