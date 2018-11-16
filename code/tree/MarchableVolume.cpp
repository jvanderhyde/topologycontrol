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
#include <math.h>

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

int MarchableVolume::getKnownFlag(int x,int y,int z)
{
  return 1;
}

int MarchableVolume::getCarvedFlag(int x,int y,int z)
{
  return 0;
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
    /*    else if (!strcmp(suffix,".vri"))
    {
        //VRIP (Michaelangelo format) file
		v=new VRIPVolume();
		}*/
    else if (!strcmp(suffix,".v2"))
    {
        //2 bits per voxel file
		v=new Bit2Volume();
    }
    /*else if (!strcmp(suffix,".dfo"))
    {
        //depth-first octree file
		v=new OctreeVolume();
    }*/
    else if (!strcmp(suffix,".vb"))
    {
        //1 byte per voxel file
		v=new ByteVolume();
    }
    else if (!strcmp(suffix,".raw"))
    {
        //2 bytes per voxel file with separate header file
	v=new RawVolume();
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
		if (!v->slice1) v->slice1=new float[v->sliceSize];
		if (!v->slice2) v->slice2=new float[v->sliceSize];
		if (!v->slice3) v->slice3=new float[v->sliceSize];
		v->d(0,0,0);
	}
	if (result)
	  {
	    cerr << "Error reading file " << filename << '\n';
	    if (v) delete v;
	    return NULL;
	  }
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
    d(0,0,0);
		
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

int FloatVolume::isFloatingType()
{
    return 1;
}

ByteVolume::ByteVolume() : MarchableVolume::MarchableVolume()
{
  data=NULL;
}

ByteVolume::~ByteVolume()
{
  if (data) delete[] data;
}

void ByteVolume::copySlice(int z,float* slice)
{
  int offset=z*sliceSize;
  for (int index=0; index<sliceSize; index++)
    slice[index]=(float)data[offset+index];
}

void ByteVolume::storeSlice(int z,float* slice)
{
  int offset=z*sliceSize;
  for (int index=0; index<sliceSize; index++)
    data[offset+index]=(unsigned char)floor(slice[index]+0.5);
}

int ByteVolume::readFile(char* filename)
{
  ifstream fin(filename);
  if (!fin) return 1;
  fin.read((char*)size,sizeof(size));
  sliceSize=size[1]*size[0];
  numVoxels=size[2]*sliceSize;
  data=new unsigned char[numVoxels];
	
  unsigned char* slice=new unsigned char[sliceSize];
  int x,y,z,voxelIndex=0,index;
  for (z=0; z<size[2]; z++)
    {
      index=0;
      fin.read((char*)slice,sliceSize*sizeof(unsigned char));
      for (y=0; y<size[1]; y++) for (x=0; x<size[0]; x++)
	{				
	  data[voxelIndex++]=slice[index++];
	}
    }
  fin.close();
  d(0,0,0);
  
  delete[] slice;
  return 0;
}

int ByteVolume::writeFile(char* filename)
{
  ofstream fout(filename);
  if (!fout) return 1;
  d(0,0,0);
  fout.write((char*)size,sizeof(size));
  fout.write((char*)data,numVoxels*sizeof(unsigned char));
  fout.close();
  return 0;
}

int ByteVolume::isFloatingType()
{
    return 0;
}


/*VRIPVolume::VRIPVolume() : MarchableVolume::MarchableVolume()
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

int VRIPVolume::getKnownFlag(int x,int y,int z)
{
  OccElement* occSlice;
  int junk;
  occSlice=ogSrc->getSlice("z",z+1,&junk,&junk);
  if (occSlice[x+y*size[0]].totalWeight>0)
    return 1;
  else
    return 0;
}

int VRIPVolume::readFile(char* filename)
{
    if (!ogSrc->read(filename)) return 1;
	
    size[0]=ogSrc->xdim;
    size[1]=ogSrc->ydim;
    size[2]=ogSrc->zdim;
	sliceSize=size[1]*size[0];
	numVoxels=size[2]*sliceSize;
    d(0,0,0);
	
	return 0;
}

int VRIPVolume::writeFile(char* filename)
{
	d(0,0,0);
	if (!ogSrc->write(filename)) return 1;
	return 0;
}

int VRIPVolume::isFloatingType()
{
    return 1;
}*/


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

int Bit2Volume::getKnownFlag(int x,int y,int z)
{
  return known.getbit(x+size[0]*(y+size[1]*z));
}

int Bit2Volume::getCarvedFlag(int x,int y,int z)
{
  return getKnownFlag(x,y,z);
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
    d(0,0,0);
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

int Bit2Volume::isFloatingType()
{
    return 0;
}


/*OctreeVolume::OctreeVolume() : MarchableVolume::MarchableVolume()
{
	dataroot=new octree();
	treeHeight=0;
}

OctreeVolume::OctreeVolume(octree* rootOfOctree) : MarchableVolume::MarchableVolume()
{
	dataroot=rootOfOctree;
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
			//else if (dataroot->findDeepestAncestor(treeHeight,x,y,z)->getValue() != slice[index])
			//	dataroot->addNewVoxel(treeHeight,x,y,z,slice[index]);
			index++;
		}
}

int OctreeVolume::getKnownFlag(int x,int y,int z)
{
  octree* voxel=dataroot->findDeepestAncestor(treeHeight,x,y,z);
  return !voxel->getFlag(OTFL_UNKNOWN);
}

int OctreeVolume::getCarvedFlag(int x,int y,int z)
{
  octree* voxel=dataroot->findDeepestAncestor(treeHeight,x,y,z);
  return voxel->getFlag(OTFL_CARVED);
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
    d(0,0,0);
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

int OctreeVolume::isFloatingType()
{
    return 1;
}*/


RawVolume::RawVolume() : MarchableVolume::MarchableVolume()
{
    data=NULL;
}

RawVolume::~RawVolume()
{
    if (data) delete[] data;
}

void RawVolume::copySlice(int z,float* slice)
{
    int offset=z*sliceSize;
    for (int index=0; index<sliceSize; index++)
	slice[index]=(float)data[offset+index];
}

void RawVolume::storeSlice(int z,float* slice)
{
    int offset=z*sliceSize;
    for (int index=0; index<sliceSize; index++)
	data[offset+index]=(unsigned short)floor(slice[index]+0.5);
}

int RawVolume::readFile(char* filename)
{
    ifstream fin(filename);
    if (!fin) return 1;
    char* headerFilename=new char[strlen(filename)+8];
    headerFilename[0]='\0';
    strcat(headerFilename,filename);
    strcat(headerFilename,".header");
    ifstream headerin(headerFilename);
    if (!headerin)
    {
	cerr << "Can't find file " << headerFilename << "\n";
	cerr << "Header information for raw file " << filename << " required.\n";
	delete[] headerFilename;
	return 2;
    }
    delete[] headerFilename;
    
    headerin >> size[0] >> size[1] >> size[2];
    sliceSize=size[1]*size[0];
    numVoxels=size[2]*sliceSize;
    data=new unsigned short[numVoxels];
    
    unsigned short* slice=new unsigned short[sliceSize];
    int x,y,z,voxelIndex=0,index;
    for (z=0; z<size[2]; z++)
    {
	index=0;
	fin.read((char*)slice,sliceSize*sizeof(unsigned short));
	for (y=0; y<size[1]; y++) for (x=0; x<size[0]; x++)
	{
#ifdef __APPLE__
	    //swap byte order
	    int val=slice[index++];
	    data[voxelIndex++]=val/256+(val%256)*256;
#else
	    data[voxelIndex++]=slice[index++];
#endif
	}
    }
    fin.close();
    d(0,0,0);
    
    delete[] slice;
    return 0;
}

int RawVolume::writeFile(char* filename)
{
    ofstream fout(filename);
    if (!fout) return 1;
    char* headerFilename=new char[strlen(filename)+8];
    headerFilename[0]='\0';
    strcat(headerFilename,filename);
    strcat(headerFilename,".header");
    ofstream headerout(headerFilename);
    delete[] headerFilename;
    if (!headerout) return 2;
    d(0,0,0);
    headerout << size[0] << "\n" << size[1] << "\n" << size[2] << "\n";
    headerout.close();
    fout.write((char*)data,numVoxels*sizeof(unsigned short));
    fout.close();
    return 0;
}

int RawVolume::isFloatingType()
{
    return 0;
}


