CC = g++

CFLAGS = -framework OpenGL -framework GLUT -framework Foundation

default: all

all: meshviewer

meshviewer: TriangleMesh.o Vector3D.o meshviewer.cpp
	$(CC) $(CFLAGS) -o meshviewer meshviewer.cpp TriangleMesh.o Vector3D.o
	
convertt: TriangleMesh.o Vector3D.o

clean :
	rm -f *.o core meshviewer convertt

veryclean: clean
	rm -f *~


