//Volume3DplusT.cpp
//James Vanderhyde, 14 May 2007.

#include <iostream.h>
#include <fstream.h>
#include <math.h>

#include "Volume3DplusT.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)<(b))?(b):(a))
#define ABS(x) (((x)<0)?-(x):(x))
#define MAX_SIGNED_SHORT 32767

voxelComparator4D::voxelComparator4D(Volume3DplusT* p_vol) : vol(p_vol) {}

int voxelComparator4D::operator()(int a,int b)
{
    unsigned short vala,valb;
    vala=vol->getVoxel(a);
    valb=vol->getVoxel(b);
    if (vala != valb) return (vala<valb);
    else return (a<b);
}

Volume3DplusT::Volume3DplusT()
{
    data=NULL;
    numSlices=0;
    strictTopoCheck=NULL;
    comboTopoCheck=NULL;
    sliceTopoCheck=NULL;
    thickSliceTopoCheck=NULL;
    numCarvedRegions=0;
    carvedRegions=NULL;
    initialOrder=NULL;
    carvedInsideOrder=NULL;
    setUpNeighborhoods();
}

Volume3DplusT::~Volume3DplusT()
{
    int i;
    if (data) delete[] data;
    if (strictTopoCheck) delete strictTopoCheck;
    if (comboTopoCheck) delete comboTopoCheck;
    if (sliceTopoCheck) delete sliceTopoCheck;
    if (thickSliceTopoCheck) delete thickSliceTopoCheck;
    if (carvedRegions)
    {
	for (i=0; i<numCarvedRegions; i++) delete carvedRegions[i];
	delete[] carvedRegions;
    }
    if (initialOrder) delete[] initialOrder;
    if (carvedInsideOrder) delete[] carvedInsideOrder;
    for (i=0; i<numNeighborsVertex; i++) delete[] neighborhoodVertex[i];
    delete[] neighborhoodVertex;
    for (i=0; i<numNeighborsFace; i++) delete[] neighborhoodFace[i];
    delete[] neighborhoodFace;
    for (i=0; i<numNeighborsSliceVertex; i++) delete[] neighborhoodSliceVertex[i];
    delete[] neighborhoodSliceVertex;
    for (i=0; i<numNeighborsSliceFace; i++) delete[] neighborhoodSliceFace[i];
    delete[] neighborhoodSliceFace;
}

void Volume3DplusT::setUpNeighborhoods()
{
    int i,j,x,y,z,t;
    
    //full volume
    numNeighborsVertex=80;
    neighborhoodVertex=new int*[numNeighborsVertex];
    for (i=0; i<numNeighborsVertex; i++)
    {
	neighborhoodVertex[i]=new int[4];
    }
    i=0;
    for (t=-1; t<=1; t++) for (z=-1; z<=1; z++) for (y=-1; y<=1; y++) for (x=-1; x<=1; x++) if ((t!=0) || (z!=0) || (y!=0) || (x!=0))
    {
	neighborhoodVertex[i][0]=x;
	neighborhoodVertex[i][1]=y;
	neighborhoodVertex[i][2]=z;
	neighborhoodVertex[i][3]=t;
	i++;
    }
    numNeighborsFace=8;
    neighborhoodFace=new int*[numNeighborsFace];
    for (i=0; i<numNeighborsFace; i++)
    {
	neighborhoodFace[i]=new int[4];
	for (j=0; j<4; j++) neighborhoodFace[i][j]=0;
    }
    neighborhoodFace[0][3]=-1;
    neighborhoodFace[1][2]=-1;
    neighborhoodFace[2][1]=-1;
    neighborhoodFace[3][0]=-1;
    neighborhoodFace[4][0]=1;
    neighborhoodFace[5][1]=1;
    neighborhoodFace[6][2]=1;
    neighborhoodFace[7][3]=1;
    
    //slices
    numNeighborsSliceVertex=26;
    neighborhoodSliceVertex=new int*[numNeighborsSliceVertex];
    for (i=0; i<numNeighborsSliceVertex; i++)
    {
	neighborhoodSliceVertex[i]=new int[3];
    }
    i=0;
    for (z=-1; z<=1; z++) for (y=-1; y<=1; y++) for (x=-1; x<=1; x++) if ((z!=0) || (y!=0) || (x!=0))
    {
	neighborhoodSliceVertex[i][0]=x;
	neighborhoodSliceVertex[i][1]=y;
	neighborhoodSliceVertex[i][2]=z;
	i++;
    }
    numNeighborsSliceFace=6;
    neighborhoodSliceFace=new int*[numNeighborsSliceFace];
    for (i=0; i<numNeighborsSliceFace; i++)
    {
	neighborhoodSliceFace[i]=new int[3];
	for (j=0; j<3; j++) neighborhoodSliceFace[i][j]=0;
    }
    neighborhoodSliceFace[0][2]=-1;
    neighborhoodSliceFace[1][1]=-1;
    neighborhoodSliceFace[2][0]=-1;
    neighborhoodSliceFace[3][0]=1;
    neighborhoodSliceFace[4][1]=1;
    neighborhoodSliceFace[5][2]=1;
}

int* Volume3DplusT::getSize()
{
    return size;
}

int Volume3DplusT::getNumVoxels()
{
    return size[0]*size[1]*size[2]*size[3];
}

int Volume3DplusT::getNumSlices()
{
    return numSlices;
}

int Volume3DplusT::getSliceSize()
{
    return size[0]*size[1]*size[2];
}

int Volume3DplusT::getVoxelIndex(int x,int y,int z,int t)
{
    return ((t*size[2]+z)*size[1]+y)*size[0]+x;
}

void Volume3DplusT::getVoxelLocFromIndex(int index,int* x,int* y,int* z,int* t)
{
    int rest=index;
    *x=rest%size[0];
    rest/=size[0];
    *y=rest%size[1];
    rest/=size[1];
    *z=rest%size[2];
    rest/=size[2];
    *t=rest%size[3];
}

//does bounds checking and subtracts isovalue
float Volume3DplusT::d(int x,int y,int z,int t)
{
    /*if (x<0) return d(0,y,z,t);
    if (y<0) return d(x,0,z,t);
    if (z<0) return d(x,y,0,t);
    if (t<0) return d(x,y,z,0);
    if (x>=size[0]) return d(size[0]-1,y,z,t);
    if (y>=size[1]) return d(x,size[1]-1,z,t);
    if (z>=size[2]) return d(x,y,size[2]-1,t);
    if (t>=size[3]) return d(x,y,z,size[3]-1);
    return data[getVoxelIndex(x,y,z,t)];*/
    if ((x<0) || (y<0) || (z<0) || (t<0) || 
	(x>=size[0]) || (y>=size[1]) || (z>=size[2]) || (t>=size[3])) 
	return BIGNUM;
    unsigned short isovalue=0;
    return data[getVoxelIndex(x,y,z,t)]-isovalue;
}

unsigned short Volume3DplusT::getVoxel(int index)
{
    return data[index];
}

unsigned short Volume3DplusT::getVoxel(int x,int y,int z,int t)
{
    return data[getVoxelIndex(x,y,z,t)];
}

void Volume3DplusT::setVoxel(int x,int y,int z,int t,unsigned short val)
{
    data[getVoxelIndex(x,y,z,t)]=val;
}

int Volume3DplusT::voxelCarved(int index)
{
    return carved.getbit(index);
}

void Volume3DplusT::setVoxelCarved(int index,char label)
{
    carved.setbit(index);
}

void Volume3DplusT::clearVoxelCarved(int index)
{
    carved.resetbit(index);
}

int Volume3DplusT::voxelQueued(int index)
{
    return queued.getbit(index);
}

void Volume3DplusT::setVoxelQueued(int index)
{
    queued.setbit(index);
}

void Volume3DplusT::clearVoxelQueued(int index)
{
    queued.resetbit(index);
}

int Volume3DplusT::getVoxelOrder(int index,int* order)
{
    if (order==NULL) order=getDefaultOrder();
    return order[index];
}

int Volume3DplusT::getVoxelInitialOrder(int index)
{
    if (!initialOrder) return -1;
    return getVoxelOrder(index,initialOrder);
}

int Volume3DplusT::getVoxelCarvedInsideOrder(int index)
{
    if (!carvedInsideOrder) return -1;
    return getVoxelOrder(index,carvedInsideOrder);
}

void Volume3DplusT::readTopoinfoFiles()
{
    if (size[2]>1)
    {
	strictTopoCheck=new TopoCheckStrict4D();
	comboTopoCheck=new TopoCheck3D4DCombo();
	sliceTopoCheck=new TopoCheck4DLowerDim(new TopoCheckStrict());
	thickSliceTopoCheck=new TopoCheckThick3D4DDouble();
    }
    else
    {
	strictTopoCheck=new TopoCheck4DRestrictZ(new TopoCheckStrict4D());
	comboTopoCheck=new TopoCheck4DRestrictZ(new TopoCheck3D4DCombo());
	sliceTopoCheck=new TopoCheck4DRestrictZ(new TopoCheck4DLowerDim(new TopoCheckStrict()));
	thickSliceTopoCheck=new TopoCheck4DRestrictZ(new TopoCheckThick3D4DDouble());
    }
}

//Swaps the sign on every voxel in the volume
//Rotates around midway through max and min values
void Volume3DplusT::changeAllSigns()
{
    cout << "Negating data.\n";
    unsigned short min=65535,max=0;
	int numVoxels=getNumVoxels();
    for (int i=0; i<numVoxels; i++)
    {
	if (data[i]<min) min=data[i];
	if (data[i]>max) max=data[i];
    }
    for (int i=0; i<numVoxels; i++)
	data[i]=max+min-data[i];
}

//Puts all the voxels into a priority queue for each slice
void Volume3DplusT::constructInitialBoundaryForSlice(int slice)
{
    int index;
    int numVoxels=getNumVoxels();
    int sliceSize=getSliceSize();
    
    cout << " Constructing initial boundary..."; cout.flush();
    for (index=slice*sliceSize; index<(slice+1)*sliceSize; index++)
    {
	setVoxelQueued(index);
	carvedRegions[0]->addToBoundary(pqitem(index,numVoxels+1,data[index]));
    }
    cout << "done.\n";
}

//Puts all the voxels into a priority queue
void Volume3DplusT::constructInitialBoundary(int component)
{
    int index;
    int numVoxels=getNumVoxels();
    
    cout << "Constructing initial boundary..."; cout.flush();
    for (index=0; index<numVoxels; index++)
    {
	setVoxelQueued(index);
	carvedRegions[component]->addToBoundary(pqitem(index,numVoxels+1,data[index]));
    }
    cout << "done.\n";
}

//Inverts an order so that carving from either direction can be compared.
void Volume3DplusT::invertOrder(int* order)
{
    int i,n=getNumVoxels();
    for (i=0; i<n; i++)
    {
	order[i]=n-1-order[i];
    }
}

void Volume3DplusT::invertSliceOrder(int* order,int slice)
{
    int i,n=getSliceSize();
    for (i=slice*n; i<(slice+1)*n; i++)
    {
	order[i]=n-1-order[i];
    }
}

//Inverts a permutation so that we can use the result of sort the way we want,
// an index into the sorted order.
void Volume3DplusT::invertPermutation(int* order)
{
    int i,n=getNumVoxels();
    int* temp=new int[n];
    for (i=0; i<n; i++) temp[order[i]]=i;
    for (i=0; i<n; i++) order[i]=temp[i];
    delete[] temp;
}

//Uses stdlib quicksort to sort the voxels.
void Volume3DplusT::sortVoxels()
{
    int numVoxels=getNumVoxels();
    if (!initialOrder) initialOrder=new int[numVoxels];
    for (int i=0; i<numVoxels; i++)
    {
	initialOrder[i]=i;
    }
    voxelComparator4D vc(this);
    
    //sort voxels
    cout << "Sorting voxels..."; cout.flush();
    sort(initialOrder,initialOrder+getNumVoxels(),vc);
    invertPermutation(initialOrder);
    cout << "done.\n";
}

void Volume3DplusT::checkAndAddNeighborToQueue(pqitem voxel,int component,int neighborIndex)
{
    if ((!voxelCarved(neighborIndex)) && (!voxelQueued(neighborIndex)))
    {
	//This must be a delayed voxel, because we added all the voxels to the queue to begin with.
	//We set its queued value to the current carving value, voxel.value.
	pqitem neighbor(neighborIndex,carvedRegions[component]->getNumCarved(),voxel.value);
	carvedRegions[component]->addToBoundary(neighbor);
	setVoxelQueued(neighborIndex);
    }
}

void Volume3DplusT::addSliceNeighborsToQueue(pqitem voxel,int component)
{
    int x,y,z,t,ox,oy,oz,ot=0;
    for (int i=0; i<numNeighborsSliceVertex; i++)
    {
	getVoxelLocFromIndex(voxel.index,&x,&y,&z,&t);
	ox=neighborhoodSliceVertex[i][0]; oy=neighborhoodSliceVertex[i][1]; oz=neighborhoodSliceVertex[i][2];
	if ((z+oz>=0) && (z+oz<size[2]) &&
	    (y+oy>=0) && (y+oy<size[1]) &&
	    (x+ox>=0) && (x+ox<size[0]))
	{
	    checkAndAddNeighborToQueue(voxel,component,getVoxelIndex(x+ox,y+oy,z+oz,t+ot));
	}
    }
}

void Volume3DplusT::addNeighborsToQueue(pqitem voxel,int component)
{
    int x,y,z,t,ox,oy,oz,ot;
    for (int i=0; i<numNeighborsVertex; i++)
    {
	getVoxelLocFromIndex(voxel.index,&x,&y,&z,&t);
	ox=neighborhoodVertex[i][0]; oy=neighborhoodVertex[i][1]; oz=neighborhoodVertex[i][2]; ot=neighborhoodVertex[i][3]; 
	if ((t+ot>=0) && (t+ot<size[3]) &&
	    (z+oz>=0) && (z+oz<size[2]) &&
	    (y+oy>=0) && (y+oy<size[1]) &&
	    (x+ox>=0) && (x+ox<size[0]))
	{
	    checkAndAddNeighborToQueue(voxel,component,getVoxelIndex(x+ox,y+oy,z+oz,t+ot));
	}
    }
}

//Carves without changing topology.
void Volume3DplusT::carveSimultaneously(unsigned short featureSize,int bySlices)
{
    int lastNumCarved=-1;
    int questionVoxel=-1;
    cout << "Carving"; cout.flush();
    
    //find first voxel to carve
    int maxPriority=-1;
    int maxi=-1;
    //  pqitem maxVoxel(-1,0,-1,1,getNumVoxels());
    for (int i=0; i<numCarvedRegions; i++)
    {
	int p=carvedRegions[i]->peekNextVoxel();
	if (p>maxPriority)
	{
	    maxPriority=p;
	    maxi=i;
	}
    }
    while (maxi>-1)
    {
	if ((carvedRegions[maxi]->getNumCarved() % 10000 == 0) && (carvedRegions[maxi]->getNumCarved() != lastNumCarved))
	{
	    //cout << "." << carvedRegions[maxi]->getNumCarved(); cout.flush();
	    cout << "."; cout.flush();
	    lastNumCarved=carvedRegions[maxi]->getNumCarved();
	}
	if (carvedRegions[maxi]->getNumCarved() > 10000)
	{
	    if (carvedRegions[maxi]->getNumCarved() % 100 == 0)
	    {
		//cout << "." << carvedRegions[maxi]->getNumCarved(); cout.flush();
	    }
	}
	
	//pop voxel from top of appropriate priority queue
	pqitem voxel=carvedRegions[maxi]->getNextVoxel();
	if (voxel.index==questionVoxel) cout << "v(" << voxel.index << "," << voxel.value << "," << data[voxel.index] << ")";
	if (voxel.index==questionVoxel) cout << "(" << carvedRegions[maxi]->getNumCarved() << ")";
	
	if (!voxelCarved(voxel.index))
	{
	    //check whether removing this voxel changes the topology
	    if (!carvedRegions[maxi]->topologyCheck(voxel.index))
	    {
		if (voxel.index==questionVoxel) cout << "a";
		//If it doesn't, carve this voxel.
		carvedRegions[maxi]->carveVoxel(voxel);
		//Update the voxel value to reflect carving order.
		data[voxel.index]=MAX(MIN(voxel.value,65535),0);
		//Add neighbors to the queue
		if (bySlices) addSliceNeighborsToQueue(voxel,maxi);
		else addNeighborsToQueue(voxel,maxi);
		//Update contour tree
	    }
	    else
	    {
		//If it does, add it back to the queue with adjusted priority,
		// unless this is a persistent topology change,
		// in which case we carve this voxel anyway.
		if ((ABS(voxel.value-data[voxel.index])>=featureSize) && 
		    (voxel.subdelay==getNumVoxels()+1))
		{
		    if (voxel.index==questionVoxel) cout << "b";
		    //Carve the voxel anyway.
		    carvedRegions[maxi]->carveVoxel(voxel);
		    //Update the voxel value to reflect carving order.
		    data[voxel.index]=MAX(MIN(voxel.value,65535),0);
		    //Add neighbors to the queue
		    if (bySlices) addSliceNeighborsToQueue(voxel,maxi);
		    else addNeighborsToQueue(voxel,maxi);
		    //Update contour tree
		}
		else
		{
		    if (voxel.index==questionVoxel) cout << "c";
		    //Add back to the queue with delayed priority.
		    carvedRegions[maxi]->addDelayedToBoundary(voxel);
		}
	    }
	}
	
	//find next voxel to carve
	maxPriority=-1;
	maxi=-1;
	for (int i=0; i<numCarvedRegions; i++)
	{
	    int p=carvedRegions[i]->peekNextVoxel();
	    if (p>maxPriority)
	    {
		maxPriority=p;
		maxi=i;
	    }
	}
    }
    
    cout << "done.\n";
    
    int numCarved=0;
    for (int i=0; i<numCarvedRegions; i++)
    {
	cout << " " << carvedRegions[i]->getNumCarved() << " carved from component " << i << ".\n";
	numCarved+=carvedRegions[i]->getNumCarved();
    }
    int numSupposedToBeCarved=getNumVoxels();
    if (bySlices) numSupposedToBeCarved=getSliceSize();
    if (numCarved<numSupposedToBeCarved) 
    {
	cout << " Some voxels not carved!!!\n"; //house of 2 rooms?
    }
    if (numCarved>numSupposedToBeCarved) cout << " Too many voxels carved!\n";
}

void Volume3DplusT::fixTopologyBySlices(unsigned short featureSize,int fixDirection)
{
    int sliceSize=getSliceSize(),numVoxels=getNumVoxels();
    sortVoxels();
    countCriticalsInThickSlices(initialOrder);
    
    if (fixDirection==1)
    {
	//adjust values so they stay positive
	unsigned short adj=MIN(featureSize,MAX_SIGNED_SHORT);
	for (int i=0; i<numVoxels; i++) data[i]+=adj;
    }

    //set up array to hold the carving order
    if (!carvedInsideOrder) carvedInsideOrder=new int[numVoxels];
    for (int i=0; i<numVoxels; i++) carvedInsideOrder[i]=-1;
    
    //set up carve component boundaries
    if (carvedRegions) 
    {
	for (int i=0; i<numCarvedRegions; i++) delete carvedRegions[i];
	delete[] carvedRegions;
    }
    numCarvedRegions=1;
    carvedRegions=new carvecomp4D*[numCarvedRegions];
    
    for (int slice=0; slice<numSlices; slice++)
    {
	carvedRegions[0]=new carvecomp4D(1,(fixDirection==-1),sliceTopoCheck,this,carvedInsideOrder);
	constructInitialBoundaryForSlice(slice);
	
	if (fixDirection==-1)
	{
	    //carve the first voxel (global min)
	    pqitem voxel=carvedRegions[0]->getNextVoxel();
	    carvedRegions[0]->carveVoxel(voxel);
	}
	
	//carve simultaneously
	carveSimultaneously(featureSize,1);
	//checkUncarvedVoxels(slice);
	if (fixDirection==1) invertSliceOrder(carvedInsideOrder,slice);

	//fix the carved order in this slice so all the values in the array are unique
	for (int index=slice*sliceSize; index<(slice+1)*sliceSize; index++)
	    carvedInsideOrder[index]+=slice*sliceSize;
	
	delete carvedRegions[0];
    }
    delete[] carvedRegions;
    carvedRegions=NULL;

    
    //fix global carving order so that thick slices make sense (numSlices-way merge)
    cout << "Fixing carving order..."; cout.flush();
    int* process=new int[numVoxels]; //indices into the data array
    for (int i=0; i<numVoxels; i++) process[carvedInsideOrder[i]]=i;
    int* pointers=new int[numSlices]; //indices into the process array
    for (int slice=0; slice<numSlices; slice++) pointers[slice]=slice*sliceSize;
    int numProcessed=0;
    while (numProcessed<numVoxels)
    {
	unsigned short min=65535;
	int minSlice,slice;
	//find slice with min voxel (each slice is already sorted)
	for (slice=0; slice<numSlices; slice++)
	{
	    if ((pointers[slice]<(slice+1)*sliceSize) && (data[process[pointers[slice]]]<min))
	    {
		minSlice=slice;
		min=data[process[pointers[minSlice]]];
	    }
	}
	//copy all min voxels in this slice to order array
	while ((pointers[minSlice]<(minSlice+1)*sliceSize) && (data[process[pointers[minSlice]]]==min))
	{
	    carvedInsideOrder[process[pointers[minSlice]]]=numProcessed++;
	    pointers[minSlice]++;
	}
    }
    delete[] pointers;
    delete[] process;
    cout << "done.\n";

    countCriticalsInThickSlices(carvedInsideOrder);
    
    //build contour tree
}

void Volume3DplusT::fixTopologyStrict(unsigned short featureSize,int fixDirection)
{
    int numVoxels=getNumVoxels();
    if (fixDirection==1)
    {
	//adjust values so they stay positive
	unsigned short adj=MIN(featureSize,MAX_SIGNED_SHORT);
	for (int i=0; i<numVoxels; i++) data[i]+=adj;
    }
    
    //set up array to hold the carving order
    if (!carvedInsideOrder) carvedInsideOrder=new int[numVoxels];
    for (int i=0; i<numVoxels; i++) carvedInsideOrder[i]=-1;
    
    //set up carve component boundaries
    if (carvedRegions) 
    {
	for (int i=0; i<numCarvedRegions; i++) delete carvedRegions[i];
	delete[] carvedRegions;
    }
    numCarvedRegions=1;
    carvedRegions=new carvecomp4D*[numCarvedRegions];
#ifdef FULLCHECK
    carvedRegions[0]=new carvecomp4D(1,(fixDirection==-1),strictTopoCheck,this,carvedInsideOrder);
#else
    carvedRegions[0]=new carvecomp4D(1,(fixDirection==-1),comboTopoCheck,this,carvedInsideOrder);
#endif
    constructInitialBoundary(0);
    
    if (fixDirection==-1)
    {
	//carve the first voxel (global min)
	pqitem voxel=carvedRegions[0]->getNextVoxel();
	carvedRegions[0]->carveVoxel(voxel);
    }
    
    //carve simultaneously
    sortVoxels();
    countCriticalsInThickSlices(initialOrder);
    carveSimultaneously(featureSize,0);
    if (fixDirection==1) invertOrder(carvedInsideOrder);
    countCriticalsInThickSlices(carvedInsideOrder);
    
    //build contour tree
}

void Volume3DplusT::fixTopologyLoadedOrder(char* filename)
{
    //set up array to hold the carving order
    sortVoxels();
    int numVoxels=getNumVoxels();
    if (!carvedInsideOrder) carvedInsideOrder=new int[numVoxels];
    
    //load order from file
    loadOrder(filename,carvedInsideOrder);
}

void Volume3DplusT::fixTopology(char* arg)
{
    char* suffix=strrchr(arg,'.');
    if ((suffix) && (!strcmp(suffix,".vo4d"))) fixTopologyLoadedOrder(arg);
    else 
    {
	int bySlices=0;
#ifdef BYSLICES
	bySlices=1;
#endif
	int fixDirection=1;
#ifdef FROMINSIDE
	fixDirection=-1;
#endif
	if (fixDirection>0) cout << "Carving from outside.\n";
	else cout << "Adding from inside.\n";
	if (bySlices) fixTopologyBySlices(atoi(arg),fixDirection);
	else fixTopologyStrict(atoi(arg),fixDirection);
    }
}

void Volume3DplusT::checkUncarvedVoxels(int slice)
{
    int n=getSliceSize();
    int index;
    for (index=slice*n; index<(slice+1)*n; index++)
    {
	if (carvedInsideOrder[index]==-1)
	{
	    int x,y,z,t;
	    getVoxelLocFromIndex(index,&x,&y,&z,&t);
	    cout << "Problem voxel: " << index;
	    cout << "(" << x << "," << y << "," << z << ";" << t << ")" << "\n";
	}
    }
}

int* Volume3DplusT::getDefaultOrder()
{
    if (carvedInsideOrder) return carvedInsideOrder;
    else 
    {
	if (!initialOrder) sortVoxels();
	return initialOrder;
    }
}

void Volume3DplusT::getTopoType(bool* topoType,int index,int* order)
{
    return getTopoTypeInRange(topoType,index,order,0,size[0]-1,0,size[1]-1,0,size[2]-1,0,size[3]-1);
}

void Volume3DplusT::getTopoTypeInRange(bool* topoType,int index,int* order,int xmin,int xmax,int ymin,int ymax,int zmin,int zmax,int tmin,int tmax)
{
    int bit=0;
    int x,y,z,t,ox,oy,oz,ot;
    int neighborIndex;
    
    getVoxelLocFromIndex(index,&x,&y,&z,&t);
    for (ot=-1; ot<=1; ot++)
      for (oz=-1; oz<=1; oz++)
	for (oy=-1; oy<=1; oy++)
	    for (ox=-1; ox<=1; ox++)
		if ((ot!=0) || (oz!=0) || (oy!=0) || (ox!=0))
		{
		    topoType[bit]=false;
		    if ((t+ot>=tmin) && (t+ot<=tmax) &&
			(z+oz>=zmin) && (z+oz<=zmax) &&
			(y+oy>=ymin) && (y+oy<=ymax) &&
			(x+ox>=xmin) && (x+ox<=xmax))
		    {
			neighborIndex=getVoxelIndex(x+ox,y+oy,z+oz,t+ot);
			//If the neighbor is before the current voxel, we set the bit.
			if (order[neighborIndex]<order[index])
			    topoType[bit]=true;
		    }
		    else
		    {
			//Out of range is considered large positive, so we do not set the bit, because we're increasing.
		    }
		    bit++;
		}
}

int Volume3DplusT::voxelCriticalInVolume(bool* topoType,int index,int* order)
{
    if (!order) order=getDefaultOrder();
    getTopoType(topoType,index,order);
    if (strictTopoCheck->topologyChange(topoType)) return 1;
    else return 0;
}

int Volume3DplusT::voxelCriticalInSlice(bool* topoType,int index,int* order)
{
    if (!order) order=getDefaultOrder();
    getTopoType(topoType,index,order);
    if (sliceTopoCheck->topologyChange(topoType)) return 1;
    else return 0;
}

//leftOrRight==0 when the voxel is on the left side, and its right neighborhood is full.
//leftOrRight==1 when the voxel is on the right side, and its left neighborhood is full.
int Volume3DplusT::voxelCriticalInThickSlice(bool* topoType,int index,int leftOrRight,int* order)
{
    int bit;
    if (!order) order=getDefaultOrder();
    getTopoType(topoType,index,order);
	
    //Turn topotype around for topology test
    if (leftOrRight) for (bit=0; bit<27; bit++) topoType[27+26+bit]=topoType[bit];
    
    if (thickSliceTopoCheck->topologyChange(topoType)) return 1;
    else return 0;
}

int Volume3DplusT::countCriticalsInVolume(int* order)
{
    int index;
    int numCritical=0;
    int numVoxels=getNumVoxels();
    bool topoType[80];
    bool topoTypeForPrinting[80];
    
    cout << "Counting critical points in volume..."; cout.flush();
    for (index=0; index<numVoxels; index++)
    {
	if (voxelCriticalInVolume(topoType,index,order))
	{
	    numCritical++;
	    //getTopoType(topoTypeForPrinting,index,order);
	    //strictTopoCheck->printNeighborhood(topoTypeForPrinting);
	}
    }
    cout << "done: " << numCritical << "\n";
    return numCritical;
}

int Volume3DplusT::countCriticalsInSlices(int* order)
{
    int numCritical=0;
    cout << "Counting critical points in slices..."; cout.flush();
    
    for (int slice=0; slice<numSlices; slice++)
	numCritical+=countCriticalsInSlice(slice,order);
    
    cout << "done: " << numCritical << "\n";
    return numCritical;
}

int Volume3DplusT::countCriticalsInSlice(int slice,int* order)
{
    int index;
    int numCritical=0;
    int sliceSize=getSliceSize();
    bool topoType[80];
    
    for (index=slice*sliceSize; index<(slice+1)*sliceSize; index++)
    {
	if (voxelCriticalInSlice(topoType,index,order))
	{
	    numCritical++;
	}
    }
    return numCritical;
}

int Volume3DplusT::countCriticalsInThickSlices(int* order)
{
    int numCritical=0;
    cout << "Counting critical points in thick slices..."; cout.flush();
    
    for (int slice=0; slice<numSlices-1; slice++)
	numCritical+=countCriticalsInThickSlice(slice,order);
    
    cout << "done: " << numCritical << "\n";
    return numCritical;
}

int Volume3DplusT::countCriticalsInThickSlice(int sliceLeft,int* order,int augment)
{
    int index;
    int numCritical=0;
    int sliceSize=getSliceSize();
    bool topoType[80];
    
    for (index=sliceLeft*sliceSize; index<(sliceLeft+1)*sliceSize; index++)
    {
	if ((voxelCriticalInThickSlice(topoType,index,0,order)) || 
	    ((augment) && ((voxelCriticalInSlice(topoType,index,order)) || (voxelCriticalInThickSlice(topoType,index,1,order)))))
	{
	    numCritical++;
	}
	if ((voxelCriticalInThickSlice(topoType,index+sliceSize,1,order)) || 
	    ((augment) && ((voxelCriticalInSlice(topoType,index+sliceSize,order)) || (voxelCriticalInThickSlice(topoType,index+sliceSize,0,order)))))
	{
	    numCritical++;
	}
    }
    cout << "."; cout.flush();
    return numCritical;
}

int Volume3DplusT::loadOrder(char* filename,int* order)
{
    ifstream fin(filename);
    int dontclobbersize[4];
    if (fin)
    {
	fin.read((char*)dontclobbersize,sizeof(dontclobbersize));
	fin.read((char*)order,getNumVoxels()*sizeof(int));
	fin.close();
	return 0;
    }
    else
    {
	cerr << "Error reading from file " << filename << " !\n";
	return 1;
    }
}

int Volume3DplusT::saveOrder(char* filename,int* order)
{
    if (!order) order=getDefaultOrder();
    ofstream fout(filename);
    if (fout)
    {
	fout.write((char*)size,sizeof(size));
	fout.write((char*)order,getNumVoxels()*sizeof(int));
	fout.close();
	return 0;
    }
    else
    {
	cerr << "Error saving to file " << filename << " !\n";
	return 1;
    }
}

int Volume3DplusT::readByteFile(char* filename)
{
    ifstream fin(filename);
    if (!fin) 
    {
	cerr << "Can't open file " << filename << " for reading!\n";
	return 1;
    }
    
    fin.read((char*)size,sizeof(size));
    numSlices=size[3];
    int sliceSize=size[2]*size[1]*size[0];
    int numVoxels=numSlices*sliceSize;
    data=new unsigned short[numVoxels];
    unsigned char* bytedata=new unsigned char[numVoxels];
    fin.read((char*)bytedata,numVoxels*sizeof(unsigned char));
    fin.close();
    
    int index;
    for (index=0; index<numVoxels; index++)
    {
	data[index]=(unsigned short)bytedata[index];
    }
    
    delete[] bytedata;
    return 0;
}

void Volume3DplusT::storeFloatDataIntoShorts(float* floatdata,int numVoxels)
{
    int index;
    float min=BIGNUM,max=-BIGNUM;
    for (index=0; index<numVoxels; index++)
    {
	if (floatdata[index]<min) min=floatdata[index];
	if (floatdata[index]>max) max=floatdata[index];
    }
    
    unsigned short maxshort=MAX_SIGNED_SHORT;
    float scaleFactor;
    if ((128<=max-min) && (max-min<maxshort)) scaleFactor=1.0;
    else scaleFactor=maxshort/(max-min);
    int numVoxelsAbove0=0;
    for (index=0; index<numVoxels; index++)
    {
	data[index]=(unsigned short)floor(scaleFactor*(floatdata[index]-min)+0.5);
	if (floatdata[index]>0.0) numVoxelsAbove0++;
    }
    
    cout << "Float data converted to short ints.\n";
    cout << "Min: " << min << ", Max=" << max << "\n";
    cout << "Isovalue 0.0 mapped to " << (unsigned short)floor(scaleFactor*(0.0-min)+0.5) << "\n";
    cout << "Total voxels above 0.0 is " << numVoxelsAbove0 << ", below is " << numVoxels-numVoxelsAbove0 << "\n";
}

int Volume3DplusT::readFloatFile(char* filename)
{
    ifstream fin(filename);
    if (!fin) 
    {
	cerr << "Can't open file " << filename << " for reading!\n";
	return 1;
    }
    
    fin.read((char*)size,sizeof(size));
    numSlices=size[3];
    int sliceSize=size[2]*size[1]*size[0];
    int numVoxels=numSlices*sliceSize;
    data=new unsigned short[numVoxels];
    float* floatdata=new float[numVoxels];
    fin.read((char*)floatdata,numVoxels*sizeof(float));
    fin.close();
    
    storeFloatDataIntoShorts(floatdata,numVoxels);
	
    delete[] floatdata;
    return 0;
}

int Volume3DplusT::readFloat3DFile(char* filename)
{
    ifstream fin(filename);
    if (!fin) 
    {
	cerr << "Can't open file " << filename << " for reading!\n";
	return 1;
    }
    
    int vsize3D[3];
    fin.read((char*)vsize3D,sizeof(vsize3D));
    size[0]=vsize3D[0];
    size[1]=vsize3D[1];
    size[2]=vsize3D[2];
    size[3]=1;
    numSlices=size[3];
    int sliceSize=size[2]*size[1]*size[0];
    int numVoxels=numSlices*sliceSize;
    data=new unsigned short[numVoxels];
    float* floatdata=new float[numVoxels];
    fin.read((char*)floatdata,numVoxels*sizeof(float));
    fin.close();
    
    storeFloatDataIntoShorts(floatdata,numVoxels);
    
    delete[] floatdata;
    return 0;
}

int Volume3DplusT::readFile(char* filename)
{
    char* suffix=strrchr(filename,'.');
    if (!strcmp(suffix,".v4d"))
    {
        //1 float per voxel file
	return readFloatFile(filename);
    }
    else if (!strcmp(suffix,".v"))
    {
        //1 float per voxel 3D file
	return readFloat3DFile(filename);
    }
    else if (!strcmp(suffix,".v4db"))
    {
        //1 byte per voxel file
	return readByteFile(filename);
    }
    else
    {
	//unsupported format
	cerr << "File " << filename << " of unsupported format.\n";
	return 2;
    }
}

int Volume3DplusT::writeFile(char* filename)
{
    ofstream fout(filename);
    if (!fout)
    {
	cerr << "Can't open file " << filename << " for writing!\n";
	return 1;
    }
    
    int sliceSize=size[2]*size[1]*size[0];
    int numVoxels=numSlices*sliceSize;
    float* floatdata=new float[numVoxels];
    for (int index=0; index<numVoxels; index++)
    {
	floatdata[index]=(float)data[index];
    }
    fout.write((char*)size,sizeof(size));
    fout.write((char*)data,numVoxels*sizeof(float));
    fout.close();
    delete[] floatdata;
    return 0;
}


#ifdef FIXTOP3DP
int main(int argc,char* argv[])
{
    if (argc<=2)
    {
	cerr << "Usage: " << argv[0] << " <input .v4d file> <output .v4d file> [[threshold] <output .vo4d file>]\n";
	return 1;
    }
    
    unsigned short threshold=65535;
    if (argc>3)
    {
	threshold=atoi(argv[3]);
    }
    
    int result;
    
    Volume3DplusT v;
    
    result=v.readFile(argv[1]);
    if (result) return result;
    
    cout << v.getSize()[0] << 'x' << v.getSize()[1] << 'x' << v.getSize()[2] << 'x' << v.getSize()[3] << '=';
    cout << v.getNumVoxels() << '\n';
    
    v.readTopoinfoFiles();
    
    int signsNeedToBeChanged=0;
    if (signsNeedToBeChanged) v.changeAllSigns();
    
    unsigned short min=65535,max=0;
    for (int i=0; i<v.getNumVoxels(); i++)
    {
	if (v.getVoxel(i)<min) min=v.getVoxel(i);
	if (v.getVoxel(i)>max) max=v.getVoxel(i);
    }
    cout << "Min: " << min << ", Max: " << max << "\n";
    
    clock_t starttime = clock();
    v.fixTopology(argv[3]);
    clock_t endtime = clock();
    cout << "Topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";
    
    if (signsNeedToBeChanged) v.changeAllSigns();
    cout << "Writing to file " << argv[2] << " ..."; cout.flush();
    result=v.writeFile(argv[2]);
    if (result)
    {
	cout << "error!\n";
	return result;
    }
    else cout << "done.\n";
    
    if (argc>4) v.saveOrder(argv[4]);
    
    return 0;
}
#endif

