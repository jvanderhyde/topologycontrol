//project4D3D.cpp
//James Vanderhyde, 28 May 2007

#include <fstream.h>
#include <iostream.h>

int main(int argc,char* argv[])
{
    if (argc<=2)
    {
        cerr << "Usage: " << argv[0] << " <input .v4d file> <output .v file>\n";
        return 1;
    }
    
    int vsize1[4],vsize2[3];
    int numVoxels1,numVoxels2;
    int sliceSize1;
    float* vdata1,* vdata2;
    
    ifstream fin(argv[1]);
    if (!fin) 
    {
	cerr << "Can't open file " << argv[1] << " for reading!\n";
	return 1;
    }
    ofstream fout(argv[2]);
    if (!fout) 
    {
	cerr << "Can't open file " << argv[2] << " for writing!\n";
	return 1;
    }
    
    fin.read((char*)vsize1,sizeof(vsize1));
    cout << vsize1[0] << 'x' << vsize1[1] << 'x' << vsize1[2] << 'x' << vsize1[3] << '\n';
    sliceSize1=vsize1[2]*vsize1[1]*vsize1[0];
    numVoxels1=vsize1[3]*sliceSize1;
    vdata1=new float[numVoxels1];
    fin.read((char*)vdata1,numVoxels1*sizeof(float));
    fin.close();
    
    vsize2[2]=vsize1[3];
    vsize2[1]=vsize1[1];
    vsize2[0]=vsize1[0];
    fout.write((char*)vsize2,sizeof(vsize2));
    numVoxels2=vsize2[2]*vsize2[1]*vsize2[0];
    vdata2=new float[numVoxels2];
    
    int x,y,z,t,index1,index2;
    index1=index2=0;
    for (t=0; t<vsize1[3]; t++)
	for (z=0; z<vsize1[2]; z++)
	    for (y=0; y<vsize1[1]; y++)
		for (x=0; x<vsize1[0]; x++)
		{
		    if (z==0)
		    {
			vdata2[index2]=vdata1[index1];
			index2++;
		    }
		    index1++;
		}
		    
    fout.write((char*)vdata2,numVoxels2*sizeof(float));
    fout.close();
    
    delete[] vdata1;
    delete[] vdata2;

    return 0;
}
