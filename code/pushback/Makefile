
CC = g++
OPT = -g
LIBOPT = -lm -lc 
BINDIR = .
VOLFILLOBJ = volfill/ChunkAllocator.o volfill/ChunkLink.o volfill/OccGrid.o volfill/OccGridRLE.o
GLFLAGS = 

include Makefile-platformspecific

all : fixall volumeviewer

fixall : fixall.o bitc.o volume.o
	$(CC) $(OPT) -o fixall fixall.o bitc.o volume.o $(LIBOPT)

bitc.o : bitc.c bitc.h
	$(CC) $(OPT) -c bitc.c

volume.o : shortvolume.cpp shortvolume.h bitc.h
	$(CC) $(OPT) -o volume.o -c shortvolume.cpp

fixall.o : fixall.cpp
	$(CC) $(OPT) -c fixall.cpp

volumeviewer: volumeviewer.o Vector3D.o bitc.o volume.o
	$(CC) $(OPT) $(GLFLAGS) -o volumeviewer volumeviewer.o Vector3D.o bitc.o volume.o $(LIBOPT)

createvolume: createvolume.o bitc.o volume.o
	$(CC) $(OPT) -o createvolume createvolume.o bitc.o volume.o $(LIBOPT)


clean :
	rm *.o fixall volumeviewer
cleanall :
	rm *.o fixall volumeviewer
