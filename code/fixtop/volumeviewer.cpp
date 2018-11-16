//volumeviewer.cpp
//James Vanderhyde, 16 October 2004


#include <math.h>
#include <iostream.h>
#include <stdlib.h>
#include <fstream.h>

#ifdef __APPLE__
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 #include <GLUT/glut.h>
#else
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glut.h>
#endif

//#include "MarchableVolume.h"
#include "Vector3D.h"

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
#define MENU_ISO_ORDER 13
#define MENU_INITIAL_ORDER 14
#define MENU_CARVEDINSIDE_ORDER 15
#define MENU_CARVEDOUTSIDE_ORDER 16


#define TWOPI (2.0 * M_PI)
#define BIGNUM 1e15
#define DEGPERRAD (180/M_PI)
#define ROOT2 1.414213562373
#define ROOT3 1.732050807569

//MarchableVolume* v;
float* vdata;
int* vsize;

float orderIndices[4];
int activeOrder=0;
int faceColor=0;
int minLifeSpan=0;

Vector3D worldCenter;
float worldSize;

int animate=0;         //animate or not?
int gouraudShading=0;  //Gouraud shading or flat shading?
int showFaces=1;
int showEdges=0;
int showVertices=0;
int backFaceCulling=1;

int displayIsovalue=1;

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

float vd(int x,int y,int z)
{
  //return v->d(x,y,z);
  if ((x<0) || (y<0) || (z<0) || (x>=vsize[0]) || (y>=vsize[1]) || (z>=vsize[2])) return BIGNUM;
  return vdata[x+vsize[0]*(y+vsize[1]*(z))];
}

int readVolumeFile(char* filename)
{
  ifstream fin(filename);
  if (!fin) return 1;
  vsize=new int[3];
  fin.read((char*)vsize,3*sizeof(int));
  vdata=new float[vsize[2]*vsize[1]*vsize[0]];
  fin.read((char*)vdata,vsize[2]*vsize[1]*vsize[0]*sizeof(float));
  fin.close();
  return 0;
}

int set_up_volume(char* filename)
{
  //v=MarchableVolume::createVolume(filename);
  //vsize=v->getSize();
  int result = readVolumeFile(filename);
  if (result) return result;

	float minval=0;
	/*short minval=v.findMinimum();
	v.addToAll(-minval+1);
	v.sortVoxels();
	v.carveFromInside();
	v.carveFromOutside();
	//v.carveFromOutside(4,1);
	//v.carveFromInside(7);
	//v.carveFromOutside(5,0);
	v.countCriticals();*/
	
	worldCenter.x=(vsize[0])/2.0;
	worldCenter.y=(vsize[1])/2.0;
	worldCenter.z=(vsize[2])/2.0;
	worldSize=(vsize[1]);
	
	orderIndices[0]=0;  //isovalue
	orderIndices[1]=0;			//initialOrder
	orderIndices[2]=0;			//carvedInsideOrder
	orderIndices[3]=0;			//carvedOutsideOrder

	return 0;
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

/*int inside(int index)
{
	int condition;
	switch (activeOrder)
	{
	case 0:
		condition=(v.d(index)<=orderIndices[activeOrder]);
		break;
	case 1:
		condition=(v.getInitialOrderAt(index)>orderIndices[activeOrder]);
		break;
	case 2:
		condition=(v.getCarvedInsideOrderAt(index)<=orderIndices[activeOrder]);
		break;
	case 3:
		condition=(v.getCarvedOutsideOrderAt(index)>orderIndices[activeOrder]);
		break;
	default:
		condition=0;
	}
	return condition;
}*/

int inside(int x, int y, int z)
{
	if ((x<0) || (y<0) || (z<0) || (x>=vsize[0]) || (y>=vsize[1]) || (z>=vsize[2])) return 0;
	else return (vd(x,y,z)<=orderIndices[activeOrder]);
}

//Determines color for face, assuming point 1 is inside and point 2 is outside.
void setFaceColor(int x1, int y1, int z1, int x2, int y2, int z2)
{
  /*int hot;
	if ((activeOrder==0) || (activeOrder==1)) hot=0;
	else if (activeOrder==2)
	{
		if ((x2<0) || (y2<0) || (z2<0) || (x2>=vsize[0]) || (y2>=vsize[1]) || (z2>=vsize[2])) hot=0;
		else hot=(v.isInitiallyCritical(v.getVoxelIndex(x2,y2,z2)));
	}
	else if (activeOrder==3) hot=(v.isInitiallyCritical(v.getVoxelIndex(x1,y1,z1)));
	else hot=0;
	if ((hot) && (faceColor!=2))
	{
		faceColor=2;
		set_material_properties(0.9,0.9,1.0);
	}
	else if ((!hot) && (faceColor!=1))
	{
		faceColor=1;
		set_material_properties(0.8,0.4,0.6);
		}*/
	faceColor=1;
	set_material_properties(0.8,0.4,0.6);
}

GLvoid draw_volume()
{
  int x,y,z;

  glPushMatrix();
  glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
  glTranslatef(-worldCenter.x,-worldCenter.y,-worldCenter.z);

  glBegin(GL_QUADS);
      
  for (z=0; z<vsize[2]; z++) for (y=0; y<vsize[1]; y++) for (x=0; x<vsize[0]; x++)
	{
	  if (!inside(x,y,z))
	  {
		  //check whether prior neighbors are on the other side
		  if (inside(x-1,y,z))
		  {
			  setFaceColor(x-1,y,z,x,y,z);
			  glNormal3f(1,0,0);
			  glVertex3f(x-0.5,y-0.5,z-0.5);
			  glVertex3f(x-0.5,y+0.5,z-0.5);
			  glVertex3f(x-0.5,y+0.5,z+0.5);
			  glVertex3f(x-0.5,y-0.5,z+0.5);
		  }
		  if (inside(x,y-1,z))
		  {
			  setFaceColor(x,y-1,z,x,y,z);
			  glNormal3f(0,1,0);
			  glVertex3f(x-0.5,y-0.5,z-0.5);
			  glVertex3f(x-0.5,y-0.5,z+0.5);
			  glVertex3f(x+0.5,y-0.5,z+0.5);
			  glVertex3f(x+0.5,y-0.5,z-0.5);
		  }
		  if (inside(x,y,z-1))
		  {
			  setFaceColor(x,y,z-1,x,y,z);
			  glNormal3f(0,0,1);
			  glVertex3f(x-0.5,y-0.5,z-0.5);
			  glVertex3f(x+0.5,y-0.5,z-0.5);
			  glVertex3f(x+0.5,y+0.5,z-0.5);
			  glVertex3f(x-0.5,y+0.5,z-0.5);
		  }
	  }
	  if (inside(x,y,z))
	  {
		  //check whether prior neighbors are on the other side
		  if (!inside(x-1,y,z))
		  {
			  setFaceColor(x,y,z,x-1,y,z);
			  glNormal3f(-1,0,0);
			  glVertex3f(x-0.5,y-0.5,z-0.5);
			  glVertex3f(x-0.5,y-0.5,z+0.5);
			  glVertex3f(x-0.5,y+0.5,z+0.5);
			  glVertex3f(x-0.5,y+0.5,z-0.5);
		  }
		  if (!inside(x,y-1,z))
		  {
			  setFaceColor(x,y,z,x,y-1,z);
			  glNormal3f(0,-1,0);
			  glVertex3f(x-0.5,y-0.5,z-0.5);
			  glVertex3f(x+0.5,y-0.5,z-0.5);
			  glVertex3f(x+0.5,y-0.5,z+0.5);
			  glVertex3f(x-0.5,y-0.5,z+0.5);
		  }
		  if (!inside(x,y,z-1))
		  {
			  setFaceColor(x,y,z,x,y,z-1);
			  glNormal3f(0,0,-1);
			  glVertex3f(x-0.5,y-0.5,z-0.5);
			  glVertex3f(x-0.5,y+0.5,z-0.5);
			  glVertex3f(x+0.5,y+0.5,z-0.5);
			  glVertex3f(x+0.5,y-0.5,z-0.5);
		  }
		  //add faces at the end
		  if (x==vsize[0]-1)
		  {
			  setFaceColor(x,y,z,x+1,y,z);
			  glNormal3f(1,0,0);
			  glVertex3f(x+1-0.5,y-0.5,z-0.5);
			  glVertex3f(x+1-0.5,y+0.5,z-0.5);
			  glVertex3f(x+1-0.5,y+0.5,z+0.5);
			  glVertex3f(x+1-0.5,y-0.5,z+0.5);
		  }
		  if (y==vsize[1]-1)
		  {
			  setFaceColor(x,y,z,x,y+1,z);
			  glNormal3f(0,1,0);
			  glVertex3f(x-0.5,y+1-0.5,z-0.5);
			  glVertex3f(x-0.5,y+1-0.5,z+0.5);
			  glVertex3f(x+0.5,y+1-0.5,z+0.5);
			  glVertex3f(x+0.5,y+1-0.5,z-0.5);
		  }
		  if (z==vsize[2]-1)
		  {
			  setFaceColor(x,y,z,x,y,z+1);
			  glNormal3f(0,0,1);
			  glVertex3f(x-0.5,y-0.5,z+1-0.5);
			  glVertex3f(x+0.5,y-0.5,z+1-0.5);
			  glVertex3f(x+0.5,y+0.5,z+1-0.5);
			  glVertex3f(x-0.5,y+0.5,z+1-0.5);
		  }
	  }
	}
      
  glEnd();
  
  glPopMatrix();
}

/*GLvoid draw_critical_points()
{
	int x,y,z;
	int hot;
	
	glPushMatrix();
	glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
	glTranslatef(-worldCenter.x,-worldCenter.y,-worldCenter.z);
	
	glBegin(GL_POINTS);
	for (int i=0; i<vsize[2]*vsize[1]*vsize[0]; i++)
	{
		v.getVoxelLocFromIndex(i,&x,&y,&z);
		if ((v.isInitiallyCritical(i)) && (v.getLifeSpansAt(i)>=minLifeSpan))
		{
			hot=0;
			if (hot) glColor3f(0.475,0.35,0.3);
			else glColor3f(0.15,0.2,0.35);
			if ((x==10) && (y==5) && (z==14))
				glColor3f(0.6,0.8,0.35);
			glVertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
		}
		else
		{
			if ((x==10) && (y==5) && (z==14))
			{
				glColor3f(0.8,0.8,0.35);
				glVertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
			}
		}
	}
	glEnd();
	
	glPopMatrix();
}

GLvoid draw_critical_paths()
{
	int x,y,z,p;

	glPushMatrix();
	glScalef(2.0/worldSize,2.0/worldSize,2.0/worldSize);
	glTranslatef(-worldCenter.x,-worldCenter.y,-worldCenter.z);
	
	for (int i=0; i<vsize[2]*vsize[1]*vsize[0]; i++)
	{
		if ((v.isInitiallyCritical(i)) && (v.getLifeSpansAt(i)>=minLifeSpan))
		{		
		  if ((activeOrder==2))
			{
				glBegin(GL_LINE_STRIP);
				p=i;
				v.getVoxelLocFromIndex(p,&x,&y,&z);
				glVertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
				p=v.getParentInsideAt(p);
				while ((!v.isInitiallyCritical(p)) && (p!=v.getParentInsideAt(p)))
				{
					v.getVoxelLocFromIndex(p,&x,&y,&z);
					glVertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
					p=v.getParentInsideAt(p);
				}
				v.getVoxelLocFromIndex(p,&x,&y,&z);
				glVertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
				glEnd();
			}
		  else if ((activeOrder==3))
			{
				glBegin(GL_LINE_STRIP);
				p=i;
				v.getVoxelLocFromIndex(p,&x,&y,&z);
				glVertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
				p=v.getParentOutsideAt(p);
				while ((!v.isInitiallyCritical(p)) && (p!=v.getParentOutsideAt(p)))
				{
					v.getVoxelLocFromIndex(p,&x,&y,&z);
					glVertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
					p=v.getParentOutsideAt(p);
				}
				v.getVoxelLocFromIndex(p,&x,&y,&z);
				glVertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
				glEnd();
			}
		}
	}
	
	glPopMatrix();
}*/

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

  //draw the object
  /*if (showVertices)
    {
      glDisable(GL_LIGHTING);
      glColor3f(0.3,0.4,0.7);
      glPolygonOffset(-2,-2);
      glPointSize(2.0);
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      draw_critical_points();
    }
  if (showEdges)
    {
      glDisable(GL_LIGHTING);
      glColor3f(0.8,1.0,0.8);
      glPolygonOffset(-1,-1);
      glLineWidth(1.0);
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      draw_critical_paths();
      }*/
  if (showFaces)
    {
      glEnable(GL_LIGHTING);
      set_material_properties(0.8,0.4,0.6);
      glPolygonOffset(0,0);
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      draw_volume();
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
	case MENU_ISO_ORDER:
		activeOrder=0;
		break;
	case MENU_INITIAL_ORDER:
		activeOrder=1;
		break;
	case MENU_CARVEDINSIDE_ORDER:
		activeOrder=2;
		break;
	case MENU_CARVEDOUTSIDE_ORDER:
		activeOrder=3;
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
	case '1':
		orderIndices[activeOrder] += 1;
		break;
	case '2':
		orderIndices[activeOrder] += 10;
		break;
	case '3':
		orderIndices[activeOrder] += 100;
		break;
	case '4':
		orderIndices[activeOrder] += 1000;
		break;
	case '5':
		orderIndices[activeOrder] += 10000;
		break;
	case '!':
		orderIndices[activeOrder] -= 1;
		break;
	case '@':
		orderIndices[activeOrder] -= 10;
		break;
	case '#':
		orderIndices[activeOrder] -= 100;
		break;
	case '$':
		orderIndices[activeOrder] -= 1000;
		break;
	case '%':
		orderIndices[activeOrder] -= 10000;
		break;
	case 'l':
		minLifeSpan++;
		break;
	case 'L':
		minLifeSpan--;
		break;
		
    default:  break;
  }
  if (displayIsovalue) cout << orderIndices[activeOrder] << '\n';
	glutPostRedisplay();
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
  glutAddMenuEntry("toggle faces",MENU_TRIANGLES_TOGGLE);
  glutAddMenuEntry("toggle paths",MENU_LINES_TOGGLE);
  glutAddMenuEntry("toggle criticals",MENU_POINTS_TOGGLE);
  glutAddMenuEntry("order by isovalue",MENU_ISO_ORDER);
  glutAddMenuEntry("order by initial",MENU_INITIAL_ORDER);
  glutAddMenuEntry("order by carved inside",MENU_CARVEDINSIDE_ORDER);
  glutAddMenuEntry("order by carved outside",MENU_CARVEDOUTSIDE_ORDER);
  glutSetMenu(menuID);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
  init_glut(&argc, argv);
  init_opengl();

  if (argc>1) set_up_volume(argv[1]);
  
  if (argc>2)
  {
	  if (strstr(argv[2],"s")) displayIsovalue=0;
	  if (strstr(argv[2],"v")) displayIsovalue=1;
  }

  glutMainLoop();

  return 0;
}

