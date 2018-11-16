/*
 *  trimunknown.cpp
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
        cerr << "Usage: " << argv[0] << " <input .v file> <output .v file> [<parameter>]\n";
        return 1;
    }
    
    int result;
	
	float parameter=0.0;
	if (argc>3) parameter=atof(argv[3]);
	
    volume v;
    result=v.readFile(argv[1]);
    if (result) return result;
    
    cout << v.getSize()[0] << 'x' << v.getSize()[1] << 'x' << v.getSize()[2] << '=';
    cout << v.getSize()[0]*v.getSize()[1]*v.getSize()[2] << '\n';
	//v.printVolume(cout);
	
    clock_t starttime = clock();

	int* vsize=v.getSize();
	int x,y,z;
	for (z=0; z<vsize[2]; z++)
		for (y=0; y<vsize[1]; y++)
			for (x=0; x<vsize[0]; x++)
			{
				if (y+z<parameter)
					v.setVoxel(x,y,z,0);
			}
	
    clock_t endtime = clock();
    cout << "Running time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";
	//v.printVolume(cout);
	
    result=v.writeFile(argv[2]);
    if (result) return result;
	
    return 0;
}
