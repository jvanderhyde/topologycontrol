//t2pov.cpp
//James Vanderhyde, 23 August 2005
//Reads in a triangle mesh and a camera.dat file and writes a .pov POVRay file

#include <sstream>
#include <math.h>

#include "TriangleMesh.h"

#define BIGNUM 1e15
#define ZOOMBASE 7.95

int main(int argc, char** argv)
{
  if (argc<=2)
    {
      cerr << "Usage: " << argv[0] << " <input .t file> <output .pov file>";
      cerr << " [<camera.dat file>|- [<output .png file>]]\n";
      return 1;
    }

  char* triFilename;
  char* cameraFilename;
  char* povFilename;
  char* imageFilename;

  Vector3D worldCenter,worldMinCorner,worldMaxCorner;
  float worldSize;
  Vector3D cameraPos(0,0,0);
  float zoomFactor=1.0;
  Matrix rotateMatrix={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
  int screenWidth=400,screenHeight=400;

  int colorTrisByLabel=1;
  
  triFilename=argv[1];
  if (argc>=3) cameraFilename=argv[3];
  else cameraFilename=NULL;
  povFilename=argv[2];
  if (argc>=4) imageFilename=argv[4];
  else imageFilename=NULL;

  int result;
  TriangleMesh mesh;
  result=mesh.readFile(triFilename);
  if (result) return result;
  if ((cameraFilename) && (strcmp(cameraFilename,"-")))
    {
      ifstream fin(cameraFilename);
      if (!fin)
	{
	  cerr << "Error reading camera file " << cameraFilename << '\n';
	  return 1;
	}
      fin >> cameraPos.x;
      fin >> cameraPos.y;
      fin >> cameraPos.z;
      fin >> zoomFactor;
      for (int i=0; i<16; i++)
	fin >> rotateMatrix[i];
      fin >> screenWidth >> screenHeight;
    }
  ofstream fout(povFilename);
  if (!fout)
    {
      cerr << "Error writing to POVray file " << povFilename << '\n';
      return 1;
    }
  
  int i;
  Vertex v;
  Triangle t;
  float minx=BIGNUM,miny=BIGNUM,minz=BIGNUM,maxx=-BIGNUM,maxy=-BIGNUM,maxz=-BIGNUM;
  int numVerts=mesh.getNumVerts();
  int numTris=mesh.getNumTris();
  for (i=0; i<numVerts; i++)
    {
      v=mesh.getVert(i);
      if (-v.x<minx) minx=-v.x;
      if (-v.x>maxx) maxx=-v.x;
      if (v.y<miny) miny=v.y;
      if (v.y>maxy) maxy=v.y;
      if (v.z<minz) minz=v.z;
      if (v.z>maxz) maxz=v.z;
    }
  worldCenter.x=(maxx+minx)/2.0;
  worldCenter.y=(maxy+miny)/2.0;
  worldCenter.z=(maxz+minz)/2.0;
  worldSize=(maxy-miny);
  worldMinCorner.x=minx;
  worldMaxCorner.x=maxx;
  worldMinCorner.y=miny;
  worldMaxCorner.y=maxy;
  worldMinCorner.z=minz;
  worldMaxCorner.z=maxz;
  mesh.calculateNormals();
  double upv = 2*sin(3.1415926/180*ZOOMBASE*zoomFactor/2);


  fout << "camera {" << endl;
  fout << "  location <0,0,0>" << endl;
  fout << "  up <0," << upv << ",0>" << endl;
  fout << "  right <" << upv*screenWidth/(float)screenHeight << ",0,0>" << endl;
  fout << "  look_at <0,0,-1>" << endl;
  fout << "}" << endl << endl;
  
  fout << "light_source {" << endl;
  fout << "  <-2,2,-18>" << endl;
  fout << "  color rgb <1.7,1.7,1.7>" << endl;
  fout << "}" << endl << endl;

  fout << "light_source {" << endl;
  fout << "  <2,0,-19>" << endl;
  fout << "  color rgb <0.4,0.4,0.4>" << endl;
  fout << "}" << endl << endl;

  fout << "light_source {" << endl;
  fout << "  <0,-2,-19>" << endl;
  fout << "  color rgb <0.2,0.2,0.2>" << endl;
  fout << "}" << endl << endl;

  fout << "background { rgb <1,1,1> }" << endl << endl;

  int m,numMeshes=1,numLabeledTris;
  if (colorTrisByLabel) numMeshes=3;
  for (m=0; m<numMeshes; m++)
    {
      if (colorTrisByLabel)
	{
	  numLabeledTris=0;
	  for (i=0; i<numTris; i++) if (mesh.getTri(i).label==m) numLabeledTris++;
	}
      else
	{
	  numLabeledTris=numTris;
	}
      
      if (numLabeledTris>0)
	{
	  fout << "mesh2 {" << endl;
	  fout << "    vertex_vectors {" << endl;
	  fout << "      " << numVerts << endl;
	  for (i=0; i<numVerts; i++)
	    {
	      v=mesh.getVert(i);
	      fout << "      <" << -v.x << "," << v.y << "," << v.z << ">," << endl;
	    }
	  fout << "    }" << endl << endl;
	  fout << "    normal_vectors {" << endl;
	  fout << "      " << numVerts << endl;
	  for (i=0; i<numVerts; i++)
	    {
	      v=mesh.getVert(i);
	      fout << "      <" << -v.n.x << "," << v.n.y << "," << v.n.z << ">" << endl;
	    }
	  fout << "    }" << endl << endl;
	  
	  fout << "    face_indices {" << endl;
	  fout << "      " << numLabeledTris << endl;
	  for (i=0; i<numTris; i++)
	    {
	      t=mesh.getTri(i);
	      if ((!colorTrisByLabel) || (t.label==m))
		fout << "      <" << t.a << "," << t.b << "," << t.c << ">" << endl;
	    }
	  fout << "    }" << endl << endl;
	  
	  switch (m)
	    {
	    case 0:
	      fout << "  pigment { rgb <0.8,0.4,0.6> }" << endl << endl;
	      break;
	    case 1:
	      fout << "  pigment { rgb <0.45,0.9,0.45> }" << endl << endl;
	      break;
	    case 2:
	      fout << "  pigment { rgb <0.45,0.45,0.9> }" << endl << endl;
	      break;
	    case 3:
	      fout << "  pigment { rgb <0.45,0.9,0.9> }" << endl << endl;
	      break;
	    default:
	      fout << "  pigment { rgb <0.6,0.6,0.6> }" << endl << endl;
	    }
	  
	  fout << "  translate <" << -worldCenter.x << "," << -worldCenter.y << "," << -worldCenter.z << ">" << endl;

	  //for nonuniform CT scan resolution
	  //fout << "  scale <" << 0.625 << "," << 0.625 << "," << 1.0 << ">" << endl; //colon
	  fout << "  scale <" << 1.0 << "," << 1.0 << "," << 2.0 << ">" << endl; //brain

	  fout << "  scale <" << 2.0/worldSize << "," << 2.0/worldSize << "," << 2.0/worldSize << ">" << endl;
	  fout << "  matrix <" << rotateMatrix[0] << "," << -rotateMatrix[1] << "," << -rotateMatrix[2] << "," << endl;
	  fout << "          " << -rotateMatrix[4] << "," << rotateMatrix[5] << "," << rotateMatrix[6] << "," << endl;
	  fout << "          " << -rotateMatrix[8] << "," << rotateMatrix[9] << "," << rotateMatrix[10] << "," << endl;
	  fout << "          " << -rotateMatrix[12] << "," << rotateMatrix[13] << "," << rotateMatrix[14] << ">" << endl;
	  fout << "  translate <" << cameraPos.x << "," << -cameraPos.y << "," << -cameraPos.z << ">" << endl;
	  fout << "  translate <0,0,-20>" << endl;
	  fout << "}" << endl << endl;
	}
    }

  fout.close();
  mesh.clear();

  if (imageFilename)
    {
      std::stringstream sout;
      sout << "povray +I" << povFilename;
      sout << " +W" << screenWidth << " +H" << screenHeight;
      sout << " +A +R16 +FN" << " +O" << imageFilename << " +D";
      system(sout.str().c_str());
    }
  
  return 0;
}
