//subsample.cpp
//James Vanderhyde, 22 May 2007

#include <fstream.h>
#include <iostream.h>

void printSize(int bytes)
{
    int mb,kb;
    mb=bytes>>20;
    kb=bytes>>10;
    if (mb) cout << mb << " MB\n";
    else if (kb) cout << kb << " kB\n";
    else cout << bytes << " B\n";
}

int main(int argc,char* argv[])
{
    if (argc<=2)
    {
        cerr << "Usage: " << argv[0] << " <input .v4db file> <output .v4bd file>\n";
        return 1;
    }
    
    int vsize1[4],vsize2[4];
    int numVoxels1,numVoxels2;
    int sliceSize1,sliceSize2;
    unsigned char* vdata1,* vdata2;
    
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
    printSize(numVoxels1);
    vdata1=new unsigned char[2*sliceSize1];
    
    vsize2[3]=vsize1[3]/2;
    vsize2[2]=vsize1[2]/2;
    vsize2[1]=vsize1[1]/2;
    vsize2[0]=vsize1[0]/2;
    fout.write((char*)vsize2,sizeof(vsize2));
    sliceSize2=vsize2[2]*vsize2[1]*vsize2[0];
    numVoxels2=vsize2[3]*sliceSize2;
    vdata2=new unsigned char[sliceSize2];
    
    int x,y,z,t,index1,index2,sum,ox,oy,oz,ot;
    
    cout << "Downsampling";
    for (t=0; t<vsize1[3]; t+=2)
    {
	cout << "."; cout.flush();
	fin.read((char*)vdata1,2*sliceSize1*sizeof(unsigned char));
	index2=0;
	for (z=0; z<vsize1[2]; z+=2)
	    for (y=0; y<vsize1[1]; y+=2)
		for (x=0; x<vsize1[0]; x+=2)
		{
		    sum=0;
		    for (ot=0; ot<2; ot++)
			for (oz=0; oz<2; oz++)
			    for (oy=0; oy<2; oy++)
				for (ox=0; ox<2; ox++)
				{
				    index1=(((ot)*vsize1[2]+(z+oz))*vsize1[1]+(y+oy))*vsize1[0]+(x+ox);
				    sum+=vdata1[index1];
				}
		    vdata2[index2]=sum/16;
		    index2++;
		}
	fout.write((char*)vdata2,sliceSize2*sizeof(unsigned char));
    }
    fin.close();
    fout.close();
    cout << "done.\n";
    
    delete[] vdata1;
    delete[] vdata2;

    return 0;
}
