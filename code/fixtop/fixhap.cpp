#include <fstream.h>

int main(int argc, char* argv[])
{
	int i;
	int size[3];
	ifstream fin(argv[1]);
	fin.read((char*)size, sizeof(size));
	int sliceSize=size[0]*size[1],numVoxels=sliceSize*size[2];
	float* data=new float[numVoxels];
	fin.read((char*)data, numVoxels*sizeof(float));
	
	int firstSlice=81;
	int numSlices=28;

	for (i=0; i<sliceSize; i++)
		data[i+sliceSize*firstSlice]=data[i+sliceSize*(firstSlice+numSlices-1)]=1e15;
	size[2]=numSlices;

	ofstream fout(argv[2]);
	fout.write((char*)size, sizeof(size));
	fout.write((char*)(data+firstSlice*sliceSize), numSlices*sliceSize*sizeof(float));
	fout.close();
	delete[] data;
}