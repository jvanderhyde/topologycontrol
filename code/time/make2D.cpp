//make2D.cpp
//James Vanderhyde, 1 June 2007.

#include <iostream.h>
#include <fstream.h>
#include <math.h>
#include <assert.h>

float disc1(float x,float y,float z)
{
    float cx,cy;
    cx=28/63.0;
    cy=37/63.0;
    return sqrt((x-cx)*(x-cx)+(y-cy)*(y-cy));
}

void smooth(int* vsize,float* vdata)
{
    cout << "smoothing";
    int numVoxels=vsize[0]*vsize[1]*vsize[2];
    float* tempdata=new float[numVoxels];
    for (int i=0; i<250; i++)
    {
	cout << "."; cout.flush();
	int x,y,index,ni1,ni2,ni3,ni4;
	for (index=0; index<numVoxels; index++) tempdata[index]=vdata[index];
	for (index=0; index<numVoxels; index++)
	{
	    ni1=index-1;
	    ni2=index-vsize[0];
	    ni3=index+1;
	    ni4=index+vsize[0];
	    x=index%vsize[0];
	    y=index/vsize[0];
	    if ((i<225) && (((x==15) && (y==15)) || ((x==50) && (y==48)) || ((x==48) && (y==16)) || ((x==32) && (y==32))))
	    {
		if ((x==15) && (y==15)) tempdata[index]=1.2;
		else if ((x==50) && (y==48)) tempdata[index]=1.0;
		else if ((x==48) && (y==16)) tempdata[index]=-0.1;
		else if ((x==28) && (y==37)) tempdata[index]=0.0;
	    }
	    else
	    {
		if ((x==0) && (y==0)) tempdata[index]=(2*vdata[index]+vdata[ni3]+vdata[ni4])/4.0;
		else if ((x==vsize[0]-1) && (y==0)) tempdata[index]=(2*vdata[index]+vdata[ni1]+vdata[ni4])/4.0;
		else if ((x==vsize[0]-1) && (y==vsize[1]-1)) tempdata[index]=(2*vdata[index]+vdata[ni1]+vdata[ni2])/4.0;
		else if ((x==0) && (y==vsize[1]-1)) tempdata[index]=(2*vdata[index]+vdata[ni3]+vdata[ni2])/4.0;
		else if (x==0) tempdata[index]=(3*vdata[index]+vdata[ni2]+vdata[ni3]+vdata[ni4])/6.0;
		else if (x==vsize[0]-1) tempdata[index]=(3*vdata[index]+vdata[ni1]+vdata[ni2]+vdata[ni4])/6.0;
		else if (y==0) tempdata[index]=(3*vdata[index]+vdata[ni1]+vdata[ni3]+vdata[ni4])/6.0;
		else if (y==vsize[1]-1) tempdata[index]=(3*vdata[index]+vdata[ni1]+vdata[ni2]+vdata[ni3])/6.0;
		else tempdata[index]=(4*vdata[index]+vdata[ni1]+vdata[ni2]+vdata[ni3]+vdata[ni4])/8.0;
	    }
	}
	for (index=0; index<numVoxels; index++) vdata[index]=tempdata[index];
    }
    cout << "done.\n";
}

int main(int argc, char* argv[])
{
    if (argc<2)
    {
	cout << "Usage: " << argv[0] << " <output .v file>\n";
	return 1;
    }
    
    int vsize[3];
    vsize[0]=64;
    vsize[1]=64;
    vsize[2]=1;
    float spaceScaleFactor=64.0;
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
	z=0;
	rest/=vsize[2];
	assert(rest==0);
	vdata[index]=disc1(x,y,z);
	//cout << "(" << x << "," << y << "," << z << ")" << vdata[index] << "\n";
    }
    smooth(vsize,vdata);
    for (index=0; index<numVoxels; index++) vdata[index]*=1000.0;
    
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
