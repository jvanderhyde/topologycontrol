/*
 * Updated 06 May 2004, James Vanderhyde
 *   Now only uses 3 slices at a time to preserve memory usage
 *   for computation of triangles and vertices.
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
  bitc outside,known,carved; //a flag for each voxel

  int *xevix; /* indices of verices on edges parallel to x-axis */
  int *yevix;
  int *zevix;

  int xoff[3],yoff[3],zoff[3];
  int xtotal,ytotal,ztotal;

  TriangleMesh mesh;
  int triangle_count, point_count;

  int get_vid ( int x, int y, int k, int eix );
  
  float d(int x,int y,int z,int* suspect=NULL);
  int vsize[3];
  int side;
  float isovalue;

  int checkCarvedInfo;
  int checkKnownInfo;
  int dontPatchHoles;
  int numComponentsToSave;

  void createTriangles(int i,int j,int k);
  void createXVertices(int i,int j,int k);
  void createYVertices(int i,int j,int k);
  void createZVertices(int i,int j,int k);

 public:

	  
  mcvolume ( char *filename, double iso, char* options, char* kfilename, char* cfilename, int whichside );
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

clock_t starttime,endtime;

/* ---------------------------------------------- */

mcvolume::mcvolume ( char *filename, double iso, char* options, char* kfilename, char* cfilename, int whichside )
  : v(NULL), outside(),known(),carved(),side(whichside),isovalue(iso),
     checkCarvedInfo(0),checkKnownInfo(0),dontPatchHoles(0),numComponentsToSave(-1)
{
  initialize();
	  
  v=MarchableVolume::createVolume(filename);
  if (!v)
    {
      cout << "Can't create volume from file " << filename << "!" << endl;
      exit(0);
    }
  vsize[0]=v->getSize()[0];
  vsize[1]=v->getSize()[1];
  vsize[2]=v->getSize()[2];
  cout << vsize[0] << 'x' << vsize[1] << 'x' << vsize[2] << '=';
  cout << vsize[0]*vsize[1]*vsize[2] << " voxels.\n";
  
  //handle 4D
  int fourDimensional=0; //could extract this from filename
  if (fourDimensional)
  {
	((MarchableVolume4D*)v)->setCurrentTimeSlice(0);
  }
  
  //swap in the first slices
  v->d(0,0,0);

  //check options
  if (options)
    {
      if (strstr(options,"c"))
	{
	  checkCarvedInfo=1;
	}
      if (strstr(options,"k"))
	{
	  checkKnownInfo=1;
	}
      if (strstr(options,"h"))
	{
	  dontPatchHoles=1;
	  checkKnownInfo=1;
	}
      if (strstr(options,"1"))
	{
	  numComponentsToSave=1;
	}
    }

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
		  if (!kfilename)
		  {
			  //Assume everything is known if there's no known file.
			  // The code that uses the carved flags also needs the known flags,
			  // so we define them here.
			  for (int i=numVoxels-1; i>=0; i--) known.setbit(i);
		  }
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

  int label;
  for ( i=0; i<triangle_count; i++ )
  {  
    label=mesh.getVert(mesh.getTri(i).a).label |
          mesh.getVert(mesh.getTri(i).b).label |
          mesh.getVert(mesh.getTri(i).c).label;
    mesh.setTriLabel(i,label);
  }
  
  endtime = clock();
  cout << "Total running time no io: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << endl;
}

void mcvolume::createTriangles(int i,int j,int k)
{
  //cout << i << ',' << j << ',' << k << ','; cout.flush();
  int a,b,c;
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
      if (tmap[index][l]==-1) break;

      a=get_vid(i,j,k,tmap[index][l]);
      b=get_vid(i,j,k,tmap[index][l+1]);
      c=get_vid(i,j,k,tmap[index][l+2]);
      if ((!dontPatchHoles) || ((mesh.getVert(a).label==0) && (mesh.getVert(b).label==0) && (mesh.getVert(c).label==0)))
	{
	  mesh.addTri(Triangle(a,b,c));
	  triangle_count++;
	}
      /*if (get_vid(i,j,k,tmap[index][l])==-1 ||
	get_vid(i,j,k,tmap[index][l+1])==-1 ||
	get_vid(i,j,k,tmap[index][l+2])==-1)
	cout << (int)index << " ";*/
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
      mesh.addVert(Vertex(i+res,j,k));
      xevix[i*xoff[0]+j*xoff[1]+1*xoff[2]] = point_count;
      if ((sus1|sus2)&1>0)
	mesh.setVertLabel(point_count,1); //one or both uncarved
      else if ((sus1|sus2)>1)
	mesh.setVertLabel(point_count,2); //one or both unknown
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
      mesh.addVert(Vertex(i,j+res,k));
      yevix[i*yoff[0]+j*yoff[1]+1*yoff[2]] = point_count;
      if ((sus1|sus2)&1>0)
	mesh.setVertLabel(point_count,1); //one or both uncarved
      else if ((sus1|sus2)>1)
	mesh.setVertLabel(point_count,2); //one or both unknown
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
      mesh.addVert(Vertex(i,j,k+res));
      zevix[i*zoff[0]+j*zoff[1]+1*zoff[2]] = point_count;
      if ((sus1|sus2)&1>0)
	mesh.setVertLabel(point_count,1); //one or both uncarved
      else if ((sus1|sus2)>1)
	mesh.setVertLabel(point_count,2); //one or both unknown
      point_count++;
    }
}

float mcvolume::d(int x,int y,int z,int* suspect)
{
  //To handle the boundary conditions,
  // mc considers a volume larger by one pixel on each side in each dimension.
  // v->d() is boundary protected, so we can safely subtract 1 to get the correct result.
  float val=v->d(x-1,y-1,z-1);
  //float val=v->d(x,y,z);

  if (suspect)
    {
      *suspect=0;
      if (checkCarvedInfo)
	{
	  if (!v->getCarvedFlag(x-1,y-1,z-1))
	    {
	      *suspect|=1;
	    }
	}
      
      if (checkKnownInfo)
	{
	  if (!v->getKnownFlag(x-1,y-1,z-1))
	    {
	      *suspect|=2;
	    }
	}
    }
  
  if (carved.hasdata())
    {
      //int index=(z*vsize[1]+y)*vsize[0]+x;
      int index=((z-1)*(vsize[1]-2)+(y-1))*(vsize[0]-2)+(x-1);
      if ((x-1>=0) && (y-1>=0) && (z-1>=0) && (x-1<vsize[0]-2) && (y-1<vsize[1]-2) && (z-1<vsize[2]-2))
	{ 
	  if (1)  //This changes the data so the topology is repaired.
	    {
	      if (carved.getbit(index))
		{
		  //sign given by outside is correct if carved
		  //if ((known.getbit(index)) && (val<=0)) cout << '!';
		  val=fabs(val)*(outside.getbit(index)?1:-1);
		  //if ((known.getbit(index)) && (val<=0)) cout << '@';
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
	  else    //This just marks vertices that are on uncollapsable cycles.
	    {
	      if ((carved.getbit(index)) && (known.getbit(index)))
		{
		  if (suspect) *suspect=0;
		}
	      else
		{
		  if (suspect) *suspect=1;
		}
	    }
	}
      else
	{
	  if (suspect) *suspect=0;
	}
    }
  else if (known.hasdata())
    {
      //int index=(z*vsize[1]+y)*vsize[0]+x;
      int index=((z-1)*(vsize[1]-2)+(y-1))*(vsize[0]-2)+(x-1);
      if ((x-1>=0) && (y-1>=0) && (z-1>=0) && (x-1<vsize[0]-2) && (y-1<vsize[1]-2) && (z-1<vsize[2]-2))
	{ 
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
      else
	{
	  if (suspect) *suspect=0;
	}
    }
  
  //if (val<=0) cout << "(" << x-1 << "," << y-1 << "," << z-1 << "):" << val << '\n';

  return val;
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
  if ((argc<4) || (argc>7))
    {
      cout << "Usage: mc <original volume file> <isovalue> <output .t file> ";
      cout << "[-<options> [<known .v2 file> [<carved .v2 file> [inside|outside]]]]" << endl;
      return 0;
    }

  double isovalue = atof(argv[2]);

  //cout << "Using isovalue " << isovalue << endl;

  char* options=NULL,* knownFilename=NULL,* carvedFilename=NULL;
  if (argc>4) options=argv[4];
  if (argc>5) knownFilename=argv[5];
  if (argc>6) carvedFilename=argv[6];
  int side=-1; //default to inside, for now
  if (argc>7) if (!strncmp(argv[7],"in",2)) side=-1;
  if (argc>7) if (!strncmp(argv[7],"out",3)) side=1;
  mcvolume mcv(argv[1],isovalue,options,knownFilename,carvedFilename,side);

  mcv.save_mesh(argv[3]);
  return 0;
}
