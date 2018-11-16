/*
 * Updated 06 May 2004, James Vanderhyde
 *   Now only uses 3 slices at a time to preserve memory usage
 *   for computation of triangles and vertices.
 */




#include <iostream.h>
#include <vector>
#include <time.h>

#include <fstream.h>

#include "MarchableVolume.h"

#define SGN(x) (((x)<0)?-1:(((x)>0)?1:0))

/* ---------------------------------------------- */

class Point {
 public:
  double x,y,z;
  Point ( double xx, double yy, double zz ) : x(xx), y(yy), z(zz) {}
};

static ostream & operator << ( ostream &o, Point p )
{
  return o << p.x << " " << p.y << " " << p.z << endl;
}

class Triple {
 public:
  int a,b,c;
  Triple ( int aa, int bb, int cc ) : a(aa), b(bb), c(cc) {}
};

static ostream & operator << ( ostream &o, Triple t )
{
  return o << t.a << " " << t.b << " " << t.c << endl;
}

class mcvolume  {

  MarchableVolume* v;
  bitc outside,known,carved; //a flag for each voxel

  int *xevix; /* indices of verices on edges parallel to x-axis */
  int *yevix;
  int *zevix;

  int xoff[3],yoff[3],zoff[3];
  int xtotal,ytotal,ztotal;

  int triangle_count, point_count;

  int get_vid ( int x, int y, int k, int eix );
  
  float d(int x,int y,int z,int* suspect=NULL);
  int* vsize;
  int side;
  float isovalue;
  void createTriangles(int i,int j,int k);
  void createXVertices(int i,int j,int k);
  void createYVertices(int i,int j,int k);
  void createZVertices(int i,int j,int k);
  bitc suspect; //a flag for each vertex
  void eliminate_excess_vertices();

 public:

  mcvolume ( char *filename, double iso, char* kfilename, char* cfilename, int whichside );
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

static int get_intersection ( double a, double x, double b, double *res )
{
  double t;

  if ((a<x && b<x) || (a>x && b>x))
    return 0;

  t = (x-a)/(b-a);

  *res = clamp01(t);
  return 1;
}

/* ---------------------------------------------- */

std::vector <int> trs;
std::vector <double> vts;
clock_t starttime,endtime;

/* ---------------------------------------------- */

mcvolume::mcvolume ( char *filename, double iso, char* kfilename, char* cfilename, int whichside )
  : v(NULL), outside(),known(),carved(),side(whichside),isovalue(iso),suspect()
{
  initialize();

  v=MarchableVolume::createVolume(filename);
  if (!v)
    {
      cout << "Can't create volume from file " << filename << "!" << endl;
      exit(0);
    }
  vsize=v->getSize();
  
  //allocate enough memory for the vertex flags
  suspect.setbit(3*vsize[2]*vsize[1]*vsize[0]-1); suspect.resetbit(3*vsize[2]*vsize[1]*vsize[0]-1);
  
  //swap in the first slices
  v->d(0,0,0);

  //init the flags
  if (kfilename)
  {
	  ifstream fin(kfilename);
	  if (fin)
	  {
		  cout << "Reading known info file..."; cout.flush();
		  int size[3];
		  fin.read((char*)size,sizeof(size));
		  int numVoxels=size[2]*size[1]*size[0];
		  outside.setbit(numVoxels-1); outside.resetbit(numVoxels-1);
		  outside.read(fin,numVoxels);
		  known.setbit(numVoxels-1); known.resetbit(numVoxels-1);
		  known.read(fin,numVoxels);
		  fin.close();
		  cout << "done.\n";
	  }
	  else
	  {
		  cout << "Problem reading file " << kfilename << '\n';
		  kfilename=NULL;
	  }
  }
  if (cfilename)
  {
	  ifstream fin(cfilename);
	  if (fin)
	  {
		  cout << "Reading carved info file..."; cout.flush();
		  int size[3];
		  fin.read((char*)size,sizeof(size));
		  int numVoxels=size[2]*size[1]*size[0];
		  outside.setbit(numVoxels-1); outside.resetbit(numVoxels-1);
		  outside.read(fin,numVoxels);
		  carved.setbit(numVoxels-1); carved.resetbit(numVoxels-1);
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
  
  starttime = clock();

  xoff[0] = yoff[0] = zoff[0] = 1;
  xoff[1] = xoff[0]*(vsize[0]-1);
  yoff[1] = yoff[0]*vsize[0];
  zoff[1] = zoff[0]*vsize[0];
  xoff[2] = xoff[1]*vsize[1];
  yoff[2] = yoff[1]*(vsize[1]-1);
  zoff[2] = zoff[1]*vsize[1];
  xtotal  = xoff[2]*2;
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
  
  cout << "Eliminating excess vertices..."; cout.flush();
  eliminate_excess_vertices();
  cout << "done.\n";
  
  endtime = clock();
  cout << "Total running time no io: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << endl;
}

void mcvolume::createTriangles(int i,int j,int k)
{
	//cout << i << ',' << j << ',' << k << ','; cout.flush();
	unsigned char index = 
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
		if (tmap[index][l]==-1)
			break;
		trs.push_back(get_vid(i,j,k,tmap[index][l]));
		trs.push_back(get_vid(i,j,k,tmap[index][l+1]));
		trs.push_back(get_vid(i,j,k,tmap[index][l+2]));
		triangle_count++;
		if (get_vid(i,j,k,tmap[index][l])==-1 ||
			get_vid(i,j,k,tmap[index][l+1])==-1 ||
			get_vid(i,j,k,tmap[index][l+2])==-1)
			cout << (int)index << " ";
		//cout << tmap[index][l] << ',' << tmap[index][l+1] << ',' << tmap[index][l+2] << ' ';
	}
	//cout << '\n';
}

void mcvolume::createXVertices(int i,int j,int k)
{
	double res;
	int sus1,sus2;
	if (get_intersection(d(i,j,k,&sus1),isovalue,d(i+1,j,k,&sus2),&res))
	{
		int index1=(k*vsize[1]+j)*vsize[0]+i,index2=index1+1;
	    vts.push_back(i+res);
	    vts.push_back(j);
	    vts.push_back(k);
	    xevix[i*xoff[0]+j*xoff[1]+1*xoff[2]] = point_count;
		if ((sus1) || (sus2))
			suspect.setbit(point_count);
	    point_count++;
	}
}

void mcvolume::createYVertices(int i,int j,int k)
{
	double res;
	int sus1,sus2;
	if (get_intersection(d(i,j,k,&sus1),isovalue,d(i,j+1,k,&sus2),&res))
	{
		int index1=(k*vsize[1]+j)*vsize[0]+i,index2=index1+vsize[0];
	    vts.push_back(i);
	    vts.push_back(j+res);
	    vts.push_back(k);
	    yevix[i*yoff[0]+j*yoff[1]+1*yoff[2]] = point_count;
		if ((sus1) || (sus2))
			suspect.setbit(point_count);
	    point_count++;
	}
}

void mcvolume::createZVertices(int i,int j,int k)
{
	double res;
	int sus1,sus2;
	if (get_intersection(d(i,j,k,&sus1),isovalue,d(i,j,k+1,&sus2),&res))
	{
		int index1=(k*vsize[1]+j)*vsize[0]+i,index2=index1+vsize[1]*vsize[0];
	    vts.push_back(i);
	    vts.push_back(j);
	    vts.push_back(k+res);
	    zevix[i*zoff[0]+j*zoff[1]+1*zoff[2]] = point_count;
		if ((sus1) || (sus2))
			suspect.setbit(point_count);
	    point_count++;
	}
}

float mcvolume::d(int x,int y,int z,int* suspect)
{
	float val=v->d(x,y,z);
	
	if (carved.hasdata())
	{
		int index=(z*vsize[1]+y)*vsize[0]+x;
		if (carved.getbit(index))
		{
			//sign given by outside is correct if carved
			val=fabs(val)*(outside.getbit(index)?1:-1);
			if (suspect)
			{
				*suspect=0;
				if (!known.getbit(index)) *suspect=1;
			}
		}
		else if ((SGN(val)==side) || (!known.getbit(index)))
		{
			//uncarved on side (or unknown) put slightly on the other side
			val=0.5*(-side);
			if (suspect) *suspect=1;
		}
	}
	else if (known.hasdata())
	{
		int index=(z*vsize[1]+y)*vsize[0]+x;
		if (known.getbit(index))
		{
			//sign given by outside is correct if known
			val=fabs(val)*(outside.getbit(index)?1:-1);
			if (suspect) *suspect=0;
		}
		else
		{
			//unknown voxels left alone
			val=val;
			if (suspect) *suspect=1;
		}
	}
		
	return val;
}

void mcvolume::eliminate_excess_vertices()
{
	int* used=new int[point_count];
	int n=0;
	int i;
	for (i=0; i<point_count; i++)
		used[i]=0;
	for (i=0; i<triangle_count*3; i++)
		used[trs[i]]=1;
	for (i=0; i<point_count; i++)
	{
		if (used[i])
		{
			vts[3*n+0]=vts[3*i+0];
			vts[3*n+1]=vts[3*i+1];
			vts[3*n+2]=vts[3*i+2];
			used[i]=n;
			n++;
		}
	}
	for (i=0; i<triangle_count*3; i++)
		trs[i]=used[trs[i]];
	delete[] used;
	point_count=n;
}

/* ---------------------------------------------- */

void mcvolume::save_mesh ( char *filename )
{
  ofstream ofs(filename);

  if (!ofs)
    {
      cout << "Can't open output file " << filename << endl;
      exit(0);
    }

  ofs << triangle_count << " " << point_count << endl << endl;

  int a,b,c;
  double x,y,z;
  int i;

  for ( i=0; i<triangle_count; i++ )
    {
      ofs << trs[3*i+0] << " " << trs[3*i+1] << " " << trs[3*i+2] << endl;
    }

  ofs << endl;

  for ( i=0; i<point_count; i++ )
    {
      ofs << vts[3*i+0] << " " << vts[3*i+1] << " " << vts[3*i+2] << endl;
    }
  
  ofs << endl;
  
  int label;
  for ( i=0; i<triangle_count; i++ )
  {  
	  label=0;
	  //cout << suspect.getbit(trs[3*i+0]);
	  if (suspect.getbit(trs[3*i+0])) label++;
	  if (suspect.getbit(trs[3*i+1])) label++;
	  if (suspect.getbit(trs[3*i+2])) label++;
	  ofs << label << endl;
  }
}

/* ---------------------------------------------- */

int main ( int argc, char *argv[] )
{
  if ((argc<4) || (argc>7))
    {
      cout << "Usage: mc <original volume file> <isovalue> <output .t file> [<known .v2 file> [<carved .v2 file> [inside|outside]]]" << endl;
      return 0;
    }

  double isovalue = atof(argv[2]);

  cout << "Using isovalue " << isovalue << endl;

  char* knownFilename=NULL,* carvedFilename=NULL;
  if (argc>4) knownFilename=argv[4];
  if (argc>5) carvedFilename=argv[5];
  int side=-1; //default to inside, for now
  if (argc>6) if (!strncmp(argv[6],"in",2)) side=-1;
  if (argc>6) if (!strncmp(argv[6],"out",3)) side=1;
  mcvolume mcv(argv[1],isovalue,knownFilename,carvedFilename,side);

  mcv.save_mesh(argv[3]);

  return 0;
}
