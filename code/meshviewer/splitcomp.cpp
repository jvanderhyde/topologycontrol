//splitcomp.cpp
//James Vanderhyde, 4 May 2005

#include "TriangleMesh.h"

int main ( int argc, char *argv[] )
{
  if (argc<3)
    {
      cout << "Usage: " << argv[0] << " <input triangle file> <output triangle file> [<remainder triangle file>]\n";
      return 1;
    }

  int error;
  TriangleMesh m;

  error=m.readFile(argv[1]);
  if (error)
    {
      cout << "Error reading file " << argv[1] << '\n';
      return 1;
    }
  cout << "File read from " << argv[1] << '\n';

  m.setVerbose(1);
  m.eliminateAllButLargestComponent();

  error=m.writeFile(argv[2]);
  if (error)
    {
      cout << "Error writing file " << argv[2] << '\n';
      return 1;
    }
  cout << "File written to " << argv[2] << '\n';

  if (argc>3)
    {
      int error;
      TriangleMesh m2;

      error=m2.readFile(argv[1]);
      if (error)
	{
	  cout << "Error reading file " << argv[1] << '\n';
	  return 1;
	}
      cout << "File read from " << argv[1] << '\n';

      m2.setVerbose(1);
      m2.eliminateLargestComponent();

      error=m2.writeFile(argv[3]);
      if (error)
	{
	  cout << "Error writing file " << argv[3] << '\n';
	  return 1;
	}
      cout << "File written to " << argv[3] << '\n';
    }

  return 0;
}
