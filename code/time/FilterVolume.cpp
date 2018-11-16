//FilterVolume.cpp
//James Vanderhyde, 18 June 2007.

#include "FilterVolume.h"
#include <math.h>

FloatComparator::FloatComparator(float* p_data) : data(p_data) {}

int FloatComparator::operator()(int a,int b)
{
    unsigned short vala,valb;
    vala=data[a];
    valb=data[b];
    if (vala != valb) return (vala<valb);
    else return (a<b);
}

FilterVolume::FilterVolume(unsigned short p_featureSize,int p_fixStyle) : volume(p_featureSize,p_fixStyle)
{
}

FilterVolume::~FilterVolume()
{
}

unsigned short FilterVolume::floatToShort(float val)
{
    if (val<0.0) return 0;
    if (val>65535.0) return 65535;
    return (unsigned short)floor(val);
}

float FilterVolume::filter(int x,int y,int z)
{
    float total=0.0;
    total+=d(x-1,y,z);
    total+=d(x,y-1,z);
    total+=d(x,y,z-1);
    total+=d(x+1,y,z);
    total+=d(x,y+1,z);
    total+=d(x,y,z+1);
    total+=2*d(x,y,z);
    return total/8.0;
}

int FilterVolume::fixTopologyByFiltering()
{
    //set up arrays to hold the carving order
    if (!carvedInsideOrder) carvedInsideOrder=new int[size[0]*size[1]*size[2]];
    for (int i=0; i<size[0]*size[1]*size[2]; i++) carvedInsideOrder[i]=i;

    //filter the voxel data
    cout << "Filtering...";
    cout.flush();
    int numVoxels=getNumVoxels(),index,x,y,z;
    float* newData=new float[numVoxels];
    for (index=0; index<numVoxels; index++)
    {
	getVoxelLocFromIndex(index,&x,&y,&z);
	newData[index]=filter(x,y,z);
    }
    
    //store back into unsigned short format
    for (index=0; index<numVoxels; index++)
	data[index]=floatToShort(newData[index]);
    
    //sort for carving order
    cout << "sorting...";
    cout.flush();
    FloatComparator fc(newData);
    sort(carvedInsideOrder,carvedInsideOrder+numVoxels,fc);
    delete[] newData;
    invertPermutation(carvedInsideOrder);
    
    cout << "done.\n";
    return countCriticals(carvedInsideOrder);
}


#ifdef FIXTOPFILTER
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
    
    FilterVolume v(threshold);
    
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
    
    int signsNeedToBeChanged=v.dataShouldBeNegated();
    if (signsNeedToBeChanged) v.changeAllSigns();
    clock_t starttime = clock();
    v.fixTopologyByFiltering();
    clock_t endtime = clock();
    if (signsNeedToBeChanged) v.changeAllSigns();
    cout << "Topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";
    
    //if (signsNeedToBeChanged) v.changeAllSigns();
    cout << "Writing to file " << argv[2] << " ..."; cout.flush();
    result=v.writeFile(argv[2]);
    if (result)
    {
	cout << "error!\n";
	return result;
    }
    else cout << "done.\n";
    
    v.saveOrder("carvingorder_filter.vo",v.getDefaultOrder());
    
    return 0;
}
#endif
