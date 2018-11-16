
/* betti.cpp */

#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "TriangleMesh.h"

/* ------------------------------------ */
 
TriangleMesh m;
int *stack;
int sp;
unsigned char *flags;

/* ------------------------------------ */

void init_all ( char *fname )
{
	int error=0;
	error=m.readFile(fname);
  if (error) exit(1);
  //cout << "Processing..."; cout.flush();
  m.computeConnectivity();
  //cout << "done.\n";
  sp = 0;
  stack = (int*)malloc(m.getNumTris()*sizeof(int));
  flags = (unsigned char*)malloc(m.getNumTris()*sizeof(unsigned char));
  memset(flags,0,m.getNumTris()*sizeof(unsigned char));
}

/* ------------------------------------ */

void push ( int i )
{
  if (flags[i]) return;
  assert(sp<m.getNumTris());
  stack[sp++] = i;
}

/* ------------------------------------ */

int pop()
{
  assert(sp>0);
  return stack[--sp];
}

/* ------------------------------------ */

void clean_up ( )
{
  free(stack);
}

/* ------------------------------------ */

void _traverse()
{
  int i,j,k;

  while(sp>0)
    {
      i = pop();
      flags[i] = 1;

      for ( j=0; j<3; j++ )
	{
	  if ((k=(m.getOpposite(i,j)))==-1) 
	    continue;

	  if (flags[k])
	    continue;

	  push(k);
	}
    }
}

/* ------------------------------------ */

void traverse ( int i )
{
  assert(!sp);
  push(i);
  _traverse();
}

/* ------------------------------------ */

int traverse_all()
{
  int i,cs;

  for ( cs=i=0; i<m.getNumTris(); i++ )
    if (!flags[i])
      {
	cs++;
	traverse(i);
      }

  return cs;
}

/* ------------------------------------ */

int main ( int argc, char *argv[] )
{
  int components;

  if (argc!=2)
    {
      cout << "Usage: " << argv[0] << " <input triangle file>\n";
      return 1;
    }
  init_all(argv[1]);
  components = traverse_all();
  clean_up();

  int euler=m.getNumVerts()+m.getNumTris()-(m.getNumInternalEdges()+m.getNumExternalEdges());
  
  cout << "B0 = "<< components << '\n';
  cout << "B1 = "<< 2*components-euler << '\n';
  cout << "B2 = "<< components << '\n';

  cout << m.getNumVerts() << " vertices, " << m.getNumTris() << " triangles, ";
  cout << m.getNumInternalEdges() << " internal edges, ";
  cout << m.getNumExternalEdges() << " external edges.\n";

  cout << "EULER = " << euler << '\n';

  return 0;
}
