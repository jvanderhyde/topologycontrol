//TopoCheck.cpp
//James Vanderhyde, 29 November 2005

#include <iostream.h>
#include <fstream.h>

#include "TopoCheck.h"

#define BIT(b,n) (!!((n)&(1<<(b))))
#define SETBIT(b,n) ((n)|(1<<(b)))
#define CLRBIT(b,n) ((n)&(~(1<<(b))))

TopoCheck::TopoCheck() {}
TopoCheck::~TopoCheck() {}

TopoCheckNever::TopoCheckNever() {}
TopoCheckNever::~TopoCheckNever() {}

int TopoCheckNever::topologyChange(int neighborhood)
{
  return 0;
}

int TopoCheckNever::getDimension()
{
    return 0;
}

TopoCheckFromFile::TopoCheckFromFile()
{
  lookupTable=NULL;
}

TopoCheckFromFile::~TopoCheckFromFile()
{
  if (lookupTable) delete[] lookupTable;
}

int TopoCheckFromFile::readTopoinfoFile(char* filename)
{
  ifstream fin(filename);
  if (fin)
    {
      if (lookupTable) delete[] lookupTable;
      lookupTable=new unsigned char[8*1024*1024]; // 2^26/8 = 2^23 = 8M
      fin.read((char*)lookupTable,8*1024*1024);
      return 0;
    }
  else
    {
      cerr << "Can't open topoinfo file " << filename << '\n';
      return 1;
    }
}

int TopoCheckFromFile::topologyChange(int neighborhood)
{
  return !!(lookupTable[neighborhood >> 3] & (1<<(neighborhood & 0x07)));
}

int TopoCheckFromFile::getDimension()
{
    return 3;
}

TopoCheckStrict::TopoCheckStrict() : TopoCheckFromFile()
{
  readTopoinfoFile("topoinfo_vertex_all");
}

TopoCheckPatch::TopoCheckPatch() : TopoCheckFromFile()
{
  readTopoinfoFile("topoinfo_vertex_patch");
}

TopoCheckThread::TopoCheckThread() : TopoCheckFromFile()
{
  readTopoinfoFile("topoinfo_vertex_thread");
}

TopoCheck2DFromFile::TopoCheck2DFromFile() : TopoCheckFromFile()
{
}

int TopoCheck2DFromFile::readTopoinfoFile(char* filename)
{
  ifstream fin(filename);
  if (fin)
    {
      if (lookupTable) delete[] lookupTable;
      lookupTable=new unsigned char[32]; // 2^8/8 = 2^5 = 32
      fin.read((char*)lookupTable,32);
      return 0;
    }
  else
    {
      cerr << "Can't open topoinfo file " << filename << '\n';
      return 1;
    }
}

int TopoCheck2DFromFile::getDimension()
{
    return 2;
}

TopoCheck2DVertex::TopoCheck2DVertex() : TopoCheck2DFromFile()
{
  readTopoinfoFile("topoinfo2D_vertex");
}

TopoCheck2D3DCombo::TopoCheck2D3DCombo() : TopoCheck()
{
}

int TopoCheck2D3DCombo::topologyChange(int neighborhood)
{
  return (topo2D.topologyChange((neighborhood>>9)&((1<<8)-1)) || topo3D.topologyChange(neighborhood));
}

int TopoCheck2D3DCombo::getDimension()
{
    return 3;
}

TopoCheckGraph::TopoCheckGraph() : TopoCheck()
{
    dimension=0;
    numNodes=0;
    nodeConnectionLengths=NULL;
    nodeConnections=NULL;
    //if ((!0)!=(1)) cerr << "Error in math: !0 is not equal to 1\n";
    stack=new int[256];
}

TopoCheckGraph::~TopoCheckGraph()
{
    if (nodeConnectionLengths)
    {
	for (int i=0; i<numNodes; i++)
	{
	    delete[] nodeConnections[i];
	}
	delete[] nodeConnectionLengths;
    }
}

int TopoCheckGraph::fixForConnectivityModel(int neighborhood)
{
    return neighborhood;
}

int TopoCheckGraph::topologyChange(int neighborhood)
{
    int i;
    int numMarked;
    int labels;
    int n,nn;
    int numLabeled;
    
    neighborhood=fixForConnectivityModel(neighborhood);
    
    for (int insideOutside=0; insideOutside<2; insideOutside++)
    {
	numMarked=0;
	stackPointer=0;
	for (i=0; i<numNodes; i++)
	{
	    if (BIT(i,neighborhood))
	    {
		numMarked++;
		stack[stackPointer]=i; //set the first thing in the stack to a marked node
	    }
	}
	//if (numMarked==0) cout << (char)('a'+insideOutside);
	if (numMarked==0) return 1; //0 components of intersection/complement
	stackPointer++;
	labels=0;
	numLabeled=0;
	
	while (stackPointer>0)
	{
	    n=stack[--stackPointer];
	    if (!BIT(n,labels))
	    {
		labels=SETBIT(n,labels);
		numLabeled++;
		for (i=0; i<nodeConnectionLengths[n]; i++)
		{
		    nn=nodeConnections[n][i];
		    if ((BIT(nn,neighborhood)) && (!BIT(nn,labels)))
			stack[stackPointer++]=nn;
		}
	    }
	}
	//if (numMarked != numLabeled) cout << (char)('c'+insideOutside);
	if (numMarked != numLabeled) return 1; //more than 1 component of intersection/complement
	neighborhood=(~neighborhood)&0x1ffff;
    }    
    //cout << 'e';
    return 0;
}

int TopoCheckGraph::saveTopoinfoFile(char* filename)
{
    ofstream fout(filename);
    if (fout)
    {
	int bit,byte,numBytes=1<<(numNodes-3);
	unsigned char* lookupTable=new unsigned char[numBytes];
	for (byte=0; byte<numBytes; byte++) 
	{
	    lookupTable[byte]=0;
	    for (bit=0; bit<8; bit++) lookupTable[byte]|=(topologyChange((byte<<3)+bit)<<bit);
	}
	fout.write((char*)lookupTable,numBytes);
	return 0;
    }
    else
    {
	cerr << "Can't open topoinfo file " << filename << " for writing.\n";
	return 1;
    }
}

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


