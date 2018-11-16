//changesigns.cpp
//James Vanderhyde, 6 June 2007

#include <iostream.h>
#include <fstream.h>

int size[4];
unsigned char* data=NULL;
int* order=NULL;
int numVoxels=0;

int readVBFile(char* filename)
{
    ifstream fin(filename);
    if (!fin) return 1;
    fin.read((char*)size,sizeof(size));
    numVoxels=size[3]*size[2]*size[1]*size[0];
    data=new unsigned char[numVoxels];
    fin.read((char*)data,numVoxels*sizeof(unsigned char));
    fin.close();
    return 0;
}

int writeVBFile(char* filename)
{
    ofstream fout(filename);
    if (!fout) return 1;
    fout.write((char*)size,sizeof(size));
    fout.write((char*)data,numVoxels*sizeof(unsigned char));
    fout.close();
    return 0;
}

void fixVolume()
{
    unsigned char min=255,max=0;
    for (int i=0; i<numVoxels; i++)
    {
	if (data[i]<min) min=data[i];
	if (data[i]>max) max=data[i];
    }
    for (int i=0; i<numVoxels; i++)
	data[i]=max+min-data[i];
}

int quit(int exitCondition)
{
    if (data) delete[] data;
    return exitCondition;
}

int main(int argc, char* argv[])
{
    if (argc<3)
    {
	cerr << "Usage: " << argv[0] << " <input .v4db file> <output .v4db file>\n";
	return 1;
    }
    
    int result;
    result=readVBFile(argv[1]);
    if (result)
    {
	cerr << "Error reading file " << argv[1] << "\n";
	return quit(result);
    }

    fixVolume();
    
    result=writeVBFile(argv[2]);
    if (result)
    {
	cerr << "Error writing to file " << argv[2] << '\n';
	return quit(result);
    }

    return quit(0);
}
