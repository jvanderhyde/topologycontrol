//subsample.cpp
//James Vanderhyde, 21 February 2005

#include <fstream.h>

#include "bitc.h"

int main(int argc,char* argv[])
{
    if (argc<=2)
    {
        cerr << "Usage: " << argv[0] << " <input .v2 file> <output .v2 file>\n";
        return 1;
    }
    
    int vsize1[3],vsize2[3];
    int numVoxels1,numVoxels2;
    bitc positive1,positive2;
    bitc known1,known2;
    
    ifstream fin(argv[1]);
    if (!fin) return 1;
    ofstream fout(argv[2]);
    if (!fout) return 1;

    fin.read((char*)vsize1,sizeof(vsize1));
    numVoxels1=vsize1[2]*vsize1[1]*vsize1[0];
    positive1.read(fin,numVoxels1);
    known1.read(fin,numVoxels1);
    fin.close();

    vsize2[2]=vsize1[2]/2;
    vsize2[1]=vsize1[1]/2;
    vsize2[0]=vsize1[0]/2;
    numVoxels2=vsize2[2]*vsize2[1]*vsize2[0];
    positive2.setbit(numVoxels2-1);
    positive2.resetbit(numVoxels2-1);
    known2.setbit(numVoxels2-1);
    known2.resetbit(numVoxels2-1);

    int x,y,z,index1,index2=0,score,ox,oy,oz;

    for (z=1; z<vsize1[2]; z+=2)
      for (y=1; y<vsize1[1]; y+=2)
	for (x=1; x<vsize1[0]; x+=2)
	  {
	    score=0;
	    for (oz=0; oz<2; oz++)
	      for (oy=0; oy<2; oy++)
		for (ox=0; ox<2; ox++)
		  {
		    index1=((z+oz)*vsize1[1]+(y+oy))*vsize1[0]+(x+ox);
		    if (known1.getbit(index1))
		      {
			if (positive1.getbit(index1)) score+=10;
			else score-=10;
			known2.setbit(index2);
		      }
		    else
		      {
			if (positive1.getbit(index1)) score+=1;
			else score-=1;
		      }
		  }
	    if (score >= 0) positive2.setbit(index2);
	    index2++;
	  }

    fout.write((char*)vsize2,sizeof(vsize2));
    positive2.write(fout,numVoxels2);
    known2.write(fout,numVoxels2);
    fout.close();

    return 0;
}
