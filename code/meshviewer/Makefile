
CC = g++
OPTPS = 
GLFLAGS = 
include ../Makefile-platformspecific
LIBOPT = -lm 
BINDIR = .
VOLFILLOBJ = volfill/ChunkAllocator.o volfill/ChunkLink.o volfill/OccGrid.o volfill/OccGridRLE.o
VOLUMEOBJ = octree.o $(VOLFILLOBJ)
VOLUMEOBJ = 
OPT = $(OPTPS) -O3

all : mc betti onecompt convertt meshviewer removesmallcomps reorient

reorient: reorient.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o reorient reorient.cpp TriangleMesh.o Vector3D.o

removesmallcomps: removesmallcomps.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o removesmallcomps removesmallcomps.cpp TriangleMesh.o Vector3D.o

onecompt: splitcomp.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o onecompt splitcomp.cpp TriangleMesh.o Vector3D.o

meshviewer: meshviewer.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) $(GLFLAGS) -o meshviewer meshviewer.cpp TriangleMesh.o Vector3D.o $(LIBOPT)

betti : betti.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o betti betti.cpp TriangleMesh.o Vector3D.o

mc : mc.o ../bitc.o ../MarchableVolume.o TriangleMesh.o Vector3D.o $(VOLUMEOBJ)
	$(CC) $(OPT) -o mc mc.o ../bitc.o ../MarchableVolume.o TriangleMesh.o Vector3D.o $(VOLUMEOBJ) $(LIBOPT)

convertt : convertt.o TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o convertt convertt.o TriangleMesh.o Vector3D.o $(LIBOPT)

t2pov : t2pov.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o t2pov t2pov.cpp TriangleMesh.o Vector3D.o $(LIBOPT)

bitc.o : bitc.c bitc.h
	$(CC) $(OPT) -c bitc.c

Vector3D.o : Vector3D.cpp Vector3D.h
	$(CC) $(OPT) -c Vector3D.cpp

mc.o : mc.c ../MarchableVolume.h ../bitc.h TriangleMesh.h
	$(CC) $(OPT) -o mc.o -c mc.c

TriangleMesh.o : TriangleMesh.cpp TriangleMesh.h Vector3D.h
	$(CC) $(OPT) -c TriangleMesh.cpp

convertt.o : convertt.cpp TriangleMesh.h
	$(CC) $(OPT) -c convertt.cpp

clean :
	rm *.o mc betti onecompt convertt meshviewer removesmallcomps
