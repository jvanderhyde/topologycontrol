/*
 * Updated 06 May 2004, James Vanderhyde
 *   Now only uses 3 slices at a time to preserve memory usage
 *   for computation of triangles and vertices.
 * Updated 23 Mar 2007, James Vanderhyde
 *   Uses carved info instead of an isovalue to determine 
 *   inside/outside.
 */




#include <iostream.h>
#include <vector>
#include <time.h>
#include <math.h>

#include <fstream.h>

#include "../MarchableVolume.h"
#include "TriangleMesh.h"

#define SGN(x) (((x)<0)?-1:(((x)>0)?1:0))

/* ---------------------------------------------- */

class mcvolume  {

  MarchableVolume* v;
  bitc carved; //a flag for each voxel

  int *xevix; /* indices of verices on edges parallel to x-axis */
  int *yevix;
  int *zevix;

  int xoff[3],yoff[3],zoff[3];
  int xtotal,ytotal,ztotal;

  TriangleMesh mesh;
  int triangle_count, point_count;

  int get_vid ( int x, int y, int k, int eix );
  
  int carv(int x,int y,int z); //returns carved info
  float d(int x,int y,int z);
  int vsize[3];
  int side;
  float isovalue;

  int checkCarvedInfo;
  int checkKnownInfo;
  int dontPatchHoles;
  int numComponentsToSave;

  int intersects(int x1,int y1,int z1,int x2,int y2,int z2);
  double intersection(int x1,int y1,int z1,int x2,int y2,int z2);
  void createTriangles(int i,int j,int k);
  void createXVertices(int i,int j,int k);
  void createYVertices(int i,int j,int k);
  void createZVertices(int i,int j,int k);

 public:

	  
  mcvolume ( char *filename, double iso, char* cfilename, char* options );
  void save_mesh ( char *filename );
};

/* ---------------------------------------------- */

static int tmap[256][19];

/* ---------------------------------------------- */

int mcvolume::get_vid ( int x, int y, int z, int eix )
{
  int dx,dy,dz;

  dx = dy = dz = 0;

  switch (eix)
    {
    case 0:
    case 2:
    case 8:
      break;
    case 3:
    case 9:
      dx++;
      break;
    case 1:
    case 10:
      dy++;
      break;
    case 4:
    case 6:
      dz++;
      break;
    case 5:
      dz++;
      dy++;
      break;
    case 7:
      dx++;
      dz++;
      break;
    case 11:
      dx++;
      dy++;
      break;
    default:
      cout << "eix = " << eix << "!: check for bugs in get_vid (first switch statement)" << endl;
      exit(0);
      break;
    }

  switch (eix)
    {
    case 0:
    case 1:
    case 4:
    case 5:
      return xevix[xoff[0]*(x+dx)+xoff[1]*(y+dy)+xoff[2]*(dz)];
      break;
    case 2:
    case 3:
    case 6:
    case 7:
      return yevix[yoff[0]*(x+dx)+yoff[1]*(y+dy)+yoff[2]*(dz)];
      break;
    case 8:
    case 9:
    case 10:
    case 11:
      return zevix[zoff[0]*(x+dx)+zoff[1]*(y+dy)+zoff[2]*(dz)];
      break;
    default:
      cout << "check for bugs in get_vid (second switch statement)" << endl;
      exit(0);
      break;
    }

  cout << "check for bugs in get_vid" << endl;
  exit(0);
  return -1;
}

/* ---------------------------------------------- */

static void initialize()
{
  ifstream f("mc.ttab");
  if (!f)
    {
      cout << "Can't open mc.ttab - run 'mcsetup' first!" << endl;
      exit(0);
    }

  for ( int i=0; i<256; i++ )
    for ( int j=0; j<19; j++ )
      f >> tmap[i][j];
}

/* ---------------------------------------------- */

static double clamp01 ( double x )
{
  if (x<0) return 0;
  if (x>1) return 1;
  return x;
}

/* ---------------------------------------------- */

clock_t starttime,endtime;

/* ---------------------------------------------- */

mcvolume::mcvolume ( char *filename, double iso, char* cfilename, char* options )
{
  initialize();
  numComponentsToSave=-1;
  isovalue=iso;
	  
  v=MarchableVolume::createVolume(filename);
  if (!v)
    {
      cout << "Can't create volume from file " << filename << "!" << endl;
      exit(1);
    }
  vsize[0]=v->getSize()[0];
  vsize[1]=v->getSize()[1];
  vsize[2]=v->getSize()[2];
  cout << vsize[0] << 'x' << vsize[1] << 'x' << vsize[2] << '=';
  cout << vsize[0]*vsize[1]*vsize[2] << " voxels.\n";
  
  //swap in the first slices
  v->d(0,0,0);

  //check options
  if (options)
    {
      if (strstr(options,"1"))
	{
	  numComponentsToSave=1;
	}
    }

  //init the flags
  if (cfilename)
  {
	  ifstream fin(cfilename);
	  if (fin)
	  {
		  cout << "Reading carved info file..."; cout.flush();
		  int size[3];
		  fin.read((char*)size,sizeof(size));
		  int numVoxels=size[2]*size[1]*size[0];
		  carved.setbit(numVoxels-1); carved.resetbit(numVoxels-1);
		  carved.read(fin,numVoxels);
		  carved.read(fin,numVoxels);
		  fin.close();
		  cout << "done.\n";
	  }
	  else
	  {
		  cout << "Problem reading file " << cfilename << '\n';
		  cfilename=NULL;
	  }
  }

  //To handle the boundary conditions, we increase all the dimensions by 2.
  // Then d(x,y,z) will call v->d(x-1,y-1,z-1).
  // Since v->d() is boundary protected, this will work fine.
  vsize[0]+=2;
  vsize[1]+=2;
  vsize[2]+=2;
  
  starttime = clock();

  xoff[0] = yoff[0] = zoff[0] = 1;
  xoff[1] = xoff[0]*(vsize[0]-1);
  yoff[1] = yoff[0]*vsize[0];
  zoff[1] = zoff[0]*vsize[0];
  xoff[2] = xoff[1]*vsize[1];
  yoff[2] = yoff[1]*(vsize[1]-1);
  zoff[2] = zoff[1]*vsize[1];
  xtotal  = xoff[2]*2; //We store only 2 slices worth of vertex indices
  ytotal  = yoff[2]*2;
  ztotal  = zoff[2]*2;

  xevix = new int[xtotal];
  yevix = new int[ytotal];
  zevix = new int[ztotal];

  int i,j,k;

  point_count = 0;
  triangle_count = 0;

  for ( i=xoff[2]; i<xtotal; i++ )
    xevix[i] = -1;
  for ( i=yoff[2]; i<ytotal; i++ )
    yevix[i] = -1;
  for ( i=zoff[2]; i<ztotal; i++ )
    zevix[i] = -1;

	for ( j=0; j<vsize[1]; j++ )
	  for ( i=0; i<vsize[0]-1; i++ )
		  createXVertices(i,j,0);
	for ( j=0; j<vsize[1]-1; j++ )
	  for ( i=0; i<vsize[0]; i++ )
		  createYVertices(i,j,0);
	for ( j=0; j<vsize[1]; j++ )
	  for ( i=0; i<vsize[0]; i++ )
		  createZVertices(i,j,0);
		
  cout << "running main loop for " << vsize[2] << " slices.";
  for ( k=0; k<vsize[2]-1; k++ )
  {
	  for ( i=0; i<xoff[2]; i++ )
		  xevix[i] = xevix[i+xoff[2]];
	  for ( i=0; i<yoff[2]; i++ )
		  yevix[i] = yevix[i+yoff[2]];
	  for ( i=0; i<zoff[2]; i++ )
		  zevix[i] = zevix[i+zoff[2]];

	  for ( i=xoff[2]; i<xtotal; i++ )
		  xevix[i] = -1;
	  for ( i=yoff[2]; i<ytotal; i++ )
		  yevix[i] = -1;
	  for ( i=zoff[2]; i<ztotal; i++ )
		  zevix[i] = -1;
	  
	  for ( j=0; j<vsize[1]; j++ )
		  for ( i=0; i<vsize[0]-1; i++ )
			  createXVertices(i,j,k+1);
	  for ( j=0; j<vsize[1]-1; j++ )
		  for ( i=0; i<vsize[0]; i++ )
			  createYVertices(i,j,k+1);
	  if (k+1<vsize[2])
		  for ( j=0; j<vsize[1]; j++ )
			  for ( i=0; i<vsize[0]; i++ )
				  createZVertices(i,j,k+1);
	  
	  for ( j=0; j<vsize[1]-1; j++ )
		  for ( i=0; i<vsize[0]-1; i++ )
		  {
			  createTriangles(i,j,k);
		  }
	cout << '.'; cout.flush();
  }
  cout << "done.\n";
  
  if (v) delete v;
  v=NULL;
  
  //mesh.setVerbose(1);
  cout << "Eliminating excess vertices..."; cout.flush();
  mesh.eliminateExcessVertices();
  point_count=mesh.getNumVerts();
  cout << "done.\n";
  if (numComponentsToSave==1)
    {
      cout << "Eliminating excess components..."; cout.flush();
      mesh.eliminateAllButLargestComponent();
      cout << "done.\n";
    }

  endtime = clock();
  cout << "Total running time no io: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << endl;
}

void mcvolume::createTriangles(int i,int j,int k)
{
    //cout << i << ',' << j << ',' << k << ','; cout.flush();
    int a,b,c;
    unsigned char index;
    if (carved.hasdata()) index = 
	((!!(carv(i,j,k)))<<0) | 
	((!!(carv(i+1,j,k)))<<1) |
	((!!(carv(i,j+1,k)))<<2) |
	((!!(carv(i+1,j+1,k)))<<3) |
	((!!(carv(i,j,k+1)))<<4) |
	((!!(carv(i+1,j,k+1)))<<5) |
	((!!(carv(i,j+1,k+1)))<<6) |
	((!!(carv(i+1,j+1,k+1)))<<7);
    else index = 
	((!!(d(i,j,k)<isovalue))<<0) | 
	((!!(d(i+1,j,k)<isovalue))<<1) |
	((!!(d(i,j+1,k)<isovalue))<<2) |
	((!!(d(i+1,j+1,k)<isovalue))<<3) |
	((!!(d(i,j,k+1)<isovalue))<<4) |
	((!!(d(i+1,j,k+1)<isovalue))<<5) |
	((!!(d(i,j+1,k+1)<isovalue))<<6) |
	((!!(d(i+1,j+1,k+1)<isovalue))<<7);
    
  
  for ( int l=0;;l+=3 )
    {
      //cout << l << ' '; cout.flush();
      if (l>=19) cout << "!";
      if (tmap[index][l]==-1) break;

      a=get_vid(i,j,k,tmap[index][l]);
      b=get_vid(i,j,k,tmap[index][l+1]);
      c=get_vid(i,j,k,tmap[index][l+2]);
      mesh.addTri(Triangle(a,b,c));
      triangle_count++;
      /*if (get_vid(i,j,k,tmap[index][l])==-1 ||
	get_vid(i,j,k,tmap[index][l+1])==-1 ||
	get_vid(i,j,k,tmap[index][l+2])==-1)
	cout << (int)index << " ";*/
      //cout << tmap[index][l] << ',' << tmap[index][l+1] << ',' << tmap[index][l+2] << ' ';
    }
  //cout << '\n';
}

int mcvolume::intersects(int x1,int y1,int z1,int x2,int y2,int z2)
{
    if (carved.hasdata())
    {
	int a=carv(x1,y1,z1),b=carv(x2,y2,z2);
	return (a!=b);
    }
    else
    {
	double a=d(x1,y1,z1),x=isovalue,b=d(x2,y2,z2);
	return ((a<x && x<b) || (a>x && x>b));
    }
}

double mcvolume::intersection(int x1,int y1,int z1,int x2,int y2,int z2)
{
    double a=d(x1,y1,z1),x=isovalue,b=d(x2,y2,z2);
    return clamp01((x-a)/(b-a));
}

void mcvolume::createXVertices(int i,int j,int k)
{
  double res;
  if (intersects(i,j,k,i+1,j,k))
    {
      res=intersection(i,j,k,i+1,j,k);
      int index1=(k*vsize[1]+j)*vsize[0]+i,index2=index1+1;
      mesh.addVert(Vertex(i+res,j,k));
      xevix[i*xoff[0]+j*xoff[1]+1*xoff[2]] = point_count;
      point_count++;
    }
}

void mcvolume::createYVertices(int i,int j,int k)
{
  double res;
  if (intersects(i,j,k,i,j+1,k))
    {
      res=intersection(i,j,k,i,j+1,k);
      int index1=(k*vsize[1]+j)*vsize[0]+i,index2=index1+vsize[0];
      mesh.addVert(Vertex(i,j+res,k));
      yevix[i*yoff[0]+j*yoff[1]+1*yoff[2]] = point_count;
      point_count++;
    }
}

void mcvolume::createZVertices(int i,int j,int k)
{
  double res;
  if (intersects(i,j,k,i,j,k+1))
    {
      res=intersection(i,j,k,i,j,k+1);
      int index1=(k*vsize[1]+j)*vsize[0]+i,index2=index1+vsize[1]*vsize[0];
      mesh.addVert(Vertex(i,j,k+res));
      zevix[i*zoff[0]+j*zoff[1]+1*zoff[2]] = point_count;
      point_count++;
    }
}

int mcvolume::carv(int x,int y,int z)
{
    if ((x-1<0) || (y-1<0) || (z-1<0) || (x+1>=vsize[0]) || (y+1>=vsize[1]) || (z+1>=vsize[2])) return 0;
    int index=((z-1)*(vsize[1]-2)+(y-1))*(vsize[0]-2)+(x-1);
    return carved.getbit(index);
}

float mcvolume::d(int x,int y,int z)
{
  //To handle the boundary conditions,
  // mc considers a volume larger by one pixel on each side in each dimension.
  // v->d() is boundary protected, so we can safely subtract 1 to get the correct result.
  float val=v->d(x-1,y-1,z-1);
}

/* ---------------------------------------------- */

void mcvolume::save_mesh ( char *filename )
{
	int result;
	result=mesh.writeFile(filename);
	if (result) exit(0);
}

/* ---------------------------------------------- */

int main ( int argc, char *argv[] )
{
  if ((argc<4) || (argc>6))
    {
      cout << "Usage: mc <original volume file> <isovalue> <output .t file> ";
      cout << "[<carved .v2 file> [-<options>]]" << endl;
      return 1;
    }

  char* options=NULL,* cfilename=NULL;
  if (argc>4) cfilename=argv[4];
  if (argc>5) options=argv[5];
  mcvolume mcv(argv[1],atof(argv[2]),cfilename,options);

  mcv.save_mesh(argv[3]);
  return 0;
}
