/*
 *  fixall.cpp
 *  
 *
 *  Created by James Vanderhyde on Tue Apr 27 2004.
 *
 */

#include "shortvolume.h"

int main(int argc,char* argv[])
{
    if (argc<=1)
    {
        cerr << "Usage: " << argv[0] << " <input volume file>\n";
        return 1;
    }
    
    int result;
	
	int numFeatures=0;
	if (argc>3) numFeatures=atoi(argv[3]);
	
	float isovalue=0.0;
	if (argc>4) isovalue=atof(argv[4]);
	
    int inside=0;
    if (argc>5) if (!strncmp(argv[5],"in",2)) inside=-1;
    if (argc>5) if (!strncmp(argv[5],"out",3)) inside=1;
	
    volume v;
    result=v.readTopoinfoFile();
    if (result) return result;
    result=v.readFile(argv[1],isovalue);
    if (result) return result;
	//v.turnOnAnimation();
    
    cout << v.getSize()[0] << 'x' << v.getSize()[1] << 'x' << v.getSize()[2] << '=';
    cout << v.getSize()[0]*v.getSize()[1]*v.getSize()[2] << '\n';
	//v.printVolume(cout);
	
    clock_t starttime,endtime;
	
	short minval=v.findMinimum();
	v.addToAll(-minval+1);
	
	starttime = clock();
	v.sortVoxels();
	//v.carveSimultaneously();
	//v.carveFromInside(4);
	v.carveFromOutside(4,1);
	v.countCriticals();
	v.printLifeSpans(5);
	//v.carveFromInside(11);
	v.carveFromOutside(5,0);
	v.countCriticals();
    endtime = clock();
    cout << "Running time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";

	if (inside != 0) v.fixVolumeTopology(inside);
	//v.printVolume(cout);
	
	//v.renderVolume(1);
	
	v.addToAll(minval-1);
	
    //result=v.writeFile(argv[2]);
    //if (result) return result;
	
    return 0;
}
