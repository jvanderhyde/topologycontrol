//fixtopo4D.cpp
//James Vanderhyde, 16 May 2007

#include <iostream.h>
#include <fstream.h>

int size[4];
float* data=NULL;
int* order=NULL;
int numVoxels=0;

int readVFile(char* filename)
{
    ifstream fin(filename);
    if (!fin) return 1;
    fin.read((char*)size,sizeof(size));
    numVoxels=size[3]*size[2]*size[1]*size[0];
    data=new float[numVoxels];
    fin.read((char*)data,numVoxels*sizeof(float));
    fin.close();
    return 0;
}

int readVOFile(char* filename)
{
    int osize[4];
    ifstream fin(filename);
    if (!fin) return 1;
    fin.read((char*)osize,sizeof(osize));
    if ((osize[0]!=size[0]) || (osize[1]!=size[1]) || (osize[2]!=size[2]) || (osize[3]!=size[3]))
    {
	cerr << "Error: data set dimensions do not match.\n";
	fin.close();
	return 2;
    }
    order=new int[numVoxels];
    fin.read((char*)order,numVoxels*sizeof(int));
    fin.close();
    return 0;
}

int writeVFile(char* filename)
{
    ofstream fout(filename);
    if (!fout) return 1;
    fout.write((char*)size,sizeof(size));
    fout.write((char*)data,numVoxels*sizeof(float));
    fout.close();
    return 0;
}

void fixVolume()
{
    int* process=new int[numVoxels];
    int i;
    for (i=0; i<numVoxels; i++) process[order[i]]=i;
    float curVal=data[process[0]];
    for (i=1; i<numVoxels; i++)
    {
	if (data[process[i]]<curVal) data[process[i]]=curVal;
	else curVal=data[process[i]];
    }
    delete[] process;
}

void fixVolumeReverse()
{
    int* process=new int[numVoxels];
    int i;
    for (i=0; i<numVoxels; i++) process[numVoxels-1-order[i]]=i;
    float curVal=data[process[0]];
    for (i=1; i<numVoxels; i++)
    {
	if (data[process[i]]>curVal) data[process[i]]=curVal;
	else curVal=data[process[i]];
    }
    delete[] process;
}

int quit(int exitCondition)
{
    if (data) delete[] data;
    if (order) delete[] order;
    return exitCondition;
}

int main(int argc, char* argv[])
{
    if (argc<4)
    {
	cerr << "Usage: " << argv[0] << " <input .v file> <output .v file> <input .vo order file>\n";
	return 1;
    }
    
    int result;
    result=readVFile(argv[1]);
    if (result)
    {
	cerr << "Error reading file " << argv[1] << "\n";
	return quit(result);
    }
    result=readVOFile(argv[3]);
    if (result)
    {
	cerr << "Error reading file " << argv[3] << "\n";
	return quit(result);
    }
    
    fixVolume();
    
    result=writeVFile(argv[2]);
    if (result)
    {
	cerr << "Error writing to file " << argv[2] << '\n';
	return quit(result);
    }

    return quit(0);
}
