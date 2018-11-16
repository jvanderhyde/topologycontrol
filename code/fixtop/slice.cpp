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
// If t<0 or t>=1 then t=t-floor(t).
float periodic(float t)
{
  //float theta=t-floor(t);
  //if (theta>0.5) return 1-theta;
  //return theta;
  if (t<0.0) return 0.0;
  if (t>1.0) return 1.0;
  return t;
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
    float maxval=0.0,minnonzeroval=1e20;

    if (1)
      {
	cout << "Finding min and max absolute values..."; cout.flush();
	float absval,maxabsval=0.0,minabsval=1e20;
	int x,y,z;
	for (z=0; z<vsize[2]; z++) for (y=0; y<vsize[1]; y++) for (x=0; x<vsize[0]; x++)
	  {
	    absval=fabs(v->d(x,y,z));
	    if (absval<minabsval) minabsval=absval;
	    if (absval>maxabsval) maxabsval=absval;
	    if ((absval<minnonzeroval) && (absval>0.0)) minnonzeroval=absval;
	  }
	cout << "done: " << minnonzeroval << '(' << minabsval << ')' << " and " << maxabsval << '\n';
	maxval=maxabsval;
      }
	
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
                float freq=1.0;
                //float normalized=fabs(val)/1.0;			//for .v2 files
                //float normalized=fabs(val)/32767.5;		//for .vri files
                //float normalized=fabs(val)/(float)maxdim;   //for .v (signed distance) files
		float normalized=1.0;                          //for just in/out
		//float normalized=fabs(val)/maxval;              //if we calculated the max absolute value
		//float normalized=(fabs(val))<0.1?0.0:1.0; //for in/surface/out
                if (val<0)	//red
                {
                    red  =255;
                    green=255-(int)floor(255*periodic(normalized*freq));
                    blue =255-(int)floor(255*periodic(normalized*freq));
                }
                else		//gray
                {
                    red  =255-(int)floor(200*periodic(normalized*freq));
                    green=255-(int)floor(200*periodic(normalized*freq));
                    blue =255-(int)floor(200*periodic(normalized*freq));
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
