
CC = g++
OPTPS = 
GLFLAGS = 
include Makefile-platformspecific
LIBOPT = -lm 
BINDIR = .
VOLFILLOBJ = volfill/ChunkAllocator.o volfill/ChunkLink.o volfill/OccGrid.o volfill/OccGridRLE.o
VOLUMEOBJ = octree.o $(VOLFILLOBJ)
VOLUMEOBJ = 
OPT = $(OPTPS) -O3

all : fixtopo traceseeds volumerotate fixtop fixtop2D+ slicer downsample contoursearch convert combinev2 maketrees

maketrees : maketrees.cpp Volume2DplusT.h Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ)
	$(CC) $(OPT) -o maketrees maketrees.cpp Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ) $(LIBOPT)

combinev2 : combinev2.cpp bitc.o
	$(CC) $(OPT) -o combinev2 combinev2.cpp bitc.o

fixtopo : fixtopo.cpp
	$(CC) $(OPT) -o fixtopo fixtopo.cpp

traceseeds : traceseeds.cpp volume.o Volume2DplusT.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ)
	$(CC) $(OPT) -o traceseeds traceseeds.cpp volume.o Volume2DplusT.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ) $(LIBOPT)

volumerotate : volumerotate.cpp Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ)
	$(CC) $(OPT) -o volumerotate volumerotate.cpp Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ) $(LIBOPT)

compare : shortvolume.cpp shortvolume.h carvecomp.o TopoCheck.o priorityQueue.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ)
	$(CC) $(OPT) -D COMPARE -o compare shortvolume.cpp carvecomp.o TopoCheck.o priorityQueue.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ) $(LIBOPT)

fixtop : shortvolume.cpp shortvolume.h carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ)
	$(CC) $(OPT) -D FIXTOP -o fixtop shortvolume.cpp carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ) $(LIBOPT)

fixtop2D+ : Volume2DplusT.cpp Volume2DplusT.h shortvolume.h volume.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ)
	$(CC) $(OPT) -D FIXTOP2DP -o fixtop2D+ Volume2DplusT.cpp volume.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ) $(LIBOPT)

slicer : slicer.cpp Vector3D.o bitc.o ContourTree.o MarchableVolume.o Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o $(VOLUMEOBJ)
	$(CC) $(OPT) $(GLFLAGS) -o slicer slicer.cpp Vector3D.o bitc.o ContourTree.o MarchableVolume.o Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o $(VOLUMEOBJ) $(LIBOPT)

downsample : downsample.cpp Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ)
	$(CC) $(OPT) -o downsample downsample.cpp Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ) $(LIBOPT)

volumeviewer : volumeviewer.cpp Vector3D.o bitc.o Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o MarchableVolume.o $(VOLUMEOBJ)
	$(CC) $(OPT) $(GLFLAGS) -o volumeviewer volumeviewer.cpp Vector3D.o bitc.o Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o MarchableVolume.o $(VOLUMEOBJ) $(LIBOPT)

contoursearch : contoursearch.cpp ContourQuery.h ContourQuery.o Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ)
	$(CC) $(OPT) -o contoursearch contoursearch.cpp ContourQuery.o Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o ContourTree.o Vector3D.o bitc.o MarchableVolume.o $(VOLUMEOBJ) $(LIBOPT)

convert : convert.cpp MarchableVolume.o bitc.o $(VOLUMEOBJ)
	$(CC) $(OPT) -o convert convert.cpp MarchableVolume.o bitc.o $(VOLUMEOBJ) $(LIBOPT)

ContourTree.o: ContourTree.cpp ContourTree.h
	$(CC) $(OPT) -o ContourTree.o -c ContourTree.cpp

priorityQueue.o: priorityQueue.cpp priorityQueue.h
	$(CC) $(OPT) -c priorityQueue.cpp

TopoCheck.o: TopoCheck.cpp TopoCheck.h
	$(CC) $(OPT) -c TopoCheck.cpp

carvecomp.o: carvecomp.cpp carvecomp.h shortvolume.h priorityQueue.h
	$(CC) $(OPT) -c carvecomp.cpp

bitc.o : bitc.c bitc.h
	$(CC) $(OPT) -c bitc.c

MarchableVolume.o : MarchableVolume.cpp MarchableVolume.h
	$(CC) $(OPT) -c MarchableVolume.cpp

volume.o : shortvolume.cpp shortvolume.h
	$(CC) $(OPT) -o volume.o -c shortvolume.cpp

Volume2DplusT.o : Volume2DplusT.cpp Volume2DplusT.h shortvolume.h
	$(CC) $(OPT) -o Volume2DplusT.o -c Volume2DplusT.cpp

ContourQuery.o : ContourQuery.cpp ContourQuery.h Volume2DplusT.h
	$(CC) $(OPT) -o ContourQuery.o -c ContourQuery.cpp

volfill/ChunkAllocator.o : volfill/ChunkAllocator.cc volfill/ChunkAllocator.h volfill/defines.h volfill/vrip.h
volfill/ChunkLink.o : volfill/ChunkLink.cc volfill/ChunkLink.h volfill/defines.h volfill/vrip.h
volfill/OccGrid.o : volfill/OccGrid.cc volfill/OccGrid.h volfill/defines.h volfill/vrip.h
volfill/OccGridRLE.o : volfill/OccGridRLE.cc volfill/OccGridRLE.h volfill/defines.h volfill/vrip.h

Vector3D.o : Vector3D.cpp Vector3D.h
	$(CC) $(OPT) -c Vector3D.cpp

octree.o : octree.cpp octree.h Vector3D.h bitc.h pqitem.h volfill/OccGridRLE.h
	$(CC) $(OPT) -c octree.cpp

pqitem.o : pqitem.cpp pqitem.h
	$(CC) $(OPT) -c pqitem.cpp

clean :
	rm *.o volumeviewer downsample slicer fixtop compare fixtop2D+ convert
cleanall : clean
	rm volfill/*.o
