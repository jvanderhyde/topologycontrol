//TopoCheck4D.cpp
//James Vanderhyde, 14 May 2007

#include <iostream.h>
#include <fstream.h>

#include "TopoCheck4D.h"

#define BIT(b,n) (!!((n)&(1<<(b))))
#define SETBIT(b,n) ((n)|(1<<(b)))
#define CLRBIT(b,n) ((n)&(~(1<<(b))))

TopoCheck4D::TopoCheck4D() {}
TopoCheck4D::~TopoCheck4D() {}

int TopoCheck4D::getDimension()
{
    return 4;
}


TopoCheck4DLowerDim::TopoCheck4DLowerDim(TopoCheck* new_topoLowD) : TopoCheck4D()
{
    topoLowD=new_topoLowD;
}

TopoCheck4DLowerDim::~TopoCheck4DLowerDim()
{
    delete topoLowD;
}

int TopoCheck4DLowerDim::topologyChange(bool* neighborhood)
{
    int nbhd=0,bit;
    for (bit=27; bit<27+26; bit++) if (neighborhood[bit]) nbhd|=(1<<(bit-27));
    return topoLowD->topologyChange(nbhd);
}

int TopoCheck4DLowerDim::getDimension()
{
    return topoLowD->getDimension();
}


TopoCheck4DRestrictZ::TopoCheck4DRestrictZ(TopoCheck4D* new_topoCheck) : TopoCheck4D()
{
    topoCheck=new_topoCheck;
    int zBit=0;
    for (int bit=0; bit<80; bit++)
    {
	int index=bit;
	if (bit>=40) index++;
	index/=3;
	index/=3;
	int z=index%3;
	//cout << bit;
	//if (z!=1) cout << "\t" << zBit;
	//cout << '\n';
	assert(zBit<54);
	if (z!=1) zBits[zBit++]=bit;
    }
    assert(zBit==54);
}

TopoCheck4DRestrictZ::~TopoCheck4DRestrictZ()
{
    delete topoCheck;
}

int TopoCheck4DRestrictZ::topologyChange(bool* neighborhood)
{
    int r,bit;
    //printNeighborhood(neighborhood);
    for (bit=0; bit<54; bit++) neighborhood[zBits[bit]]=false;
    //printNeighborhood(neighborhood);
    r=topoCheck->topologyChange(neighborhood);
    for (bit=0; bit<54; bit++) neighborhood[zBits[bit]]=true;
    //printNeighborhood(neighborhood);
    //cout << "\n-\n";
    return r || topoCheck->topologyChange(neighborhood);
}

int TopoCheck4DRestrictZ::getDimension()
{
    return topoCheck->getDimension();
}


//Helper functions for TopoCheckGraph4D

int checkRange(int d1,int d2,int d3,int d4)
{
    if (d1<0) return 0;
    if (d2<0) return 0;
    if (d3<0) return 0;
    if (d4<0) return 0;
    if (d1>2) return 0;
    if (d2>2) return 0;
    if (d3>2) return 0;
    if (d4>2) return 0;
    if ((d1==1) && (d2==1) && (d3==1) && (d4==1)) return 0;
    return 1;
}

int cellToInt(int d1,int d2,int d3,int d4)
{
    if (!checkRange(d1,d2,d3,d4)) return -1;
    int index=(((d4)*3+d3)*3+d2)*3+d1; //value between 0 and 80
    if (index>40) return index-1;
    else return index;
}

int* getCellNeighbors(int d1,int d2,int d3,int d4)
{
    int* neighbors=new int[8];
    int numNeighbors=0;
    int n;
    n=cellToInt(d1-1,d2,d3,d4);
    if (n>=0) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2-1,d3,d4);
    if (n>=0) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2,d3-1,d4);
    if (n>=0) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2,d3,d4-1);
    if (n>=0) neighbors[numNeighbors++]=n;
    n=cellToInt(d1+1,d2,d3,d4);
    if (n>=0) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2+1,d3,d4);
    if (n>=0) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2,d3+1,d4);
    if (n>=0) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2,d3,d4+1);
    if (n>=0) neighbors[numNeighbors++]=n;
    neighbors[numNeighbors]=-1;//sentinel
    assert(numNeighbors<8);
    return neighbors;
}

//cell is 0 through 79, corresponding to a 4D voxel neighbor
int* getCellNeighbors(int cell)
{
    int d1,d2,d3,d4;
    if (cell>=40) cell++;
    d1=cell%3;
    cell/=3;
    d2=cell%3;
    cell/=3;
    d3=cell%3;
    cell/=3;
    d4=cell%3;
    return getCellNeighbors(d1,d2,d3,d4);
}

int getCellDim(int d1,int d2,int d3,int d4)
{
    int r=0;
    if (d1%2==1) r++;
    if (d2%2==1) r++;
    if (d3%2==1) r++;
    if (d4%2==1) r++;
    return r;
}

int getCellDim(int cell)
{
    int d1,d2,d3,d4;
    if (cell>=40) cell++;
    d1=cell%3;
    cell/=3;
    d2=cell%3;
    cell/=3;
    d3=cell%3;
    cell/=3;
    d4=cell%3;
    return getCellDim(d1,d2,d3,d4);
}

int* getLowerDimNeighbors(int d1,int d2,int d3,int d4)
{
    int dim=getCellDim(d1,d2,d3,d4);
    int* neighbors=new int[8];
    int numNeighbors=0;
    int n;
    n=cellToInt(d1-1,d2,d3,d4);
    if ((n>=0) && (getCellDim(n)<dim)) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2-1,d3,d4);
    if ((n>=0) && (getCellDim(n)<dim)) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2,d3-1,d4);
    if ((n>=0) && (getCellDim(n)<dim)) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2,d3,d4-1);
    if ((n>=0) && (getCellDim(n)<dim)) neighbors[numNeighbors++]=n;
    n=cellToInt(d1+1,d2,d3,d4);
    if ((n>=0) && (getCellDim(n)<dim)) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2+1,d3,d4);
    if ((n>=0) && (getCellDim(n)<dim)) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2,d3+1,d4);
    if ((n>=0) && (getCellDim(n)<dim)) neighbors[numNeighbors++]=n;
    n=cellToInt(d1,d2,d3,d4+1);
    if ((n>=0) && (getCellDim(n)<dim)) neighbors[numNeighbors++]=n;
    neighbors[numNeighbors]=-1; //sentinel
    assert(numNeighbors<8);
    return neighbors;
}

//cell is 0 through 79, corresponding to a 4D voxel neighbor
int* getLowerDimNeighbors(int cell)
{
    int d1,d2,d3,d4;
    if (cell>=40) cell++;
    d1=cell%3;
    cell/=3;
    d2=cell%3;
    cell/=3;
    d3=cell%3;
    cell/=3;
    d4=cell%3;
    return getLowerDimNeighbors(d1,d2,d3,d4);
}

int getListLength(int* list)
{
    int r=0;
    while (list[r]!=-1) r++;
    return r;
}


TopoCheckGraph4D::TopoCheckGraph4D() : TopoCheck4D()
{
    dimension=4;
    numNodes=0;
    nodeConnectionLengths=NULL;
    nodeConnections=NULL;
    nodeConnectionLDLengths=NULL;
    nodeConnectionsLD=NULL;
    hiDimNodes=NULL;
    
    stack=new int[256];
}

TopoCheckGraph4D::~TopoCheckGraph4D()
{
    if (nodeConnectionLengths)
    {
	for (int i=0; i<numNodes; i++)
	{
	    delete[] nodeConnections[i];
	}
	delete[] nodeConnectionLengths;
    }
    if (nodeConnectionLDLengths)
    {
	for (int i=0; i<numNodes; i++)
	{
	    delete[] nodeConnectionsLD[i];
	}
	delete[] nodeConnectionLDLengths;
	delete[] nodeConnectionsLD;
    }
    if (hiDimNodes) delete[] hiDimNodes;
    delete[] stack;
}

int TopoCheckGraph4D::topologyChange(bool* neighborhood)
{
    int i;
    int numMarked;
    bool labels[80];
    int n,nn;
    int numLabeled;
    
    fixForConnectivityModel(neighborhood);
    
    for (int insideOutside=0; insideOutside<2; insideOutside++)
    {
	numMarked=0;
	stackPointer=0;
	for (i=0; i<numNodes; i++)
	{
	    if (neighborhood[i])
	    {
		numMarked++;
		stack[stackPointer]=i; //set the first thing in the stack to a marked node
	    }
	}
	//if (numMarked==0) cout << (char)('a'+insideOutside);
	if (numMarked==0) return 1; //0 components of intersection/complement
	stackPointer++;
	for (i=0; i<80; i++) labels[i]=false;
	numLabeled=0;
	
	while (stackPointer>0)
	{
	    n=stack[--stackPointer];
	    if (!labels[n])
	    {
		labels[n]=true;
		numLabeled++;
		for (i=0; i<nodeConnectionLengths[n]; i++)
		{
		    nn=nodeConnections[n][i];
		    if ((neighborhood[nn]) && (!labels[nn]))
			stack[stackPointer++]=nn;
		}
	    }
	}
	//if (numMarked != numLabeled) cout << (char)('c'+insideOutside);
	if (numMarked != numLabeled) return 1; //more than 1 component of intersection/complement
	for (i=0; i<80; i++) neighborhood[i]=!neighborhood[i];
    }    
    //cout << 'e';
    int chi=calcEulerCh(neighborhood);
    if (chi!=1) return 1;
    return 0;
}

int TopoCheckGraph4D::calcEulerCh(bool* neighborhood)
{
    int chi=0,dim;
    for (int bit=0; bit<80; bit++) if (neighborhood[bit])
    {
	dim=getCellDim(bit);
	if (dim%2==0) chi++;
	else chi--;
    }
    return chi;
}

void TopoCheck4D::printNeighborhood(bool* neighborhood)
{
    int x,y,z,t;
    for (t=0; t<3; t++)
    {
	for (y=0; y<3; y++)
	{
	    for (z=0; z<3; z++)
	    {
		for (x=0; x<3; x++)
		{
		    if ((t!=1) || (z!=1) || (y!=1) || (x!=1))
		    {
			if (neighborhood[cellToInt(x,y,z,t)]) cout << "*";
			else cout << ".";
		    }
		    else cout << "o";
		    cout << " ";
		}
		cout << "   ";
	    }
	    cout << "\n";
	}
	cout << "\n";
    }
}

void TopoCheckGraph4D::printNeighborhood(bool* neighborhood)
{
    cout << "Neighborhood:\n";
    TopoCheck4D::printNeighborhood(neighborhood);
    cout << "Fixed:\n";
    fixForConnectivityModel(neighborhood);
    TopoCheck4D::printNeighborhood(neighborhood);
}


TopoCheckStrict4D::TopoCheckStrict4D() : TopoCheckGraph4D()
{
    numNodes=80;
    
    nodeConnectionLDLengths=new int[numNodes];
    nodeConnectionsLD=new int*[numNodes];
    numHiDimNodes=0;
    for (int n=0; n<numNodes; n++)
    {
	nodeConnectionsLD[n]=getLowerDimNeighbors(n);
	nodeConnectionLDLengths[n]=getListLength(nodeConnectionsLD[n]);
	if (getCellDim(n)>0) numHiDimNodes++;
    }
    hiDimNodes=new int[numHiDimNodes];
    numHiDimNodes=0;
    for (int d=3; d>=1; d--)
    {
	for (int n=0; n<numNodes; n++)
	    if (getCellDim(n)==d) hiDimNodes[numHiDimNodes++]=n;
    }
    
    nodeConnectionLengths=new int[numNodes];
    nodeConnections=new int*[numNodes];
    for (int n=0; n<numNodes; n++)
    {
	nodeConnections[n]=getCellNeighbors(n);
	nodeConnectionLengths[n]=getListLength(nodeConnections[n]);
    }
}

void TopoCheckStrict4D::fixForConnectivityModel(bool* neighborhood)
{
    int cell,hd,n;
    for (hd=0; hd<numHiDimNodes; hd++)
    {
	cell=hiDimNodes[hd];
	if (neighborhood[cell]) 
	    for (n=0; n<nodeConnectionLDLengths[cell]; n++) 
		neighborhood[nodeConnectionsLD[cell][n]]=true;
    }
}


TopoCheck3D4DCombo::TopoCheck3D4DCombo()
{}

int TopoCheck3D4DCombo::topologyChange(bool* neighborhood)
{
    int nbhd=0,bit;
    for (bit=27; bit<27+26; bit++) if (neighborhood[bit]) nbhd|=(1<<(bit-27));
    return (topo3D.topologyChange(nbhd)) || (topo4D.topologyChange(neighborhood));
}


TopoCheckThick3D4DDouble::TopoCheckThick3D4DDouble() : TopoCheck4D()
{
}

int TopoCheckThick3D4DDouble::topologyChange(bool* neighborhood)
{
    int r,bit;
    for (bit=0; bit<27; bit++) neighborhood[bit]=false;
    r=topo4D.topologyChange(neighborhood);
    for (bit=0; bit<27; bit++) neighborhood[bit]=true;
    return r || topo4D.topologyChange(neighborhood);
}


/*
//For thick slice in 2D, one side of the boundary of a cube x:
//
//  0 1 2    8  9 10
//  3 x 4   11 12 13
//  5 6 7   14 15 16

TopoCheckThick2D3D::TopoCheckThick2D3D() : TopoCheckGraph()
{
    const int d=3;
    const int nn=17;
    dimension=d;
    numNodes=nn;
    int cl[nn]={3,3,3, 3,3, 3,3,3, 3,4,3, 4,4,4, 3,4,3};
    int conn[nn][2*d]={
	{1,3,8},
	{0,2,9},
	{1,4,10},
	{0,5,11},
	{2,7,13},
	{3,6,14},
	{5,7,15},
	{4,6,16},
	{0,9,11},
	{1,8,10,12},
	{2,9,13},
	{3,8,12,14},
	{9,11,13,15},
	{4,10,12,16},
	{5,11,15},
	{6,12,14,16},
	{7,13,15}
    };
    nodeConnectionLengths=new int[numNodes];
    nodeConnections=new int*[numNodes];
    for (int n=0; n<numNodes; n++)
    {
	nodeConnectionLengths[n]=cl[n];
	nodeConnections[n]=new int[nodeConnectionLengths[n]];
	for (int c=0; c<nodeConnectionLengths[n]; c++)
	{
	    nodeConnections[n][c]=conn[n][c];
	}
    }
}

int TopoCheckThick2D3D::fixForConnectivityModel(int neighborhood)
{
    //Vertex connectivity on thick slice
    if (BIT(1,neighborhood)) neighborhood |= 0x205; //(1<<0)|(1<<2)|(1<<9)
    if (BIT(3,neighborhood)) neighborhood |= 0x821; //(1<<0)|(1<<5)|(1<<11)
    if (BIT(4,neighborhood)) neighborhood |= 0x2084; //(1<<2)|(1<<7)|(1<<13)
    if (BIT(6,neighborhood)) neighborhood |= 0x80a0; //(1<<5)|(1<<7)|(1<<15)
    if (BIT(12,neighborhood)) neighborhood |= 0xaa00; //(1<<9)|(1<<11)|(1<<13)|(1<<15)
    if (BIT(0,neighborhood)) neighborhood |= 0x100; //(1<<8)
    if (BIT(2,neighborhood)) neighborhood |= 0x400; //(1<<10)
    if (BIT(5,neighborhood)) neighborhood |= 0x4000; //(1<<14)
    if (BIT(7,neighborhood)) neighborhood |= 0x10000; //(1<<16)
    if (BIT(9,neighborhood)) neighborhood |= 0x500; //(1<<8)|(1<<10)
    if (BIT(11,neighborhood)) neighborhood |= 0x4100; //(1<<8)|(1<<14)
    if (BIT(13,neighborhood)) neighborhood |= 0x10400; //(1<<10)|(1<<16)
    if (BIT(15,neighborhood)) neighborhood |= 0x14000; //(1<<14)|(1<<16)
    return neighborhood;
}

int TopoCheckThick2D3D::getDimension()
{
    return 3;
}

TopoCheckThick2D3DTable::TopoCheckThick2D3DTable() : TopoCheckFromFile()
{
    readTopoinfoFile("topoinfo_vertex_thick");
}

*/
