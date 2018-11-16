
CC = g++
OPTPS = 
GLFLAGS = 
include Makefile-platformspecific
LIBOPT = -lm 
BINDIR = .
VOLFILLOBJ = volfill/ChunkAllocator.o volfill/ChunkLink.o volfill/OccGrid.o volfill/OccGridRLE.o
OPT = $(OPTPS)

all : fixtop trimunknown convert mc slice sliceoct convertoct fixtopoct betti onecomp onecompt convertt meshviewer gauss

topogauss: topogauss.cpp bitc.o MarchableVolume.o octree.o pqitem.o Vector3D.o $(VOLFILLOBJ)
	$(CC) $(OPT) -o topogauss topogauss.cpp bitc.o MarchableVolume.o octree.o pqitem.o Vector3D.o $(VOLFILLOBJ)

gauss: gauss.cpp bitc.o MarchableVolume.o octree.o pqitem.o Vector3D.o $(VOLFILLOBJ)
	$(CC) $(OPT) -o gauss gauss.cpp bitc.o MarchableVolume.o octree.o pqitem.o Vector3D.o $(VOLFILLOBJ)

onecompt: splitcomp.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o onecompt splitcomp.cpp TriangleMesh.o Vector3D.o

meshviewer: meshviewer.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) $(GLFLAGS) -o meshviewer meshviewer.cpp TriangleMesh.o Vector3D.o $(LIBOPT)

onecomp: octree.o octvolume.h octvolume.cpp bitc.o Vector3D.o pqitem.o $(VOLFILLOBJ)
	$(CC) $(OPT) -DFIXTOP -DONECOMP -o onecomp octvolume.cpp octree.o bitc.o Vector3D.o pqitem.o $(VOLFILLOBJ) $(LIBOPT)

volumeviewer : volumeviewer.cpp Vector3D.o bitc.o MarchableVolume.o octree.o $(VOLFILLOBJ)
	$(CC) $(OPT) $(GLFLAGS) -o volumeviewer volumeviewer.cpp Vector3D.o bitc.o MarchableVolume.o octree.o $(VOLFILLOBJ) $(LIBOPT)

comparevol : comparevol.cpp bitc.o MarchableVolume.o octree.o $(VOLFILLOBJ)
	$(CC) $(OPT) -o comparevol comparevol.cpp bitc.o MarchableVolume.o octree.o $(VOLFILLOBJ) $(LIBOPT)

subsample : subsample.cpp bitc.o
	$(CC) $(OPT) -o subsample subsample.cpp bitc.o

betti : betti.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o betti betti.cpp TriangleMesh.o Vector3D.o

fixtopoct : octree.o octvolume.h octvolume.cpp bitc.o pqitem.o $(VOLFILLOBJ)
	$(CC) $(OPT) -DFIXTOP -o fixtopoct octvolume.cpp octree.o bitc.o pqitem.o Vector3D.o $(VOLFILLOBJ) $(LIBOPT)

convertoct : octree.h octree.cpp bitc.o Vector3D.o pqitem.o $(VOLFILLOBJ)
	$(CC) $(OPT) -DCONVERT -o convertoct octree.cpp bitc.o Vector3D.o pqitem.o $(VOLFILLOBJ) $(LIBOPT)

sliceoct : octree.o octvolume.h octvolume.cpp bitc.o pqitem.o $(VOLFILLOBJ)
	$(CC) $(OPT) -DSLICE -o sliceoct octvolume.cpp octree.o bitc.o pqitem.o Vector3D.o $(VOLFILLOBJ) $(LIBOPT)

repairvol : repairvol.cpp bitc.o MarchableVolume.o octree.o $(VOLFILLOBJ)
	$(CC) $(OPT) -o repairvol repairvol.cpp bitc.o MarchableVolume.o octree.o $(VOLFILLOBJ) $(LIBOPT)

fixtop : fixtop.o bitc.o volume.o $(VOLFILLOBJ)
	$(CC) $(OPT) -o fixtop fixtop.o bitc.o volume.o $(VOLFILLOBJ) $(LIBOPT)

trimunknown : trimunknown.o bitc.o volume.o $(VOLFILLOBJ)
	$(CC) $(OPT) -o trimunknown trimunknown.o bitc.o volume.o $(VOLFILLOBJ) $(LIBOPT)

convert : convert.o bitc.o volume.o $(VOLFILLOBJ)
	$(CC) $(OPT) -o convert convert.o bitc.o volume.o $(VOLFILLOBJ) $(LIBOPT)

slice : slice.o bitc.o MarchableVolume.o octree.o Vector3D.o $(VOLFILLOBJ)
	$(CC) $(OPT) -o slice slice.o bitc.o MarchableVolume.o octree.o Vector3D.o $(VOLFILLOBJ) $(LIBOPT)

mc : mc.o bitc.o MarchableVolume.o TriangleMesh.o Vector3D.o octree.o pqitem.o $(VOLFILLOBJ)
	$(CC) $(OPT) -o mc mc.o bitc.o MarchableVolume.o TriangleMesh.o Vector3D.o octree.o pqitem.o $(VOLFILLOBJ) $(LIBOPT)

convertt : convertt.o TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o convertt convertt.o TriangleMesh.o Vector3D.o $(LIBOPT)

t2pov : t2pov.cpp TriangleMesh.o Vector3D.o
	$(CC) $(OPT) -o t2pov t2pov.cpp TriangleMesh.o Vector3D.o $(LIBOPT)

bitc.o : bitc.c bitc.h
	$(CC) $(OPT) -c bitc.c

Vector3D.o : Vector3D.cpp Vector3D.h
	$(CC) $(OPT) -c Vector3D.cpp

volume.o : shortvolume.cpp shortvolume.h bitc.h volfill/OccGridRLE.h Makefile
	$(CC) $(OPT) -o volume.o -c shortvolume.cpp

MarchableVolume.o : MarchableVolume.cpp MarchableVolume.h volfill/OccGridRLE.h octree.h
	$(CC) $(OPT) -c MarchableVolume.cpp

fixtop.o : fixtop.cpp shortvolume.h
	$(CC) $(OPT) -c fixtop.cpp

trimunknown.o : trimunknown.cpp volume.h
	$(CC) $(OPT) -c trimunknown.cpp

convert.o : convert.cpp shortvolume.h
	$(CC) $(OPT) -c convert.cpp

slice.o : slice.cpp MarchableVolume.h
	$(CC) $(OPT) -c slice.cpp

volfill/ChunkAllocator.o : volfill/ChunkAllocator.cc volfill/ChunkAllocator.h volfill/defines.h volfill/vrip.h
volfill/ChunkLink.o : volfill/ChunkLink.cc volfill/ChunkLink.h volfill/defines.h volfill/vrip.h
volfill/OccGrid.o : volfill/OccGrid.cc volfill/OccGrid.h volfill/defines.h volfill/vrip.h
volfill/OccGridRLE.o : volfill/OccGridRLE.cc volfill/OccGridRLE.h volfill/defines.h volfill/vrip.h

mc.o : mc.c MarchableVolume.h bitc.h TriangleMesh.h
	$(CC) $(OPT) -o mc.o -c mc.c

TriangleMesh.o : TriangleMesh.cpp TriangleMesh.h Vector3D.h
	$(CC) $(OPT) -c TriangleMesh.cpp

convertt.o : convertt.cpp TriangleMesh.h
	$(CC) $(OPT) -c convertt.cpp

octree.o : octree.cpp octree.h Vector3D.h bitc.h pqitem.h volfill/OccGridRLE.h
	$(CC) $(OPT) -c octree.cpp

pqitem.o : pqitem.cpp pqitem.h
	$(CC) $(OPT) -c pqitem.cpp

clean :
	rm *.o fixtop trimunknown convert mc slice sliceoct convertoct fixtopoct betti onecomp onecompt convertt meshviewer gauss
cleanall : clean
	rm *.o volfill/*.o
