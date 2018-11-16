//meshviewer.cpp
//James Vanderhyde, 16 October 2004


#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 #include <GLUT/glut.h>
#else
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glut.h>
#endif

#include "TriangleMesh.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MENU_SLOWER 1
#define MENU_FASTER 2
#define MENU_STOP_RUN 3
#define MENU_SWITCH_SHADING 7
#define MENU_LINES_TOGGLE 9
#define MENU_POINTS_TOGGLE 10
#define MENU_TRIANGLES_TOGGLE 11
#define MENU_CULLING_TOGGLE 12
#define MENU_BOUNDING_BOX 13

#define TWOPI (2.0 * M_PI)
#define BIGNUM 1e15
#define DEGPERRAD (180/M_PI)
#define ROOT2 1.414213562373
#define ROOT3 1.732050807569

TriangleMesh mesh;

Vector3D worldCenter,worldMinCorner,worldMaxCorner;
float worldSize;

int animate=0;         //animate or not?
int gouraudShading=0;  //Gouraud shading or flat shading?
int showFaces=1;
int showEdges=0;
int showVertices=0;
int backFaceCulling=1;
int boundingBox=0;

//angles used in animation
float angle1 = 0;
float angle2 = 0;
float dangle1 = 1.2;
float dangle2 = 0.2;

//stuff used for trackball rotation
Vector3D rotateStart;
Vector3D rotateEnd;
Vector3D rotateAxis(0,0,1);
float rotateAngle=0.0;
Matrix rotateMatrix={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

void construct_triangle_mesh(char* filename)
{
	int i;
	Vertex v;
	float minx=BIGNUM,miny=BIGNUM,minz=BIGNUM,maxx=-BIGNUM,maxy=-BIGNUM,maxz=-BIGNUM;
	int numVerts;
	
	mesh.readFile(filename);
	
	cout << mesh.getNumTris() << " triangles.\n";
	if (mesh.getNumTris() > 100000) boundingBox=1;
	
	numVerts=mesh.getNumVerts();
	for (i=0; i<numVerts; i++)
	{
		v=mesh.getVert(i);
		if (v.x<minx) minx=v.x;
		if (v.x>maxx) maxx=v.x;
		if (v.y<miny) miny=v.y;
		if (v.y>maxy) maxy=v.y;
		if (v.z<minz) minz=v.z;
		if (v.z>maxz) maxz=v.z;
	}
	
	worldCenter.x=(maxx+minx)/2.0;
	worldCenter.y=(maxy+miny)/2.0;
	worldCenter.z=(maxz+minz)/2.0;
	worldSize=(maxy-miny);
	worldMinCorner.x=minx;
	worldMaxCorner.x=maxx;
	worldMinCorner.y=miny;
	worldMaxCorner.y=maxy;
	worldMinCorner.z=minz;
	worldMaxCorner.z=maxz;
	
	mesh.calculateNormals();
}

GLvoid position_and_turn_on_light()
{
  //colors of different aspects of the light
  GLfloat light_ambient[] = { .05, .05, .05, 1.0 };
  GLfloat light_diffuse[] = { .85, .85, .85, 1.0 };
  GLfloat light_specular[] = { 0, 0, 0, 1.0 };

  //light location: behind and slightly above the camera
  GLfloat light_position[] = { 0.0, 0.5, 1.0, 0.0 };

  //set the properties for the light (light source #0)
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.0);
  glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.0);
  glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.0);

  //enable lighting and turn on the light
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
}

GLvoid set_material_properties ( GLfloat r, GLfloat g, GLfloat b )
{
  //set up some default values for the material color properties
  GLfloat mat_specular[4] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat mat_ambient_and_diffuse[4] = { 0.5, 0.5, 0.5, 1.0 };
  GLfloat mat_shininess[1] = { 0.0 };
  
  //use the values specifed in the parameters
  mat_specular[0] = mat_ambient_and_diffuse[0] = r;
  mat_specular[1] = mat_ambient_and_diffuse[1] = g;
  mat_specular[2] = mat_ambient_and_diffuse[2] = b;

  //set the properties for the material
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient_and_diffuse);
}

GLvoid draw_triangle_mesh()
{
  int i,numTris=mesh.getNumTris();
	Vertex va,vb,vc;
	Triangle t;

  glPushMatrix();
  glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
  glTranslatef(-worldCenter.x,-worldCenter.y,-worldCenter.z);

  if (gouraudShading)
    {
      glBegin(GL_TRIANGLES);
      
      for (i=0; i<numTris; i++)
	{
		  t=mesh.getTri(i);
		  va=mesh.getVert(t.a);
		  vb=mesh.getVert(t.b);
		  vc=mesh.getVert(t.c);
	  glNormal3f(va.n.x,va.n.y,va.n.z);
	  glVertex3f(va.x,va.y,va.z);
	  glNormal3f(vb.n.x,vb.n.y,vb.n.z);
	  glVertex3f(vb.x,vb.y,vb.z);
	  glNormal3f(vc.n.x,vc.n.y,vc.n.z);
	  glVertex3f(vc.x,vc.y,vc.z);
	}
      
      glEnd();
    }
  else
    {
      glBegin(GL_TRIANGLES);
      
      for (i=0; i<numTris; i++)
	{
		  t=mesh.getTri(i);
		  va=mesh.getVert(t.a);
		  vb=mesh.getVert(t.b);
		  vc=mesh.getVert(t.c);
		  glNormal3f(t.n.x,t.n.y,t.n.z);
		  glVertex3f(va.x,va.y,va.z);
		  glVertex3f(vb.x,vb.y,vb.z);
		  glVertex3f(vc.x,vc.y,vc.z);
	}
      
      glEnd();
    }
  
  glPopMatrix();
}

GLvoid draw_marked_points()
{
	int i,numVerts=mesh.getNumVerts();
	Vertex v;
	
	glPushMatrix();
	glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
	glTranslatef(-worldCenter.x,-worldCenter.y,-worldCenter.z);
	
	glBegin(GL_POINTS);
	for (i=0; i<numVerts; i++)
	{
		v=mesh.getVert(i);
		if (v.label>0)
			glVertex3f(v.x,v.y,v.z);
	}
	glEnd();
	
	glPopMatrix();
}

GLvoid draw_marked_triangles()
{
	int i,numTris=mesh.getNumTris();
	Vertex va,vb,vc;
	Triangle t;
	
	glPushMatrix();
	glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
	glTranslatef(-worldCenter.x,-worldCenter.y,-worldCenter.z);
	
		glBegin(GL_LINES);
		
		for (i=0; i<numTris; i++)
		{
			t=mesh.getTri(i);
			va=mesh.getVert(t.a);
			vb=mesh.getVert(t.b);
			vc=mesh.getVert(t.c);
				
			if ((va.label>0) && (vb.label>0))
			{
				glVertex3f(va.x,va.y,va.z);
				glVertex3f(vb.x,vb.y,vb.z);
			}
			if ((vb.label>0) && (vc.label>0))
			{
				glVertex3f(vb.x,vb.y,vb.z);
				glVertex3f(vc.x,vc.y,vc.z);
			}
			if ((vc.label>0) && (va.label>0))
			{
				glVertex3f(vc.x,vc.y,vc.z);
				glVertex3f(va.x,va.y,va.z);
			}
		}
		
		glEnd();
	
	glPopMatrix();
}

GLvoid draw_bounding_box()
{
	glPushMatrix();
	glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
	glTranslatef(-worldCenter.x,-worldCenter.y,-worldCenter.z);

	glBegin(GL_QUADS);
	  glNormal3f(0,0,1);
	  glVertex3f(worldMinCorner.x,worldMinCorner.y,worldMaxCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMinCorner.y,worldMaxCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMaxCorner.y,worldMaxCorner.z);
	  glVertex3f(worldMinCorner.x,worldMaxCorner.y,worldMaxCorner.z);
	  glNormal3f(0,0,-1);
	  glVertex3f(worldMinCorner.x,worldMinCorner.y,worldMinCorner.z);
	  glVertex3f(worldMinCorner.x,worldMaxCorner.y,worldMinCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMaxCorner.y,worldMinCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMinCorner.y,worldMinCorner.z);
	  glNormal3f(0,1,0);
	  glVertex3f(worldMinCorner.x,worldMaxCorner.y,worldMinCorner.z);
	  glVertex3f(worldMinCorner.x,worldMaxCorner.y,worldMaxCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMaxCorner.y,worldMaxCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMaxCorner.y,worldMinCorner.z);
	  glNormal3f(0,-1,0);
	  glVertex3f(worldMinCorner.x,worldMinCorner.y,worldMinCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMinCorner.y,worldMinCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMinCorner.y,worldMaxCorner.z);
	  glVertex3f(worldMinCorner.x,worldMinCorner.y,worldMaxCorner.z);
	  glNormal3f(1,0,0);
	  glVertex3f(worldMaxCorner.x,worldMinCorner.y,worldMinCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMaxCorner.y,worldMinCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMaxCorner.y,worldMaxCorner.z);
	  glVertex3f(worldMaxCorner.x,worldMinCorner.y,worldMaxCorner.z);
	  glNormal3f(-1,0,0);
	  glVertex3f(worldMinCorner.x,worldMinCorner.y,worldMinCorner.z);
	  glVertex3f(worldMinCorner.x,worldMinCorner.y,worldMaxCorner.z);
	  glVertex3f(worldMinCorner.x,worldMaxCorner.y,worldMaxCorner.z);
	  glVertex3f(worldMinCorner.x,worldMaxCorner.y,worldMinCorner.z);
	glEnd();
	
	glPopMatrix();
}

GLvoid display(GLvoid)
{
  //clear the color and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //set up the model-view matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  position_and_turn_on_light();

  //place object between clipping planes
  glTranslatef(0,0,-20);

  //apply animated rotations
  glRotatef(angle1,0.0,1.0,0.0);
  glRotatef(angle2,1.0,0.0,1.0);
  
  //apply trackball rotation
  glRotatef(rotateAngle,rotateAxis.x,rotateAxis.y,rotateAxis.z);
  glMultMatrixf(rotateMatrix);

  if (boundingBox)
  {
	  draw_bounding_box();
  }
  else
  {
	  //draw the object
	  if (showVertices)
	  {
		  glDisable(GL_LIGHTING);
		  glColor3f(0.3,0.4,0.7);
		  glPolygonOffset(-2,-2);
		  glPointSize(4.0);
		  glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
		  //draw_triangle_mesh();
		  draw_marked_points();
	  }
	  if (showEdges)
	  {
		  glDisable(GL_LIGHTING);
		  glColor3f(0.8,1.0,0.8);
		  glPolygonOffset(-1,-1);
		  glLineWidth(2.0);
		  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		  //draw_triangle_mesh();
		  draw_marked_triangles();
	  }
	  if (showFaces)
	  {
		  glEnable(GL_LIGHTING);
		  set_material_properties(0.8,0.4,0.6);
		  glPolygonOffset(0,0);
		  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		  draw_triangle_mesh();
	  }
  }
 
  //flush the pipeline
  glFlush();

  //look at our handiwork
  glutSwapBuffers();

}

void reshape(int w, int h)
{
  //change the screen window (viewport) size
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);

  //set up the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(15.0,(GLfloat) w/(GLfloat) h,15.0,25.0);
}

void idle()
{
  if (animate)
    {
      angle1 += dangle1;
      angle2 += dangle2;
      glutPostRedisplay();
    }
}

Vector3D pixelToTrackball(GLint x,GLint y)
{
	GLint viewport[4];
	Vector3D r;
	glGetIntegerv(GL_VIEWPORT, viewport);
	int vpd=(viewport[2]<viewport[3])?viewport[2]:viewport[3];
	r.x=2.0*((x-viewport[2]/2.0)/vpd);
	r.y=-2.0*((y-viewport[3]/2.0)/vpd);
	double d=1-r.x*r.x-r.y*r.y;
	if (d>=0.0) r.z=sqrt(d);
	else
    {
		d=1.0/sqrt(1-d);
		r.x*=d;
		r.y*=d;
		r.z=0.0;
    }
	return r;
}

void recordRotation()
{
	float sint,cost,x,y,z;
	sint=length(cross(rotateStart,rotateEnd));
	cost=dot(rotateStart,rotateEnd);
	x=rotateAxis.x; y=rotateAxis.y; z=rotateAxis.z;
	Matrix rot;
	rot[3]=rot[7]=rot[11]=rot[12]=rot[13]=rot[14]=0;
	rot[0]=1+(1-cost)*(x*x-1);
	rot[5]=1+(1-cost)*(y*y-1);
	rot[10]=1+(1-cost)*(z*z-1);
	rot[15]=1;
	rot[6]= x*sint+(1-cost)*y*z;
	rot[9]=-x*sint+(1-cost)*y*z;
	rot[8]= y*sint+(1-cost)*z*x;
	rot[2]=-y*sint+(1-cost)*z*x;
	rot[1]= z*sint+(1-cost)*x*y;
	rot[4]=-z*sint+(1-cost)*x*y;
	Matrix oldRot;
	copy(rotateMatrix,oldRot);
	times(rot,oldRot,rotateMatrix);
}

void mouse_button(int btn, int state, int mx, int my)
{
	switch( btn ) {
		case GLUT_LEFT_BUTTON:
			switch( state ) {
				case GLUT_DOWN: 
					rotateStart=pixelToTrackball(mx,my);
					rotateAngle=0.0;
					break;
				case GLUT_UP:  
					if (rotateAngle != 0.0) recordRotation();
					rotateAngle=0.0;
					break;
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			switch( state ) {
				case GLUT_DOWN: 
					break;
				case GLUT_UP:   
					break;
			}
			break;
		case GLUT_RIGHT_BUTTON:
			switch( state ) {
				case GLUT_DOWN: 
					break;
				case GLUT_UP:   
					break;
			}
			break;
	}
}

void button_motion(int mx, int my)
{
	rotateEnd=pixelToTrackball(mx,my);
	rotateAxis=normalized(cross(rotateStart,rotateEnd));
	rotateAngle=DEGPERRAD*acos(dot(rotateStart,rotateEnd));
	glutPostRedisplay();
	return;
}

void menu(int value)
{
  switch(value)
    {
    case MENU_SLOWER:
      dangle1 *= .66667;
      dangle2 *= .66667;
      break;
    case MENU_FASTER:
      dangle1 *= 1.5;
      dangle2 *= 1.5;
      break;
    case MENU_STOP_RUN:
      animate = !animate;
      if (animate) glutIdleFunc(idle);
      else glutIdleFunc(NULL);
      break;
    case MENU_CULLING_TOGGLE:
      backFaceCulling = !backFaceCulling;
      if (backFaceCulling) glEnable(GL_CULL_FACE);
      else glDisable(GL_CULL_FACE);
      break;
    case MENU_SWITCH_SHADING:
      gouraudShading = !gouraudShading;
      break;
    case MENU_LINES_TOGGLE:
      showEdges = !showEdges;
      break;
    case MENU_POINTS_TOGGLE:
      showVertices = !showVertices;
      break;
    case MENU_TRIANGLES_TOGGLE:
      showFaces = !showFaces;
      break;
    case MENU_BOUNDING_BOX:
      boundingBox = !boundingBox;
      break;
    }
  glutPostRedisplay();
}

void quit(int exitCondition)
{
  exit(exitCondition);
}

void keyboard(unsigned char key, int x, int y)
{
  switch(key) 
    {
    case 27:  /* Esc */
      quit(0);
    case '/':
      menu(MENU_STOP_RUN);
      break;
    case ',':
      menu(MENU_SLOWER);
      break;
    case '.':
      menu(MENU_FASTER);
      break;
 
    default:  break;
  }
}

void init_opengl()
{
  //turn on back-face culling
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  //automatically scale normals to unit length after transformation
  glEnable(GL_NORMALIZE);

  //set clear color to black
  glClearColor(0.0, 0.0, 0.0, 1.0);

  //enable depth test (z-buffer)
  glEnable(GL_DEPTH_TEST);

  //allow the edges and vertices to be drawn on top of the triangles
  glEnable(GL_POLYGON_OFFSET_LINE);
  glEnable(GL_POLYGON_OFFSET_POINT);
}

void init_glut(int *argc, char **argv)
{
  glutInit(argc,argv);

  //size and placement hints to the window system
  glutInitWindowSize(400, 400);
  glutInitWindowPosition(10,10);

  //double buffered, RGB color mode, with depth test
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  //create a GLUT window (not drawn until glutMainLoop() is entered)
  glutCreateWindow("Mesh Viewer");

  //register callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse_button);
  glutMotionFunc(button_motion);

  //set up pop-up menu
  GLint menuID = glutCreateMenu(menu);
  glutAddMenuEntry("slower",MENU_SLOWER);
  glutAddMenuEntry("faster",MENU_FASTER);
  glutAddMenuEntry("stop/run",MENU_STOP_RUN);
  glutAddMenuEntry("toggle back face culling",MENU_CULLING_TOGGLE);
  glutAddMenuEntry("switch shading",MENU_SWITCH_SHADING);
  glutAddMenuEntry("toggle triangles",MENU_TRIANGLES_TOGGLE);
  glutAddMenuEntry("toggle edges",MENU_LINES_TOGGLE);
  glutAddMenuEntry("toggle vertices",MENU_POINTS_TOGGLE);
  glutAddMenuEntry("toggle bounding box",MENU_BOUNDING_BOX);
  glutSetMenu(menuID);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
  init_glut(&argc, argv);
  init_opengl();

  if (argc>1) construct_triangle_mesh(argv[1]);

  glutMainLoop();

  return 0;
}

