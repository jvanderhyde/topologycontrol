//transformvolume.cpp
//James Vanderhyde, 2 June 2005

#include <iostream.h>
#include <fstream.h>

int main(int argc, char* argv[])
{
	if (argc<3)
	{
		cout << "Usage: " << argv[0] << " <input volume file> <output volume file>\n";
		return 1;
	}
	
    ifstream fin(argv[1]);
    if (!fin) return 1;
    ofstream fout(argv[2]);
    if (!fout) return 1;

	int vsize1[3],vsize2[3];
	float* data1,* data2;
	
    fin.read((char*)vsize1,sizeof(vsize1));
	data1=new float[vsize1[2]*vsize1[1]*vsize1[0]];
	fin.read((char*)data1,vsize1[2]*vsize1[1]*vsize1[0]*sizeof(float));

	vsize2[2]=vsize1[2]+20;
	vsize2[1]=vsize1[1]+20;
	vsize2[0]=vsize1[0]+20;
	data2=new float[vsize2[2]*vsize2[1]*vsize2[0]];

	int x,y,z,index1=0,index2=0;
	for (z=0; z<vsize2[2]-vsize1[2]; z++)
	{
		for (y=0; y<vsize2[1]; y++)
		{
			for (x=0; x<vsize2[0]; x++)
			{
				data2[index2++]=1e+20;
			}
		}
	}
	for (z; z<vsize2[2]; z++)
	{
		for (y=0; y<vsize1[1]; y++)
		{
			for (x=0; x<vsize2[0]-vsize1[0]; x++)
			{
				data2[index2++]=1e+20;
			}
			for (x; x<vsize2[0]; x++)
			{
				data2[index2++]=data1[index1++];
			}
		}
		for (y; y<vsize2[1]; y++)
		{
			for (x=0; x<vsize2[0]; x++)
			{
				data2[index2++]=1e+20;
			}
		}
	}

	fout.write((char*)vsize2,sizeof(vsize2));
	fout.write((char*)data2,vsize2[2]*vsize2[1]*vsize2[0]*sizeof(float));
	fout.close();
	
	delete[] data1;
	delete[] data2;
	
	return 0;
}