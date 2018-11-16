/*
 *  TriangleMesh.cpp
 *  
 *
 *  Created by James Vanderhyde on Thu May 6 2004.
 *
 */

#include <stack>

#include "TriangleMesh.h"

Vertex::Vertex() : x(0.0), y(0.0), z(0.0), label(0), n()
{
}

Vertex::Vertex(float xx,float yy,float zz) : x(xx), y(yy), z(zz), label(0), n()
{
}

Vector3D Vertex::getLocation()
{
	return Vector3D(x,y,z);
}

Triangle::Triangle() : a(0), b(0), c(0), label(0), n()
{
}

Triangle::Triangle(int aa,int bb,int cc) : a(aa), b(bb), c(cc), label(0), n()
{
}

int Triangle::vertex(int abc)
{
  if (abc==0) return a;
  if (abc==1) return b;
  if (abc==2) return c;
  return -1;
}

int Triangle::getOneVertex(int notVert1,int notVert2)
{
  if ((a != notVert1) && (a != notVert2)) return a;
  if ((b != notVert1) && (b != notVert2)) return b;
  if ((c != notVert1) && (c != notVert2)) return c;
  return -1;
}

TriangleMesh::TriangleMesh()
{
  numInternalEdges=numExternalEdges=0;
  numComponents=0;
  verbose=0;
}

Vertex TriangleMesh::getVert(int i)
{
	return verts[i];
}

Triangle TriangleMesh::getTri(int i)
{
	return tris[i];
}

int TriangleMesh::getOpposite(int tri,int abc)
{
	return opposites[3*tri+abc];
}

void TriangleMesh::addVert(Vertex v)
{
	verts.push_back(v);
}

void TriangleMesh::addTri(Triangle tri)
{
	tris.push_back(tri);
}

void TriangleMesh::setVertLabel(int i,int label)
{
	verts[i].label=label;
}

void TriangleMesh::setTriLabel(int i,int label)
{
	tris[i].label=label;
}

void TriangleMesh::setVerbose(int p_verbose)
{
  verbose=p_verbose;
}

int TriangleMesh::getVerbose()
{
  return verbose;
}

int TriangleMesh::getNumVerts()
{
	return verts.size();
}

int TriangleMesh::getNumTris()
{
	return tris.size();
}

int TriangleMesh::getNumInternalEdges()
{
	return numInternalEdges;
}

int TriangleMesh::getNumExternalEdges()
{
	return numExternalEdges;
}

int TriangleMesh::getNumComponents()
{
  return numComponents;
}

void TriangleMesh::eliminateExcessVertices()
{
	int numTris=getNumTris(),numVerts=getNumVerts();
	int* used=new int[getNumVerts()];
	int n=0;
	int i;
	for (i=0; i<numVerts; i++)
		used[i]=0;
	for (i=0; i<numTris; i++)
		used[tris[i].a]=used[tris[i].b]=used[tris[i].c]=1;
	for (i=0; i<numVerts; i++)
	{
		if (used[i])
		{
			verts[n]=verts[i];
			used[i]=n;
			n++;
		}
	}
	for (i=0; i<numTris; i++)
	{
		tris[i].a=used[tris[i].a];
		tris[i].b=used[tris[i].b];
		tris[i].c=used[tris[i].c];
	}
	for (i=n; i<numVerts; i++)
		verts.pop_back();
	delete[] used;
}

void TriangleMesh::calculateNormals()
{
	Vector3D n;
	int i,numVerts=getNumVerts(),numTris=getNumTris();
	for (i=0; i<numVerts; i++)
    {
		verts[i].n.x=verts[i].n.y=verts[i].n.z=0.0;
    }
	for (i=0; i<numTris; i++)
    {
		n=cross(minus(verts[tris[i].b].getLocation(),verts[tris[i].a].getLocation()),
				minus(verts[tris[i].c].getLocation(),verts[tris[i].a].getLocation())); 
		verts[tris[i].a].n=plus(verts[tris[i].a].n,n);
		verts[tris[i].b].n=plus(verts[tris[i].b].n,n);
		verts[tris[i].c].n=plus(verts[tris[i].c].n,n);
		tris[i].n=normalized(n);
    }
	for (i=0; i<numVerts; i++)
    {
		verts[i].n=normalized(verts[i].n);
    }
}

int TriangleMesh::triangleHasVertex(int t,int v)
{
	return ((tris[t].a==v) || (tris[t].b==v) || (tris[t].c==v));
}

int TriangleMesh::findIncidentTriangle(int* degrees,int** incidences,int va,int vb,int notTri)
{
	int i,t,v1,v2,r;
	if (degrees[va]<degrees[vb])
    {
		v1=va;
		v2=vb;
    }
	else
    {
		v1=vb;
		v2=va;
    }
	r=-1;
	for (i=0; i<degrees[v1]; i++)
    {
		t=incidences[v1][i];
		if ((t != notTri) && (triangleHasVertex(t,v2)))
		{
			if (r != -1)
			{
				cerr << "An edge with 3 incident triangles found!\n";
				cerr << "Triangles are " << notTri << ", " << r << ", and " << t << ".\n";
				cerr << "Edge is from vertex " << va << " to vertex " << vb << ".\n";
				//cerr << "Bailing out.\n";
				//exit(1);
			}
			else r=t;
		}
    }
	return r;
}

void TriangleMesh::computeConnectivity()
{
  if (verbose) { cout << "Computing connectivity..."; cout.flush(); }
  int i,numVerts=getNumVerts(),numTris=getNumTris();
  int* degrees=new int[numVerts];
  int** incidences=new int*[numVerts];
  for (i=0; i<numVerts; i++)
    {
      degrees[i]=0;
    }
  for (i=0; i<numTris; i++)
    {
      degrees[tris[i].a]++;
      degrees[tris[i].b]++;
      degrees[tris[i].c]++;
    }
  for (i=0; i<numVerts; i++)
    {
      incidences[i]=new int[degrees[i]];
      degrees[i]=0;
    }
  for (i=0; i<numTris; i++)
    {
      incidences[tris[i].a][degrees[tris[i].a]++]=i;
      incidences[tris[i].b][degrees[tris[i].b]++]=i;
      incidences[tris[i].c][degrees[tris[i].c]++]=i;
    }
  for (i=0; i<numTris; i++)
    {
      opposites.push_back(findIncidentTriangle(degrees,incidences,tris[i].b,tris[i].c,i));
      opposites.push_back(findIncidentTriangle(degrees,incidences,tris[i].c,tris[i].a,i));
      opposites.push_back(findIncidentTriangle(degrees,incidences,tris[i].a,tris[i].b,i));
    }
  for (i=0; i<numVerts; i++)
    {
      delete[] incidences[i];
    }
  delete[] incidences;
  delete[] degrees;
  numInternalEdges=numExternalEdges=0;
  for (i=0; i<3*numTris; i++)
    {	
      if (opposites[i]==-1) numExternalEdges++;
      else numInternalEdges++;
    }
  numInternalEdges/=2;
  if (verbose) cout << "done.\n";
}

void TriangleMesh::markBoundaryVertices()
{
  int i,numVerts=getNumVerts(),numTris=getNumTris();
  int v1,v2,v3;
  if (opposites.size()==0) computeConnectivity();
  for (i=0; i<numVerts; i++)
    {
      setVertLabel(i,0);
    }
  for (i=0; i<3*numTris; i++)
    {
      if (opposites[i]==-1)
	{
	  v1=tris[i/3].vertex(i%3);
	  v2=tris[i/3].getOneVertex(v1);
	  v3=tris[i/3].getOneVertex(v1,v2);
	  setVertLabel(v2,1);
	  setVertLabel(v3,1);
	}
    }
}

void TriangleMesh::traverseAndLabel(int startTri,int label)
{
  std::stack<int,std::vector<int> > st;
  int t,t1,t2,t3;
  st.push(startTri);
  while (!st.empty())
    {
      t=st.top();
      st.pop();
      setTriLabel(t,label);
      t1=opposites[t*3+0];
      t2=opposites[t*3+1];
      t3=opposites[t*3+2];
      if ((t1>=0) && (tris[t1].label==0)) st.push(t1);
      if ((t2>=0) && (tris[t2].label==0)) st.push(t2);
      if ((t3>=0) && (tris[t3].label==0)) st.push(t3);
    }
}

void TriangleMesh::labelComponents()
{
  if (opposites.size()==0) computeConnectivity();
  if (verbose) { cout << "Counting components..."; cout.flush(); }
  int i,numVerts=getNumVerts(),numTris=getNumTris();
  numComponents=0;
  for (i=0; i<numTris; i++)
    {
      setTriLabel(i,0);
    }
  for (i=0; i<numTris; i++)
    {
      if (tris[i].label==0)
	{
	  numComponents++;
	  traverseAndLabel(i,numComponents);
	}
    }
  if (verbose) cout << "done.\n";
}

void TriangleMesh::printBettiInfo(ostream& out)
{
  if (opposites.size()==0) computeConnectivity();
  if (numComponents==0) labelComponents();
  int euler=getNumVerts()+getNumTris()-(numInternalEdges+numExternalEdges);
  
  out << "B0 = "<< numComponents << '\n';
  out << "B1 = "<< 2*numComponents-euler << '\n';
  out << "B2 = "<< numComponents << '\n';
  out << getNumVerts() << " vertices, " << getNumTris() << " triangles, ";
  out << numInternalEdges << " internal edges, ";
  out << numExternalEdges << " external edges.\n";
  out << "EULER = " << euler << '\n';
}

void TriangleMesh::eliminateAllButLargestComponent()
{
  if (numComponents==0) labelComponents();
  int* sizes=new int[numComponents];
  int i,numTris=getNumTris(),n=0,max=0;
  for (i=0; i<numComponents; i++)
    {
      sizes[i]=0;
    }
  for (i=0; i<numTris; i++)
    {
      sizes[getTri(i).label-1]++;
    }
  for (i=0; i<numComponents; i++)
    {
      if (verbose) cout << "Component " << i+1 << " has " << sizes[i] << " triangles.\n";
      if (sizes[max]<sizes[i]) max=i;
    }
  for (i=0; i<numTris; i++)
    {
      if (getTri(i).label-1==max)
	tris[n++]=tris[i];
    }
  for (n; n<numTris; n++)
    {
      tris.pop_back();
    }
  eliminateExcessVertices();
}

void TriangleMesh::translate(float x,float y,float z)
{
	int i,numVerts=getNumVerts();	
	Vector3D offset(x,y,z);
	Vector3D result;
	for (i=0; i<numVerts; i++)
	{
		result=plus(verts[i].getLocation(),offset);
		verts[i].x=result.x;
		verts[i].y=result.y;
		verts[i].z=result.z;
	}
}

int TriangleMesh::readBinaryFile(istream& in)
{
	int numTris,numVerts,i;
	Triangle t;
	Vertex v;
	
	in.read((char*)&numTris,sizeof(int));
	in.read((char*)&numVerts,sizeof(int));
	for (i=0; i<numTris; i++)
	{
		in.read((char*)&t.a,sizeof(t.a));
		in.read((char*)&t.b,sizeof(t.b));
		in.read((char*)&t.c,sizeof(t.c));
		in.read((char*)&t.label,sizeof(t.label));
		//if (t.label>0) cout << t.label << ' ';
		addTri(t);
	}
	//cout << "...";
	for (i=0; i<numVerts; i++)
	{
		in.read((char*)&v.x,sizeof(v.x));
		in.read((char*)&v.y,sizeof(v.y));
		in.read((char*)&v.z,sizeof(v.z));
		in.read((char*)&v.label,sizeof(v.label));
		//if (v.label>0) cout << v.label << ' ';
		addVert(v);
	}
	return 0;
}

int TriangleMesh::writeBinaryFile(ostream& out)
{
	int numTris=tris.size(),numVerts=verts.size(),i;
	
	out.write((char*)&numTris,sizeof(int));
	out.write((char*)&numVerts,sizeof(int));
	for (i=0; i<numTris; i++)
	{
		out.write((char*)&tris[i].a,sizeof(tris[i].a));
		out.write((char*)&tris[i].b,sizeof(tris[i].b));
		out.write((char*)&tris[i].c,sizeof(tris[i].c));
		out.write((char*)&tris[i].label,sizeof(tris[i].label));
	}
	for (i=0; i<numVerts; i++)
	{
		out.write((char*)&verts[i].x,sizeof(verts[i].x));
		out.write((char*)&verts[i].y,sizeof(verts[i].y));
		out.write((char*)&verts[i].z,sizeof(verts[i].z));
		out.write((char*)&verts[i].label,sizeof(verts[i].label));
	}
	return 0;
}

int TriangleMesh::readASCIIFile(istream& in)
{
	int numTris,numVerts,i;
	int a,b,c;
	float x,y,z;
	char junk[2]; junk[1]='\0';
	
	in >> numTris >> numVerts;
	for (i=0; i<numTris; i++)
	{
		in >> a >> b >> c;
		addTri(Triangle(a,b,c));
	}
	for (i=0; i<numVerts; i++)
	{
		in >> x >> y >> z;
		addVert(Vertex(x,y,z));
	}
	junk[0]=(char)in.peek();
	while ((!in.eof()) && (!strstr(" \t\r\n",junk)))
	{
		in.get(); junk[0]=(char)in.peek();
	}
	if (!in.eof()) for (i=0; i<numTris; i++) in >> tris[i].label;
	junk[0]=(char)in.peek();
	while ((!in.eof()) && (!strstr(" \t\r\n",junk)))
	{
		in.get(); junk[0]=(char)in.peek();
	}
	if (!in.eof()) for (i=0; i<numVerts; i++) in >> verts[i].label;
	return 0;
}

int TriangleMesh::writeASCIIFile(ostream& out)
{
	int numTris=tris.size(),numVerts=verts.size(),i;

	out << numTris << ' ' << numVerts << '\n';
	for (i=0; i<numTris; i++) out << tris[i].a << ' ' << tris[i].b << ' ' << tris[i].c << '\n';
	for (i=0; i<numVerts; i++) out << verts[i].x << ' ' << verts[i].y << ' ' << verts[i].z << '\n';
	for (i=0; i<numTris; i++) out << tris[i].label << '\n';
	for (i=0; i<numVerts; i++) out << verts[i].label << '\n';
	return 0;
}

int TriangleMesh::readFile(char* filename)
{
    int result=0;
	ifstream fin(filename);
	if (!fin)
	{
        cerr << "Cannot open " << filename << " for reading.\n";
        return 1;
    }
    cout << "Reading " << filename << "..."; cout.flush();
	
    char* suffix=strrchr(filename,'.');
    if (!strcmp(suffix,".t"))
    {
        //Read triangle ASCII file
		result=readASCIIFile(fin);
    }
    else if (!strcmp(suffix,".tb"))
    {
        //Read triangle binary file
		result=readBinaryFile(fin);
    }
    else
    {
		//unsupported format
		result=2;
    }
	
    fin.close();
    if (result) cout << "error!!\n";
	else cout << "done.\n";
	if (result==2) cerr << "File " << filename << " of unsupported format.\n";
    return result;
}

int TriangleMesh::writeFile(char* filename)
{
    ofstream out(filename);
    if (!out)
    {
        cerr << "Cannot open " << filename << " for writing.\n";
        return 1;
    }
	cout << "Writing to " << filename << "..."; cout.flush();
	
	int result=0;
    char* suffix=strrchr(filename,'.');
    if (!strcmp(suffix,".t"))
    {
        //Write triangle ASCII file
		result=writeASCIIFile(out);
    }
    else if (!strcmp(suffix,".tb"))
    {
        //Write triangle binary file
		result=writeBinaryFile(out);
    }
    else
    {
		//unsupported format
		result=2;
    }
    out.close();
	if (result) cout << "error!!\n";
	else cout << "done.\n";
	if (result==2) cerr << "File " << filename << " of unsupported format.\n";
    return result;	
}

