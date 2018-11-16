//testoct.cpp
//James Vanderhyde, 4 February 2005

#include "octvolume.h"

int main(int argc,char* argv[])
{
    if (argc<=2)
    {
        cerr << "Usage: " << argv[0] << " <input .v file> <output .v file>\n";
        return 1;
    }
    
    int result;

    volume v(0,1);
    result=v.readFile(argv[1]);
    if (result) return result;

    v.getDataroot()->setFlagRecursively(OTFL_CARVED);
    
    v.renderVolume();

    result=v.writeFile(argv[2]);
    if (result) return result;

    return 0;
}
