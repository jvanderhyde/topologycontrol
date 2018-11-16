//reversemc.cpp
//James Vanderhyde, 23 June 2007

#include "TriangleMesh.h"
#include "../bitc.h"

#include <vector>
#include <queue>
#include <algorithm>
#include <math.h>

const float reliabilityThreshold=0.00002; //fraction of a unit voxel width //0.00001562

const float rangeMin=-0.05,rangeMax=0.05;
const int numVoxelsOnASide=256;


class IntTriple
{
public:
    int x,y,z;
    float parameter;
};

bool operator<(const IntTriple& a, const IntTriple& b)
{
    if (a.z<b.z) return true;
    if (a.z>b.z) return false;
    if (a.x<b.x) return true;
    if (a.x>b.x) return false;
    if (a.y<b.y) return true;
    if (a.y>b.y) return false;
    return false;
}

bool operator!=(const IntTriple& a, const IntTriple& b)
{
    if (a.z!=b.z) return true;
    if (a.y!=b.y) return true;
    if (a.x!=b.x) return true;
    return false;
}


TriangleMesh mesh;
std::vector<IntTriple> xDiffs, yDiffs, zDiffs;
int numUnreliableVertices;
int numOffCellVertices;
bitc inside;
float* data=NULL;
int yMin=4,yMax=97;
int reductionFactor=3;
float unitFactor=2.0;

void coalesceDiffList(std::vector<IntTriple>& diffs)
{
    //assumes sorted
    if (diffs.empty()) return;
    std::vector<IntTriple> coalesced;
    std::vector<IntTriple>::iterator i;
    for (i=diffs.begin(); i!=diffs.end(); ++i)
	coalesced.push_back(*i);
    diffs.clear();
    IntTriple prev;
    i=coalesced.begin();
    diffs.push_back(*i);
    prev=*i;
    ++i;
    for (i; i!=coalesced.end(); ++i)
    {
	if (*i!=prev)
	{
	    diffs.push_back(*i);
	    prev=*i;
	}
    }
}

float coordToCell(float vertCoord)
{
    return (vertCoord-rangeMin)/(rangeMax-rangeMin)*numVoxelsOnASide;
}

void addToXDiffList(float cx,float cy,float cz)
{
    //The vertex is between voxels (t.x,t.y,t.z) and (t.x+1,t.y,t.z)
    IntTriple t;
    t.x=(int)floor(cx);
    t.y=(int)floor(cy);
    t.z=(int)floor(cz);
    t.parameter=(cx+1.5)-floor(cx+1.5);
    //if (t.parameter<0.001) cout << 'x' << t.parameter << ' ';
    xDiffs.push_back(t);
}

void addToYDiffList(float cx,float cy,float cz)
{
    //The vertex is between voxels (t.x,t.y,t.z) and (t.x,t.y+1,t.z)
    IntTriple t;
    t.x=(int)floor(cx);
    t.y=(int)floor(cy);
    t.z=(int)floor(cz);
    t.parameter=(cy+1.5)-floor(cy+1.5);
    //if (t.parameter<0.001) cout << 'y' << t.parameter << ' ';
    yDiffs.push_back(t);
}

void addToZDiffList(float cx,float cy,float cz)
{
    //The vertex is between voxels (t.x,t.y,t.z) and (t.x,t.y,t.z+1)
    IntTriple t;
    t.x=(int)floor(cx);
    t.y=(int)floor(cy);
    t.z=(int)floor(cz);
    t.parameter=(cz+1.5)-floor(cz+1.5);
    //if (t.parameter<0.001) cout << 'z' << t.parameter << ' ';
    zDiffs.push_back(t);
}

void computeDiffLists()
{
    int numVerts=mesh.getNumVerts();
    Vertex vert;
    float cx,cy,cz,dx,dy,dz;
    int numNearFromHalf;
    numUnreliableVertices=0;
    numOffCellVertices=0;
    for (int i=0; i<numVerts; i++)
    {
	vert=mesh.getVert(i);
	cx=coordToCell(vert.x);
	cy=coordToCell(vert.y);
	cz=coordToCell(vert.z);
	dx=fabs(floor(cx)+0.5-cx);
	dy=fabs(floor(cy)+0.5-cy);
	dz=fabs(floor(cz)+0.5-cz);
	if ((dx<reliabilityThreshold) && (dy<reliabilityThreshold) && (dz<reliabilityThreshold))
	    numUnreliableVertices++;
	else if ((dx<reliabilityThreshold) && (dy<reliabilityThreshold))
	{
	    addToZDiffList(cx,cy,cz);
	}
	else if ((dy<reliabilityThreshold) && (dz<reliabilityThreshold))
	{
	    addToXDiffList(cx,cy,cz);
	}
	else if ((dz<reliabilityThreshold) && (dx<reliabilityThreshold))
	{
	    addToYDiffList(cx,cy,cz);
	}
	else
	    numOffCellVertices++;
    }
    if (numUnreliableVertices>0) cout << numUnreliableVertices << " unreliable vertices.\n";
    if (numOffCellVertices>0) cout << numOffCellVertices << " vertices outside of cells.\n";
    
    sort(xDiffs.begin(),xDiffs.end());
    sort(yDiffs.begin(),yDiffs.end());
    sort(zDiffs.begin(),zDiffs.end());
}

void printDiffList(std::vector<IntTriple>& diffs)
{
    std::vector<IntTriple>::iterator i;
    for (i=diffs.begin(); i!=diffs.end(); ++i)
	cout << "(" << i->x << "," << i->y << "," << i->z << ")\n";
}

void printDiffLists()
{
    cout << "x vertices\n";
    printDiffList(xDiffs);
    cout << "\ny vertices\n";
    printDiffList(yDiffs);
    cout << "\nz vertices\n";
    printDiffList(zDiffs);
    cout << "\n";
}

int getVoxelIndex(int x,int y,int z)
{
    return x+numVoxelsOnASide*(y+numVoxelsOnASide*(z));
}

void computeInsides()
{
    int numVoxels=numVoxelsOnASide*numVoxelsOnASide*numVoxelsOnASide;
    inside.setbit(numVoxels);
    inside.resetbit(numVoxels);
    int x,y,z;
    int fillingInside;
    std::vector<IntTriple>::iterator xi,yi,zi;
    zi=zDiffs.begin();
    yi=yDiffs.begin();
    xi=xDiffs.begin();
    
    for (z=0; z<numVoxelsOnASide; z++)
    {
	for (x=0; x<numVoxelsOnASide; x++)
	{
	    y=0;
	    fillingInside=0;
	    if ((yi!=yDiffs.end()) && (yi->z==z) && (yi->x==x) && (yi->y==-1))
	    {
		fillingInside=1;
		yi++;
	    }
	    for (yi; (yi!=yDiffs.end()) && (yi->z==z) && (yi->x==x); yi++)
	    {
		if (y<numVoxelsOnASide) 
		{
		    for (y; y<=yi->y; y++)
		    {
			if (fillingInside) inside.setbit(getVoxelIndex(x,y,z));
			else inside.resetbit(getVoxelIndex(x,y,z));
		    }
		    fillingInside=!fillingInside;
		}
	    }
	    while (y<numVoxelsOnASide)
	    {
		if (fillingInside) inside.setbit(getVoxelIndex(x,y,z));
		else inside.resetbit(getVoxelIndex(x,y,z));
		y++;
	    }
	}
    }
}

void filterShafts()
{
    int x,y,z,pixelInside;
    for (y=0; y<numVoxelsOnASide; y++)
    {
	for (x=1; x<numVoxelsOnASide-1; x++)
	    for (z=1; z<numVoxelsOnASide-1; z++)
	    {
		pixelInside=inside.getbit(getVoxelIndex(x,y,z));
		if ((inside.getbit(getVoxelIndex(x-1,y,z))!=pixelInside) &&
		    (inside.getbit(getVoxelIndex(x,y,z-1))!=pixelInside) &&
		    (inside.getbit(getVoxelIndex(x+1,y,z))!=pixelInside) &&
		    (inside.getbit(getVoxelIndex(x,y,z+1))!=pixelInside))
		{
		    if (!pixelInside) inside.setbit(getVoxelIndex(x,y,z));
		    else inside.resetbit(getVoxelIndex(x,y,z));
		}
	    }
    }
}

void findMinAndMaxY()
{
    int numVoxels=numVoxelsOnASide*numVoxelsOnASide*numVoxelsOnASide;
    int minInside=numVoxelsOnASide,minOutside=numVoxelsOnASide,maxInside=0,maxOutside=0;
    int x,y,z;
    for (z=1; z<numVoxelsOnASide-1; z++)
	for (y=1; y<numVoxelsOnASide-1; y++)
	    for (x=1; x<numVoxelsOnASide-1; x++)
	    {
		if (inside.getbit(getVoxelIndex(x,y,z)))
		{
		    if (y<minInside) minInside=y;
		    if (y>maxInside) maxInside=y;
		}
		else
		{
		    if (y<minOutside) minOutside=y;
		    if (y>maxOutside) maxOutside=y;
		}
	    }
    cout << "Min y  inside: " << minInside << "\n";
    cout << "Max y outside: " << maxOutside << "\n";
    //cout << "Min y outside: " << minOutside << "\n";
    //cout << "Max y  inside: " << maxInside << "\n";
}

int writeV2File(char* filename)
{
    ofstream fout(filename);
    if (!fout)
    {
	cerr << "Error writing to file " << filename << "\n";
	return 1;
    }
    
    fout.write((char*)&numVoxelsOnASide,sizeof(int));
    fout.write((char*)&numVoxelsOnASide,sizeof(int));
    fout.write((char*)&numVoxelsOnASide,sizeof(int));
    
    int numVoxels=numVoxelsOnASide*numVoxelsOnASide*numVoxelsOnASide;
    inside.write(fout,numVoxels);
    inside.write(fout,numVoxels);
    return 0;
}

void createFloatVolumeSmooth()
{
    int numVoxels=numVoxelsOnASide*numVoxelsOnASide*numVoxelsOnASide;
    float* fullData=new float[numVoxels];
    int index;
    for (index=0; index<numVoxels; index++)
    {
	if (inside.getbit(index)) fullData[index]=-1.0;
	else fullData[index]=1.0;
    }
    float a,b,t;
    std::vector<IntTriple>::iterator yi;
    for (yi=yDiffs.begin(); yi!=yDiffs.end(); ++yi)
    {
	if ((yi->y>=0) && (yi->y+1<numVoxelsOnASide) && (inside.getbit(getVoxelIndex(yi->x,yi->y,yi->z))!=inside.getbit(getVoxelIndex(yi->x+1,yi->y,yi->z))))
	{
	    a=fullData[getVoxelIndex(yi->x,yi->y,yi->z)];
	    t=yi->parameter;
	    b=-a*(1-t)/t;
	    fullData[getVoxelIndex(yi->x+1,yi->y,yi->z)]=b;
	}
    }
    std::vector<IntTriple>::iterator xi;
    for (xi=xDiffs.begin(); xi!=xDiffs.end(); ++xi)
    {
	if ((xi->x>=0) && (xi->x+1<numVoxelsOnASide) && (inside.getbit(getVoxelIndex(xi->x,xi->y,xi->z))!=inside.getbit(getVoxelIndex(xi->x,xi->y+1,xi->z))))
	{
	    a=fullData[getVoxelIndex(xi->x,xi->y,xi->z)];
	    t=xi->parameter;
	    b=-a*(1-t)/t;
	    fullData[getVoxelIndex(xi->x,xi->y+1,xi->z)]=b;
	}
    }
    std::vector<IntTriple>::iterator zi;
    for (zi=zDiffs.begin(); zi!=zDiffs.end(); ++zi)
    {
	if ((zi->z>=0) && (zi->z+1<numVoxelsOnASide) && (inside.getbit(getVoxelIndex(zi->x,zi->y,zi->z))!=inside.getbit(getVoxelIndex(zi->x,zi->y,zi->z+1))))
	{
	    a=fullData[getVoxelIndex(zi->x,zi->y,zi->z)];
	    t=zi->parameter;
	    b=-a*(1-t)/t;
	    fullData[getVoxelIndex(zi->x,zi->y,zi->z+1)]=b;
	}
    }
    
    int reducedVoxels=numVoxelsOnASide-2;
    int numDataVoxels=reducedVoxels*reducedVoxels*reducedVoxels;
    data=new float[numDataVoxels];
    int x,y,z;
    index=0;
    for (z=1; z<numVoxelsOnASide-1; z++) for (y=1; y<numVoxelsOnASide-1; y++) for (x=1; x<numVoxelsOnASide-1; x++)
    {
	data[index]=fullData[getVoxelIndex(x,y,z)];
	index++;
    }
    delete[] fullData;
}

void createFloatVolume()
{
    int numVoxels=numVoxelsOnASide*numVoxelsOnASide*numVoxelsOnASide;
    int reducedVoxels=(numVoxelsOnASide-2)/reductionFactor;
    int numDataVoxels=reducedVoxels*reducedVoxels*reducedVoxels;
    data=new float[numDataVoxels];
    float isosurfaceOffset=-0.5*unitFactor/(reductionFactor*reductionFactor*reductionFactor);
    int x,y,z,index=0,ox,oy,oz;
    for (z=0; z<reducedVoxels; z++) for (y=0; y<reducedVoxels; y++) for (x=0; x<reducedVoxels; x++)
    {
	data[index]=0.0;
	for (oz=1; oz<=reductionFactor; oz++) for (oy=1; oy<=reductionFactor; oy++) for (ox=1; ox<=reductionFactor; ox++)
	    data[index]+=((inside.getbit(getVoxelIndex(reductionFactor*x+ox,reductionFactor*y+oy,reductionFactor*z+oz)))?-unitFactor:unitFactor);
	data[index]/=27.0;
	data[index]+=isosurfaceOffset;
	index++;
    }
}

int getNeighborsInReducedVolume(int index,int* n)
{
    int reducedVoxels=(numVoxelsOnASide-2)/reductionFactor;
    int x,y,z;
    int indexTemp=index;
    x=indexTemp%reducedVoxels;
    indexTemp/=reducedVoxels;
    y=indexTemp%reducedVoxels;
    indexTemp/=reducedVoxels;
    z=indexTemp;
    int ox,oy,oz;
    int numNeighbors=0;
    for (oz=-1; oz<=1; oz++) if ((0<=z+oz) && (z+oz<reducedVoxels)) for (oy=-1; oy<=1; oy++) if ((0<=y+oy) && (y+oy<reducedVoxels)) for (ox=-1; ox<=1; ox++) if ((0<=x+ox) && (x+ox<reducedVoxels))
	n[numNeighbors++]=(x+ox)+reducedVoxels*((y+oy)+reducedVoxels*(z+oz));
    return numNeighbors;
}

void calcDistances()
{
    int reducedVoxels=(numVoxelsOnASide-2)/reductionFactor;
    int numDataVoxels=reducedVoxels*reducedVoxels*reducedVoxels;
    bitc queued;
    queued.setbit(numDataVoxels);
    queued.resetbit(numDataVoxels);
    std::queue<int> q;
    int index,numNeighbors,n;
    int neighbors[27];
    int onBoundary;
    for (index=0; index<numDataVoxels; index++)
    {
	onBoundary=0;
	numNeighbors=getNeighborsInReducedVolume(index,neighbors);
	for (n=0; n<numNeighbors; n++)
	    if (data[index]*data[neighbors[n]]<0.0) onBoundary=1;
	if (onBoundary)
	{
	    q.push(index);
	    queued.setbit(index);
	}
    }
    while (!q.empty())
    {
	index=q.front();
	q.pop();
	numNeighbors=getNeighborsInReducedVolume(index,neighbors);
	for (n=0; n<numNeighbors; n++)
	{	
	    if (!queued.getbit(neighbors[n]))
	    {
		if (data[index]<0.0) data[neighbors[n]]=data[index]-unitFactor;
		else data[neighbors[n]]=data[index]+unitFactor;
		q.push(neighbors[n]);
		queued.setbit(neighbors[n]);
	    }
	}
    }
}

int writeVFile(char* filename)
{
    ofstream fout(filename);
    if (!fout)
    {
	cerr << "Error writing to file " << filename << "\n";
	return 1;
    }
    
    int reducedVoxels=(numVoxelsOnASide-2)/reductionFactor;
    //int numY=yMax-yMin+1;
    fout.write((char*)&reducedVoxels,sizeof(int));
    fout.write((char*)&reducedVoxels,sizeof(int));
    fout.write((char*)&reducedVoxels,sizeof(int));
    
    int numDataVoxels=reducedVoxels*reducedVoxels*reducedVoxels;
    fout.write((char*)data,numDataVoxels*sizeof(float));
    
    return 0;
}


int main(int argc,char* argv[])
{
    if (argc<=2)
    {
	cerr << "Usage: " << argv[0] << " <input .t file> <output .v file>\n";
	return 1;
    }
    
    int result=mesh.readFile(argv[1]);
    if (result)
    {
	cerr << "Error reading file " << argv[1] << "\n";
	return 1;
    }
    
    computeDiffLists();
    //printDiffLists();
    computeInsides();
    filterShafts();
    //findMinAndMaxY();
    //result=writeV2File(argv[2]);
    createFloatVolume();
    calcDistances();
    result=writeVFile(argv[2]);
    
    if (data) delete[] data;
    return result;
}
