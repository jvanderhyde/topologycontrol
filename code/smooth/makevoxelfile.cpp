/*
 *  makevoxelfile.cpp
 *  
 *
 *  Created by James Vanderhyde on Thu Jul 03 2003.
 *
 */

#include <fstream.h>
#include <iostream.h>
#include <math.h>

float sqr(float x)
{
    return x*x;
}

int main(int argc,char* argv[])
{
    if (argc<=1)
    {
        cerr << "Usage: makevoxelfile <output .v file> [<input .v file>]\n";
        return 1;
    }
    
    ofstream fout(argv[1]);
    if (!fout)
    {
        cerr << "Cannot open " << argv[1] << " for writing.\n";
        return 1;
    }

    if (0) //double resolution
    {
        int x,y,z;
        float* slice,* bigslice;
        int index,bigindex;
        ifstream fin(argv[2]);
        if (!fin)
        {
            cerr << "Cannot open " << argv[2] << " for reading.\n";
            return 1;
        }
        int size[3],bigsize[3];
        fin.read((char*)size,3*sizeof(int));
        bigsize[0]=2*size[0]; bigsize[1]=2*size[1]; bigsize[2]=2*size[2];
        fout.write((char*)bigsize,3*sizeof(int));
        int slicesize=size[0]*size[1],bigslicesize=bigsize[0]*bigsize[1];
        slice=new float[slicesize];
        bigslice=new float[bigslicesize];
        for (z=0; z<size[2]; z++)
        {
            fin.read((char*)slice,slicesize*sizeof(float));
            for (y=0; y<bigsize[1]; y++)
            {
                for (x=0; x<bigsize[0]; x++)
                    bigslice[x+bigsize[0]*y]=slice[x/2+size[0]*(y/2)];
            }
            fout.write((char*)bigslice,bigslicesize*sizeof(float));
            fout.write((char*)bigslice,bigslicesize*sizeof(float));
            cout << '.';
        }
        fout.close();
        cout << '\n';
        delete[] bigslice;
        delete[] slice;
        return 0;
    }
    if ((0) && (argc>2)) //half resolution (subsample)
    {
        int x,y,z;
        float* slice1,* slice2,* smallslice;
        int index,smallindex;
        ifstream fin(argv[2]);
        if (!fin)
        {
            cerr << "Cannot open " << argv[2] << " for reading.\n";
            return 1;
        }
        int size[3],smallsize[3];
        fin.read((char*)size,3*sizeof(int));
        smallsize[0]=size[0]/2; smallsize[1]=size[1]/2; smallsize[2]=size[2]/2;
        fout.write((char*)smallsize,3*sizeof(int));
        int slicesize=size[0]*size[1],smallslicesize=smallsize[0]*smallsize[1];
        slice1=new float[slicesize];
        slice2=new float[slicesize];
        smallslice=new float[smallslicesize];
        for (z=0; z<smallsize[2]; z++)
        {
            fin.read((char*)slice1,slicesize*sizeof(float));
            fin.read((char*)slice2,slicesize*sizeof(float));
            for (y=0; y<smallsize[1]; y++)
            {
                for (x=0; x<smallsize[0]; x++)
                {
                    smallslice[x+smallsize[0]*y]=
                    (slice1[x*2+size[0]*(y*2)]+slice2[x*2+size[0]*(y*2)]+
                     slice1[x*2+1+size[0]*(y*2)]+slice2[x*2+1+size[0]*(y*2)]+
                     slice1[x*2+size[0]*(y*2+1)]+slice2[x*2+size[0]*(y*2+1)]+
                     slice1[x*2+1+size[0]*(y*2+1)]+slice2[x*2+1+size[0]*(y*2+1)])/8.0;
                }
            }
            fout.write((char*)smallslice,smallslicesize*sizeof(float));
            cout << '.';
        }
        fout.close();
        cout << '\n';
        delete[] smallslice;
        delete[] slice1;
        delete[] slice2;
        return 0;
    }
    if ((1) && (argc>2)) //half resolution (crop) for .vb file
    {
        int x,y,z;
        char* slice1,* slice2,* smallslice;
        int index,smallindex;
        ifstream fin(argv[2]);
        if (!fin)
        {
            cerr << "Cannot open " << argv[2] << " for reading.\n";
            return 1;
        }
        int size[3],smallsize[3];
        fin.read((char*)size,3*sizeof(int));
        smallsize[0]=size[0]/2; smallsize[1]=size[1]/2; smallsize[2]=size[2]/2;
        fout.write((char*)smallsize,3*sizeof(int));
        int slicesize=size[0]*size[1],smallslicesize=smallsize[0]*smallsize[1];
        slice1=new char[slicesize];
        slice2=new char[slicesize];
        smallslice=new char[smallslicesize];
        for (z=0; z<smallsize[2]; z++)
        {
            fin.read((char*)slice1,slicesize*sizeof(char));
            for (y=0; y<smallsize[1]; y++)
            {
                for (x=0; x<smallsize[0]; x++)
                {
                    smallslice[x+smallsize[0]*y]=slice1[x+size[0]*y];
                }
            }
            fout.write((char*)smallslice,smallslicesize*sizeof(char));
            cout << '.';
        }
        fout.close();
        cout << '\n';
        delete[] smallslice;
        delete[] slice1;
        delete[] slice2;
        return 0;
    }
    if (0) //torus
    {
        int x,y,z;
        float* slice;
        int index;
        int size[3];
        size[0]=64; size[1]=64; size[2]=16;
        fout.write((char*)size,3*sizeof(int));
        int slicesize=size[0]*size[1];
        slice=new float[slicesize];
        for (z=0; z<size[2]; z++)
        {
            for (y=0; y<size[1]; y++)
            {
                for (x=0; x<size[0]; x++)
                {
		    float r1=25,r2=5.9;
		    float x0=x-size[0]/2,y0=y-size[1]/2,z0=z-size[2]/2-1;
		    float dist=sqrt(sqr(sqrt(sqr(x0)+sqr(y0))-r1)+sqr(z0))-r2;
		    slice[x+size[0]*y]=dist;
                }
            }
            fout.write((char*)slice,slicesize*sizeof(float));
            cout << '.';
        }
        fout.close();
        cout << '\n';
        delete[] slice;
        return 0;
    }
    if (0) //hemisphere with floaters
    {
        int x,y,z;
        float* slice;
        int index;
        int size[3];
        size[0]=32; size[1]=32; size[2]=32;
        fout.write((char*)size,3*sizeof(int));
        int slicesize=size[0]*size[1];
        slice=new float[slicesize];
        for (z=0; z<size[2]; z++)
        {
            for (y=0; y<size[1]; y++)
            {
                for (x=0; x<size[0]; x++)
                {
		    float r1=13.9,r2=0.9;
		    float x0=x-size[0]/2,y0=y-size[1]/2,z0=z-size[2]/2;
		    float dist1=sqrt(sqr(x0)+sqr(y0)+sqr(z0))-r1;
		    float dist2=sqrt(sqr(x0-9)+sqr(y0-15)+sqr(z0))-r2;
		    float dist3=sqrt(sqr(x0-13)+sqr(y0-13)+sqr(z0))-r2;
		    float dist4=sqrt(sqr(x0-15)+sqr(y0-9)+sqr(z0))-r2;
		    slice[x+size[0]*y]=fmax(fmin(fmin(dist1,dist2),fmin(dist3,dist4)),-x0-y0-0.1);
                }
            }
            fout.write((char*)slice,slicesize*sizeof(float));
            cout << '.';
        }
        fout.close();
        cout << '\n';
        delete[] slice;
        return 0;
    }
    if (0) //eight spheres
    {
        int x,y,z;
        float* slice;
        int index;
        int size[3];
        size[0]=32; size[1]=32; size[2]=32;
        fout.write((char*)size,3*sizeof(int));
        int slicesize=size[0]*size[1];
        slice=new float[slicesize];
        for (z=0; z<size[2]; z++)
        {
            for (y=0; y<size[1]; y++)
            {
                for (x=0; x<size[0]; x++)
                {
		    float r1=8.4,r2=7.2,r3=6.8,r4=7.4,r5=5.4,r6=6.0,r7=5.8,r8=7.8;
		    float x0=x-size[0]/2,y0=y-size[1]/2,z0=z-size[2]/2;
		    float dist1=sqrt(sqr(x0-8)+sqr(y0-8)+sqr(z0-8))-r1;
		    float dist2=sqrt(sqr(x0-8)+sqr(y0-8)+sqr(z0+8))-r2;
		    float dist3=sqrt(sqr(x0-8)+sqr(y0+8)+sqr(z0-8))-r3;
		    float dist4=sqrt(sqr(x0-8)+sqr(y0+8)+sqr(z0+8))-r4;
		    float dist5=sqrt(sqr(x0+8)+sqr(y0-8)+sqr(z0-8))-r5;
		    float dist6=sqrt(sqr(x0+8)+sqr(y0-8)+sqr(z0+8))-r6;
		    float dist7=sqrt(sqr(x0+8)+sqr(y0+8)+sqr(z0-8))-r7;
		    float dist8=sqrt(sqr(x0+8)+sqr(y0+8)+sqr(z0+8))-r8;
		    slice[x+size[0]*y]=fmin(fmin(fmin(dist1,dist2),fmin(dist3,dist4)),
					    fmin(fmin(dist5,dist6),fmin(dist7,dist8)));
                }
            }
            fout.write((char*)slice,slicesize*sizeof(float));
            cout << '.';
        }
        fout.close();
        cout << '\n';
        delete[] slice;
        return 0;
    }
    
    float posslice[]={
        1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0
    };
    float negslice[]={
        -0.8, -0.8, -0.8, -0.8,
        -0.8, -0.8, -0.8, -0.8,
        -0.8, -0.8, -0.8, -0.8,
        -0.8, -0.8, -0.8, -0.8
    };
    float ceeslice[]={
        -0.8, -0.8, -0.8, -0.8,
        -0.8, 0.6, 0.6, -0.8,
        -0.8, 0.6, 0.6, 0.2,
        -0.8, -0.8, -0.8, -0.8
    };
    float ohslice[]={
        -0.8, -0.8, -0.8, -0.8,
        -0.8, 0.6, 0.6, -0.8,
        -0.8, 0.6, 0.6, -0.8,
        -0.8, -0.8, -0.8, -0.8
    };
    float geeslice[]={
        -0.8, -0.8, -0.8, 0.6,
        -0.8, 0.6, -0.5, 0.4,
        -0.8, 0.6, 0.4, -0.5,
        -0.8, -0.8, -0.8, -0.8
    };
    
    float posslice8[]={
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    };
    float island3[]={
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, -.8, -.8, -.8, 1.0, 1.0, 1.0, 1.0,
        1.0, -.8, -.8, -.8, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    };
    float island2[]={
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, -.8, -.8, -.8, 1.0,
        1.0, 1.0, 1.0, 1.0, -.8, -.8, -.8, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    };
    float island1[]={
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, -.8, -.8, -.8, 1.0, 1.0, 1.0, 1.0,
        1.0, -.8, -.8, -.8, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    };
    
	float list[]={
		1, 6, 2, 7, 2, 9, 2, 5
	};
    
    int size[]={4,4,4};
    fout.write((char*)size,3*sizeof(int));
	
	//fout.write((char*)list,size[0]*size[1]*size[2]*sizeof(float));
	
    fout.write((char*)posslice,size[0]*size[1]*sizeof(float));
    fout.write((char*)posslice,size[0]*size[1]*sizeof(float));
    fout.write((char*)ohslice,size[0]*size[1]*sizeof(float));
    fout.write((char*)posslice,size[0]*size[1]*sizeof(float));

    /*fout.write((char*)posslice8,size[0]*size[1]*sizeof(float));
    fout.write((char*)island1,size[0]*size[1]*sizeof(float));
    fout.write((char*)island1,size[0]*size[1]*sizeof(float));
    fout.write((char*)island2,size[0]*size[1]*sizeof(float));
    fout.write((char*)island2,size[0]*size[1]*sizeof(float));
    fout.write((char*)island3,size[0]*size[1]*sizeof(float));
    fout.write((char*)island3,size[0]*size[1]*sizeof(float));
    fout.write((char*)posslice8,size[0]*size[1]*sizeof(float));*/

    fout.close();

    return 0;
}
