/*
 *  MarchableVolume.cpp
 *  
 *
 *  Created by James Vanderhyde on Wed May 5 2004.
 *
 */

#include <string.h>
#include <iostream.h>
#include <fstream.h>

#include "MarchableVolume.h"

MarchableVolume::MarchableVolume()
{
    size[0]=0;
    size[1]=0;
    size[2]=0;
	sliceSize=size[1]*size[0];
	numVoxels=size[2]*sliceSize;
	slice1=NULL;
	slice2=NULL;
	slice3=NULL;
	z1=z2=z3=-1;
	modified1=modified2=modified3=0;
}

MarchableVolume::~MarchableVolume()
{
	if (slice1) delete[] slice1;
	if (slice2) delete[] slice2;
	if (slice3) delete[] slice3;
}

int* MarchableVolume::getSize()
{
    return size;
}

void MarchableVolume::swapSlice1(int z)
{
	if (modified1) storeSlice(z1,slice1);
	z1=z;
	modified1=0;
	copySlice(z1,slice1);
}

void MarchableVolume::swapSlice2(int z)
{
	if (modified2) storeSlice(z2,slice2);
	z2=z;
	modified2=0;
	copySlice(z2,slice2);
}

void MarchableVolume::swapSlice3(int z)
{
	if (modified3) storeSlice(z3,slice3);
	z3=z;
	modified3=0;
	copySlice(z3,slice3);
}

float MarchableVolume::d(int x,int y,int z)
{
  //  if (z>66)  cout << '(' << x << ',' << y << ',' << z << ')'; cout.flush();
    if ((x<0) || (y<0) || (z<0) || (x>=size[0]) || (y>=size[1]) || (z>=size[2])) return BIGNUM;
	return d3(x,y,z);
}

float MarchableVolume::d2(int x,int y,int z)
{
	if (z==z1) return slice1[size[0]*y+x];
	if (z==z2) return slice2[size[0]*y+x];
	//cout << "z1=" << z1 << " and z2=" << z2 << " but z=" << z << '\n';
	if (z==0)
	{
		if (!slice1) slice1=new float[sliceSize];
		if (!slice2) slice2=new float[sliceSize];
		z1=0;
		z2=1;
		copySlice(z1,slice1);
		copySlice(z2,slice2);
		return slice1[size[0]*y+x];
	}
	if (z1<z2)
	{
		z1=z;
		copySlice(z1,slice1);
		return slice1[size[0]*y+x];
	}
	else
	{
		z2=z;
		copySlice(z2,slice2);
		return slice2[size[0]*y+x];
	}
}

float MarchableVolume::d3(int x,int y,int z)
{
	if (z==z1) return slice1[size[0]*y+x];
	if (z==z2) return slice2[size[0]*y+x];
	if (z==z3) return slice3[size[0]*y+x];
	//cout << "z1=" << z1 << " and z2=" << z2 << " and z3=" << z3 << " but z=" << z << '\n';
	if (z==0)
	{
		if (!slice1) slice1=new float[sliceSize];
		if (!slice2) slice2=new float[sliceSize];
		if (!slice3) slice3=new float[sliceSize];
		if (0<size[2]) swapSlice1(0);
		if (1<size[2]) swapSlice2(1);
		if (2<size[2]) swapSlice3(2);
		return slice1[size[0]*y+x];
	}
	if ((z1<z2) && (z1<z3))
	{
		swapSlice1(z);
		return slice1[size[0]*y+x];
	}
	else if (z2<z3)
	{
		swapSlice2(z);
		return slice2[size[0]*y+x];
	}
	else
	{
		swapSlice3(z);
		return slice3[size[0]*y+x];
	}
}

void MarchableVolume::d(int x,int y,int z,float val)
{
	if ((x<0) || (y<0) || (z<0) || (x>=size[0]) || (y>=size[1]) || (z>=size[2])) return;
	d3(x,y,z,val);
}

void MarchableVolume::d3(int x,int y,int z,float val)
{
	float oldVal=d(x,y,z); //swaps in slice if necessary
	if (z==z1)
	{
		slice1[size[0]*y+x]=val;
		modified1=1;
		return;
	}
	if (z==z2)
	{
		slice2[size[0]*y+x]=val;
		modified2=1;
		return;
	}
	if (z==z3)
	{
		slice3[size[0]*y+x]=val;
		modified3=1;
		return;
	}
}

MarchableVolume* MarchableVolume::createVolume(char* filename)
{
	MarchableVolume* v;
    int result=0;
    char* suffix=strrchr(filename,'.');
	
    if (!strcmp(suffix,".v"))
    {
        //Voxel file
		v=new FloatVolume();
    }
    else if (!strcmp(suffix,".vri"))
    {
        //VRIP (Michaelangelo format) file
		v=new VRIPVolume();
    }
    else if (!strcmp(suffix,".v2"))
    {
        //2 bits per voxel file
		v=new Bit2Volume();
    }
    else if (!strcmp(suffix,".dfo"))
    {
        //depth-first octree file
		v=new OctreeVolume();
    }
    else
    {
		//unsupported format
		cerr << "File " << filename << " of unsupported format.\n";
		v=NULL;
		result=2;
    }
	
	if (v)
	{
		cout << "Reading file " << filename << "..."; cout.flush();
		result=v->readFile(filename);
		if (result) cout << "error!!\n";
		else cout << "done.\n";
		v->sliceSize=v->size[1]*v->size[0];
		v->slice1=new float[v->sliceSize];
		v->slice2=new float[v->sliceSize];
		v->slice3=new float[v->sliceSize];
		v->d(0,0,0);
	}
	if (result) cerr << "Error reading file " << filename << '\n';
	return v;
}


FloatVolume::FloatVolume() : MarchableVolume::MarchableVolume()
{
	data=NULL;
}

FloatVolume::~FloatVolume()
{
	if (data) delete[] data;
}

void FloatVolume::copySlice(int z,float* slice)
{
	int offset=z*sliceSize;
	for (int index=0; index<sliceSize; index++)
		slice[index]=data[offset+index];
}

void FloatVolume::storeSlice(int z,float* slice)
{
	int offset=z*sliceSize;
	for (int index=0; index<sliceSize; index++)
		data[offset+index]=slice[index];
}

int FloatVolume::readFile(char* filename)
{
    ifstream fin(filename);
    if (!fin) return 1;
    fin.read((char*)size,sizeof(size));
	sliceSize=size[1]*size[0];
	numVoxels=size[2]*sliceSize;
	data=new float[numVoxels];
	
	float* slice=new float[sliceSize];
	int x,y,z,voxelIndex=0,index;
	for (z=0; z<size[2]; z++)
	{
		index=0;
		fin.read((char*)slice,sliceSize*sizeof(float));
		for (y=0; y<size[1]; y++) for (x=0; x<size[0]; x++)
		{				
			data[voxelIndex++]=slice[index++];
		}
	}
	fin.close();
		
	delete[] slice;
	return 0;
}

int FloatVolume::writeFile(char* filename)
{
	ofstream fout(filename);
	if (!fout) return 1;
	d(0,0,0);
	fout.write((char*)size,sizeof(size));
	fout.write((char*)data,numVoxels*sizeof(float));
	fout.close();
	return 0;
}


VRIPVolume::VRIPVolume() : MarchableVolume::MarchableVolume()
{
    ogSrc = new OccGridRLE(1,1,1, CHUNK_SIZE);
}

VRIPVolume::~VRIPVolume()
{
	delete ogSrc;
}

void VRIPVolume::copySlice(int z,float* slice)
{
    OccElement* occSlice;
	int x,y,index=0;
    int junk;
	
	occSlice=ogSrc->getSlice("z",z+1,&junk,&junk);
	for (y=0; y<size[1]; y++) for (x=0; x<size[0]; x++)
	{				
		slice[index]=32767.5-(float)occSlice[index].value;
		//slice[index]=(float)occSlice[index].value-32767.5;
		index++;
	}
}

void VRIPVolume::storeSlice(int z,float* slice)
{
    OccElement* occLine;
	int x,y,offset;
	
	for (y=0; y<size[1]; y++)
	{
		offset=y*size[0];
		for (x=0; x<size[0]; x++)
		{
			occLine[x].value=(unsigned short)(32767.5-slice[offset+x]);
		}
		ogSrc->putScanline(occLine,y,z+1);
	}
}

int VRIPVolume::readFile(char* filename)
{
    if (!ogSrc->read(filename)) return 1;
	
    size[0]=ogSrc->xdim;
    size[1]=ogSrc->ydim;
    size[2]=ogSrc->zdim;
	sliceSize=size[1]*size[0];
	numVoxels=size[2]*sliceSize;
	
	return 0;
}

int VRIPVolume::writeFile(char* filename)
{
	d(0,0,0);
	if (!ogSrc->write(filename)) return 1;
	return 0;
}


Bit2Volume::Bit2Volume() : MarchableVolume::MarchableVolume()
{
}

Bit2Volume::~Bit2Volume()
{
	positive.freespace();
	known.freespace();
}

void Bit2Volume::copySlice(int z,float* slice)
{
	int offset=z*sliceSize;
	for (int index=0; index<sliceSize; index++)
	  //slice[index]=((known.getbit(offset+index))?((positive.getbit(offset+index))?1.0:-1.0):0.0);
	  slice[index]=((positive.getbit(offset+index))?1.0:-1.0);
}

void Bit2Volume::storeSlice(int z,float* slice)
{
	int offset=z*sliceSize;
	for (int index=0; index<sliceSize; index++)
	{
		if (slice[index]>0.0)
		{
			known.setbit(offset+index);
			positive.setbit(offset+index);
		}
		else if (slice[index]<0.0)
		{
			known.setbit(offset+index);
			positive.resetbit(offset+index);
		}
		else
		{
			known.resetbit(offset+index);
			positive.setbit(offset+index);
		}
	}
}

int Bit2Volume::readFile(char* filename)
{
    ifstream fin(filename);
    if (!fin) return 1;
    fin.read((char*)size,sizeof(size));
	sliceSize=size[1]*size[0];
	numVoxels=size[2]*sliceSize;
	positive.read(fin,numVoxels);
	known.read(fin,numVoxels);
	fin.close();
	return 0;
}

int Bit2Volume::writeFile(char* filename)
{
	ofstream fout(filename);
	if (!fout) return 1;
	d(0,0,0);
	fout.write((char*)size,sizeof(size));
	positive.write(fout,numVoxels);
	known.write(fout,numVoxels);
	fout.close();
	return 0;
}


OctreeVolume::OctreeVolume() : MarchableVolume::MarchableVolume()
{
	dataroot=new octree();
	treeHeight=0;
}

OctreeVolume::~OctreeVolume()
{
	delete dataroot;
}

void OctreeVolume::copySlice(int z,float* slice)
{
    int x,y,index=0;
    for (y=0; y<size[1]; y++)
        for (x=0; x<size[0]; x++)
            slice[index++]=dataroot->findDeepestAncestor(treeHeight,x,y,z)->getValue();
}

void OctreeVolume::storeSlice(int z,float* slice)
{
	//For simplicity at this point, only store the value if there is a max-resolution voxel already there.
    int x,y,index=0;
    for (y=0; y<size[1]; y++)
        for (x=0; x<size[0]; x++)
		{
			if (dataroot->levelOfDeepestAncestor(treeHeight,x,y,z)==0)
				dataroot->findDeepestAncestor(treeHeight,x,y,z)->setValue(slice[index]);
			else if (dataroot->findDeepestAncestor(treeHeight,x,y,z)->getValue() != slice[index])
				dataroot->addNewVoxel(treeHeight,x,y,z,slice[index]);
			index++;
		}
}

int OctreeVolume::readFile(char* filename)
{
	size[0]=size[1]=size[2]=0;
    ifstream fin(filename);
    if (!fin) return 1;
	dataroot->readDF(fin,size,0,0,0);
	fin.close();
	sliceSize=size[1]*size[0];
    treeHeight=ceillogbase2(MAX(MAX(size[0],size[1]),size[2]));
	return 0;
}

int OctreeVolume::writeFile(char* filename)
{
	ofstream fout(filename);
	if (!fout) return 1;
	d(0,0,0);
	dataroot->writeDF(fout);
	fout.close();
	return 0;
}


