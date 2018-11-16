/*
 *  convert.cpp
 *  
 *
 *  Created by James Vanderhyde on Mon May 03 2004.
 *
 */

#include "shortvolume.h"

int main(int argc,char* argv[])
{
    if (argc<=2)
    {
        cerr << "Usage: " << argv[0] << " <input volume file> <output volume file>\n";
        return 1;
    }
    
    int result;	
    volume v;
	
    result=v.readFile(argv[1]);
    if (result) return result;

    result=v.writeFile(argv[2]);
    if (result) return result;
	
    return 0;
}
