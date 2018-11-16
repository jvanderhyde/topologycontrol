/*
 *  TriangleMesh.cpp
 *  
 *
 *  Created by James Vanderhyde on Thu May 6 2004.
 *
 */

#include "TriangleMesh.h"

Triangle::Triangle() : a(0), b(0), c(0), label(0)
{
}

Triangle::Triangle(int aa,int bb,int cc) : a(aa), b(bb), c(cc), label(0)
{
}

Point3D::Point3D() : x(0.0), y(0.0), z(0.0), label(0)
{
}

Point3D::Point3D(float xx,float yy,float zz) : x(xx), y(yy), z(zz), label(0)
{
}

Point3D TriangleMesh::getVert(int i)
{
	return verts[i];
}

Triangle TriangleMesh::getTri(int i)
{
	return tris[i];
}

void TriangleMesh::addVert(Point3D pt)
{
	verts.push_back(pt);
}

void TriangleMesh::addTri(Triangle tri)
{
	tris.push_back(tri);
}

void TriangleMesh::setVertLabel(int i,char label)
{
	verts[i].label=label;
}

void TriangleMesh::setTriLabel(int i,char label)
{
	tris[i].label=label;
}

int TriangleMesh::getNumVerts()
{
	return verts.size();
}

int TriangleMesh::getNumTris()
{
	return tris.size();
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

int TriangleMesh::readBinaryFile(istream& in)
{
	int numTris,numVerts,i;
	Triangle t;
	Point3D v;
	
	in.read((char*)&numTris,sizeof(int));
	in.read((char*)&numVerts,sizeof(int));
	for (i=0; i<numTris; i++)
	{
		in.read((char*)&t,sizeof(Triangle));
		addTri(t);
	}
	for (i=0; i<numVerts; i++)
	{
		in.read((char*)&v,sizeof(Point3D));
		addVert(v);
	}
	return 0;
}

int TriangleMesh::writeBinaryFile(ostream& out)
{
	int numTris=tris.size(),numVerts=verts.size(),i;
	
	out.write((char*)&numTris,sizeof(int));
	out.write((char*)&numVerts,sizeof(int));
	for (i=0; i<numTris; i++) out.write((char*)&tris[i],sizeof(Triangle));
	for (i=0; i<numVerts; i++) out.write((char*)&verts[i],sizeof(Point3D));
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
		addVert(Point3D(x,y,z));
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

