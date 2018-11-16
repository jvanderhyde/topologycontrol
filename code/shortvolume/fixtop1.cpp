/*
 *  fixtop.cpp
 *  
 *
 *  Created by James Vanderhyde on Tue Apr 27 2004.
 *
 */

#include "shortvolume.h"

int main(int argc,char* argv[])
{
    if (argc<=2)
    {
        cerr << "Usage: " << argv[0] << " <input volume file> <output .v2 file> [<num features> [<isovalue>]]\n";
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
	
	starttime = clock();
	v.calcDistances();
    endtime = clock();	
    cout << "Running time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";
	//v.writeFile("distances.v");
	
	starttime = clock();
	v.carveSimultaneously(numFeatures);
    endtime = clock();
    cout << "Running time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";

	if (inside != 0) v.fixVolumeTopology(inside);
	//v.printVolume(cout);
	
    result=v.writeFile(argv[2]);
    if (result) return result;
	
    return 0;
}
