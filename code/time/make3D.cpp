//make3D.cpp
//James Vanderhyde, 28 May 2007.

#include <iostream.h>
#include <fstream.h>
#include <math.h>
#include <assert.h>

float disc1(float x,float y,float z)
{
    float cx,cy;
    cx=0.15+0.35*z;
    cy=0.5;
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy));
}

float disc2(float x,float y,float z)
{
    float cx,cy;
    cx=1.0-(0.15+0.35*z);
    cy=0.5;
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy));
}

/*float torus(float x,float y,float z,float t)
{
    float c1x=0.5,c1y=0.5,c1z=0.5;
    float r1=0.3;
    float theta=atan2(z-c1z,x-c1x);
    float d1=sqrt((x-c1x)*(x-c1x)+(y-c1y)*(y-c1y)+(z-c1z)*(z-c1z));
    float c2x=r1*cos(theta)+c1x,c2y=c1y,c2z=r1*sin(theta)+c1z;
    float dotprod=(x-c1x)*(c2x-c1x)+(y-c1y)*(c2y-c1y)+(z-c1z)*(c2z-c1z);
    return sqrt(d1*d1+r1*r1-2*dotprod);
}

float sphere3(float x,float y,float z,float t)
{
    float cx,cy,cz;
    cx=0.25;
    cy=0.75;
    cz=0.25;
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy)+(z-cz)*(z-cz));
}

float sphere4(float x,float y,float z,float t)
{
    float cx,cy,cz;
    cx=0.75;
    cy=0.75;
    cz=0.25;
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy)+(z-cz)*(z-cz));
}

float sphere5(float x,float y,float z,float t)
{
    float cx,cy,cz;
    cx=0.25+t*0.5;
    cy=0.25;
    cz=0.25;
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy)+(z-cz)*(z-cz));
}*/

void makeBox(float* data,int* size,int sx,int sy,int sz,int lx,int ly,int lz)
{
	int x,y,z,index;
	for (z=sz; z<sz+lz; z++) for (y=sy; y<sy+ly; y++) for (x=sx; x<sx+lx; x++)
	{
		index=0;
		index*=size[2];
		index+=z;
		index*=size[1];
		index+=y;
		index*=size[0];
		index+=x;
		data[index]=-1.0;
	}
}

void makeDataSet(float* data,int* size)
{
	makeBox(data,size,0,0,0,30,6,6);
	makeBox(data,size,0,24,0,30,6,6);
	makeBox(data,size,0,0,0,6,30,6);
	makeBox(data,size,24,0,0,6,30,6);
	makeBox(data,size,9,15,3,1,1,1);
	makeBox(data,size,10,8,3,1,1,1);
	makeBox(data,size,20,19,3,1,1,1);
	makeBox(data,size,14,14,3,3,1,1);
	makeBox(data,size,14,16,3,3,1,1);
	makeBox(data,size,14,14,3,1,3,1);
	makeBox(data,size,16,14,3,1,3,1);
	makeBox(data,size,4,4,3,22,22,1);
}

int main(int argc, char* argv[])
{
    if (argc<2)
    {
	cout << "Usage: " << argv[0] << " <output .v file>\n";
	return 1;
    }
    
    int vsize[3];
    //vsize[0]=16;
    //vsize[1]=16;
    //vsize[2]=16;
    vsize[0]=31;
    vsize[1]=31;
    vsize[2]=7;
    float spaceScaleFactor=16.0;
    int numVoxels=vsize[0]*vsize[1]*vsize[2];
    
    float* vdata=new float[numVoxels];
    float x,y,z;
    int index;
    for (index=0; index<numVoxels; index++)
    {
	int rest=index;
	x=(rest%vsize[0])/spaceScaleFactor;
	rest/=vsize[0];
	y=(rest%vsize[1])/spaceScaleFactor;
	rest/=vsize[1];
	z=(rest%vsize[2])/(float)(vsize[2]-1);
	rest/=vsize[2];
	assert(rest==0);
	//vdata[index]=fmin(fmin(sphere1(x,y,z,t),sphere2(x,y,z,t)),torus(x,y,z,t))+0.01*index/(float)numVoxels;
	//vdata[index]=1000.0*fmin(disc1(x,y,z),disc2(x,y,z));
	//cout << "(" << x << "," << y << "," << z << ")" << vdata[index] << "\n";
	vdata[index]=1.0;
    }
	makeDataSet(vdata,vsize);
    
    ofstream fout(argv[1]);
    if (!fout)
    {
	cerr << "Can't open file " << argv[1] << " for writing!\n";
	return 1;
    }
    fout.write((char*)vsize,sizeof(vsize));
    fout.write((char*)vdata,numVoxels*sizeof(float));
    fout.close();
    
    delete[] vdata;
    return 0;
}
