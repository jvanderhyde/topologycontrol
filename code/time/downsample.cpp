//downsample.cpp
//James Vanderhyde, 26 October 2005

#include "MarchableVolume.h"

#include <fstream.h>

int main(int argc, char* argv[])
{
  if (argc<3)
    {
      cerr << "Usage: " << argv[0] << " <input volume file> <output .v file>\n";
      return 1;
    }

  MarchableVolume* v=MarchableVolume::createVolume(argv[1]);
  if (!v)
    {
      cerr << "Error reading file " << argv[1] << '\n';
      return 2;
    }

  //return v->writeFile(argv[2]);

  ofstream fout(argv[2]);
  if (!fout)
    {
      cerr << "Error writing to file " << argv[2] << '\n';
      return 3;
    }

  int* size1=v->getSize();
  int size2[3];
  size2[0]=size1[0]/2;
  size2[1]=size1[1]/2;
  size2[2]=size1[2]/2;
  cout << size1[0] << 'x' << size1[1] << 'x' << size1[2] << '\n';
  fout.write((char*)size2,sizeof(size2));

  cout << "Writing " << size2[2] << " slices to " << argv[2] << ' ';
  int x,y,z;
  for (z=0; z<size2[2]; z++)
    {
      cout << '.';
      cout.flush();
      for (y=0; y<size2[1]; y++)
	for (x=0; x<size2[0]; x++)
	  {
	    float average=(v->d(2*x,2*y,2*z)+v->d(2*x+1,2*y,2*z)+v->d(2*x,2*y+1,2*z)+v->d(2*x,2*y+1,2*z)+
			   v->d(2*x+1,2*y+1,2*z)+v->d(2*x,2*y+1,2*z+1)+v->d(2*x+1,2*y,2*z+1)+v->d(2*x+1,2*y+1,2*z+1))/8.0;
	    fout.write((char*)&average,sizeof(average));
	  }
    }
  fout.close();
  cout << "done.\n";

  return 0;
}
