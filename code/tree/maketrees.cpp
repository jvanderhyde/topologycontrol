//maketrees.cpp
//James Vanderhyde, 2 May 2007.

#include "Volume2DplusT.h"

int verbose=1;

ContourTree windowJT,windowST,windowCT;

void defineWindow(int windowStart,int windowLength,Volume2DplusT* vol)
{
    std::vector<int>* nodes;
    
    windowJT.clear();
    windowST.clear();
    windowCT.clear();
    
    if (verbose)
    {
	cout << "\nConstructing merged  join tree for window " << windowStart << "-" << windowStart+windowLength;
	cout.flush();
    }
    nodes=new std::vector<int>;
    vol->mergeJoinTrees(vol->getDefaultOrder(),windowStart,windowStart+windowLength,windowJT,nodes);
    delete nodes;
    if (verbose)
    {
	cout << "done.\n";
	cout << "Constructing merged split tree for window " << windowStart << "-" << windowStart+windowLength; 
	cout.flush();
    }
    nodes=new std::vector<int>;
    vol->mergeSplitTrees(vol->getDefaultOrder(),windowStart,windowStart+windowLength,windowST,nodes);
    delete nodes;
    if (verbose)
    {
	cout << "done.\n";
	cout << "Constructing contour tree for window " << windowStart << "-" << windowStart+windowLength << "..."; 
	cout.flush();
    }
    ContourTree mergedCT(windowJT,windowST);
    mergedCT.clone(windowCT);
    if (verbose)
    {
	cout << "done.\n";
    }	
}



int main(int argc,char* argv[])
{
    if (argc<=2)
    {
	cerr << "Usage: " << argv[0] << " <input .v file> <tree file prefix> [threshold | <.vo order file>]\n";
	return 1;
    }
    
    int simplifyTopology=0;
    unsigned short threshold=65535;
    char* orderfilename=NULL;
    if (argc>3)
    {
	char* suffix=strrchr(argv[3],'.');
	if ((suffix) && (!strcmp(suffix,".vo"))) orderfilename=argv[3];
	else
	{
	    threshold=atoi(argv[3]);
	    simplifyTopology=1;
	}
    }
    
    int result;
    
    Volume2DplusT v(threshold);
    
    v.readTopoinfoFiles();
    
    result=v.readFile(argv[1]);
    if (result) return result;
    
    cout << v.getSize()[0] << 'x' << v.getSize()[1] << 'x' << v.getSize()[2] << '=';
    cout << v.getSize()[0]*v.getSize()[1]*v.getSize()[2] << '\n';
    
    int signsNeedToBeChanged=v.dataShouldBeNegated();
    signsNeedToBeChanged=0;
    if (signsNeedToBeChanged) v.changeAllSigns();
    
    unsigned short min=65535,max=0;
    for (int i=0; i<v.getSize()[2]*v.getSize()[1]*v.getSize()[0]; i++)
    {
	if (v.getVoxel(i)<min) min=v.getVoxel(i);
	if (v.getVoxel(i)>max) max=v.getVoxel(i);
    }
    cout << "Min: " << min << ", Max: " << max << "\n";
    
    if (simplifyTopology)
    {
	clock_t starttime = clock();
	v.fixTopologyStrict();
	clock_t endtime = clock();
	cout << "Topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";
	//v.saveOrder("carvingorder.vo");
    }
    else
    {
	v.sortVoxels();
	//v.saveOrder("sortedorder.vo");
	if (orderfilename) v.readCarvedOrder(orderfilename);
	//v.countCriticalsInThickSlices();
    }
    
    int outFilenameSize=strlen(argv[2])+10+4;
    char* outFilename=new char[outFilenameSize+1];
    
    int windowSize=10;
    
    for (int window=0; window<v.getNumSlices()-windowSize; window+=(windowSize/2))
    {
	defineWindow(window,windowSize,&v);
	sprintf(outFilename,"%s_jt%03d-%03d.ctb",argv[2],window,window+windowSize);
	result=windowJT.writeFile(outFilename);
	if (result) cerr << "Error writing to file " << outFilename << "\n";
	sprintf(outFilename,"%s_st%03d-%03d.ctb",argv[2],window,window+windowSize);
	result=windowST.writeFile(outFilename);
	if (result) cerr << "Error writing to file " << outFilename << "\n";
	sprintf(outFilename,"%s_ct%03d-%03d.ctb",argv[2],window,window+windowSize);
	result=windowCT.writeFile(outFilename);
	if (result) cerr << "Error writing to file " << outFilename << "\n";
    }
    
    delete[] outFilename;
    return 0;
}
