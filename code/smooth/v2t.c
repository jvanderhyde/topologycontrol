
#include <fstream.h>

#include "volume.h"
#include "mesh.h"

/* ---------------------------------------------- */

class triangle {
 public:
  M_point p1,p2,p3;
  triangle ( double p1x, double p1y, double p1z,
	     double p2x, double p2y, double p2z,
	     double p3x, double p3y, double p3z );
};

/* ---------------------------------------------- */

triangle::triangle ( double p1x, double p1y, double p1z,
		     double p2x, double p2y, double p2z,
		     double p3x, double p3y, double p3z )
{
  p1[0] = p1x; p1[1] = p1y; p1[2] = p1z;
  p2[0] = p2x; p2[1] = p2y; p2[2] = p2z;
  p3[0] = p3x; p3[1] = p3y; p3[2] = p3z;
  
}

/* ---------------------------------------------- */

int in_order ( double a, double b, double c )
{
  if (a<=b && b<=c)
    return 1;
  if (a>=b && b>=c)
    return -1;
  return 0;
}

/* ---------------------------------------------- */

vector<triangle> faces ( volume &v, double threshold )
{
  vector<triangle> res;
  int i[3];

  for ( i[0]=0; i[0]<v.size[0]; i[0]++ )
    for ( i[1]=0; i[1]<v.size[1]; i[1]++ )
      for ( i[2]=0; i[2]<v.size[2]; i[2]++ )
	for ( int j=0; j<3; j++ )
	  {
	    int ii[3];
	    int sq[2][2][3];
	    memcpy(sq[0][0],i,sizeof(i));
	    memcpy(sq[0][1],i,sizeof(i));
	    memcpy(sq[1][0],i,sizeof(i));
	    memcpy(sq[1][1],i,sizeof(i));
	    sq[0][0][j]++;
	    sq[0][1][j]++;
	    sq[1][0][j]++;
	    sq[1][1][j]++;
	    sq[1][0][(j+1)%3]++;
	    sq[0][1][(j+2)%3]++;
	    sq[1][1][(j+1)%3]++;
	    sq[1][1][(j+2)%3]++;

	    memcpy(ii,i,sizeof(i));
	    ii[j]++;
	    if (v.out_of_range(ii[0],ii[1],ii[2]))
	      continue;
	    switch (in_order(v.d(i[0],i[1],i[2]),threshold,v.d(ii[0],ii[1],ii[2])))
	      {
	      case -1:
		res.push_back(triangle(
				       sq[0][0][0],sq[0][0][1],sq[0][0][2],
				       sq[1][1][0],sq[1][1][1],sq[1][1][2],
				       sq[1][0][0],sq[1][0][1],sq[1][0][2]
				       )
			      );
		res.push_back(triangle(
				       sq[1][1][0],sq[1][1][1],sq[1][1][2],
				       sq[0][0][0],sq[0][0][1],sq[0][0][2],
				       sq[0][1][0],sq[0][1][1],sq[0][1][2]
				       )
			      );
		break;
	      case +1:
		res.push_back(triangle(
				       sq[0][0][0],sq[0][0][1],sq[0][0][2],
				       sq[1][0][0],sq[1][0][1],sq[1][0][2],
				       sq[1][1][0],sq[1][1][1],sq[1][1][2]
				       )
			      );
		res.push_back(triangle(
				       sq[1][1][0],sq[1][1][1],sq[1][1][2],
				       sq[0][1][0],sq[0][1][1],sq[0][1][2],
				       sq[0][0][0],sq[0][0][1],sq[0][0][2]
				       )
			      );
		break;
	      case 0:
		break;
	      }
	  }

  return res;
}

/* ---------------------------------------------- */

int main ( int argc, char *argv[] )
{
  if (argc!=4)
    {
      cerr << "Usage: v2t <.v file (input)> <.t file (output)> <threshold>" << endl;
      return 0;
    }

  ofstream o(argv[2]);
  if (!o)
    {
      cerr << "Can't open output file " << argv[2] << endl;
      return 0;
    }

  ifstream i(argv[1]);
  if (!i)
    {
      cerr << "Can't open " << argv[1] << endl;
      return 0;
    }

  double threshold = atof(argv[3]);

  volume v;

  i >> v;

  i.close();

  cout << "min = " << v.mind() << ", max = " << v.maxd() << endl;

  vector<triangle> msh = faces(v,threshold);

  o << msh.size() << " " << (3*msh.size()) << endl << endl;

  for ( int k=0; k<msh.size(); k++ )
    o << (3*k) << " " << (3*k+1) << " " << (3*k+2) << endl;

  o << endl;

  for ( int l=0; l<msh.size(); l++ )
    {
      o << msh[l].p1[0] << " " << msh[l].p1[1] << " " << msh[l].p1[2] << endl;
      o << msh[l].p2[0] << " " << msh[l].p2[1] << " " << msh[l].p2[2] << endl;
      o << msh[l].p3[0] << " " << msh[l].p3[1] << " " << msh[l].p3[2] << endl;
    }

  return 0;
}
