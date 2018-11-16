//Volume2DplusT.cpp
//James Vanderhyde, 28 September 2006.

#include "Volume2DplusT.h"

Volume2DplusT::Volume2DplusT(unsigned short p_featureSize,int p_fixStyle) : volume(p_featureSize,p_fixStyle)
{
    numSlices=0;
    thickSliceTopoCheck=NULL;
    int i;
    setUpSliceNeighborhoods();
    sliceJoinTrees=thickSliceJoinTrees=NULL;
    sliceSplitTrees=thickSliceSplitTrees=NULL;
    sliceTrees=thickSliceTrees=NULL;
}

Volume2DplusT::~Volume2DplusT()
{
    int i;
    if (thickSliceTopoCheck) delete thickSliceTopoCheck;
    for (i=0; i<numNeighborsSliceVertex; i++) delete[] neighborhoodSliceVertex[i];
    delete[] neighborhoodSliceVertex;
    for (i=0; i<numNeighborsSliceFace; i++) delete[] neighborhoodSliceFace[i];
    delete[] neighborhoodSliceFace;
    for (i=0; i<numNeighborsThickLeftVertex; i++) delete[] neighborhoodThickLeftVertex[i];
    delete[] neighborhoodThickLeftVertex;
    for (i=0; i<numNeighborsThickLeftFace; i++) delete[] neighborhoodThickLeftFace[i];
    delete[] neighborhoodThickLeftFace;
    for (i=0; i<numNeighborsThickRightVertex; i++) delete[] neighborhoodThickRightVertex[i];
    delete[] neighborhoodThickRightVertex;
    for (i=0; i<numNeighborsThickRightFace; i++) delete[] neighborhoodThickRightFace[i];
    delete[] neighborhoodThickRightFace;
    if (sliceTrees) delete[] sliceTrees;
    if (thickSliceTrees) delete[] thickSliceTrees;
    if (sliceJoinTrees) delete[] sliceJoinTrees;
    if (thickSliceJoinTrees) delete[] thickSliceJoinTrees;
    if (sliceSplitTrees) delete[] sliceSplitTrees;
    if (thickSliceSplitTrees) delete[] thickSliceSplitTrees;
}

void Volume2DplusT::setUpSliceNeighborhoods()
{
    int i,j,x,y,z;
    
    //Slices
    numNeighborsSliceVertex=8;
    neighborhoodSliceVertex=new int*[numNeighborsSliceVertex];
    for (i=0; i<numNeighborsSliceVertex; i++)
    {
	neighborhoodSliceVertex[i]=new int[3];
    }
    i=0;
    z=0;
    for (y=-1; y<=1; y++) for (x=-1; x<=1; x++) if ((z!=0) || (y!=0) || (x!=0))
    {
	neighborhoodSliceVertex[i][0]=x;
	neighborhoodSliceVertex[i][1]=y;
	neighborhoodSliceVertex[i][2]=z;
	i++;
    }
    numNeighborsSliceFace=4;
    neighborhoodSliceFace=new int*[numNeighborsSliceFace];
    for (i=0; i<numNeighborsSliceFace; i++)
    {
	neighborhoodSliceFace[i]=new int[3];
	for (j=0; j<3; j++) neighborhoodSliceFace[i][j]=0;
    }
    neighborhoodSliceFace[0][1]=-1;
    neighborhoodSliceFace[1][0]=-1;
    neighborhoodSliceFace[2][0]=1;
    neighborhoodSliceFace[3][1]=1;
    
    //Thick slices left (center voxel on the left)
    numNeighborsThickLeftVertex=17;
    neighborhoodThickLeftVertex=new int*[numNeighborsThickLeftVertex];
    for (i=0; i<numNeighborsThickLeftVertex; i++)
    {
	neighborhoodThickLeftVertex[i]=new int[3];
    }
    i=0;
    for (z=0; z<=1; z++) for (y=-1; y<=1; y++) for (x=-1; x<=1; x++) if ((z!=0) || (y!=0) || (x!=0))
    {
	neighborhoodThickLeftVertex[i][0]=x;
	neighborhoodThickLeftVertex[i][1]=y;
	neighborhoodThickLeftVertex[i][2]=z;
	i++;
    }
    numNeighborsThickLeftFace=5;
    neighborhoodThickLeftFace=new int*[numNeighborsThickLeftFace];
    for (i=0; i<numNeighborsThickLeftFace; i++)
    {
	neighborhoodThickLeftFace[i]=new int[3];
	for (j=0; j<3; j++) neighborhoodThickLeftFace[i][j]=0;
    }
    neighborhoodThickLeftFace[0][1]=-1;
    neighborhoodThickLeftFace[1][0]=-1;
    neighborhoodThickLeftFace[2][0]=1;
    neighborhoodThickLeftFace[3][1]=1;
    neighborhoodThickLeftFace[4][2]=1;
    
    //Thick slices right (center voxel on the right)
    numNeighborsThickRightVertex=17;
    neighborhoodThickRightVertex=new int*[numNeighborsThickRightVertex];
    for (i=0; i<numNeighborsThickRightVertex; i++)
    {
	neighborhoodThickRightVertex[i]=new int[3];
    }
    i=0;
    for (z=-1; z<=0; z++) for (y=-1; y<=1; y++) for (x=-1; x<=1; x++) if ((z!=0) || (y!=0) || (x!=0))
    {
	neighborhoodThickRightVertex[i][0]=x;
	neighborhoodThickRightVertex[i][1]=y;
	neighborhoodThickRightVertex[i][2]=z;
	i++;
    }
    numNeighborsThickRightFace=5;
    neighborhoodThickRightFace=new int*[numNeighborsThickRightFace];
    for (i=0; i<numNeighborsThickRightFace; i++)
    {
	neighborhoodThickRightFace[i]=new int[3];
	for (j=0; j<3; j++) neighborhoodThickRightFace[i][j]=0;
    }
    neighborhoodThickRightFace[0][2]=-1;
    neighborhoodThickRightFace[1][1]=-1;
    neighborhoodThickRightFace[2][0]=-1;
    neighborhoodThickRightFace[3][0]=1;
    neighborhoodThickRightFace[4][1]=1;
}

int Volume2DplusT::getNumSlices()
{
    return numSlices;
}

int Volume2DplusT::getSliceSize()
{
    return size[0]*size[1];
}

void Volume2DplusT::readTopoinfoFiles()
{
    volume::readTopoinfoFiles();
    thickSliceTopoCheck=new TopoCheckThick2D3DTable();
    //((TopoCheckThick2D3D*)thickSliceTopoCheck)->saveTopoinfoFile("topoinfo_vertex_thick");
}

//Puts all the voxels into a priority queue for each slice
void Volume2DplusT::constructInitialInnerBoundaryForSlice(int slice)
{
    int index;
    int numVoxels=getNumVoxels();
    int voxelIndex;
    int neighbors[27];
    int i,n;
    int localmin;
    int sliceSize=size[1]*size[0];
    
    cout << " Constructing initial boundary..."; cout.flush();
    for (index=slice*sliceSize; index<(slice+1)*sliceSize; index++)
    {
	setVoxelQueued(index);
	carvedRegions[0]->addToBoundary(pqitem(index,numVoxels+1,data[index]));
    }
    cout << "done.\n";
}

int Volume2DplusT::fixTopologyBySlices()
{
    sortVoxels();
    countCriticalsInThickSlices(initialOrder);
    
    //set up arrays to hold the carving order
    if (!carvedSlicesOrder) carvedSlicesOrder=new int[size[0]*size[1]*size[2]];
    for (int i=0; i<size[0]*size[1]*size[2]; i++) carvedSlicesOrder[i]=-1;
    
    //set up carve component boundaries
    if (carvedRegions) 
    {
	for (int i=0; i<numCarvedRegions; i++) delete carvedRegions[i];
	delete[] carvedRegions;
    }
    numCarvedRegions=1;
    carvedRegions=new carvecomp*[numCarvedRegions];
    
    for (int slice=0; slice<numSlices; slice++)
    {
	cout << "Slice " << slice << ":\n";
	carvedRegions[0]=new carvecomp(1,1,BIGNUM,sliceTopoCheck,this,carvedSlicesOrder);
	constructInitialInnerBoundaryForSlice(slice);
	
	//carve the first voxel (global min)
	pqitem voxel=carvedRegions[0]->getNextVoxel();
	carvedRegions[0]->carveVoxel(voxel);
	if (criticalParent) criticalParent[voxel.index]=voxel.index;
	
	//carve simultaneously
	cout << " ";
	carveSimultaneously();
	
	//fix the carved order in this slice so all the values in the array are unique
	for (int index=slice*size[1]*size[0]; index<(slice+1)*size[1]*size[0]; index++)
	    carvedSlicesOrder[index]+=slice*size[1]*size[0];
	//We'll have to do this in a more sophisticated way so that the values in thick slices
	// will have meaning. Simply being unique isn't enough.
	
	delete carvedRegions[0];
    }
    delete[] carvedRegions;
    carvedRegions=NULL;
    
    //fix global carving order so that thick slices make sense
    cout << "Fixing carving order..."; cout.flush();
    int* process=new int[size[0]*size[1]*size[2]]; //indices into the data array
    for (int i=0; i<size[0]*size[1]*size[2]; i++) process[carvedSlicesOrder[i]]=i;
    int* pointers=new int[numSlices]; //indices into the process array
    for (int slice=0; slice<numSlices; slice++) pointers[slice]=slice*size[1]*size[0];
    int numProcessed=0;
    while (numProcessed<size[0]*size[1]*size[2])
    {
	unsigned short min=65535;
	int minSlice,slice;
	//find slice with min voxel (each slice is already sorted)
	for (slice=0; slice<numSlices; slice++)
	{
	    if ((pointers[slice]<(slice+1)*size[1]*size[0]) && (data[process[pointers[slice]]]<min))
	    {
		minSlice=slice;
		min=data[process[pointers[minSlice]]];
	    }
	}
	//copy all min voxels in this slice to order array
	while ((pointers[minSlice]<(minSlice+1)*size[1]*size[0]) && (data[process[pointers[minSlice]]]==min))
	{
	    carvedSlicesOrder[process[pointers[minSlice]]]=numProcessed++;
	    pointers[minSlice]++;
	}
    }
    delete[] pointers;
    delete[] process;
    cout << "done.\n";
    
    return countCriticalsInThickSlices(carvedSlicesOrder);
}

int Volume2DplusT::fixTopologyStrict(int useUnionFind)
{
    sortVoxels();
    countCriticalsInThickSlices(initialOrder);

    //set up arrays to hold the carving order
    if (!carvedInsideOrder) carvedInsideOrder=new int[size[0]*size[1]*size[2]];
    for (int i=0; i<size[0]*size[1]*size[2]; i++) carvedInsideOrder[i]=-1;
    if (useUnionFind)
    {
	if (!criticalParent) criticalParent=new int[size[0]*size[1]*size[2]];
	for (int i=0; i<size[0]*size[1]*size[2]; i++) criticalParent[i]=-1;
    }
    
    //set up carve component boundaries
    //one for each slice
    if (carvedRegions) 
    {
	for (int i=0; i<numCarvedRegions; i++) delete carvedRegions[i];
	delete[] carvedRegions;
    }
    numCarvedRegions=1;
    carvedRegions=new carvecomp*[numCarvedRegions];
    carvedRegions[0]=new carvecomp(1,1,BIGNUM,comboTopoCheck,this,carvedInsideOrder);
    constructInitialInnerBoundary(0);
    
    //carve the first voxel (global min)
    pqitem voxel=carvedRegions[0]->getNextVoxel();
    carvedRegions[0]->carveVoxel(voxel);
    if (criticalParent) criticalParent[voxel.index]=voxel.index;
    
    //carve simultaneously
    carveSimultaneously();
    
    return countCriticalsInThickSlices(carvedInsideOrder);
}

TopoCheck* Volume2DplusT::getTopoCheck()
{
    //The combo check seems to be working OK.
    //We return the strict check here for other purposes, however.
    //return comboTopoCheck;
    cerr << "Out of date method Volume2DplusT::getTopoCheck()!\n";
    assert(0);
    return strictTopoCheck;
}

int Volume2DplusT::getSliceVoxelsInOrder(int slice,int* data,int* order)
{
    int i,index,dataIndex,x,y,z;
    if (!order) order=getDefaultOrder();
    
    int* process=new int[size[0]*size[1]*size[2]];
    for (index=0; index<size[0]*size[1]*size[2]; index++) process[order[index]]=index;
    
    dataIndex=0;
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	index=process[i];
	getVoxelLocFromIndex(index,&x,&y,&z);
	if (z==slice)
	{
	    data[dataIndex++]=index;
	}
    }
    
    delete[] process;    
}

int Volume2DplusT::voxelCriticalInSlice(int index,int* order)
{
    int topoType=0,bit=0;
    int x,y,z,ox,oy,oz;
    int neighborIndex;
    
    if (!order) order=getDefaultOrder();
    getVoxelLocFromIndex(index,&x,&y,&z);
    
    for (oy=-1; oy<=1; oy++) for (ox=-1; ox<=1; ox++) if ((oy!=0) || (ox!=0))
    {
	if ((y+oy>=0) && (y+oy<size[1]) &&
	    (x+ox>=0) && (x+ox<size[0]))
	{
	    neighborIndex=getVoxelIndex(x+ox,y+oy,z);
	    //If the neighbor is before the current voxel, we set the bit.
	    if (order[neighborIndex]<order[index])
		topoType |= (1<<bit);
	}
	else
	{
	    //Out of range is considered large positive, so we do not set the bit, because we're increasing.
	    //topoType |= (1<<bit);
	}
	bit++;
    }
    if (sliceTopoCheck->topologyChange(topoType)) return 1;
    else return 0;
}

int Volume2DplusT::voxelCriticalInVolume(int index,int* order)
{
    int topoType=0,bit=0;
    int x,y,z,ox,oy,oz;
    int neighborIndex;
    
    if (!order) order=getDefaultOrder();
    getVoxelLocFromIndex(index,&x,&y,&z);
    
    for (oz=-1; oz<=1; oz++) for (oy=-1; oy<=1; oy++) for (ox=-1; ox<=1; ox++) if ((oz!=0) || (oy!=0) || (ox!=0))
    {
	if ((z+oz>=0) && (z+oz<size[2]) &&
	    (y+oy>=0) && (y+oy<size[1]) &&
	    (x+ox>=0) && (x+ox<size[0]))
	{
	    neighborIndex=getVoxelIndex(x+ox,y+oy,z+oz);
	    //If the neighbor is before the current voxel, we set the bit.
	    if (order[neighborIndex]<order[index])
		topoType |= (1<<bit);
	}
	else
	{
	    //Out of range is considered large positive, so we do not set the bit, because we're increasing.
	    //topoType |= (1<<bit);
	}
	bit++;
    }
    if (strictTopoCheck->topologyChange(topoType)) return 1;
    else return 0;
}

//leftOrRight==0 when the voxel is on the left side, and its right neighborhood is full.
//leftOrRight==1 when the voxel is on the right side, and its left neighborhood is full.
int Volume2DplusT::voxelCriticalInThickSlice(int index,int leftOrRight,int* order)
{
    int questionVoxel=-1;
    int topoType=0,bit=0;
    int x,y,z,ox,oy,oz,z0;
    int neighborIndex;
    
    if (index==questionVoxel)
    {
	cout << "Voxel " << index;
	cout << (leftOrRight?"(right)":"(left)");
	cout << "\n neighbors:\n";
    }
    
    if (!order) order=getDefaultOrder();
    getVoxelLocFromIndex(index,&x,&y,&z);
    
    if (leftOrRight) z0=-1;
    else z0=0;
    
    for (oz=z0; oz<=z0+1; oz++) for (oy=-1; oy<=1; oy++) for (ox=-1; ox<=1; ox++) if ((oz!=0) || (oy!=0) || (ox!=0))
    {
	if ((z+oz>=0) && (z+oz<size[2]) &&
	    (y+oy>=0) && (y+oy<size[1]) &&
	    (x+ox>=0) && (x+ox<size[0]))
	{
	    neighborIndex=getVoxelIndex(x+ox,y+oy,z+oz);
	    if (index==questionVoxel)
	    {
		cout << "  " << neighborIndex << "(" << x+ox << "," << y+oy << "," << z+oz << ")";
		cout << " " << order[neighborIndex];
		cout << "(" << ((order[neighborIndex]<order[index])?"-":"+") << ")";
		cout << "\n";
	    }
	    //If the neighbor is before the current voxel, we set the bit.
	    if (order[neighborIndex]<order[index])
		topoType |= (1<<bit);
	}
	else
	{
	    //Out of range is considered large positive, so we do not set the bit, because we're increasing.
	    //topoType |= (1<<bit);
	}
	bit++;
    }
	
    //Turn topotype around for lookup table
    if (leftOrRight) topoType=((topoType<<8)|(topoType>>9))&0x1ffff;
    
    if (index==questionVoxel)
    {
	cout << " topology change: " << (thickSliceTopoCheck->topologyChange(topoType)) << "\n";
    }
    
    if (thickSliceTopoCheck->topologyChange(topoType)) return 1;
    else return 0;
}

int Volume2DplusT::countCriticalsInSlices(int* order)
{
    int numCritical=0;
    cout << "Counting critical points in slices..."; cout.flush();
    
    for (int slice=0; slice<numSlices; slice++)
	numCritical+=countCriticalsInSlice(slice,order);
	
    cout << "done: " << numCritical << "\n";
    return numCritical;
}

int Volume2DplusT::countCriticalsInSlice(int slice,int* order)
{
    if (sliceTrees) return sliceTrees[slice].getNumNodes();
    int index;
    int numCritical=0;
    
    for (index=slice*size[1]*size[0]; index<(slice+1)*size[1]*size[0]; index++)
    {
	if (voxelCriticalInSlice(index,order))
	{
	    numCritical++;
	}
    }
    return numCritical;
}

int Volume2DplusT::countCriticalsInThickSlices(int* order)
{
    int numCritical=0;
    cout << "Counting critical points in thick slices..."; cout.flush();
    
    for (int slice=0; slice<numSlices-1; slice++)
	numCritical+=countCriticalsInThickSlice(slice,order);
    
    cout << "done: " << numCritical << "\n";
    return numCritical;
}

int Volume2DplusT::countCriticalsInThickSlice(int sliceLeft,int* order,int augment)
{
    if (thickSliceTrees) return thickSliceTrees[sliceLeft].getNumNodes();
    int index;
    int numCritical=0;
    
    for (index=sliceLeft*size[1]*size[0]; index<(sliceLeft+1)*size[1]*size[0]; index++)
    {
	if ((voxelCriticalInThickSlice(index,0,order)) || 
	    ((augment) && ((voxelCriticalInSlice(index,order)) || (voxelCriticalInThickSlice(index,1,order)))))
	{
	    numCritical++;
	}
	if ((voxelCriticalInThickSlice(index+size[1]*size[0],1,order)) || 
	    ((augment) && ((voxelCriticalInSlice(index+size[1]*size[0],order)) || (voxelCriticalInThickSlice(index+size[1]*size[0],0,order)))))
	{
	    numCritical++;
	}
    }
    return numCritical;
}

void Volume2DplusT::buildJoinTreeForSlice(int* order,int slice,ContourTree& jt)
{
    int minIndex=size[0]*size[1]*slice,maxIndex=size[0]*size[1]*(slice+1);
    int i,index;
    if (!criticalParent) criticalParent=new int[size[0]*size[1]*size[2]+1];
    for (i=0; i<size[0]*size[1]*size[2]; i++) criticalParent[i]=-1;
    criticalParent[i]=-1; //an extra point for the global max at infinity
    
    //set up processing order
    int* process=new int[size[0]*size[1]*size[2]];
    for (i=0; i<size[0]*size[1]*size[2]; i++) process[order[i]]=i;
    
    //Compute structure bottom-up
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	index=process[i];
	if ((minIndex<=index) && (index<maxIndex))
	{
	    if (voxelCriticalInSlice(index,order))
	    {
		criticalParent[index]=index;
		joinCriticalPoints(index,numNeighborsSliceVertex,neighborhoodSliceVertex,&jt);
	    }
	    else
	    {
		criticalParent[index]=findCriticalAncestorOfNeighbor(index,numNeighborsSliceVertex,neighborhoodSliceVertex);
		if (criticalParent[index]==-1) criticalParent[index]=size[0]*size[1]*size[2];
	    }
	}
    }
    
    //Finish tree
    index=size[2]*size[1]*size[0];
    std::vector<int> rootChildren;
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	if (criticalParent[i]==i)
	{
	    criticalParent[i]=index;
	    rootChildren.push_back(i);
	}
    }
    int* rootChildrenArray=new int[rootChildren.size()];
    int numRootChildren=0;
    for (std::vector<int>::iterator it=rootChildren.begin(); it!=rootChildren.end(); it++)
	rootChildrenArray[numRootChildren++]=*it;
    jt.addNewJoinNode(index,rootChildrenArray,rootChildren.size());
    delete[] rootChildrenArray;
    jt.findJoinRoot();
    
    delete[] process;
}

void Volume2DplusT::buildSplitTreeForSlice(int* order,int slice,ContourTree& st)
{
    int minIndex=size[0]*size[1]*slice,maxIndex=size[0]*size[1]*(slice+1);
    int i,index;
    if (criticalParent) delete[] criticalParent;
    criticalParent=new int[size[0]*size[1]*size[2]+1];
    for (i=0; i<size[0]*size[1]*size[2]; i++) criticalParent[i]=-1;
    criticalParent[i]=i; //an extra point for the global max at infinity
    
    //set up processing order
    int* process=new int[size[0]*size[1]*size[2]];
    for (i=0; i<size[0]*size[1]*size[2]; i++) process[order[i]]=i;
    
    //Compute structure bottom-up, leaves are at the end of the order
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	index=process[size[2]*size[1]*size[0]-1-i];
	if ((minIndex<=index) && (index<maxIndex))
	{
	    if (voxelCriticalInSlice(index,order))
	    {
		criticalParent[index]=index;
		joinCriticalPoints(index,numNeighborsFace,neighborhoodFace);
	    }
	    else
	    {
		criticalParent[index]=findCriticalAncestorOfNeighbor(index,numNeighborsFace,neighborhoodFace);
		if (criticalParent[index]==-1) criticalParent[index]=size[0]*size[1]*size[2];
		assert(criticalParent[index]!=-1);
	    }
	    
	}
    }
    
    //Build tree top-down
    for (i=0; i<size[2]*size[1]*size[0]; i++)
    {
	index=process[i];
	if ((minIndex<=index) && (index<maxIndex))
	{
	    if (voxelCriticalInSlice(index,order))
	    {
		st.addNewSplitNode(index,criticalParent[index]);
	    }
	}
    }
    st.addNewSplitNode(i,criticalParent[i]);
    st.findSplitRoot();
    
    delete[] process;
}

void Volume2DplusT::buildJoinTreeForThickSlice(int* order,int sliceLeft,ContourTree& jt,int augment,std::vector<int>* nodes)
{
    int questionVoxel=-1;
    int minIndex=size[0]*size[1]*sliceLeft,midIndex=size[0]*size[1]*(sliceLeft+1),maxIndex=size[0]*size[1]*(sliceLeft+2);
    int i,index;
    if (!criticalParent) criticalParent=new int[size[0]*size[1]*size[2]+1];
    for (i=0; i<size[0]*size[1]*size[2]; i++) criticalParent[i]=-1;
    criticalParent[i]=-1; //an extra point for the global max at infinity
    
    //set up processing order
    int* process=new int[size[0]*size[1]*size[2]];
    for (i=0; i<size[0]*size[1]*size[2]; i++) process[order[i]]=i;
    
    //Compute structure bottom-up
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	index=process[i];
	if ((minIndex<=index) && (index<midIndex))
	{
	    if ((voxelCriticalInThickSlice(index,0,order)) || 
		((augment) && ((voxelCriticalInSlice(index,order)) || (voxelCriticalInThickSlice(index,1,order)))))
	    {
		criticalParent[index]=index;
		joinCriticalPoints(index,numNeighborsThickLeftVertex,neighborhoodThickLeftVertex,&jt);
		if (index==questionVoxel) cout << "Voxel " << index << " critical ";
		if (nodes) nodes->push_back(index);
	    }
	    else
	    {
		criticalParent[index]=findCriticalAncestorOfNeighbor(index,numNeighborsThickLeftVertex,neighborhoodThickLeftVertex);
		if (criticalParent[index]==-1) cout << index << " (" << index%size[0] << "," << (index/size[0])%size[1] << "," << (index/size[0])/size[1] << ")\n";
		assert(criticalParent[index]!=-1);
		if (index==questionVoxel) cout << "Voxel " << index << " not critical ";
	    }
	    if (index==questionVoxel) cout << "on left.\n";
	}
	else if ((midIndex<=index) && (index<maxIndex))
	{
	    if ((voxelCriticalInThickSlice(index,1,order)) || 
		((augment) && ((voxelCriticalInSlice(index,order)) || (voxelCriticalInThickSlice(index,0,order)))))
	    {
		criticalParent[index]=index;
		joinCriticalPoints(index,numNeighborsThickRightVertex,neighborhoodThickRightVertex,&jt);
		if (index==questionVoxel) cout << "Voxel " << index << " critical ";
		if (nodes) nodes->push_back(index);
	    }
	    else
	    {
		criticalParent[index]=findCriticalAncestorOfNeighbor(index,numNeighborsThickRightVertex,neighborhoodThickRightVertex);
		if (criticalParent[index]==-1) cout << index << " (" << index%size[0] << "," << (index/size[0])%size[1] << "," << (index/size[0])/size[1] << ")\n";
		assert(criticalParent[index]!=-1);
		if (index==questionVoxel) cout << "Voxel " << index << " not critical ";
	    }
	    if (index==questionVoxel) cout << "on right.\n";
	}
    }
    
    //Finish tree
    index=size[2]*size[1]*size[0];
    if (nodes) nodes->push_back(index);
    std::vector<int> rootChildren;
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	if (criticalParent[i]==i)
	{
	    criticalParent[i]=index;
	    rootChildren.push_back(i);
	}
    }
    int* rootChildrenArray=new int[rootChildren.size()];
    int numRootChildren=0;
    for (std::vector<int>::iterator it=rootChildren.begin(); it!=rootChildren.end(); it++)
	rootChildrenArray[numRootChildren++]=*it;
    jt.addNewJoinNode(index,rootChildrenArray,rootChildren.size());
    delete[] rootChildrenArray;
    jt.findJoinRoot();
    
    delete[] process;
}

void Volume2DplusT::buildSplitTreeForThickSlice(int* order,int sliceLeft,ContourTree& st,int augment,std::vector<int>* nodes)
{
    int questionVoxel=-1;
    int minIndex=size[0]*size[1]*sliceLeft,midIndex=size[0]*size[1]*(sliceLeft+1),maxIndex=size[0]*size[1]*(sliceLeft+2);
    int i,index;
    if (!criticalParent) criticalParent=new int[size[0]*size[1]*size[2]+1];
    for (i=0; i<size[0]*size[1]*size[2]; i++) criticalParent[i]=-1;
    criticalParent[i]=i; //an extra point for the global max at infinity
    if (nodes) nodes->push_back(i);
    
    //set up processing order
    int* process=new int[size[0]*size[1]*size[2]];
    for (i=0; i<size[0]*size[1]*size[2]; i++) process[order[i]]=i;
    
    //Compute structure bottom-up, leaves are at the end of the process array
    for (i=0; i<size[0]*size[1]*size[2]; i++)
    {
	index=process[size[2]*size[1]*size[0]-1-i];
	if ((minIndex<=index) && (index<midIndex))
	{
	    if ((voxelCriticalInThickSlice(index,0,order)) || 
		((augment) && ((voxelCriticalInSlice(index,order)) || (voxelCriticalInThickSlice(index,1,order)))))
	    {
		criticalParent[index]=index;
		splitCriticalPoints(index,numNeighborsThickLeftFace,neighborhoodThickLeftFace,&st);
		if (index==questionVoxel) cout << "Voxel " << index << " critical ";
		if (nodes) nodes->push_back(index);
	    }
	    else
	    {
		criticalParent[index]=findCriticalAncestorOfNeighbor(index,numNeighborsThickLeftFace,neighborhoodThickLeftFace);
		if (criticalParent[index]==-1) cout << index << " (" << index%size[0] << "," << (index/size[0])%size[1] << "," << (index/size[0])/size[1] << ")\n";
		assert(criticalParent[index]!=-1);
		if (index==questionVoxel) cout << "Voxel " << index << " not critical ";
	    }
	    if (index==questionVoxel) cout << "on left.\n";
	}
	else if ((midIndex<=index) && (index<maxIndex))
	{
	    if ((voxelCriticalInThickSlice(index,1,order)) || 
		((augment) && ((voxelCriticalInSlice(index,order)) || (voxelCriticalInThickSlice(index,0,order)))))
	    {
		criticalParent[index]=index;
		splitCriticalPoints(index,numNeighborsThickRightFace,neighborhoodThickRightFace,&st);
		if (index==questionVoxel) cout << "Voxel " << index << " critical ";
		if (nodes) nodes->push_back(index);
	    }
	    else
	    {
		criticalParent[index]=findCriticalAncestorOfNeighbor(index,numNeighborsThickRightFace,neighborhoodThickRightFace);
		if (criticalParent[index]==-1) cout << index << " (" << index%size[0] << "," << (index/size[0])%size[1] << "," << (index/size[0])/size[1] << ")\n";
		assert(criticalParent[index]!=-1);
		if (index==questionVoxel) cout << "Voxel " << index << " not critical ";
	    }
	    if (index==questionVoxel) cout << "on right.\n";
	}
    }
    
    st.findSplitRoot();
    
    delete[] process;
}

void Volume2DplusT::buildTrees(int* order,int augment)
{
    sliceJoinTrees=new ContourTree[numSlices];
    thickSliceJoinTrees=new ContourTree[numSlices-1];
    //sliceTrees=new ContourTree[numSlices];
    //thickSliceTrees=new ContourTree[numSlices-1];

    cout << "Building contour trees for slices"; cout.flush();
    for (int i=0; i<numSlices; i++)
    {
	cout << "."; cout.flush();
	ContourTree jt,st;
	buildJoinTreeForSlice(order,i,jt);
	//buildSplitTreeForSlice(order,i,st);
	//ContourTree ct(jt,st);
	sliceJoinTrees[i]=jt;
	//sliceTrees[i]=ct;
    }
    cout << "thick slices"; cout.flush();
    for (int i=0; i<numSlices-1; i++)
    {
	cout << "."; cout.flush();
	ContourTree jt,st;
	buildJoinTreeForThickSlice(order,i,jt,augment);
	//buildSplitTreeForThickSlice(order,i,st,augment);
	//ContourTree ct(jt,st);
	thickSliceJoinTrees[i]=jt;
	//thickSliceTrees[i]=ct;
    }
    cout << "done.\n";
}

void Volume2DplusT::mergeJoinTrees(int* order,int sliceLeft,int sliceRight,ContourTree& jt,std::vector<int>* nodes)
{
    if (sliceRight-sliceLeft<=0)
    {
	sliceJoinTrees[sliceLeft].clone(jt);
	return;
    }
    if (sliceRight-sliceLeft==1)
    {
	if (thickSliceJoinTrees) thickSliceJoinTrees[sliceLeft].clone(jt,nodes);
	else buildJoinTreeForThickSlice(order,sliceLeft,jt,1,nodes);
	//cout << "Thick slice " << sliceLeft << ": " << jt.getNumNodes() << "," << nodes->size() << "\n";
	//jt.print();
	return;
    }
    
    ContourTree leftJT,rightJT;
    std::vector<int>* leftNodes=new std::vector<int>,* rightNodes=new std::vector<int>;
    /*mergeJoinTrees(order,sliceLeft,sliceRight-1,leftJT,leftNodes);
    if (thickSliceJoinTrees) thickSliceJoinTrees[sliceRight-1].clone(rightJT,rightNodes);
    else buildJoinTreeForThickSlice(order,sliceRight-1,rightJT,1,rightNodes);*/
    //cout << "Thick slice " << sliceRight-1 << ": " << rightJT.getNumNodes() << "," << rightNodes->size() << "\n";
    //rightJT.print();
    int sliceMid=(sliceRight-sliceLeft)/2+sliceLeft;
    mergeJoinTrees(order,sliceLeft,sliceMid,leftJT,leftNodes);
    mergeJoinTrees(order,sliceMid,sliceRight,rightJT,rightNodes);
    
    std::vector<int>::iterator l,r;
    l=leftNodes->begin();
    r=rightNodes->begin();
    while ((l!=leftNodes->end()) && (r!=rightNodes->end()))
    {
	if (*l==*r)
	{
	    nodes->push_back(*l);
	    l++;
	    r++;
	}
	else if ((*r==getNumVoxels()) || ((*l!=getNumVoxels()) && (order[*l]<order[*r])))
	{
	    nodes->push_back(*l);
	    l++;
	}
	else
	{
	    nodes->push_back(*r);
	    r++;
	}
    }
    while (l!=leftNodes->end())
    {
	nodes->push_back(*l);
	l++;
    }
    while (r!=rightNodes->end())
    {
	nodes->push_back(*r);
	r++;
    }
    
    delete leftNodes;
    delete rightNodes;
    
    jt.mergeJoinTrees(leftJT,rightJT,nodes);
    
    //simplify jt by removing noncritical nodes not in left or right slice
    std::map<int,TreeNode>::iterator i;
    std::vector<int> nodelist;
    int minIndex=(sliceRight-1)*getSliceSize();
    int maxIndex=minIndex+getSliceSize();
    for (i=jt.getBeginNode(); i!=jt.getEndNode(); i++)
	if ((minIndex<=i->first) && (i->first<maxIndex))
	    if ((i->second.lowerNeighbors.size()==1) && (i->second.upperNeighbors.size()==1))
		if (!voxelCriticalInVolume(i->first))
		{
		    jt.removeNodeAndSplice(i);
		    nodelist.push_back(i->first);
		}
    std::vector<int>::iterator vi;
    for (vi=nodelist.begin(); vi!=nodelist.end(); vi++)
    {
	jt.eraseNode(*vi);
    }
		    
    cout << "."; cout.flush();
}

void Volume2DplusT::mergeSplitTrees(int* order,int sliceLeft,int sliceRight,ContourTree& st,std::vector<int>* nodes)
{
    if (sliceRight-sliceLeft<=0)
    {
	sliceJoinTrees[sliceLeft].clone(st);
	return;
    }
    if (sliceRight-sliceLeft==1)
    {
	if (thickSliceSplitTrees) thickSliceSplitTrees[sliceLeft].clone(st,nodes);
	else buildSplitTreeForThickSlice(order,sliceLeft,st,1,nodes);
	//cout << "Thick slice " << sliceLeft << ": " << jt.getNumNodes() << "," << nodes->size() << "\n";
	//jt.print();
	return;
    }
    
    ContourTree leftST,rightST;
    std::vector<int>* leftNodes=new std::vector<int>,* rightNodes=new std::vector<int>;
    /*mergeSplitTrees(order,sliceLeft,sliceRight-1,leftST,leftNodes);
    if (thickSliceSplitTrees) thickSliceSplitTrees[sliceRight-1].clone(rightST,rightNodes);
    else buildSplitTreeForThickSlice(order,sliceRight-1,rightST,1,rightNodes);*/
    //cout << "Thick slice " << sliceRight-1 << ": " << rightST.getNumNodes() << "," << rightNodes->size() << "\n";
    //rightJT.print();
    int sliceMid=(sliceRight-sliceLeft)/2+sliceLeft;
    mergeSplitTrees(order,sliceLeft,sliceMid,leftST,leftNodes);
    mergeSplitTrees(order,sliceMid,sliceRight,rightST,rightNodes);
    
    std::vector<int>::iterator l,r;
    l=leftNodes->begin();
    r=rightNodes->begin();
    while ((l!=leftNodes->end()) && (r!=rightNodes->end()))
    {
	if (*l==*r)
	{
	    nodes->push_back(*l);
	    l++;
	    r++;
	}
	else if ((*r==getNumVoxels()) || ((*l!=getNumVoxels()) && (order[*l]>order[*r])))
	{
	    nodes->push_back(*l);
	    l++;
	}
	else
	{
	    nodes->push_back(*r);
	    r++;
	}
    }
    while (l!=leftNodes->end())
    {
	nodes->push_back(*l);
	l++;
    }
    while (r!=rightNodes->end())
    {
	nodes->push_back(*r);
	r++;
    }
    
    delete leftNodes;
    delete rightNodes;
    
    st.mergeSplitTrees(leftST,rightST,nodes);
    
    //simplify st by removing noncritical nodes not in left or right slice
    std::map<int,TreeNode>::iterator i;
    std::vector<int> nodelist;
    int minIndex=(sliceRight-1)*getSliceSize();
    int maxIndex=minIndex+getSliceSize();
    for (i=st.getBeginNode(); i!=st.getEndNode(); i++)
	if ((minIndex<=i->first) && (i->first<maxIndex))
	    if ((i->second.upperNeighbors.size()==1) && (i->second.lowerNeighbors.size()==1))
		if (!voxelCriticalInVolume(i->first))
		{
		    st.removeNodeAndSplice(i);
		    nodelist.push_back(i->first);
		}
    std::vector<int>::iterator vi;
    for (vi=nodelist.begin(); vi!=nodelist.end(); vi++)
    {
	st.eraseNode(*vi);
    }
    
    cout << "."; cout.flush();
}

//Contour tracking between adjacent slices without using contour trees.
//Currently tracks sublevel sets.
int Volume2DplusT::simpleTrack(int prevSlice,int curSlice,int numSeeds,int* seeds,unsigned short isovalue)
{
    bitc queued;
    queued.setbit(getSliceSize()); queued.resetbit(getSliceSize());
    std::queue<int> q;
    
    for (int seed=0; seed<numSeeds; seed++)
    {
	q.push(seeds[seed]);
	queued.setbit(seeds[seed]%getNumSlices());
    }
    numSeeds=0;
    while (!q.empty())
    {
	int x,y,z,ox,oy;
	getVoxelLocFromIndex(q.front(),&x,&y,&z);
	if (data[q.front()+(curSlice-prevSlice)*getSliceSize()]<=isovalue)
	    seeds[numSeeds++]=q.front()+(curSlice-prevSlice)*getSliceSize();
	q.pop();
	for (oy=-1; oy<=1; oy++) if ((0<=y+oy) && (y+oy<size[1]))
	    for (ox=-1; ox<=1; ox++) if ((0<=x+ox) && (x+ox<size[0]))
	    {
		int neighbor=x+ox+size[0]*(y+oy+size[1]*z);
		int pixel=x+ox+size[0]*(y+oy);
		if ((!queued.getbit(pixel)) && (data[neighbor]<=isovalue))
		{
		    q.push(neighbor);
		    queued.setbit(pixel);
		}
	    }
    }
    return numSeeds;
}

int Volume2DplusT::trackContour(int prevSlice,int curSlice,int numSeeds,int* seeds)
{
    if (!thickSliceTrees) buildTrees(getDefaultOrder(),1);
    //use inclusion-induced maps to track contour
    //update seeds array
    seeds[0]+=(curSlice-prevSlice)*getSliceSize();
    return numSeeds;
}

//Constructs join tree from carvedInsideOrder and stores result in given arrays.
int Volume2DplusT::getJoinTreeForSlice(int slice,int* critList,int* numCrit,int* edgeList,int simplified)
{
    ContourTree jt;
    buildJoinTreeForSlice(getDefaultOrder(),slice,jt);
    return getArraysFromCT(jt,critList,numCrit,edgeList,NULL,simplified);
}

//Constructs split tree from carvedInsideOrder and stores result in given arrays.
int Volume2DplusT::getSplitTreeForSlice(int slice,int* critList,int* numCrit,int* edgeList,int simplified)
{
    ContourTree st;
    buildSplitTreeForSlice(getDefaultOrder(),slice,st);
    return getArraysFromCT(st,critList,numCrit,edgeList,NULL,simplified);
}

//Constructs contour tree from carvedInsideOrder and stores result in given arrays.
int Volume2DplusT::getContourTreeForSlice(int slice,int* critList,int* numCrit,int* edgeList,int simplified)
{
    if (sliceTrees) return getArraysFromCT(sliceTrees[slice],critList,numCrit,edgeList,NULL,simplified);
    ContourTree jt,st;
    buildJoinTreeForSlice(getDefaultOrder(),slice,jt);
    //cout << "Join tree:\n";
    //jt.printJT(cout);
    buildSplitTreeForSlice(getDefaultOrder(),slice,st);
    //cout << "Split tree:\n";
    //st.printST(cout);
    ContourTree ct(jt,st);
    //cout << "Contour tree:\n";
    //ct.print(cout);
    return getArraysFromCT(ct,critList,numCrit,edgeList,NULL,simplified);
}

//Constructs contour tree from carvedInsideOrder and stores result in given arrays.
//The augment option adds all slice critical points to the tree for inclusion-induced map purposes.
// If augment is 1 or 2, the left or right slice (resp.) is used for the incl-ind. map.
int Volume2DplusT::getContourTreeForThickSlice(int sliceLeft,int* critList,int* numCrit,int* edgeList,int* edgeLabels,int simplified,int augment)
{
    if (thickSliceTrees) return getArraysFromCT(thickSliceTrees[sliceLeft],critList,numCrit,edgeList,edgeLabels,simplified);
    ContourTree jt,st;
    buildJoinTreeForThickSlice(getDefaultOrder(),sliceLeft,jt,augment);
    //cout << "Join tree:\n";
    //jt.printJT(cout);
    buildSplitTreeForThickSlice(getDefaultOrder(),sliceLeft,st,augment);
    //cout << "Split tree:\n";
    //st.printST(cout);
    ContourTree ct(jt,st);
    if (augment)
    {
	ContourTree sliceJT,sliceST;
	buildJoinTreeForSlice(getDefaultOrder(),sliceLeft+augment-1,sliceJT);
	buildSplitTreeForSlice(getDefaultOrder(),sliceLeft+augment-1,sliceST);
	ContourTree sliceCT(sliceJT,sliceST);
	ct.labelEdgesWithInducedMap(sliceCT);
	//cout << "Thick slice " << sliceLeft << "-" << sliceLeft+1 << '\n';
	//ct.printWithData(cout,data,size[2]*size[1]*size[0]);
	//cout << " slice      " << sliceLeft+augment-1 << '\n';
	//sliceCT.print(cout);
    }
    else
    {
	//cout << "Thick slice unaugmented " << sliceLeft << "-" << sliceLeft+1 << '\n';
	//ct.printWithData(cout,data,size[2]*size[1]*size[0]);
    }
    return getArraysFromCT(ct,critList,numCrit,edgeList,edgeLabels,simplified);
}

void Volume2DplusT::getContourTreesForSlices(int** critList,int** numCrit,int* numEdges,int** edgeList,int simplified)
{
    for (int i=0; i<numSlices; i++)
    {
	numEdges[i]=getContourTreeForSlice(i,critList[i],numCrit[i],edgeList[i],simplified);
    }
}

void Volume2DplusT::getContourTreesForThickSlices(int** critList,int** numCrit,int* numEdges,int** edgeList,int simplified)
{
    for (int i=0; i<numSlices-1; i++)
    {
	numEdges[i]=getContourTreeForThickSlice(i,critList[i],numCrit[i],edgeList[i],NULL,simplified);
    }
}

int Volume2DplusT::markVoxelsBelowVoxelInSlices(int v,int firstSlice,int lastSlice,int* order)
{
    return markVoxelsBelowVoxelInRange(v,0,size[0]-1,0,size[1]-1,firstSlice,lastSlice,order);
}

void Volume2DplusT::createFromMarchableVolume(MarchableVolume* v)
{
    volume::createFromMarchableVolume(v);
    numSlices=size[2];
}


#ifdef FIXTOP2DP
int main(int argc,char* argv[])
{
    if (argc<=2)
    {
	cerr << "Usage: " << argv[0] << " <input .v file> <output .v file> [threshold]\n";
	return 1;
    }
    
    unsigned short threshold=65535;
    if (argc>3)
    {
	threshold=atoi(argv[3]);
    }
    
    int result;
    
    Volume2DplusT v(threshold);
    
    v.readTopoinfoFiles();
    
    result=v.readFile(argv[1]);
    if (result) return result;
    //v.createHistogram(); return 0;
    
    cout << v.getSize()[0] << 'x' << v.getSize()[1] << 'x' << v.getSize()[2] << '=';
    cout << v.getSize()[0]*v.getSize()[1]*v.getSize()[2] << '\n';
    
    //int signsNeedToBeChanged=v.dataShouldBeNegated();
    //if (signsNeedToBeChanged) v.changeAllSigns();
    
    unsigned short min=65535,max=0;
    for (int i=0; i<v.getSize()[2]*v.getSize()[1]*v.getSize()[0]; i++)
    {
	if (v.getVoxel(i)<min) min=v.getVoxel(i);
	if (v.getVoxel(i)>max) max=v.getVoxel(i);
    }
    cout << "Min: " << min << ", Max: " << max << "\n";
    
    clock_t starttime = clock();
    v.fixTopologyStrict();
    //v.fixTopologyBySlices();
    clock_t endtime = clock();
    cout << "Topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";
    
    if (0) //count violations of 1-to-1
    {
	cout << "Checking contour trees for thick slices"; cout.flush();
	
	int vio0=0,vio1=0;
	ContourTree jt0,st0,jt1,st1,jt,st;
	v.buildJoinTreeForSlice(v.getDefaultOrder(),0,jt0);
	v.buildSplitTreeForSlice(v.getDefaultOrder(),0,st0);
	ContourTree ct0(jt0,st0);
	
	int s;
	for (s=0; s<v.getNumSlices()-1; s++)
	{
	    jt1.clear();
	    st1.clear();
	    v.buildJoinTreeForSlice(v.getDefaultOrder(),s+1,jt1);
	    v.buildSplitTreeForSlice(v.getDefaultOrder(),s+1,st1);
	    ContourTree ct1(jt1,st1);
	    jt.clear();
	    st.clear();
	    v.buildJoinTreeForThickSlice(v.getDefaultOrder(),s,jt,1);
	    v.buildSplitTreeForThickSlice(v.getDefaultOrder(),s,st,1);
	    ContourTree ct(jt,st);
	    ct.labelEdgesWithInducedMap(ct0);
	    //vio0+=ct.countInducedMapManyToOnes(v.getData(),v.getNumVoxels());
	    vio0+=ct.countInducedMapManyToOnes();
	    cout << ct.countInducedMapManyToOnes() << " ";
	    ct.labelEdgesWithInducedMap(ct1);
	    //vio1+=ct.countInducedMapManyToOnes(v.getData(),v.getNumVoxels());
	    vio1+=ct.countInducedMapManyToOnes();
	    cout << ct.countInducedMapManyToOnes() << "\n";
	    
	    ct0=ct1;
	    cout << "."; cout.flush();
	}
	cout << "done: " << vio0 << " left violations and " << vio1 << " right violations." << "\n";
    }
    
    if (0) //merge thick slice join trees
    {
	ContourTree jt1,jt2;
	v.buildJoinTreeForThickSlice(v.getDefaultOrder(),0,jt1,1);
	v.buildJoinTreeForThickSlice(v.getDefaultOrder(),1,jt2,1);

	/*cout << "Join tree for thick slice 0-1\n";
	jt1.printWithData(cout,v.getData(),v.getNumVoxels());
	cout << "Join tree for thick slice 1-2\n";
	jt2.printWithData(cout,v.getData(),v.getNumVoxels());*/
	
	ContourTree jt;
	std::vector<int>* nodes=new std::vector<int>;
	//v.mergeJoinTrees(v.getDefaultOrder(),0,2,jt,nodes);
	delete nodes;
	/*cout << "Merged join tree for slices 0-2\n";
	jt.printWithData(cout,v.getData(),v.getNumVoxels());*/
	
	ContourTree fullJT;
	nodes=new std::vector<int>;
	cout << "\nConstructing merged join tree for all slices"; cout.flush();
	v.mergeJoinTrees(v.getDefaultOrder(),0,v.getNumSlices(),fullJT,nodes);
	delete nodes;
	cout << "done:\n";
	fullJT.printWithData(cout,v.getData(),v.getNumVoxels());
    }
    
    //if (signsNeedToBeChanged) v.changeAllSigns();
    cout << "Writing to file " << argv[2] << " ..."; cout.flush();
    result=v.writeFile(argv[2]);
    if (result)
    {
	cout << "error!\n";
	return result;
    }
    else cout << "done.\n";
    v.saveOrder("carvingorder.vo");
    
    /*if (signsNeedToBeChanged) v.changeAllSigns();
    
    ContourTree jt0,st0,jt1,st1,jt,st,jta,sta;
    v.buildJoinTreeForSlice(v.getDefaultOrder(),0,jt0);
    v.buildSplitTreeForSlice(v.getDefaultOrder(),0,st0);
    ContourTree ct0(jt0,st0);
    v.buildJoinTreeForSlice(v.getDefaultOrder(),1,jt1);
    v.buildSplitTreeForSlice(v.getDefaultOrder(),1,st1);
    ContourTree ct1(jt1,st1);
    v.buildJoinTreeForThickSlice(v.getDefaultOrder(),0,jt,0);
    v.buildSplitTreeForThickSlice(v.getDefaultOrder(),0,st,0);
    ContourTree ct(jt,st);
    v.buildJoinTreeForThickSlice(v.getDefaultOrder(),0,jta,1);
    v.buildSplitTreeForThickSlice(v.getDefaultOrder(),0,sta,1);
    ContourTree cta0(jta,sta);
    cta0.labelEdgesWithInducedMap(ct0);
    ContourTree cta1(jta,sta);
    cta1.labelEdgesWithInducedMap(ct1);
    
    cout << "\nContour tree for thick slice 0-1\n";
    ct.printWithData(cout,v.getData(),v.getNumVoxels());
    cout << "\nContour tree for slice 0\n";
    ct0.printWithData(cout,v.getData(),v.getNumVoxels());
    cout << "\nContour tree for slice 1\n";
    ct1.printWithData(cout,v.getData(),v.getNumVoxels());
    cout << "\nContour tree for thick slice 0-1 augmented by slice 0\n";
    cta0.printWithData(cout,v.getData(),v.getNumVoxels());
    cout << "Weighted number of 1-1 violations in inclusion map: ";
    int vio0=cta0.countInducedMapManyToOnes(v.getData(),v.getNumVoxels());
    cout << vio0 << "\n";
    cout << "\nContour tree for thick slice 0-1 augmented by slice 1\n";
    cta1.printWithData(cout,v.getData(),v.getNumVoxels());
    cout << "Weighted number of 1-1 violations in inclusion map: ";
    int vio1=cta1.countInducedMapManyToOnes(v.getData(),v.getNumVoxels());
    cout << vio1 << "\n";*/
    

    return 0;
}
#endif
