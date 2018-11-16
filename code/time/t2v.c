
#include <math.h>
#include <fstream.h>

#include "volume.h"
#include "mesh.h"

static int SIZE =  128;

#define MARGIN 2
#define BALL_DIJKSTRA 3
#define BALL_INSERT 2


/* ---------------------------------------------- */

static void get_bounding_box ( M_point lo, M_point hi, int vertices, M_point *vertex )
{
  memcpy(lo,vertex[0],sizeof(M_point));
  memcpy(hi,vertex[0],sizeof(M_point));
  for ( int i=1; i<vertices; i++ )
    for ( int j=0; j<3; j++ )
      {
	if (lo[j]>vertex[i][j])
	  lo[j] = vertex[i][j];
	if (hi[j]<vertex[i][j])
	  hi[j] = vertex[i][j];
      }
}

static double maxcoord ( M_point p )
{
  return max(max(p[0],p[1]),p[2]);
}

/* ---------------------------------------------- */

double round ( double x )
{
  return floor(x+.5);
}

/* ---------------------------------------------- */

int main ( int argc, char *argv[] )
{
  M_point lo,hi;
  M_point span;
  double maxspan;
  int i,j;

  M_point *vertex;
  M_int_triple *triangle;
  int triangles, vertices;

  if (argc!=4)
    {
      cerr << "Usage: v2t <file name> <scale> <output .v file>" << endl;
      return 0;
    }

  SIZE = atoi(argv[2]);

  ifstream fp(argv[1]);
  if (!fp)
    {
      cerr << "Error: Can't open " << argv[1] << endl;
      return 0;
    }

  fp >> triangles >> vertices;

  triangle = new M_int_triple[triangles];
  vertex = new M_point[vertices];

  for ( i=0; i<triangles; i++ )
    fp >> triangle[i][0] >> triangle[i][1] >> triangle[i][2];
  
  for ( i=0; i<vertices; i++ )
    fp >> vertex[i][0] >> vertex[i][1] >> vertex[i][2];

  get_bounding_box(lo,hi,vertices,vertex);
  M_point_subtract(span,hi,lo);
  maxspan = maxcoord(span);

  int shift = -1;

  if (span[0]==maxspan)
    shift = 1;
  if (span[1]==maxspan)
    shift = 2;
  if (span[2]==maxspan)
    shift = 0;

  if (shift==-1)
    {
      cout << "Bug: check shift!!" << endl;
      return 0;
    }

  for ( i=0; i<vertices; i++ )
    {
      M_point p;
      for ( j=0; j<3; j++ )
	p[j] = vertex[i][(j+shift)%3];
      M_point_copy(vertex[i],p);
    }

  get_bounding_box(lo,hi,vertices,vertex);
  M_point_subtract(span,hi,lo);
  maxspan = maxcoord(span);

  span[0] = SIZE*span[0]/maxspan;
  span[1] = SIZE*span[1]/maxspan;
  span[2] = SIZE*span[2]/maxspan;

  int sz[3];

  sz[0] = (int)round(span[0]);
  sz[1] = (int)round(span[1]);
  sz[2] = (int)round(span[2]);

  cout << "Volume size = " << sz[0]+2*MARGIN 
       << "x" << sz[1]+2*MARGIN 
       << "x" << sz[2]+2*MARGIN << endl;

  fvolume v(sz[0]+2*MARGIN,sz[1]+2*MARGIN,sz[2]+2*MARGIN);

  for ( i=0; i<vertices; i++ )
    for ( j=0; j<3; j++ )
      {
	vertex[i][j] -= lo[j];
	vertex[i][j] = vertex[i][j]/maxspan;
	vertex[i][j] *= SIZE;
	vertex[i][j] += MARGIN;
      }

  get_bounding_box(lo,hi,vertices,vertex);
  cout << "inserting triangles ..." << flush;

  for ( i=0; i<triangles; i++ )
    {
      v.insert_triangle(vertex[triangle[i][0]],
			vertex[triangle[i][1]],
			vertex[triangle[i][2]],BALL_INSERT);

      if (i%10000==0)
	cout << (i/1000) << "k " << flush;
    }

  delete[] vertex;
  delete[] triangle;
  vertex = NULL;
  triangle = NULL;

  cout << "done!" << endl << "running dijkstra ..." << flush;

  v.dijkstra(BALL_DIJKSTRA);

  cout << "done!" << endl << "running inout ..." << flush;

  v.inout();

  if (v.self_test())
    cout << "FIXME!!!" << endl;

  //  v.save_slices();

  ofstream o(argv[3]);

  if (!argv[3])
    {
      cout << "Can't open output file " << argv[3] << endl;
      return 0;
    }

  o << v;

  return 0;
}
