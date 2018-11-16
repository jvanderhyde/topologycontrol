//make4D.cpp
//James Vanderhyde, 14 May 2007.

#include <iostream.h>
#include <fstream.h>
#include <math.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float sphere1(float x,float y,float z,float t)
{
    float cx,cy,cz;
    cx=0.2;
    cy=0.2+0.6*t;
    cz=0.2+0.6*t;
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy)+(z-cz)*(z-cz));
}

float sphere2(float x,float y,float z,float t)
{
    float cx,cy,cz;
    cx=0.9-0.1*t;
    cy=0.8-0.6*t;
    cz=0.2+0.5*t;
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy)+(z-cz)*(z-cz));
}

float torus(float x,float y,float z,float t)
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
}

float disc1(float x,float y,float z,float t)
{
    float cx,cy;
    float r,theta;
    r=0.35*(1.0-t);
    theta=2*M_PI*t/2.0;
    cx=0.5+r*cos(theta);
    cy=0.5+r*sin(theta);
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy));
}

float disc2(float x,float y,float z,float t)
{
    float cx,cy;
    float r,theta;
    r=0.35*(1.0-t);
    theta=2*M_PI*(0.5+t/2.0);
    cx=0.5+r*cos(theta);
    cy=0.5+r*sin(theta);
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy));
}


int main(int argc, char* argv[])
{
    if (argc<2)
    {
	cout << "Usage: " << argv[0] << " <output .v4d file>\n";
	return 1;
    }
    
    int vsize[4];
    vsize[0]=16;
    vsize[1]=16;
    vsize[2]=16;
    vsize[3]=16;
    float spaceScaleFactor=16.0;
    int numVoxels=vsize[0]*vsize[1]*vsize[2]*vsize[3];
    
    float* vdata=new float[numVoxels];
    float x,y,z,t;
    int index;
    for (index=0; index<numVoxels; index++)
    {
	int rest=index;
	x=(rest%vsize[0])/spaceScaleFactor;
	rest/=vsize[0];
	y=(rest%vsize[1])/spaceScaleFactor;
	rest/=vsize[1];
	z=(rest%vsize[2])/spaceScaleFactor;
	rest/=vsize[2];
	t=(rest%vsize[3])/(float)(vsize[3]-1);
	rest/=vsize[3];
	assert(rest==0);
	//vdata[index]=fmin(fmin(sphere1(x,y,z,t),sphere2(x,y,z,t)),torus(x,y,z,t))+0.01*index/(float)numVoxels;
	vdata[index]=1000.0*fmin(fmin(sphere3(x,y,z,t),sphere4(x,y,z,t)),sphere5(x,y,z,t));
	//vdata[index]=1000.0*fmin(disc1(x,y,z,t),disc2(x,y,z,t));
    }
    
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
