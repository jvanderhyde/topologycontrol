
/* --------------------------------- gmesh.c --------------------------------- */

#include <fstream.h>
#include <iostream.h>

//#include <GL/gl.h>
//#include <GL/glu.h>

#include "intlist.h"
#include "gmesh2.h"

/* --------------------------------------------------------------------------- */

void gmesh::scale_to_fit_m11()
{
  int i,j;
  for ( i=0; i<vertices; i++ )
    for ( j=0; j<3; j++ )
      {
	vertex[i][j] -= center[j];
	vertex[i][j] *= 2/maxaxis;
      }
}

/* --------------------------------------------------------------------------- */

void gmesh::flip_orientations()
{
  invert_trs = !invert_trs;
  new_rendering_options = 1;
}

/* --------------------------------------------------------------------------- */


gmesh::gmesh ( istream &i )
{
  //polygon_mode = GL_FILL;
  //shading_mode = GL_SMOOTH;
  first_time_render = 1;
  dist = NULL;
  invert_nms = invert_trs = 0;
  i >> triangles >> vertices;
  triangle = new int3[triangles];
  vertex = new double3[vertices];
  int n;
  for ( n=0; n<triangles; n++ )
    i >> triangle[n][0] >> triangle[n][1] >> triangle[n][2];
  for ( n=0; n<vertices; n++ )
    i >> vertex[n][0] >> vertex[n][1] >> vertex[n][2];
  //  build_inc();
  build_normal_vectors();
  compute_data();
}

/* --------------------------------------------------------------------------- */

void gmesh::flip_normals()
{
  invert_nms = !invert_nms;
  new_rendering_options = 1;
}

/* --------------------------------------------------------------------------- */

extern ostream & operator<< ( ostream &o, gmesh &g )
{
  o << g.triangles << " " << g.vertices << endl << endl;
  int n;
  for ( n=0; n<g.triangles; n++ )
    o << g.triangle[n][0] << " " << g.triangle[n][1] << " " << g.triangle[n][2] << endl;
  o << endl;
  for ( n=0; n<g.vertices; n++ )
    o << g.vertex[n][0] << " " << g.vertex[n][1] << " " << g.vertex[n][2] << endl;
  return o;
}

/* --------------------------------------------------------------------------- */

gmesh::gmesh() : vertices(0), triangles(0), triangle(NULL), vertex(NULL), dist(NULL) //, inc(NULL)
{

}

/* --------------------------------------------------------------------------- */

gmesh::~gmesh()
{
  if (triangle)
    delete[] triangle;
  if (vertex)
    delete[] vertex;
  if (dist)
    delete[] dist;
  /*
  if (inc)
    {
      for ( int i=0; i<vertices; i++ )
	int_list_clear(&inc[i]);
      delete[] inc;
    }
  */
  vertex = NULL;
  triangle = NULL;
  //  inc = NULL;
  if (normal_vector_s)
      delete[] normal_vector_s;
  if (normal_vector_f)
      delete[] normal_vector_f;
}

/* --------------------------------------------------------------------------- */

double gmesh::area ( int t )
{
  double3 ab,ac;
  double3_subtract(ab,vertex[triangle[t][1]],vertex[triangle[t][0]]);
  double3_subtract(ac,vertex[triangle[t][2]],vertex[triangle[t][0]]);
  double3 crss;
  double3_cross(crss,ab,ac);
  return .5*double3_length(crss);
}

/* --------------------------------------------------------------------------- */
/*
void gmesh::build_inc()
{
  inc = new (int_list*)[vertices];
  int i;
  for ( i=0; i<vertices; i++ )
    inc[i] = NULL;
  for ( i=0; i<triangles; i++ )
    {
      int_list_insert(&inc[triangle[i][0]],i);
      int_list_insert(&inc[triangle[i][1]],i);
      int_list_insert(&inc[triangle[i][2]],i);
    }
}
*/
/* --------------------------------------------------------------------------- */

void gmesh::set_polygon_mode ( int pm )
{
  polygon_mode = pm;
  //glPolygonMode(GL_FRONT_AND_BACK,pm);
  new_rendering_options = 1;
}

void gmesh::set_shading_mode ( int sm )
{
  shading_mode = sm;
  new_rendering_options = 1;
}

void gmesh::build_normal_vectors ( )
{
  int i;

  normal_vector_s = new double3[vertices];
  
  for ( i=0; i<vertices; i++ )
    normal_vector_s[i][0] = normal_vector_s[i][1] = normal_vector_s[i][2] = 0;

  for ( i=0; i<triangles; i++ )
    {
      double3 ab,ac,cross;
      double3_subtract(ab,vertex[triangle[i][1]],vertex[triangle[i][0]]);
      double3_subtract(ac,vertex[triangle[i][2]],vertex[triangle[i][0]]);
      double3_cross(cross,ab,ac);
      double3_add(normal_vector_s[triangle[i][0]],
		  normal_vector_s[triangle[i][0]],cross);
      double3_add(normal_vector_s[triangle[i][1]],
		  normal_vector_s[triangle[i][1]],cross);
      double3_add(normal_vector_s[triangle[i][2]],
		  normal_vector_s[triangle[i][2]],cross);
    }

  normal_vector_f = new double3[triangles];

  for ( i=0; i<triangles; i++ )
    {
      double3 ab,ac,cross;
      double3_subtract(ab,vertex[triangle[i][1]],vertex[triangle[i][0]]);
      double3_subtract(ac,vertex[triangle[i][2]],vertex[triangle[i][0]]);
      double3_cross(cross,ab,ac);
      double3_copy(normal_vector_f[i],cross);
    }
  
}

void gmesh::export_povray ( ostream &ofs )
{
  int i;

  ofs << "    vertex_vectors {" << endl;
  ofs << "      " << vertices << endl;
  for ( i=0; i<vertices; i++ )
    ofs << "      <" << vertex[i][0] << "," << vertex[i][1] << "," << vertex[i][2] << ">," << endl;
  ofs << "    }" << endl << endl;
  ofs << "    normal_vectors {" << endl;
  ofs << "      " << vertices << endl;
  for ( i=0; i<vertices; i++ )
    ofs << "      <" << normal_vector_s[i][0] << "," << normal_vector_s[i][1] << "," << normal_vector_s[i][2] << ">" << endl;
  ofs << "    }" << endl << endl;
  ofs << "    face_indices {" << endl;
  ofs << "      " << triangles << endl;
  for ( i=0; i<triangles; i++ )
    ofs << "      <" << triangle[i][0] << "," << triangle[i][1] << "," << triangle[i][2] << ">" << endl;
  ofs << "    }" << endl << endl;
}

void glColor1f ( double d )
{
  //glColor3f(d,d,d);
}

double frac ( double x )
{
  return (1+cos(100*x))/2;
}

void gmesh::render()
{
  /*int i;

  cout << "render" << endl;

  glShadeModel(shading_mode);
  glPolygonMode(GL_FRONT_AND_BACK,polygon_mode);

  if (first_time_render || new_rendering_options)
    {
      if (!first_time_render)
	{
	  glDeleteLists(dlist,1);
	}

      first_time_render = new_rendering_options = 0;

      dlist = glGenLists(1);

      glNewList(dlist,GL_COMPILE_AND_EXECUTE);
      
      glBegin(GL_TRIANGLES);
      
      cout << "gllist" << endl;

      for ( i=0; i<triangles; i++ )
	{
	  if (shading_mode==GL_SMOOTH)
	    {
	      if (!invert_trs)
		{
		  if (!invert_nms)
		    glNormal3dv(normal_vector_s[triangle[i][0]]);
		  else
		    glNormal3d(-normal_vector_s[triangle[i][0]][0],
			       -normal_vector_s[triangle[i][0]][1],
			       -normal_vector_s[triangle[i][0]][2]);
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][0]]));
		  glVertex3dv(vertex[triangle[i][0]]);
		  if (!invert_nms)
		    glNormal3dv(normal_vector_s[triangle[i][1]]);
		  else
		    glNormal3d(-normal_vector_s[triangle[i][1]][0],
			       -normal_vector_s[triangle[i][1]][1],
			       -normal_vector_s[triangle[i][1]][2]);
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][1]]));
		  glVertex3dv(vertex[triangle[i][1]]);
		}
	      else
		{
		  if (!invert_nms)
		    glNormal3dv(normal_vector_s[triangle[i][1]]);
		  else
		    glNormal3d(-normal_vector_s[triangle[i][1]][0],
			       -normal_vector_s[triangle[i][1]][1],
			       -normal_vector_s[triangle[i][1]][2]);
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][1]]));
		  glVertex3dv(vertex[triangle[i][1]]);
		  if (!invert_nms)
		    glNormal3dv(normal_vector_s[triangle[i][0]]);
		  else
		    glNormal3d(-normal_vector_s[triangle[i][0]][0],
			       -normal_vector_s[triangle[i][0]][1],
			       -normal_vector_s[triangle[i][0]][2]);
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][0]]));
		  glVertex3dv(vertex[triangle[i][0]]);
		}
	      if (!invert_nms)
		glNormal3dv(normal_vector_s[triangle[i][2]]);
	      else
		glNormal3d(-normal_vector_s[triangle[i][2]][0],
			   -normal_vector_s[triangle[i][2]][1],
			   -normal_vector_s[triangle[i][2]][2]);
	      if (dist)
		glColor1f(frac(.3*dist[triangle[i][2]]));
	      glVertex3dv(vertex[triangle[i][2]]);
	    }
	  else
	    {
	      if (!invert_nms)
		glNormal3dv(normal_vector_f[i]);
	      else
		glNormal3d(-normal_vector_f[i][0],
			   -normal_vector_f[i][1],
			   -normal_vector_f[i][2]);
	      if (!invert_trs)
		{
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][0]]));
		  glVertex3dv(vertex[triangle[i][0]]);
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][1]]));
		  glVertex3dv(vertex[triangle[i][1]]);
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][2]]));
		  glVertex3dv(vertex[triangle[i][2]]);
		}
	      else
		{
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][1]]));
		  glVertex3dv(vertex[triangle[i][1]]);
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][0]]));
		  glVertex3dv(vertex[triangle[i][0]]);
		  if (dist)
		    glColor1f(frac(.3*dist[triangle[i][2]]));
		  glVertex3dv(vertex[triangle[i][2]]);		  
		}
	    }
	}

      cout << "glendlist" << endl;

      glEnd();

      glEndList();
    }
  else
    {
      glCallList(dlist);
    }

  glEnd();*/
}

void gmesh::compute_data()
{
  double3_copy(hi,vertex[0]);
  double3_copy(lo,vertex[0]);
  int i;
  for ( i=1; i<vertices; i++ )
    {
      double3_max(hi,hi,vertex[i]);
      double3_min(lo,lo,vertex[i]);
    }
  double3_add(center,hi,lo);
  double3_scale(center,.5,center);
  double3_subtract(dims,hi,lo);
  maxaxis = double3_maxcoord(dims);
}
