
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

all : changesigns sliceviewer v2v4d downsample4D1 downsample4D2 downsample4D3 fixtopo4D make4D make3D make2D volumeslicer fixtop3D+ fixtopo traceseeds volumerotate fixtop fixtop2D+ slicer downsample contoursearch convert combinev2 maketrees

fixtop3D+all : fixtop3D+so fixtop3D+fo fixtop3D+co fixtop3D+si fixtop3D+fi fixtop3D+ci

FIXTOP3DPFILES = Volume3DplusT.cpp carvecomp4D.o TopoCheck4D.o TopoCheck.o priorityQueue.o Vector3D.o bitc.o

fixtop3D+so : Volume3DplusT.h $(FIXTOP3DPFILES)
	$(CC) $(OPT) -D FIXTOP3DP -D BYSLICES -o fixtop3D+so $(FIXTOP3DPFILES)

fixtop3D+fo : Volume3DplusT.h $(FIXTOP3DPFILES)
	$(CC) $(OPT) -D FIXTOP3DP -D FULLCHECK -o fixtop3D+fo $(FIXTOP3DPFILES)

fixtop3D+co : Volume3DplusT.h $(FIXTOP3DPFILES)
	$(CC) $(OPT) -D FIXTOP3DP -o fixtop3D+co $(FIXTOP3DPFILES)

fixtop3D+si : Volume3DplusT.h $(FIXTOP3DPFILES)
	$(CC) $(OPT) -D FIXTOP3DP -D BYSLICES -D FROMINSIDE -o fixtop3D+si $(FIXTOP3DPFILES)

fixtop3D+fi : Volume3DplusT.h $(FIXTOP3DPFILES)
	$(CC) $(OPT) -D FIXTOP3DP -D FULLCHECK -D FROMINSIDE -o fixtop3D+fi $(FIXTOP3DPFILES)

fixtop3D+ci : Volume3DplusT.h $(FIXTOP3DPFILES)
	$(CC) $(OPT) -D FIXTOP3DP -D FROMINSIDE -o fixtop3D+ci $(FIXTOP3DPFILES)

fixtop3D+ : Volume3DplusT.h $(FIXTOP3DPFILES)
	$(CC) $(OPT) -D FIXTOP3DP -o fixtop3D+ $(FIXTOP3DPFILES)

changesigns : changesigns.cpp
	$(CC) $(OPT) -o changesigns changesigns.cpp

v2v4d : v2v4d.cpp
	$(CC) $(OPT) -o v2v4d v2v4d.cpp

project4D3D : project4D3D.cpp
	$(CC) $(OPT) -o project4D3D project4D3D.cpp

downsample4D1 : subsample1.cpp
	$(CC) $(OPT) -o downsample4D1 subsample1.cpp

downsample4D2 : subsample2.cpp
	$(CC) $(OPT) -o downsample4D2 subsample2.cpp

downsample4D3 : subsample3.cpp
	$(CC) $(OPT) -o downsample4D3 subsample3.cpp

fixtopo4D : fixtopo4D.cpp
	$(CC) $(OPT) -o fixtopo4D fixtopo4D.cpp

make4D : make4D.cpp
	$(CC) $(OPT) -o make4D make4D.cpp $(LIBOPT)

make3D : make3D.cpp
	$(CC) $(OPT) -o make3D make3D.cpp $(LIBOPT)

make2D : make2D.cpp
	$(CC) $(OPT) -o make2D make2D.cpp $(LIBOPT)

volumeviewer4D : volumeviewer4D.cpp Volume3DplusT.o carvecomp4D.o TopoCheck4D.o TopoCheck.o priorityQueue.o Vector3D.o bitc.o
	$(CC) $(OPT) $(GLFLAGS) -o volumeviewer4D volumeviewer4D.cpp Volume3DplusT.o carvecomp4D.o TopoCheck4D.o TopoCheck.o priorityQueue.o Vector3D.o bitc.o

volumeslicer : volumeslicer.cpp Volume3DplusT.o carvecomp4D.o TopoCheck4D.o TopoCheck.o priorityQueue.o Vector3D.o bitc.o
	$(CC) $(OPT) $(GLFLAGS) -o volumeslicer volumeslicer.cpp Volume3DplusT.o carvecomp4D.o TopoCheck4D.o TopoCheck.o priorityQueue.o Vector3D.o bitc.o

Volume3DplusT.o : Volume3DplusT.cpp Volume3DplusT.h Makefile
	$(CC) $(OPT) -o Volume3DplusT.o -c Volume3DplusT.cpp

carvecomp4D.o : carvecomp4D.cpp carvecomp4D.h
	$(CC) $(OPT) -o carvecomp4D.o -c carvecomp4D.cpp

TopoCheck4D.o : TopoCheck4D.cpp TopoCheck4D.h
	$(CC) $(OPT) -o TopoCheck4D.o -c TopoCheck4D.cpp

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

sliceviewer : sliceviewer.cpp Vector3D.o bitc.o ContourTree.o MarchableVolume.o Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o $(VOLUMEOBJ)
	$(CC) $(OPT) $(GLFLAGS) -o sliceviewer sliceviewer.cpp Vector3D.o bitc.o ContourTree.o MarchableVolume.o Volume2DplusT.o volume.o carvecomp.o TopoCheck.o priorityQueue.o $(VOLUMEOBJ) $(LIBOPT)

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
	rm *.o sliceviewer volumeviewer4D v2v4d downsample4D1 downsample4D2 downsample4D3 fixtopo4D make4D make3D make2D volumeslicer fixtop3D+ volumeviewer fixtopo downsample slicer fixtop compare fixtop2D+ convert
cleanall : clean
	rm volfill/*.o
