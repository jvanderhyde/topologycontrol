/*
 *  main.cpp
 *  
 *
 *  Created by James Vanderhyde on Sat Nov 29 2003.
 *
 */

#include <fstream.h>
#include <iostream.h>

#include "gmesh2.h"

typedef double Matrix[4][4];

void identity(Matrix m)
{
    m[0][0]=1; m[1][0]=0; m[2][0]=0; m[3][0]=0;
    m[0][1]=0; m[1][1]=1; m[2][1]=0; m[3][1]=0;
    m[0][2]=0; m[1][2]=0; m[2][2]=1; m[3][2]=0;
    m[0][3]=0; m[1][3]=0; m[2][3]=0; m[3][3]=1; 
}

void rotate90y(Matrix m)
{
    m[0][0]=0; m[1][0]=0; m[2][0]=1; m[3][0]=0;
    m[0][1]=0; m[1][1]=1; m[2][1]=0; m[3][1]=0;
    m[0][2]=-1; m[1][2]=0; m[2][2]=0; m[3][2]=0;
    m[0][3]=0; m[1][3]=0; m[2][3]=0; m[3][3]=1;
}

void rotate90y90z(Matrix m)
{
    m[0][0]=0; m[1][0]=0; m[2][0]=1; m[3][0]=0;
    m[0][1]=1; m[1][1]=0; m[2][1]=0; m[3][1]=0;
    m[0][2]=0; m[1][2]=1; m[2][2]=0; m[3][2]=0;
    m[0][3]=0; m[1][3]=0; m[2][3]=0; m[3][3]=1;
}

void defineMatrix(Matrix m)
{
    rotate90y90z(m);
}

int main(int argc,char** argv)
{
    Matrix m;
    
    if (argc<3)
    {
        cerr << "Usage: main <input .t file> <output .pov file>\n";
        return 1;
    }

    ifstream fin(argv[1]);
    ofstream fout(argv[2]);

    if (!fin)
    {
        cerr << "Error reading file " << argv[1] << '\n';
        return 1;
    }
    if (!fout)
    {
        cerr << "Error open file " << argv[2] << " for writing\n";
        return 1;
    }

    defineMatrix(m);

    cout << "Reading file..."; cout.flush();
    gmesh mesh(fin);
    mesh.scale_to_fit_m11();
    cout << "done.\n";

    cout << "Writing file..."; cout.flush();

    fout << "camera {" << endl;
    fout << "  location <0,0,0>" << endl;
    fout << "  up <0,1,0>" << endl;
    fout << "  look_at <0,0,-1>" << endl;
    fout << "}" << endl << endl;

    fout << "light_source {" << endl;
    fout << "  <-5,5,0>" << endl;
    fout << "  color rgb <.6,.6,.6>" << endl;
    fout << "}" << endl << endl;

    fout << "light_source {" << endl;
    fout << "  <5,5,0>" << endl;
    fout << "  color rgb <.4,.4,1>" << endl;
    fout << "}" << endl << endl;

    fout << "background { rgb <0,0,0> }" << endl << endl;

    fout << "mesh2 {" << endl;
    mesh.export_povray(fout);
    fout << endl << "  pigment { rgb 1 }" << endl << endl;
    fout << "  matrix <" << m[0][0] << "," << m[1][0] << "," << m[2][0] << "," << endl;
    fout << "          " << m[0][1] << "," << m[1][1] << "," << m[2][1] << "," << endl;
    fout << "          " << m[0][2] << "," << m[1][2] << "," << m[2][2] << "," << endl;
    fout << "          " << m[0][3] << "," << m[1][3] << "," << m[2][3] << ">" << endl;
    fout << "  translate <0,0,-2.1>" << endl;
    fout << "}" << endl;

    fout.close();
    cout << "done.\n";
}
