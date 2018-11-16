//slicer.cpp
//James Vanderhyde, 26 October 2005


#include <math.h>
#include <iostream.h>
#include <stdlib.h>
#include <fstream.h>
#include <queue>
#include <unistd.h>

#ifdef __APPLE__
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 #include <GLUT/glut.h>
#else
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glut.h>
#endif

#include "Volume2DplusT.h"
#include "Vector3D.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TWOPI (2.0 * M_PI)
//#define BIGNUM 1e15
#define DEGPERRAD (180/M_PI)
#define ROOT2 1.414213562373
#define ROOT3 1.732050807569

enum
{
    MENU_SLOWER,
    MENU_FASTER,
    MENU_STOP_RUN,
    MENU_SLICE_THICK_TOGGLE,
    MENU_THICK_PARITY,
    MENU_ISOLINES_TOGGLE,
    MENU_LEVEL_SETS_TOGGLE,
    MENU_SWITCH_SHADING,
    MENU_SIMPLIFIED_TOGGLE,
    MENU_LINES_TOGGLE,
    MENU_POINTS_TOGGLE,
    MENU_TRIANGLES_TOGGLE,
    MENU_CULLING_TOGGLE,
    MENU_ISO_ORDER,
    MENU_INITIAL_ORDER,
    MENU_CARVEDINSIDE_ORDER,
    MENU_CARVEDOUTSIDE_ORDER,
    MENU_INCR_SLICE,
    MENU_DECR_SLICE
};

MarchableVolume* v;
int* vsize=NULL;
bitc critical;
int vthreshold=-1;
char* orderfilename=NULL;
char* vfilename=NULL;
char* carveinfofilename=NULL;
Volume2DplusT vol;

int numCTEdges;
int* ctEdgeList=NULL;
int* ctEdgeLabels=NULL;
int* ctCriticals=NULL;
int ctNumCriticals[4];
int* criticalParent=NULL;
int* sortedSlice=NULL;

int curSlice=0;  //number of visible slice
int sliceDim=2;  //dimension through which to slice
unsigned short mindval,maxdval;
int isovalue;
int isovalueTieBreaker;

char* seedfilename="seedwin.txt";
int* contourSeeds=NULL;
int* seedWindowsStart=NULL,* seedWindowsEnd=NULL;
int numContourSeeds,curContourSeed;
char* contourMask=NULL;

float orderIndices[4];
int activeOrder=0;
int faceColor=0;
int minLifeSpan=0;

Vector3D worldCenter;
float worldSize;

int animate=0;         //animate or not?
int gouraudShading=0;  //Gouraud shading or flat shading?
int backFaceCulling=1;
int viewportWidth,viewportHeight;

int topologyMode=0; //0=full, 1=slices
int simplifiedTree=0;
int useThickSlices=1;
int thickSliceLeftEven=1;
int computeInducedMap=0;

int showCriticals=0;
int showFaces=1;
int showTree=0;
int showVertices=0;
int showIsolines=0;
int showLevelSets=1;
int showCriticalParent=0;
int trackContour=0;
int stepThroughSliceOrder=0;

int displayIsovalue=0; //verbose
int selectedPixelX=-1,selectedPixelY=-1;

//angles used in animation
float angle1 = 0;
float angle2 = 0;
float dangle1 = 1.2;
float dangle2 = 0.2;

//stuff used for trackball rotation
Vector3D rotateStart;
Vector3D rotateEnd;
Vector3D rotateAxis(0,0,1);
float rotateAngle=0.0;
Matrix rotateMatrix={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

float vd(int x,int y,int z)
{
  return v->d(x,y,z);
}

void set_up_criticals()
{
  int index,numVoxels;
  
  numVoxels=vol.getNumVoxels();
  critical.setbit(numVoxels);
  critical.resetbit(numVoxels);

  for (index=0; index<numVoxels; index++)
    {
      int s=index/(vsize[1]*vsize[0]);
      int leftOrRight=(((s%2==1) && (thickSliceLeftEven)) || ((s%2==0) && (!thickSliceLeftEven)));
      if (((topologyMode==1) && (vol.voxelCriticalInThickSlice(index,leftOrRight))) ||
	  ((topologyMode==0) && (vol.voxelCriticalInVolume(index))))
	critical.setbit(index);
    }
}

void calc_contour_tree()
{
    if (ctCriticals) delete[] ctCriticals;
    if (ctEdgeList) delete[] ctEdgeList;
    if (ctEdgeLabels) delete[] ctEdgeLabels;
    
    if (useThickSlices)
    {
	int s=curSlice;
	int augment=0;
	if (((curSlice%2==1) && (thickSliceLeftEven)) || ((curSlice%2==0) && (!thickSliceLeftEven))) s--;
	if (computeInducedMap) augment=1+curSlice-s;
	if ((s>=0) && (s+1<vol.getNumSlices()))
	{
	    numCTEdges=vol.countCriticalsInThickSlice(s,NULL,augment)+1;
	    ctCriticals=new int[numCTEdges];
	    ctEdgeList=new int[2*numCTEdges];
	    ctEdgeLabels=new int[numCTEdges];
	    vol.getContourTreeForThickSlice(s,ctCriticals,ctNumCriticals,ctEdgeList,ctEdgeLabels,1,0);
	    numCTEdges=vol.getContourTreeForThickSlice(s,ctCriticals,ctNumCriticals,ctEdgeList,ctEdgeLabels,simplifiedTree,augment);
	}
	else
	{
	    numCTEdges=0;
	    ctCriticals=new int[numCTEdges];
	    ctEdgeList=new int[2*numCTEdges];
	    ctEdgeLabels=new int[numCTEdges];
	    ctNumCriticals[0]=ctNumCriticals[1]=ctNumCriticals[2]=ctNumCriticals[3]=0;
	}
    }
    else
    {
	numCTEdges=vol.countCriticalsInSlice(curSlice)+1;
	ctCriticals=new int[numCTEdges];
	ctEdgeList=new int[2*numCTEdges];
	ctEdgeLabels=NULL;
	numCTEdges=vol.getContourTreeForSlice(curSlice,ctCriticals,ctNumCriticals,ctEdgeList,simplifiedTree);
    }
}

void load_slice_order()
{
    if (!sortedSlice) sortedSlice=new int[vsize[0]*vsize[1]];
    vol.getSliceVoxelsInOrder(curSlice,sortedSlice);
}

void getPixelFromVoxelIndex(int index,int* sx,int* sy)
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    int xyz[3];
    vol.getVoxelLocFromIndex(index,&(xyz[0]),&(xyz[1]),&(xyz[2]));
    *sx=xyz[dim1];
    *sy=xyz[dim2];
}

int belowIsovalue2(int sx,int sy)
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    if ((sx<0) || (sx>=vsize[dim1]) || (sy<0) || (sy>=vsize[dim2])) return 0;
    int xyz[3];
    xyz[sliceDim]=curSlice;
    xyz[dim1]=sx;
    xyz[dim2]=sy;
    unsigned short d=vol.getVoxel(xyz[0],xyz[1],xyz[2]);
    int index=xyz[dim1]+xyz[dim2]*vsize[dim1];
    if (d<isovalue) return 1;
    if ((d==isovalue) && (index<=isovalueTieBreaker)) return 1;
    return 0;
}

int belowIsovalue(int sx,int sy)
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    if ((sx<0) || (sx>=vsize[dim1]) || (sy<0) || (sy>=vsize[dim2])) return 0;
    int xyz[3];
    xyz[sliceDim]=curSlice;
    xyz[dim1]=sx;
    xyz[dim2]=sy;
    int index=vol.getVoxelIndex(xyz[0],xyz[1],xyz[2]);
    //cout << vol.getVoxelOrder(index) << ',' << vol.getVoxelOrder(sortedSlice[isovalueTieBreaker]) << '\n';
    if (vol.getVoxelOrder(index)<=vol.getVoxelOrder(sortedSlice[isovalueTieBreaker])) return 1;
    return 0;
}

int belowIsovalue3(int sx,int sy)
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    if ((sx<0) || (sx>=vsize[dim1]) || (sy<0) || (sy>=vsize[dim2])) return 0;
    int xyz[3];
    xyz[sliceDim]=curSlice;
    xyz[dim1]=sx;
    xyz[dim2]=sy;
    int index=vol.getVoxelIndex(xyz[0],xyz[1],xyz[2]);
    xyz[dim1]=selectedPixelX;
    xyz[dim2]=selectedPixelY;
    int index2=vol.getVoxelIndex(xyz[0],xyz[1],xyz[2]);
    if (vol.getVoxelOrder(index)<=vol.getVoxelOrder(index2)) return 1;
    return 0;
}

void construct_contour_mask_flood_fill()
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    for (int i=0; i<vol.getSliceSize(); i++)
	contourMask[i]=0;
    for (int seed=0; seed<numContourSeeds; seed++)
    {
	int sx,sy,ox,oy;
	std::queue<int> q;
	getPixelFromVoxelIndex(contourSeeds[seed],&sx,&sy);
	//cout << "Contour seed: " << contourSeeds[seed] << '(' << sx << ',' << sy << ')' << '\n';
	if (belowIsovalue2(sx,sy))
	{
	    q.push(sx+sy*vsize[dim1]);
	    contourMask[sx+sy*vsize[dim1]]=1;
	}
	//flood fill
	while (!q.empty())
	{
	    sx=q.front()%vsize[dim1];
	    sy=q.front()/vsize[dim1];
	    q.pop();
	    for (oy=-1; oy<=1; oy++) if ((0<=sy+oy) && (sy+oy<vsize[dim2]))
		for (ox=-1; ox<=1; ox++) if ((0<=sx+ox) && (sx+ox<vsize[dim1]))
		{
		    int pixel=(sx+ox)+(sy+oy)*vsize[dim1];
		    if ((!contourMask[pixel]) && (belowIsovalue2(sx+ox,sy+oy)))
		    {
			q.push(pixel);
			contourMask[pixel]=1;
		    }
		}
	}
    }
}

void construct_contour_mask()
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    for (int i=0; i<vol.getSliceSize(); i++)
	contourMask[i]=vol.voxelCarved(i+curSlice*vol.getSliceSize());
}

int read_contour_seeds(char* filename)
{
    ifstream fin(filename);
    if (!fin)
    {
	cerr << "Can't find file " << filename << "\n";
	return 1;
    }
    
    if (contourSeeds) delete[] contourSeeds;
    fin >> numContourSeeds;
    contourSeeds=new int[numContourSeeds+1];
    for (int i=0; i<numContourSeeds; i++)
	fin >> contourSeeds[i];
    return 0;
}

int read_contour_seeds_with_window_data(char* filename)
{
    ifstream fin(filename);
    if (!fin)
    {
	cerr << "Can't find file " << filename << "\n";
	return 1;
    }
    
    if (contourSeeds)
    {
	delete[] contourSeeds;
	delete[] seedWindowsStart;
	delete[] seedWindowsEnd;
    }
    fin >> numContourSeeds;
    contourSeeds=new int[numContourSeeds+1];
    seedWindowsStart=new int[numContourSeeds+1];
    seedWindowsEnd=new int[numContourSeeds+1];
    for (int i=0; i<numContourSeeds; i++)
    {
	fin >> contourSeeds[i];
	fin >> seedWindowsStart[i];
	fin >> seedWindowsEnd[i];
    }
	
    return 0;
}

void set_current_contour_seed(int newCurSeed)
{
    curContourSeed=newCurSeed;
    cout << "Contour seed: "; cout.flush();
    vol.clearAllVoxelsCarved();
    //vol.markVoxelsBelowVoxel(contourSeeds[curContourSeed]);
    int slice=contourSeeds[curContourSeed]/vol.getSliceSize();
    int windowSize=10;
    //vol.markVoxelsBelowVoxelInSlices(contourSeeds[curContourSeed],slice-windowSize,slice+windowSize);
    vol.markVoxelsBelowVoxelInSlices(contourSeeds[curContourSeed],curContourSeed*(windowSize/2),curContourSeed*(windowSize/2)+windowSize);
    cout << contourSeeds[curContourSeed] << " (" << vol.getVoxel(contourSeeds[curContourSeed]) << ")\n";
}

void select_all_contour_seeds()
{
    cout << "Reading contour tracking data..."; cout.flush();
    vol.clearAllVoxelsCarved();
    int windowSize=10;
    //Should sort seeds so that greatest in voxel order is handled first. Otherwise floodfill may miss some.
    for (int i=0; i<numContourSeeds; i++)
    {
	if (seedWindowsStart)
	{
	    vol.markVoxelsBelowVoxelInSlices(contourSeeds[i],seedWindowsStart[i],seedWindowsEnd[i]);
	}
	else
	{
	    int slice=contourSeeds[i]/vol.getSliceSize();
	    vol.markVoxelsBelowVoxelInSlices(contourSeeds[i],slice-windowSize,slice+windowSize);
	}
    }
    cout << "done.\n";
}

void recenterWorld()
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    worldCenter.x=(vsize[dim1])/2.0;
    worldCenter.y=(vsize[dim2])/2.0;
    worldCenter.z=(vsize[sliceDim])/2.0;
    worldSize=(vsize[dim2]);
}

int set_up_volume(/*char* filename,char* carveInfo=NULL*/)
{
  v=MarchableVolume::createVolume(vfilename);
  vsize=v->getSize();
  cout << vsize[0] << "x" << vsize[1] << "x" << vsize[2] << "=" << vsize[2]*vsize[1]*vsize[0] << "\n";
  vol.setFeatureSize(vthreshold);
  vol.createFromMarchableVolume(v);
  vol.readTopoinfoFiles();
  //int signsNeedToBeChanged=vol.dataShouldBeNegated();
  //if (signsNeedToBeChanged) vol.changeAllSigns();
  
  if (vthreshold>=0)
  {
      if (topologyMode==0) vol.fixTopologyStrict();
      if (topologyMode==1) vol.fixTopologyBySlices();
  }
  else
  {
      vol.sortVoxels();
      if (orderfilename) vol.readCarvedOrder(orderfilename);
      if (carveinfofilename) vol.loadCarveInfo(carveinfofilename);
  }
  
  set_up_criticals();
  if (showTree) calc_contour_tree();
  if (stepThroughSliceOrder) load_slice_order();
  
  //vol.buildTrees(vol.getDefaultOrder(),1);
  //contourSeeds=new int[vol.getSliceSize()];
  //numContourSeeds=0;
  //vol.buildFullJoinTree();
  if (trackContour)
  {
      int result;
      //result=read_contour_seeds("seeds.txt");
      result=read_contour_seeds_with_window_data(seedfilename);
      if (result)
      {
	  trackContour=0;
      }
      else
      {
	  //set_current_contour_seed(0);
	  select_all_contour_seeds();
	  contourMask=new char[vol.getSliceSize()];
	  construct_contour_mask();
      }
  }

  //if (signsNeedToBeChanged) vol.changeAllSigns();
  
  int x,y,z;
  unsigned short val;
  mindval=65535;
  maxdval=0;
  for (z=0; z<vsize[2]; z++) for (y=0; y<vsize[1]; y++) for (x=0; x<vsize[0]; x++)
    {
      val=vol.getVoxel(x,y,z);
      if (val<mindval) mindval=val;
      if (val>maxdval) maxdval=val;
      //if (val-floor(val) != 0) cout << "Non-integer value " << val << '\n';
    }
  isovalue=mindval-1;
  isovalueTieBreaker=0;
  //isovalueTieBreaker=vsize[1]*vsize[0];

  cout << "Min: " << mindval << "  Max: " << maxdval << '\n';

  recenterWorld();

  return 0;
}

void setColor(int x,int y,int z,int sx,int sy)
{
  int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
  int vindex=vol.getVoxelIndex(x,y,z);
  unsigned short dval=vol.getVoxel(vindex);
  float grayValue=(dval-mindval)/(float)(maxdval-mindval);
  float r,g,b;
  if ((showCriticals) && (critical.getbit((z*vsize[1]+y)*vsize[0]+x)))
    {
      r=0.7;
      g=0.1;
      b=0.7;
    }
  else
  {
      r=g=b=0.8*grayValue;
  }
  int below;
  if (stepThroughSliceOrder) below=belowIsovalue(sx,sy);
  else below=belowIsovalue2(sx,sy);
  if ((below) || (showIsolines))
  {
      if (((trackContour) && (contourMask[sx+sy*vsize[dim1]])) ||
	  ((carveinfofilename) && (vol.voxelCarved(vindex))))
      {
	  r+=0.2;
	  g+=0.05;
	  b+=0.05;
      }
      else
      {
	  r+=0.2;
	  g+=0.2;
      }
  }
  else 
  {
      b+=0.2;
  }
  if ((sx==selectedPixelX) && (sy==selectedPixelY)) r=g=b=1.0;
  glColor3f(r,g,b);
}

GLvoid draw_slice()
{
  int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
  int xyz[3];
  int i1,i2;

  glPushMatrix();
  glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
  glTranslatef(-(vsize[dim1]-1)/2.0,-(vsize[dim2]-1)/2.0,0.0);

  glBegin(GL_QUADS);

  for (i2=0; i2<vsize[dim2]; i2++) 
    {
      for (i1=0; i1<vsize[dim1]; i1++)
	{
	  xyz[sliceDim]=curSlice;
	  xyz[dim1]=i1;
	  xyz[dim2]=i2;
	  setColor(xyz[0],xyz[1],xyz[2],i1,i2);
	  glVertex3f(i1-0.5,i2-0.5,0.0);
	  glVertex3f(i1+0.5,i2-0.5,0.0);
	  glVertex3f(i1+0.5,i2+0.5,0.0);
	  glVertex3f(i1-0.5,i2+0.5,0.0);
	}
      //cout << '\n';
    }
  //cout << '\n';
      
  glEnd();
  
  glPopMatrix();
  //cout << "Total " << total << " voxels\n";
}

GLint needEdge(int sx1,int sy1,int sx2,int sy2)
{
    return ((belowIsovalue(sx1,sy1)) && (!belowIsovalue(sx2,sy2)));
}

GLvoid draw_isolines()
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    int i1,i2;
    
    glPushMatrix();
    glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
    glTranslatef(-(vsize[dim1]-1)/2.0,-(vsize[dim2]-1)/2.0,0.0);
    
    glBegin(GL_LINES);
    
    for (i2=0; i2<vsize[dim2]; i2++) 
    {
	for (i1=0; i1<vsize[dim1]; i1++)
	{
	    if (needEdge(i1,i2,i1-1,i2))
	    {
		glVertex3f(i1-0.5,i2-0.5,1.0);
		glVertex3f(i1-0.5,i2+0.5,1.0);
	    }
	    if (needEdge(i1,i2,i1+1,i2))
	    {
		glVertex3f(i1+0.5,i2-0.5,1.0);
		glVertex3f(i1+0.5,i2+0.5,1.0);
	    }
	    if (needEdge(i1,i2,i1,i2-1))
	    {
		glVertex3f(i1-0.5,i2-0.5,1.0);
		glVertex3f(i1+0.5,i2-0.5,1.0);
	    }
	    if (needEdge(i1,i2,i1,i2+1))
	    {
		glVertex3f(i1+0.5,i2+0.5,1.0);
		glVertex3f(i1-0.5,i2+0.5,1.0);
	    }
	}
	//cout << '\n';
    }
    //cout << '\n';
    
    glEnd();
    
    glPopMatrix();
    //cout << "Total " << total << " voxels\n";
}

GLvoid vertex_from_index(int index,float depth)
{
    int sx,sy;
    getPixelFromVoxelIndex(index,&sx,&sy);
    glVertex3f(sx,sy,depth);
}

GLvoid draw_critical_points_from_arrays(int* numCrits,int* crits)
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;

    glPushMatrix();
    glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
    glTranslatef(-(vsize[dim1]-1)/2.0,-(vsize[dim2]-1)/2.0,0.0);
    
    glBegin(GL_POINTS);
    int i=0;
    for (; i<numCrits[0]; i++)
    {
	glColor3f(0.15,0.2,0.35);
	vertex_from_index(crits[i],2.0);
    }
    for (; i<numCrits[0]+numCrits[1]; i++)
    {
	glColor3f(0.475,0.35,0.3);
	vertex_from_index(crits[i],2.0);
    }
    for (; i<numCrits[0]+numCrits[1]+numCrits[2]; i++)
    {
	glColor3f(0.3125,0.275,0.375);
	if (!simplifiedTree) vertex_from_index(crits[i],2.0);
    }
    for (; i<numCrits[0]+numCrits[1]+numCrits[2]+numCrits[3]; i++)
    {
	glColor3f(0.5,0.44,0.6);
	vertex_from_index(crits[i],2.0);
    }
    glEnd();
    
    glPopMatrix();
}

GLvoid draw_contour_tree_from_edges(int numEdges,int* edgeList,int* labels)
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    
    glPushMatrix();
    glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
    glTranslatef(-(vsize[dim1]-1)/2.0,-(vsize[dim2]-1)/2.0,0.0);
    
    //cout << "Contour tree edges:\n";
    for (int i=0; i<numEdges; i++)
    {
	if ((computeInducedMap) && (useThickSlices) && (!simplifiedTree))
	    glLineWidth(1.0+labels[i]);
	else glLineWidth(1.0);
	glBegin(GL_LINES);
	vertex_from_index(edgeList[2*i],2.5);
	vertex_from_index(edgeList[2*i+1],2.5);
	glEnd();
	if ((computeInducedMap) && (useThickSlices) && (!simplifiedTree))
	{
	    //cout << " " << edgeList[2*i] << "-" << edgeList[2*i+1] << "[" << labels[i] << "]\n";
	}
	else
	{
	    //cout << " " << edgeList[2*i] << "-" << edgeList[2*i+1] << "\n";
	}
    }
    
    glPopMatrix();
}

GLvoid draw_contour_tree()
{
    glColor3f(0.8,1.0,0.8);
    glPolygonOffset(-1,-1);
    glLineWidth(1.0);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    draw_contour_tree_from_edges(numCTEdges,ctEdgeList,ctEdgeLabels);

    glPolygonOffset(-2,-2);
    glPointSize(4.0);
    glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
    draw_critical_points_from_arrays(ctNumCriticals,ctCriticals);
}

int getVoxelIndexFromPixel(int sx,int sy)
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    int xyz[3];
    xyz[sliceDim]=curSlice;
    xyz[dim1]=sx;
    xyz[dim2]=sy;
    return vol.getVoxelIndex(xyz[0],xyz[1],xyz[2]);
}

GLvoid draw_critical_parents()
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    int i1,i2,index,critindex,regindex;
    
    glPushMatrix();
    glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
    glTranslatef(-(vsize[dim1]-1)/2.0,-(vsize[dim2]-1)/2.0,0.0);

    index=selectedPixelX+vsize[dim1]*(selectedPixelY+curSlice*vsize[dim2]);

    glColor3f(0.05,0.05,0.05);
    glPointSize(3.0);
    glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
    glBegin(GL_POINTS);
    vertex_from_index(index,2.8);
    glEnd();
    glColor3f(0.95,0.95,0.95);
    glPointSize(2.0);
    glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
    glBegin(GL_POINTS);
    vertex_from_index(index,3.0);
    glEnd();
    
    glColor3f(0.95,0.3,0.95);
    glLineWidth(1.0);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glBegin(GL_LINES);
    if (critical.getbit(index)) critindex=index;
    else critindex=vol.getCriticalParent(index);
    for (i2=0; i2<vsize[dim2]; i2++) 
    {
	for (i1=0; i1<vsize[dim1]; i1++)
	{
	    regindex=getVoxelIndexFromPixel(i1,i2);
	    if (vol.getCriticalParent(regindex)==critindex)
	    {
		vertex_from_index(critindex,2.0);
		vertex_from_index(regindex,2.0);
	    }
	}
    }
    glEnd();
    
    glPopMatrix();
}

GLvoid display(GLvoid)
{
  //clear the color and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //set up the model-view matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //place object between clipping planes
  glTranslatef(0,0,-20);

  //apply animated rotations
  glRotatef(angle1,0.0,1.0,0.0);
  glRotatef(angle2,1.0,0.0,1.0);
  
  //apply trackball rotation
  glRotatef(rotateAngle,rotateAxis.x,rotateAxis.y,rotateAxis.z);
  glMultMatrixf(rotateMatrix);

  //draw the object
  /*if (showVertices)
    {
      glDisable(GL_LIGHTING);
      glColor3f(0.3,0.4,0.7);
      glPolygonOffset(-2,-2);
      glPointSize(2.0);
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      draw_critical_points();
    }*/
  if (showFaces)
    {
      glPolygonOffset(0,0);
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      draw_slice();
    }
  if (showTree)
    {
      glDisable(GL_LIGHTING);
      draw_contour_tree();
    }
  if (showIsolines)
  {
      glDisable(GL_LIGHTING);
      glColor3f(0.9,0.6,0.2);
      glPolygonOffset(-1,-1);
      glLineWidth(1.0);
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      draw_isolines();
  }
  if ((showCriticalParent) && (selectedPixelX>=0))
  {
      glDisable(GL_LIGHTING);
      draw_critical_parents();
  }
 
  //flush the pipeline
  glFlush();

  //look at our handiwork
  glutSwapBuffers();

}

void reshape(int w, int h)
{
  //change the screen window (viewport) size
  viewportWidth=w;
  viewportHeight=h;
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);

  //set up the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(2*atan(1.0/20.0)*DEGPERRAD,(GLfloat) w/(GLfloat) h,15.0,25.0);
}

void idle()
{
  if (animate)
    {
      angle1 += dangle1;
      angle2 += dangle2;
      glutPostRedisplay();
    }
}

Vector3D pixelToTrackball(GLint x,GLint y)
{
	GLint viewport[4];
	Vector3D r;
	glGetIntegerv(GL_VIEWPORT, viewport);
	int vpd=(viewport[2]<viewport[3])?viewport[2]:viewport[3];
	r.x=2.0*((x-viewport[2]/2.0)/vpd);
	r.y=-2.0*((y-viewport[3]/2.0)/vpd);
	double d=1-r.x*r.x-r.y*r.y;
	if (d>=0.0) r.z=sqrt(d);
	else
    {
		d=1.0/sqrt(1-d);
		r.x*=d;
		r.y*=d;
		r.z=0.0;
    }
	return r;
}

void recordRotation()
{
	float sint,cost,x,y,z;
	sint=length(cross(rotateStart,rotateEnd));
	cost=dot(rotateStart,rotateEnd);
	x=rotateAxis.x; y=rotateAxis.y; z=rotateAxis.z;
	Matrix rot;
	rot[3]=rot[7]=rot[11]=rot[12]=rot[13]=rot[14]=0;
	rot[0]=1+(1-cost)*(x*x-1);
	rot[5]=1+(1-cost)*(y*y-1);
	rot[10]=1+(1-cost)*(z*z-1);
	rot[15]=1;
	rot[6]= x*sint+(1-cost)*y*z;
	rot[9]=-x*sint+(1-cost)*y*z;
	rot[8]= y*sint+(1-cost)*z*x;
	rot[2]=-y*sint+(1-cost)*z*x;
	rot[1]= z*sint+(1-cost)*x*y;
	rot[4]=-z*sint+(1-cost)*x*y;
	Matrix oldRot;
	copy(rotateMatrix,oldRot);
	times(rot,oldRot,rotateMatrix);
}

void select_pixel(int mouseX,int mouseY)
{
  int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
  int xOffset=viewportWidth-viewportHeight;
  xOffset=0;
  selectedPixelX=vsize[dim1]*(mouseX-xOffset/2)/(viewportWidth-xOffset);
  selectedPixelY=vsize[dim2]*(viewportHeight-1-mouseY)/viewportHeight;
  
  int xyz[3];
  xyz[dim1]=selectedPixelX;
  xyz[dim2]=selectedPixelY;
  xyz[sliceDim]=curSlice;
  int voxelIndex=vol.getVoxelIndex(xyz[0],xyz[1],xyz[2]);
  
  //use selected pixel for contour tracking
  if (trackContour)
  {
      contourSeeds[numContourSeeds]=voxelIndex;
      curContourSeed=numContourSeeds;
      set_current_contour_seed(curContourSeed);
      construct_contour_mask();
  }
  
  glutPostRedisplay();
}

void print_selected_voxel()
{
    int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
    int xyz[3];
    xyz[dim1]=selectedPixelX;
    xyz[dim2]=selectedPixelY;
    xyz[sliceDim]=curSlice;
    int voxelIndex=vol.getVoxelIndex(xyz[0],xyz[1],xyz[2]);
    cout << voxelIndex;
    cout << "(" << xyz[0] << "," << xyz[1] << "," << xyz[2] << ") " << vd(xyz[0],xyz[1],xyz[2]);
    unsigned short dval=vol.getVoxel(voxelIndex);
    cout << "->" << dval;
    cout << " Order=" << vol.getVoxelOrder(voxelIndex);
    cout << '\n';
}

void mouse_button(int btn, int state, int mx, int my)
{
	switch( btn ) {
		case GLUT_LEFT_BUTTON:
			switch( state ) {
				case GLUT_DOWN: 
					rotateStart=pixelToTrackball(mx,my);
					rotateAngle=0.0;
					break;
				case GLUT_UP:  
					if (rotateAngle != 0.0) recordRotation();
					rotateAngle=0.0;
				        select_pixel(mx,my);
					print_selected_voxel();
					break;
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			switch( state ) {
				case GLUT_DOWN: 
					break;
				case GLUT_UP:   
					break;
			}
			break;
		case GLUT_RIGHT_BUTTON:
			switch( state ) {
				case GLUT_DOWN: 
					break;
				case GLUT_UP:   
					break;
			}
			break;
	}
}

void button_motion(int mx, int my)
{
	rotateEnd=pixelToTrackball(mx,my);
	rotateAxis=normalized(cross(rotateStart,rotateEnd));
	rotateAngle=DEGPERRAD*acos(dot(rotateStart,rotateEnd));
	glutPostRedisplay();
	return;
}

void change_slice(int newSlice)
{
    int oldSlice=curSlice;
    curSlice=newSlice;
    if (showTree) calc_contour_tree();
    if (stepThroughSliceOrder) load_slice_order();
    if (trackContour)
    {
	//numContourSeeds=vol.simpleTrack(oldSlice,curSlice,numContourSeeds,contourSeeds,isovalue);
	construct_contour_mask();
    }
}

void menu(int value)
{
  switch(value)
    {
    case MENU_SLOWER:
      dangle1 *= .66667;
      dangle2 *= .66667;
      break;
    case MENU_FASTER:
      dangle1 *= 1.5;
      dangle2 *= 1.5;
      break;
    case MENU_STOP_RUN:
      animate = !animate;
      if (animate) glutIdleFunc(idle);
      else glutIdleFunc(NULL);
      break;
    case MENU_SLICE_THICK_TOGGLE:
      useThickSlices = !useThickSlices;
      if (showTree) calc_contour_tree();
      break;
    case MENU_THICK_PARITY:
      thickSliceLeftEven = !thickSliceLeftEven;
      if (showTree) calc_contour_tree();
      break;
    case MENU_CULLING_TOGGLE:
      backFaceCulling = !backFaceCulling;
      if (backFaceCulling) glEnable(GL_CULL_FACE);
      else glDisable(GL_CULL_FACE);
      break;
    case MENU_SWITCH_SHADING:
      gouraudShading = !gouraudShading;
      break;
    case MENU_SIMPLIFIED_TOGGLE:
      simplifiedTree = !simplifiedTree;
      if (showTree) calc_contour_tree();
      break;
    case MENU_ISOLINES_TOGGLE:
      showIsolines = !showIsolines;
      break;
    case MENU_LEVEL_SETS_TOGGLE:
      showLevelSets = !showLevelSets;
      break;
    case MENU_LINES_TOGGLE:
      showTree = !showTree;
      if (showTree) calc_contour_tree();
      break;
    case MENU_POINTS_TOGGLE:
      showCriticals = !showCriticals;
      break;
    case MENU_TRIANGLES_TOGGLE:
      showFaces = !showFaces;
      break;
    case MENU_ISO_ORDER:
      activeOrder=0;
      break;
    case MENU_INITIAL_ORDER:
      activeOrder=1;
      break;
    case MENU_CARVEDINSIDE_ORDER:
      activeOrder=2;
      break;
    case MENU_CARVEDOUTSIDE_ORDER:
      activeOrder=3;
      break;
    case MENU_INCR_SLICE:
      if (curSlice<vsize[sliceDim]-1)
      {
	  change_slice(curSlice+1);
      }
      break;
    case MENU_DECR_SLICE:
      if (curSlice>0)
      {
	  change_slice(curSlice-1);
      }
      break;
    }
  glutPostRedisplay();
}

void quit(int exitCondition)
{
    if (ctCriticals) delete[] ctCriticals;
    if (ctEdgeList) delete[] ctEdgeList;
    if (ctEdgeLabels) delete[] ctEdgeLabels;
    if (sortedSlice) delete[] sortedSlice;
    if (contourSeeds) delete[] contourSeeds;
    if (contourMask) delete[] contourMask;
  exit(exitCondition);
}

void calc_viewport()
{
    if (vsize)
    {
	int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
	viewportWidth=vsize[dim1];
	viewportHeight=vsize[dim2];
	while ((viewportWidth>1024-16) || (viewportHeight>768-32))
	{
	    viewportWidth*=2;
	    viewportHeight*=2;
	    viewportWidth/=3;
	    viewportHeight/=3;
	}
	while ((3*viewportWidth/2<1024-16) && (3*viewportHeight/2<768-100))
	{
	    viewportWidth*=3;
	    viewportHeight*=3;
	    viewportWidth/=2;
	    viewportHeight/=2;
	}
    }
    else
    {
	viewportWidth=400;
	viewportHeight=400;
    }
}

void keyboard(unsigned char key, int x, int y)
{
  int dim1=(sliceDim+1)%3,dim2=(sliceDim+2)%3;
  switch(key) 
    {
    case 27:  /* Esc */
      quit(0);
    case '/':
      //menu(MENU_STOP_RUN);
      break;
    case ',':
      menu(MENU_DECR_SLICE);
      break;
    case '.':
      menu(MENU_INCR_SLICE);
      break;
    case 'z':
      sliceDim=(sliceDim+1)%3;
      if (curSlice>vsize[sliceDim]-1) curSlice=vsize[sliceDim]-1;
      if (showTree) calc_contour_tree();
      recenterWorld();
      calc_viewport();
      glutReshapeWindow(viewportWidth,viewportHeight);
      break;
    case '1':
      isovalueTieBreaker += 1;
      while (isovalueTieBreaker>=vsize[dim2]*vsize[dim1])
      {
	  isovalueTieBreaker-=vsize[dim2]*vsize[dim1];
	  isovalue += 1;
      }
      if (displayIsovalue) cout << isovalue << '(' << isovalueTieBreaker << ')' << '\n';
      break;
    case '2':
      isovalueTieBreaker += vsize[dim1];
      while (isovalueTieBreaker>=vsize[dim2]*vsize[dim1])
      {
	  isovalueTieBreaker-=vsize[dim2]*vsize[dim1];
	  isovalue += 1;
      }
      if (displayIsovalue) cout << isovalue << '(' << isovalueTieBreaker << ')' << '\n';
      break;
    case '3':
      isovalue += 1;
      if (displayIsovalue) cout << isovalue << '\n';
      break;
    case '4':
      isovalue += 16;
      if (displayIsovalue) cout << isovalue << '\n';
      break;
    case '5':
      isovalue += 256;
      if (displayIsovalue) cout << isovalue << '\n';
      break;
    case '6':
      isovalue += 4096;
      if (displayIsovalue) cout << isovalue << '\n';
      break;
    case '!':
      isovalueTieBreaker -= 1;
      while (isovalueTieBreaker<0)
      {
	  isovalueTieBreaker+=vsize[dim2]*vsize[dim1];
	  isovalue -= 1;
      }
      if (displayIsovalue) cout << isovalue << '(' << isovalueTieBreaker << ')' << '\n';
      break;
    case '@':
      isovalueTieBreaker -= vsize[dim1];
      while (isovalueTieBreaker<0)
      {
	  isovalueTieBreaker+=vsize[dim2]*vsize[dim1];
	  isovalue -= 1;
      }
      if (displayIsovalue) cout << isovalue << '(' << isovalueTieBreaker << ')' << '\n';
      break;
    case '#':
      isovalue -= 1;
      if (displayIsovalue) cout << isovalue << '\n';
      break;
    case '$':
      isovalue -= 16;
      if (displayIsovalue) cout << isovalue << '\n';
      break;
    case '%':
      isovalue -= 256;
      if (displayIsovalue) cout << isovalue << '\n';
      break;
    case '^':
      isovalue -= 4096;
      if (displayIsovalue) cout << isovalue << '\n';
      break;
    case 'l':
      minLifeSpan++;
      break;
    case 'L':
      minLifeSpan--;
      break;
    case 's':
      curContourSeed++;
      if (curContourSeed>=numContourSeeds) curContourSeed=0;
      if (trackContour)
      {
	  set_current_contour_seed(curContourSeed);
	  construct_contour_mask();
      }
      break;
    case 'S':
      if (curContourSeed<=0) curContourSeed=numContourSeeds;
      curContourSeed--;
      if (trackContour)
      {
	set_current_contour_seed(curContourSeed);
	construct_contour_mask();
      }
      break;
	
    default:  break;
    }
  glutPostRedisplay();
}

void init_opengl()
{
  //turn on back-face culling
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  //automatically scale normals to unit length after transformation
  glEnable(GL_NORMALIZE);

  //set clear color to black
  glClearColor(0.0, 0.0, 0.0, 1.0);

  //enable depth test (z-buffer)
  glEnable(GL_DEPTH_TEST);

  //allow the edges and vertices to be drawn on top of the triangles
  glEnable(GL_POLYGON_OFFSET_LINE);
  glEnable(GL_POLYGON_OFFSET_POINT);
}

void init_glut(int *argc, char **argv)
{
  glutInit(argc,argv);

  //size and placement hints to the window system
  calc_viewport();
  glutInitWindowSize(viewportWidth,viewportHeight);
  glutInitWindowPosition(10,10);

  //double buffered, RGB color mode, with depth test
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  //create a GLUT window (not drawn until glutMainLoop() is entered)
  char* title=new char[strlen(vfilename)+10];
  sprintf(title,"Slicer: %s",vfilename);
  glutCreateWindow(title);

  //register callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse_button);
  glutMotionFunc(button_motion);

  //set up pop-up menu
  GLint menuID = glutCreateMenu(menu);
  //glutAddMenuEntry("slower",MENU_SLOWER);
  //glutAddMenuEntry("faster",MENU_FASTER);
  //glutAddMenuEntry("stop/run",MENU_STOP_RUN);
  //glutAddMenuEntry("switch shading",MENU_SWITCH_SHADING);
  glutAddMenuEntry("toggle faces",MENU_TRIANGLES_TOGGLE);
  glutAddMenuEntry("toggle criticals",MENU_POINTS_TOGGLE);
  glutAddMenuEntry("toggle tree",MENU_LINES_TOGGLE);
  glutAddMenuEntry("toggle tree simplification",MENU_SIMPLIFIED_TOGGLE);
  glutAddMenuEntry("toggle slice/thick slice",MENU_SLICE_THICK_TOGGLE);
  glutAddMenuEntry("toggle thick slice even/odd",MENU_THICK_PARITY);
  //glutAddMenuEntry("toggle level set display",MENU_LEVEL_SETS_TOGGLE);
  glutAddMenuEntry("toggle isolines/sublevel sets",MENU_ISOLINES_TOGGLE);
  //glutAddMenuEntry("order by isovalue",MENU_ISO_ORDER);
  //glutAddMenuEntry("order by initial",MENU_INITIAL_ORDER);
  //glutAddMenuEntry("order by carved inside",MENU_CARVEDINSIDE_ORDER);
  //glutAddMenuEntry("order by carved outside",MENU_CARVEDOUTSIDE_ORDER);
  glutSetMenu(menuID);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int process_options(int argc, char **argv)
{
    int c;
    while ((c = getopt (argc, argv, "t:o:e:m:sv")) != -1) switch (c)
    {
    case 's':
	displayIsovalue=0;
	break;
    case 'v':
	displayIsovalue=1;
	break;
    case 'o':
	orderfilename = optarg;
	break;
    case 'e':
	trackContour=1;
	seedfilename = optarg;
	break;
    case 't':
	vthreshold = atoi(optarg);
	break;
    case 'm':
	trackContour=0;
	carveinfofilename = optarg;
	break;
    case '?':
	if (isprint (optopt))
	    fprintf (stderr,"Unknown option `-%c'.\n",optopt);
	else
	    fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
	return 1;
    default:
	return 1;
    }
    vfilename=argv[optind];
    return 0;
}

int main(int argc, char **argv)
{
    if (argc<2)
    {
	cerr << "Usage: " << argv[0] << " [-t threshold] [-o <.vo order file>] [-e <seed file>] [-m <.v2 carved info file>] [-s][-v] <.v file>\n";
	return 1;
    }
    
    int result;
    result=process_options(argc,argv);
    if (result) return result;
    
    set_up_volume();
    
    init_glut(&argc, argv);
    init_opengl();
    
    glutMainLoop();
    
    return 0;
}

