//reversemc.cpp
//James Vanderhyde, 23 June 2007

#include "TriangleMesh.h"
#include "../bitc.h"

#include <vector>
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
    t.parameter=cx-floor(cx);
    xDiffs.push_back(t);
}

void addToYDiffList(float cx,float cy,float cz)
{
    //The vertex is between voxels (t.x,t.y,t.z) and (t.x,t.y+1,t.z)
    IntTriple t;
    t.x=(int)floor(cx);
    t.y=(int)floor(cy);
    t.z=(int)floor(cz);
    t.parameter=cy-floor(cy);
    yDiffs.push_back(t);
}

void addToZDiffList(float cx,float cy,float cz)
{
    //The vertex is between voxels (t.x,t.y,t.z) and (t.x,t.y,t.z+1)
    IntTriple t;
    t.x=(int)floor(cx);
    t.y=(int)floor(cy);
    t.z=(int)floor(cz);
    t.parameter=cz-floor(cz);
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
	    if (((int)floor(cx)==0) && ((int)floor(cy)==0)) addToZDiffList(cx,cy,cz);
	}
	else if ((dy<reliabilityThreshold) && (dz<reliabilityThreshold))
	{
	    if ((int)floor(cy)==0) addToXDiffList(cx,cy,cz);
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
    
    //fill in starting voxel for each slice
    x=y=z=0;
    fillingInside=0;
    if ((zi!=zDiffs.end()) && (zi->z==-1))
    {
	fillingInside=1;
	zi++;
    }
    for (zi; zi!=zDiffs.end(); zi++)
    {
	if (z<numVoxelsOnASide) 
	{
	    for (z; z<=zi->z; z++)
	    {
		if (fillingInside) inside.setbit(getVoxelIndex(x,y,z));
		else inside.resetbit(getVoxelIndex(x,y,z));
	    }
	    fillingInside=!fillingInside;
	}
    }
    while (z<numVoxelsOnASide)
    {
	if (fillingInside) inside.setbit(getVoxelIndex(x,y,z));
	else inside.resetbit(getVoxelIndex(x,y,z));
	z++;
    }
    
    for (z=0; z<numVoxelsOnASide; z++)
    {
	//fill in starting voxel for each column in current slice
	x=y=0;
	fillingInside=inside.getbit(getVoxelIndex(x,y,z));
	if ((xi!=xDiffs.end()) && (xi->z==z) && (xi->x==-1)) xi++;
	for (xi; (xi!=xDiffs.end()) && (xi->z==z); xi++)
	{
	    if (x<numVoxelsOnASide) 
	    {
		for (x; x<=xi->x; x++)
		{
		    if (fillingInside) inside.setbit(getVoxelIndex(x,y,z));
		    else inside.resetbit(getVoxelIndex(x,y,z));
		}
		fillingInside=!fillingInside;
	    }
	}
	while (x<numVoxelsOnASide)
	{
	    if (fillingInside) inside.setbit(getVoxelIndex(x,y,z));
	    else inside.resetbit(getVoxelIndex(x,y,z));
	    x++;
	}
	
	for (x=0; x<numVoxelsOnASide; x++)
	{
	    //fill in rest of voxels in current column
	    y=0;
	    fillingInside=inside.getbit(getVoxelIndex(x,y,z));
	    if ((yi!=yDiffs.end()) && (yi->z==z) && (yi->x==x) && (yi->y==-1)) yi++;
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
    //This seems like it's working, but it needs the diff lists from all three dimensions.
    int numVoxels=numVoxelsOnASide*numVoxelsOnASide*numVoxelsOnASide;
    float* fullData=new float[numVoxels];
    int index;
    for (index=0; index<numVoxels; index++)
    {
	if (inside.getbit(index)) fullData[index]=1.0;
	else fullData[index]=-1.0;
    }
    float a,b;
    std::vector<IntTriple>::iterator yi;
    for (yi=yDiffs.begin(); yi!=yDiffs.end(); ++yi)
    {
	if ((yi->y>=0) && (yi->y+1<numVoxelsOnASide))
	{
	    a=fullData[getVoxelIndex(yi->x,yi->y,yi->z)];
	    b=-a*(1-yi->parameter)/yi->parameter;
	    fullData[getVoxelIndex(yi->x,yi->y+1,yi->z)]=b;
	}
    }
    
    data=new float[numVoxels];
    for (index=0; index<numVoxels; index++) data[index]=fullData[index];
    delete[] fullData;
}

void createFloatVolume()
{
    int numVoxels=numVoxelsOnASide*numVoxelsOnASide*numVoxelsOnASide;
    int reducedVoxels=numVoxelsOnASide/2-1;
    int numDataVoxels=reducedVoxels*reducedVoxels*reducedVoxels;
    data=new float[numDataVoxels];
    float unitFactor=-65.0;
    float isosurfaceOffset=0.1;
    int x,y,z,index=0;
    for (z=0; z<reducedVoxels; z++) for (y=0; y<reducedVoxels; y++) for (x=0; x<reducedVoxels; x++)
    {
	data[index]=0.0;
	data[index]+=((inside.getbit(getVoxelIndex(2*x+1,2*y+1,2*z+1)))?-unitFactor:unitFactor);
	data[index]+=((inside.getbit(getVoxelIndex(2*x+2,2*y+1,2*z+1)))?-unitFactor:unitFactor);
	data[index]+=((inside.getbit(getVoxelIndex(2*x+1,2*y+2,2*z+1)))?-unitFactor:unitFactor);
	data[index]+=((inside.getbit(getVoxelIndex(2*x+1,2*y+1,2*z+1)))?-unitFactor:unitFactor);
	data[index]+=((inside.getbit(getVoxelIndex(2*x+2,2*y+2,2*z+1)))?-unitFactor:unitFactor);
	data[index]+=((inside.getbit(getVoxelIndex(2*x+1,2*y+2,2*z+2)))?-unitFactor:unitFactor);
	data[index]+=((inside.getbit(getVoxelIndex(2*x+2,2*y+1,2*z+2)))?-unitFactor:unitFactor);
	data[index]+=((inside.getbit(getVoxelIndex(2*x+2,2*y+2,2*z+2)))?-unitFactor:unitFactor);
	data[index]/=8.0;
	data[index]+=isosurfaceOffset;
	index++;
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
    
    fout.write((char*)&numVoxelsOnASide,sizeof(int));
    fout.write((char*)&numVoxelsOnASide,sizeof(int));
    fout.write((char*)&numVoxelsOnASide,sizeof(int));
    
    int numVoxels=numVoxelsOnASide*numVoxelsOnASide*numVoxelsOnASide;
    fout.write((char*)data,numVoxels*sizeof(float));
    
    /*int reducedVoxels=numVoxelsOnASide/2-1;
    fout.write((char*)&reducedVoxels,sizeof(int));
    fout.write((char*)&reducedVoxels,sizeof(int));
    fout.write((char*)&reducedVoxels,sizeof(int));
    
    int numDataVoxels=reducedVoxels*reducedVoxels*reducedVoxels;
    fout.write((char*)data,numDataVoxels*sizeof(float));*/
    
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
    //result=writeV2File(argv[2]);
    createFloatVolumeSmooth();
    result=writeVFile(argv[2]);
    
    if (data) delete[] data;
    return result;
}