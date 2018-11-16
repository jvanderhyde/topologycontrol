
/* ---------------- gmesh.h ------------------------- */

#include "geom3D.h"
#include "intlist.h"
//#include <GL/gl.h>

#if !defined __GMESH2_H

#define __GMESH2_H

/* -------------------------------------------------- */

class gmesh {

  int invert_nms;
  int invert_trs;
  int first_time_render;
  //GLuint dlist;
 public:
  int new_rendering_options;
  int polygon_mode;
  int shading_mode;
  int vertices,triangles;
  int3 *triangle;
  double3 *vertex;
  //  int_list **inc;
  double3 hi;
  double3 lo;
  double3 center;
  double3 dims;
  double maxaxis;
  double3 *normal_vector_s;
  double3 *normal_vector_f;

  void set_polygon_mode ( int pm );
  void set_shading_mode ( int sm );
  void build_normal_vectors();
  void render();
  void compute_data();
  void flip_normals();
  void flip_orientations();
  double *dist;

  //  void build_inc();
  double area ( int t );
  gmesh();
  ~gmesh();
  gmesh(istream &i);
  void scale_to_fit_m11();
  void export_povray(ostream &ofs);
};

extern ostream & operator<< ( ostream &o, gmesh &g );

#endif
