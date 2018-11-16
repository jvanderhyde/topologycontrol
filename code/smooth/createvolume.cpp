//createvolume.cpp
//James Vanderhyde, 2 June 2005

#include <iostream.h>
#include <fstream.h>

#include "Vector3D.h"

#define PI 3.1415926535

int saveFile(char* filename,float* volume,int* vsize)
{
    ofstream fout(filename);
    if (!fout) return 1;
    fout.write((char*)vsize,3*sizeof(int));
	fout.write((char*)volume,vsize[2]*vsize[1]*vsize[0]*sizeof(float));
	fout.close;
	return 0;
}

int main(int argc, char* argv[])
{
	int size[3];
	float* data;
	int x,y,z;
	
	size[0]=128;
	size[1]=128;
	size[2]=128;
	data=new float[size[2]*size[1]*size[0]];

	Vector3D c,u,v,p;
	
	//Circle is c+u*cos(t)+v*sin(t), 0<=t<2*PI
	
	for (z=0; z<size[2]; z++)
		for (y=0; y<size[1]; y++)
			for (x=0; x<size[0]; x++)
			{
				
			}
				
	int result=saveFile(argv[1],volume,size);
	delete[] data;
	return result;
}