/*
 *  MarchableVolume.cpp
 *  
 *
 *  Created by James Vanderhyde on Thu May 13 2004.
 *
 */

#include <iostream.h>
#include <fstream.h>

#include "MarchableVolume.h"

int max(int a,int b,int c)
{
	if ((a>b) && (a>c)) return a;
	if (b>c) return b;
	return c;
}

//t varies from 0 to 1, output varies between 0 and 1.
// If t<0 or t>1 then t=t-floor(r).
float periodic(float t)
{
	float theta=t-floor(t);
	if (theta>0.5) return 1-theta;
	return theta;
}

int main(int argc,char* argv[])
{
    if (argc<=2)
    {
        cerr << "Usage: " << argv[0] << " <input .v file> <slice> [...]\n";
        return 1;
    }
	
    int type=2;

    MarchableVolume* v=MarchableVolume::createVolume(argv[1]);
    if (!v) return 1;
    int* vsize=v->getSize();
	int maxdim=max(vsize[0],vsize[1],vsize[2]);
	
    for (int i=2; i<argc; i++)
    {
        int x,y,z=atoi(argv[i]);
        
        char name[128];
        sprintf(name,"slice%03d.ppm",z);
        ofstream ofs(name);
        if (!ofs)
        {
            cerr << "Cannot open " << name << " for writing.\n";
            return 1;
        }
        
        ofs << "P6" << endl << vsize[0] << " " << vsize[1] << endl;
        ofs << "255" << endl;
		
		for (y=0; y<vsize[1]; y++) for (x=0; x<vsize[0]; x++)
        {
            int red=0,green=0,blue=0;
            float val=v->d(x,y,z);
			
            if (type==2)		//distance field
            {
                float freq=5.0;
                //float normalized=fabs(val)/1.0;			//for .v2 files
                //float normalized=fabs(val)/32767.5;		//for .vri files
                float normalized=fabs(val)/(float)maxdim;   //for .v files
                if (val<0)	//red
                {
                    red  =255;
                    green=(int)floor(255*periodic(normalized*freq));
                    blue =(int)floor(255*periodic(normalized*freq));
                }
                else		//gray
                {
                    red  =(int)floor(170*periodic(normalized*freq));
                    green=(int)floor(170*periodic(normalized*freq));
                    blue =(int)floor(170*periodic(normalized*freq));
                }
            }
			
            ofs.put(red);
            ofs.put(green);
            ofs.put(blue);
        }
        ofs.close();
        cout << "Slice written to " << name << '\n';
    }
	
    return 0;
}
