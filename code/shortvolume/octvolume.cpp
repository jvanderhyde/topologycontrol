/*
 *  octvolume.cpp
 *  
 *
 *  Created by James Vanderhyde on Wed Jun 18 2003.
 *
 */

#include <string.h>
#include <stdio.h>
#include <fstream.h>
//#include <iostream.h>

#include <assert.h>
#include <time.h>

#include "octvolume.h"

pqitem::pqitem () : priority(0),x(0),y(0),z(0) {}
pqitem::pqitem ( float p, int xx, int yy, int zz ) : priority(p),x(xx),y(yy),z(zz) {}

pqitem::operator float()
{
    return priority;
}

ostream& operator<< (ostream& out, const pqitem& i)
{
    out << '(' << i.x << ',' << i.y << ',' << i.z << ')' << ':' << i.priority;
    return out;
}

volume::volume(int features,int p_fixStyle,int resolution)
{
    dataroot=NULL;
    size[0]=size[1]=size[2]=0;
    treeHeight=0;
    highestResolutionLevel=resolution; //default 100;
    //this is larger than any possible value because each coordinate must be addressable by an int and an int has 32 bits.
    topoinfo=NULL;
    animationOn=0;
    frameNumber=0;
    innerBoundary=NULL;
    outerBoundary=NULL;
    alreadyCarvedNegative=0;
    numFeatures=features; //default 0
    innerForLater=new pqvector();
    outerForLater=new pqvector();
    fixStyle=p_fixStyle; //default 0; that is, program decides whether to fix inside or outside.
    extractedSlice1=extractedSlice2=NULL;
    extractedSlice1z=extractedSlice2z=-1;
    extractedSlice1Age=extractedSlice2Age=2;
}

volume::~volume()
{
    if (dataroot) delete dataroot;
    if (topoinfo) delete[] topoinfo;
    if (innerBoundary) delete innerBoundary;
    if (outerBoundary) delete outerBoundary;
    innerForLater->clear();
    delete innerForLater;
    outerForLater->clear();
    delete outerForLater;
    if (extractedSlice1) delete[] extractedSlice1;
    if (extractedSlice2) delete[] extractedSlice2;
}

int* volume::getSize()
{
    return size;
}

octree* volume::getDataroot()
{
    return dataroot;
}

int volume::getTreeHeight()
{
    return treeHeight;
}

octree* volume::getVoxel(int level,int x,int y,int z)
{
    return dataroot->getAncestorAtLevel(treeHeight,level,x,y,z);
}

octree* volume::getDeepestVoxel(int x,int y,int z)
{
    return dataroot->findDeepestAncestor(treeHeight,x,y,z);
}

void volume::deleteVoxel(int level,int x,int y,int z)
{
    octree* voxel=dataroot->getAncestorAtLevel(treeHeight,level,x,y,z);
    if (voxel != NULL)
    {
        if (level<treeHeight)
        {
            octree* parent=dataroot->getAncestorAtLevel(treeHeight,level+1,x,y,z);
            parent->setChild(octree::childIndex(level,x,y,z),NULL);
        }
        delete voxel;
    }
}

float volume::d(int x,int y,int z)
{
    if ((x<0) || (y<0) || (z<0) || (x>=size[0]) || (y>=size[1]) || (z>=size[2])) return fixStyle*BIGNUM;
    //return getDeepestVoxel(x,y,z)->getValue();
    if (z==extractedSlice1z) return extractedSlice1[y*size[0]+x];
    if (z==extractedSlice2z) return extractedSlice2[y*size[0]+x];
    float* buf;
    extractedSlice1Age++;
    extractedSlice2Age++;
    if (extractedSlice1Age>extractedSlice2Age)
    {
        //swap out 1
        if (!extractedSlice1) extractedSlice1=new float[size[1]*size[0]];
        buf=extractedSlice1;
        extractedSlice1z=z;
        extractedSlice1Age=0;
    }
    else
    {
        //swap out 2
        if (!extractedSlice2) extractedSlice2=new float[size[1]*size[0]];
        buf=extractedSlice2;
        extractedSlice2z=z;
        extractedSlice2Age=0;
    }
    extractSlice(z,buf);
    return buf[y*size[0]+x];
}

int volume::readTopoinfoFile()
{
    ifstream fin("topoinfo_vertex");
    if (!fin)
    {
        cerr << "Can't open topoinfo_vertex\n";
        return 1;
    }
    if (!topoinfo) topoinfo=new unsigned char[8*1024*1024]; // 2^26/8 = 2^23 = 8*1024*1024
    fin.read((char*)topoinfo,8*1024*1024);
    return 0;
}

void volume::extractSlice(int slice,octree** buf)
{
    int x,y,z,index;
    z=slice;
    index=0;
    for (y=0; y<size[1]; y++)
        for (x=0; x<size[0]; x++)
            buf[index++]=getDeepestVoxel(x,y,z);
}

void volume::extractSlice(int slice,float* buf)
{
    int x,y,z,index;
    z=slice;
    index=0;
    for (y=0; y<size[1]; y++)
        for (x=0; x<size[0]; x++)
            buf[index++]=getDeepestVoxel(x,y,z)->getValue();
}

void volume::extractSliceLevels(int slice,int* buf)
{
    int x,y,z,index;
    z=slice;
    index=0;
    for (y=0; y<size[1]; y++)
        for (x=0; x<size[0]; x++)
            buf[index++]=dataroot->levelOfDeepestAncestor(treeHeight,x,y,z);
}

//Swap the sign on every voxel in the volume
void volume::changeAllSigns()
{
    dataroot->changeSignRecursively();
}

//Creates every voxel on the given level and all levels above
void volume::fillInLevel(int level)
{
    //Tell octree to fill in all levels up to given level
    dataroot->fillInLevels(treeHeight,level);
}

//Divides every supervoxel on the given level that wasn't carved away.
// This works the same as fillLevel(level+1) in some cases (e.g. if we started at treeHeight),
// but it's not the same at higher resolutions because if the whole level we start with
// isn't filled in, then constructBoundary() won't necessarily create a topological sphere.
//The main loop invariant for the level loop is that after a level has been divided (this function),
// if a voxel is null, it is carved.
void volume::divideLevel(int level)
{
    cout << " Dividing level..."; cout.flush();
    int x,y,z;
    int stepSize=1<<level;

    //It seems like this will be unnecessarily slow at the higher resolutions.
    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
            {
                octree* supervoxel=getVoxel(level,x,y,z);
                if ((supervoxel != NULL) && (!supervoxel->getFlag(OTFL_CARVED)))
                {
                    int c;
                    octree* child;
                    if (supervoxel->getChildren()==NULL) supervoxel->initChildren(supervoxel->getValue());
                    else for (c=0; c<8; c++)
                    {
                        child=supervoxel->getChild(c);
                        if (child->getFlag(OTFL_CANTMERGE))
                            child->setFlag(OTFL_VALID);
                    }
                }
                //No supervoxel with children was ever carved.
                //Therefore on the new level, any neighbors that should be considered carved will be null.
            }
    cout << "done.\n";
}

void volume::checkLevel(int level)
{
    int x,y,z;
    int stepSize=1<<level;

    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
            {
                octree* supervoxel=getVoxel(level,x,y,z);
                if ((supervoxel != NULL) && (!supervoxel->getFlag(OTFL_CARVED)))
                {
                    if (supervoxel->getChildren()==NULL) cout << "Problem!\n";
                    assert(supervoxel->getChildren()!=NULL);
                }
            }
}

//Adds appropriate children to outer boundary
void volume::boundaryOutsideCheck(int level,octree* supervoxel,int x,int y,int z,int nx,int ny,int nz,int outOfBounds)
{
    int cx,cy,cz,lx,ly,lz,ux,uy,uz;
    octree* neighbor;
    octree* childVoxel;
    float val,nval;
    int stepSize=1<<(level+1); //need to check level above given level

    lx=(nx<=0)?0:1;
    ly=(ny<=0)?0:1;
    lz=(nz<=0)?0:1;
    ux=(nx>=0)?2:1;
    uy=(ny>=0)?2:1;
    uz=(nz>=0)?2:1;
    
    nval=0;
    if (outOfBounds) nval=BIGNUM;
    else
    {
        neighbor=getVoxel(level+1,x+nx*stepSize,y+ny*stepSize,z+nz*stepSize);
        if (neighbor==NULL) neighbor=getDeepestVoxel(x+nx*stepSize,y+ny*stepSize,z+nz*stepSize);
        if (neighbor->getFlag(OTFL_CARVED)) nval=neighbor->getValue();
    }
    if (nval>0)
        for (cz=lz; cz<uz; cz++) if (z+(cz<<level)<size[2])
            for (cy=ly; cy<uy; cy++) if (y+(cy<<level)<size[1])
                for (cx=lx; cx<ux; cx++) if (x+(cx<<level)<size[0])
                    if (!(childVoxel=supervoxel->getChild(octree::childIndex(cx,cy,cz)))->getFlag(OTFL_BOUNDARY))
                    {
                        val=childVoxel->getValue();
                        if ((val>0) || (!childVoxel->getFlag(OTFL_CANTMERGE)))
                        {
                            outerBoundary->push(pqitem(ABS(val),x+(cx<<level),y+(cy<<level),z+(cz<<level)));
                            childVoxel->setFlag(OTFL_BOUNDARY);
                            //cout << val << ',';
                        }
                    }
                        
}

//Constructs the starting outer boundary for the given level
void volume::constructBoundaryOutside(int level)
{
    int x,y,z,nx,ny,nz;
    int stepSize=1<<(level+1); //need to check level above given level
    int halfStepSize=1<<level;
    octree* supervoxel;

    if (!outerBoundary) outerBoundary=new maxqueue();

    //The boundary queue starts out empty when there is a single supervoxel.
    if (level==treeHeight) return;

    cout << " Constructing initial outer boundary..."; cout.flush();

    //The outer boundary starts at the point at inifinity,
    // which is assumed to be positive and already carved.
    // Therefore, all out-of-range neighbors are considered carved.

    //The loop is over the level above, since we need to examine the carved voxels on that level.
    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
            {
                //If the supervoxel is not null and not carved, then
                // we check each of its 6 neighbors (function boundaryOutsideCheck).
                //If the neighbor is out of range or null or carved and positive, then
                // we check each of the supervoxel's children that are adjacent to that neighbor.
                //Any of these children that are in range are on the boundary.
                supervoxel=getVoxel(level+1,x,y,z);
                if ((supervoxel != NULL) && (!supervoxel->getFlag(OTFL_CARVED)))
                {
                    boundaryOutsideCheck(level,supervoxel,x,y,z,-1,0,0,(x-stepSize<0));
                    boundaryOutsideCheck(level,supervoxel,x,y,z,0,-1,0,(y-stepSize<0));
                    boundaryOutsideCheck(level,supervoxel,x,y,z,0,0,-1,(z-stepSize<0));
                    boundaryOutsideCheck(level,supervoxel,x,y,z,1,0,0,(x+stepSize>=size[0]));
                    boundaryOutsideCheck(level,supervoxel,x,y,z,0,1,0,(y+stepSize>=size[1]));
                    boundaryOutsideCheck(level,supervoxel,x,y,z,0,0,1,(z+stepSize>=size[2]));
                    
                    //check for case that only half (or less) of the supervoxel is in bounds
                    //OK, so this causes a problem for the inside output.
                    if ((z+halfStepSize>=size[2]) || (y+halfStepSize>=size[1]) || (x+halfStepSize>=size[0]))
                        boundaryOutsideCheck(level,supervoxel,x,y,z,0,0,0,1);
                }
            }
    //cout << " added to outer boundary.\n";
    cout << "done.\n";
}

//Constructs the starting inner boundary for the given level
void volume::constructBoundaryInside(int level)
{
    int x,y,z;
    int stepSize=1<<(level+1); //need to check level above given level
    int cx,cy,cz,nx,ny,nz;
    octree* supervoxel;
    octree* neighbor;
    octree* childVoxel;
    float val;

    if (!innerBoundary) innerBoundary=new maxqueue();

    //The boundary queue starts out empty when there is a single supervoxel.
    if (level==treeHeight) return;

    cout << " Constructing initial inner boundary..."; cout.flush();

    //The boundary needs to start with a carved point,
    // so for the inner boundary we carve the global minimum
    // if we don't lose info and if it's negative.
    //We can't just mark this voxel as the starting boundary because
    // changing from the empty set to one voxel is considered a topological change.
    if (!alreadyCarvedNegative)
    {
        octree* voxel;
        float globalMin=BIGNUM;
        int mx,my,mz;
        int n,num;
        pqitem neighborsQ[26];
        octree* neighborsO[26];
        stepSize=1<<level;
        for (z=0; z<size[2]; z+=stepSize)
            for (y=0; y<size[1]; y+=stepSize)
                for (x=0; x<size[0]; x+=stepSize)
                    if ((voxel=getVoxel(level,x,y,z)) != NULL)
                        if (((val=voxel->getValue())<globalMin) && ((!voxel->getFlag(OTFL_CANTMERGE)) || (level==0)))
                        {
                            globalMin=val;
                            mx=x; my=y; mz=z;
                        }
        if (globalMin<0)
        {
            //Carve this voxel
            voxel=getVoxel(level,mx,my,mz);
            carveVoxelInside(voxel,level,mx,my,mz,neighborsO,neighborsQ);
            alreadyCarvedNegative=1;
            //printVolume(level,cout);
            //printBoundary(level,cout);
            cout << "done.\n";
            cout << " " << 1 << " voxel carved from inside.\n";
        }
        else
        {
            cout << "done.\n";
        }
        return;
    }

    //The loop is over the level above, since we need to examine the carved voxels on that level.
    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
            {
                //If the supervoxel is not null and not carved, then
                // we check each of its children.
                //If the child is in range, then
                // we check all of the supervoxel's 26 neighbors that are adjacent to that child.
                //If any of these neighbors are (in bounds and) null or carved, and negative, then
                // the child is on the boundary.
                supervoxel=getVoxel(level+1,x,y,z);
                if ((supervoxel != NULL) && (!supervoxel->getFlag(OTFL_CARVED)))
                    for (cz=0; cz<2; cz++) if (z+(cz<<level)<size[2])
                        for (cy=0; cy<2; cy++) if (y+(cy<<level)<size[1])
                            for (cx=0; cx<2; cx++) if (x+(cx<<level)<size[0])
                            {
                                //cout << supervoxel->getValue() << " " << supervoxel->getFlag(OTFL_VALID);
                                childVoxel=supervoxel->getChild(octree::childIndex(cx,cy,cz));
                                assert(childVoxel != NULL);
                                val=childVoxel->getValue();
                                //cout << '.'; cout.flush();
                                if ((val<0) || (!childVoxel->getFlag(OTFL_CANTMERGE)))
                                    if (!childVoxel->getFlag(OTFL_BOUNDARY))
                                        for (nz=cz-1; nz<cz+1; nz++) if ((z+nz*stepSize>=0) && (z+nz*stepSize<size[2]))
                                            for (ny=cy-1; ny<cy+1; ny++) if ((y+ny*stepSize>=0) && (y+ny*stepSize<size[1]))
                                                for (nx=cx-1; nx<cx+1; nx++) if ((x+nx*stepSize>=0) && (x+nx*stepSize<size[0]))
                                                    if (((neighbor=getVoxel(level+1,x+nx*stepSize,y+ny*stepSize,z+nz*stepSize)) == NULL) || (neighbor->getFlag(OTFL_CARVED)))
                                                    {
                                                        if (neighbor==NULL) neighbor=getDeepestVoxel(x+nx*stepSize,y+ny*stepSize,z+nz*stepSize);
                                                        if (neighbor->getValue()<0)
                                                        {
                                                            octree* testvoxel=getVoxel(level,x+(cx<<level),y+(cy<<level),z+(cz<<level));
                                                            assert(testvoxel != NULL);
                                                            innerBoundary->push(pqitem(ABS(val),x+(cx<<level),y+(cy<<level),z+(cz<<level)));
                                                            //cout << "push(" << outerBoundary->size() << "): "; printBoundary(level,cout);
                                                            childVoxel->setFlag(OTFL_BOUNDARY);
                                                            //cout << val << ',';
                                                        }
                                                    }
                            }
            }
    //cout << " added to inner boundary.\n";
    cout << "done.\n"; cout.flush();
}

void volume::printBoundary(int level,ostream& out)
{
    int x,y,z;
    int stepSize=1<<level;
    octree* voxel;

    if (innerBoundary)
    {
        cout << '[' << innerBoundary->size() << ']';
        for (z=0; z<size[2]; z+=stepSize)
            for (y=0; y<size[1]; y+=stepSize)
                for (x=0; x<size[0]; x+=stepSize)
                {
                    voxel=getVoxel(level,x,y,z);
                    if (voxel != NULL)
                        if (voxel->getFlag(OTFL_BOUNDARY))
                        {
                            cout << ' ';
                            cout << pqitem(voxel->getValue(),x>>level,y>>level,z>>level);
                            //cout << pqitem(voxel->getValue(),x,y,z);
                            if (voxel->getFlag(OTFL_CANTMERGE))
                                cout << '*';
                            else cout << ' ';
                        }
                }
        cout << '\n';
    }
}

//Returns the number of non-null, in-range neighbors around the given voxel.
// The neighbors are stored in both neighborsQ and neighborsO.
int volume::getNeighbors26(int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ)
{
    int n=0;
    int stepSize=1<<level;
    int ox,oy,oz;
    for (oz=-stepSize; oz<=stepSize; oz+=stepSize)
        if ((z+oz>=0) && (z+oz<size[2]))
            for (oy=-stepSize; oy<=stepSize; oy+=stepSize)
                if ((y+oy>=0) && (y+oy<size[1]))
                    for (ox=-stepSize; ox<=stepSize; ox+=stepSize)
                        if ((x+ox>=0) && (x+ox<size[0]))
                            if ((oz!=0) || (oy!=0) || (ox!=0))
                            {
                                neighborsO[n]=getVoxel(level,x+ox,y+oy,z+oz);
                                if (neighborsO[n] != NULL)
                                {
                                    neighborsQ[n]=pqitem(neighborsO[n]->getValue(),x+ox,y+oy,z+oz);
                                    n++;
                                }
                            }
    return n;
}

//Returns the number of non-null, in-range neighbors around the given voxel.
// The neighbors are stored in both neighborsQ and neighborsO.
int volume::getNeighbors6(int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ)
{
    int n=0;
    int stepSize=1<<level;
    if (x-stepSize>=0)
    {
        neighborsO[n]=getVoxel(level,x-stepSize,y,z);
        if (neighborsO[n] != NULL)
        {
            neighborsQ[n]=pqitem(neighborsO[n]->getValue(),x-stepSize,y,z);
            n++;
        }
    }
    if (x<size[0]-stepSize)
    {
        neighborsO[n]=getVoxel(level,x+stepSize,y,z);
        if (neighborsO[n] != NULL)
        {
            neighborsQ[n]=pqitem(neighborsO[n]->getValue(),x+stepSize,y,z);
            n++;
        }
    }
    if (y-stepSize>=0)
    {
        neighborsO[n]=getVoxel(level,x,y-stepSize,z);
        if (neighborsO[n] != NULL)
        {
            neighborsQ[n]=pqitem(neighborsO[n]->getValue(),x,y-stepSize,z);
            n++;
        }
    }
    if (y<size[1]-stepSize)
    {
        neighborsO[n]=getVoxel(level,x,y+stepSize,z);
        if (neighborsO[n] != NULL)
        {
            neighborsQ[n]=pqitem(neighborsO[n]->getValue(),x,y+stepSize,z);
            n++;
        }
    }
    if (z-stepSize>=0)
    {
        neighborsO[n]=getVoxel(level,x,y,z-stepSize);
        if (neighborsO[n] != NULL)
        {
            neighborsQ[n]=pqitem(neighborsO[n]->getValue(),x,y,z-stepSize);
            n++;
        }
    }
    if (z<size[2]-stepSize)
    {
        neighborsO[n]=getVoxel(level,x,y,z+stepSize);
        if (neighborsO[n] != NULL)
        {
            neighborsQ[n]=pqitem(neighborsO[n]->getValue(),x,y,z+stepSize);
            n++;
        }
    }
    return n;
}

//Calculates the distance from the isosurface for every non-carved in-range voxel on the given level.
void volume::calcDistances(int level)
{
    cout << " Calculating distance function..."; cout.flush();
    minqueue pqabs;
    
    int x,y,z;
    int stepSize=1<<level;
    int n,num;
    pqitem neighborsQ[6];
    octree* neighborsO[6];
    octree* voxel;
    pqitem top;

    //check all valid voxels
    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
            {
                voxel=getVoxel(level,x,y,z);
                if ((voxel != NULL) && (!voxel->getFlag(OTFL_CARVED)) && (voxel->getFlag(OTFL_VALID)))
                {
                    //push neighbors onto absolute distance queue
                    num=getNeighbors6(level,x,y,z,neighborsO,neighborsQ);
                    for (n=0; n<num; n++)
                    {
                        if ((!neighborsO[n]->getFlag(OTFL_CARVED)) &&
                            (!neighborsO[n]->getFlag(OTFL_VALID)) &&
                            (!neighborsO[n]->getFlag(OTFL_QUEUED)))
                        {
                            neighborsQ[n].priority=ABS(voxel->getValue())+stepSize;
                            pqabs.push(neighborsQ[n]);
                            neighborsO[n]->setFlag(OTFL_QUEUED);
                            //cout << '(' << neighborsQ[n].x << ',' << neighborsQ[n].y << ',' << neighborsQ[n].z << ')';
                            //cout << ':' << neighborsQ[n].priority << '=' << voxel->getValue() << '+';
                            //cout << stepSize << '\n';
                        }
                    }
                }
            }

    //calculate distance from voxels in priority queue
    while (!pqabs.empty())
    {
        //pop voxel from queue
        top=pqabs.top();
        pqabs.pop();
        x=top.x; y=top.y; z=top.z;
        voxel=getVoxel(level,x,y,z);
        voxel->setValue(SGN(voxel->getValue())*top.priority);
        voxel->setFlag(OTFL_VALID);
        voxel->clearFlag(OTFL_QUEUED);
        
        //push neighbors onto absolute distance queue
        num=getNeighbors6(level,x,y,z,neighborsO,neighborsQ);
        for (n=0; n<num; n++)
        {
            if ((!neighborsO[n]->getFlag(OTFL_CARVED)) &&
                (!neighborsO[n]->getFlag(OTFL_VALID)) &&
                (!neighborsO[n]->getFlag(OTFL_QUEUED)))
            {
                neighborsQ[n].priority=ABS(voxel->getValue())+stepSize;
                pqabs.push(neighborsQ[n]);
                neighborsO[n]->setFlag(OTFL_QUEUED);
            }
        }
    }
    cout << "done.\n";
}

#ifdef ONECOMP
//Topology changes are ignored.
int volume::topologyCheckOutside(int level,int x,int y,int z)
{
    return 0;
}

int volume::topologyCheckInside(int level,int x,int y,int z)
{
    return 0;
}

#else
//Returns true if removing the specified voxel changes the topology.
// We use a look-up table based on the state of the 26 neighbors.
// For each neighbor, we set the bit if the neighbor is not carved.
int volume::topologyCheckOutside(int level,int x,int y,int z)
{
    //In this case, all negative voxels must be considered not carved.
    int topoType=0,bit=0;
    int stepSize=1<<level;
    int ox,oy,oz;
    octree* voxel;
    for (oz=-stepSize; oz<=stepSize; oz+=stepSize)
        for (oy=-stepSize; oy<=stepSize; oy+=stepSize)
            for (ox=-stepSize; ox<=stepSize; ox+=stepSize)
                if ((oz!=0) || (oy!=0) || (ox!=0))
                {
                    if ((z+oz>=0) && (z+oz<size[2]) &&
                        (y+oy>=0) && (y+oy<size[1]) &&
                        (x+ox>=0) && (x+ox<size[0]))
                    {
                        //Out of range is considered carved.
                        //If the voxel is null, then it has been carved, but we can't tell the sign correctly.
                        // So we check the deepest non-null ancestor of the voxel.
                        voxel=getDeepestVoxel(x+ox,y+oy,z+oz);
                        //If the voxel is not carved, we set the bit.
                        //Even if it is carved, if we're looking at the highest resolution we want
                        // to carve all the way up to the voxels of opposite sign, so the carved
                        // negative voxels shouldn't affect the topology test and we set the bit.
                        //However, if we're not looking at the highest resolution,
                        // we want the negative voxels to affect the topology test
                        // so that patches over undefined regions aren't too rough.
                        if ((!voxel->getFlag(OTFL_CARVED)) || (/*(level==0) && */(voxel->getValue()<0)))
                            topoType |= (1<<bit);
                    }
                    bit++;
                }
    return !!(topoinfo[topoType >> 3] & (1<<(topoType & 0x07)));
}

int volume::topologyCheckInside(int level,int x,int y,int z)
{
    //In this case, positive voxels must be considered not carved.
    int topoType=0,bit=0;
    int stepSize=1<<level;
    int ox,oy,oz;
    octree* voxel;
    for (oz=-stepSize; oz<=stepSize; oz+=stepSize)
        for (oy=-stepSize; oy<=stepSize; oy+=stepSize)
            for (ox=-stepSize; ox<=stepSize; ox+=stepSize)
                if ((oz!=0) || (oy!=0) || (ox!=0))
                {
                    if ((z+oz>=0) && (z+oz<size[2]) &&
                        (y+oy>=0) && (y+oy<size[1]) &&
                        (x+ox>=0) && (x+ox<size[0]))
                    {
                        //If the voxel is null, then it has been carved, but we can't tell the sign correctly.
                        // So we check the deepest non-null ancestor of the voxel.
                        voxel=getDeepestVoxel(x+ox,y+oy,z+oz);
                        //If the voxel is not carved, we set the bit.
                        //Even if it is carved, if we're looking at the highest resolution we want
                        // to carve all the way up to the voxels of opposite sign, so the carved
                        // positive voxels shouldn't affect the topology test and we set the bit.
                        //However, if we're not looking at the highest resolution,
                        // we want the positive voxels to affect the topology test
                        // so that patches over undefined regions aren't too rough.
                        if ((!voxel->getFlag(OTFL_CARVED)) || (/*(level==0) && */(voxel->getValue()>0)))
                            topoType |= (1<<bit);
                    }
                    else topoType |= (1<<bit); //out of range is considered positive and therefore not carved
                    bit++;
                }
    topoType ^= ((1<<26)-1); //inside is dual to outside
    return !!(topoinfo[topoType >> 3] & (1<<(topoType & 0x07)));
}
#endif

void volume::carveVoxelOutside(octree* voxel,int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ)
{
    int n,num;
    float val;
    voxel->setFlag(OTFL_CARVED);
    //After we carve, we know the sign of the voxel.
    voxel->clearFlag(OTFL_UNKNOWN);
    val=voxel->getValue();
    if (val==0) voxel->setValue(SMALLNUM);
    else voxel->setValue(ABS(val)); //positive
    //check the neighbors
    num=getNeighbors26(level,x,y,z,neighborsO,neighborsQ);
    for (n=0; n<num; n++)
    {
        if ((!neighborsO[n]->getFlag(OTFL_CARVED)) &&
            (!neighborsO[n]->getFlag(OTFL_BOUNDARY))  &&
            ((!neighborsO[n]->getFlag(OTFL_CANTMERGE)) || (neighborsO[n]->getValue() > 0)))
        {
            //add this neighbor to priority queue
            neighborsO[n]->setFlag(OTFL_BOUNDARY);
            neighborsQ[n].priority=ABS(neighborsQ[n].priority);
            octree* testvoxel=getVoxel(level,neighborsQ[n].x,neighborsQ[n].y,neighborsQ[n].z);
            assert(testvoxel != NULL);
            outerBoundary->push(neighborsQ[n]);
            //cout << "push(" << outerBoundary->size() << "): "; printBoundary(level,cout);
            //cout << neighborsO[n]->getValue() << ',';
        }
    }
    //cout << " added to outer boundary.\n";
    if (animationOn) if (fixStyle>=0) renderVolume(level);
}

void volume::carveVoxelInside(octree* voxel,int level,int x,int y,int z,octree** neighborsO,pqitem* neighborsQ)
{
    int n,num;
    float val;
    voxel->setFlag(OTFL_CARVED);
    //After we carve, we know the sign of the voxel.
    voxel->clearFlag(OTFL_UNKNOWN);
    val=voxel->getValue();
    if (val==0) voxel->setValue(-SMALLNUM);
    else voxel->setValue(-ABS(val)); //negative
    //check the neighbors
    num=getNeighbors26(level,x,y,z,neighborsO,neighborsQ);
    for (n=0; n<num; n++)
    {
        if ((!neighborsO[n]->getFlag(OTFL_CARVED)) &&
            (!neighborsO[n]->getFlag(OTFL_BOUNDARY))  &&
            ((!neighborsO[n]->getFlag(OTFL_CANTMERGE)) || (neighborsO[n]->getValue() < 0)))
        {
            //add this neighbor to priority queue
            neighborsO[n]->setFlag(OTFL_BOUNDARY);
            neighborsQ[n].priority=ABS(neighborsQ[n].priority);
            octree* testvoxel=getVoxel(level,neighborsQ[n].x,neighborsQ[n].y,neighborsQ[n].z);
            assert(testvoxel != NULL);
            innerBoundary->push(neighborsQ[n]);
            //cout << "push(" << innerBoundary->size() << "): "; printBoundary(level,cout);
            //cout << neighborsO[n]->getValue() << ',';
        }
    }
    //cout << " added to inner boundary.\n";
    if (animationOn) if (fixStyle<=0) renderVolume(level);
}

void volume::carveSimultaneousLowRes(int level)
{
    cout << " Carving... "; cout.flush();
    octree* voxel;
    pqitem top,toppos,topneg;
    pqitem neighborsQ[26];
    octree* neighborsO[26];
    int numCarvedNeg=0,numCarvedPos=0;
    int useNegative;

    //carve without changing topology
    while ((!outerBoundary->empty()) || (!innerBoundary->empty())) //pq not empty
    {
        //pop supervoxel from top of appropriate priority queue
        if (outerBoundary->empty())
        {
            useNegative=1;
            top=innerBoundary->top();
            innerBoundary->pop();
        }
        else if (innerBoundary->empty())
        {
            useNegative=0;
            top=outerBoundary->top();
            outerBoundary->pop();
        }
        else
        {
            toppos=outerBoundary->top();
            topneg=innerBoundary->top();
            if (topneg>toppos)
            {
                useNegative=1;
                top=topneg;
                innerBoundary->pop();
            }
            else
            {
                useNegative=0;
                top=toppos;
                outerBoundary->pop();
            }
            //May have to do something a little more intelligent in the case of tie,
            // here or when voxels are added to queue.
        }
        voxel=getVoxel(level,top.x,top.y,top.z);
        voxel->clearFlag(OTFL_BOUNDARY);
        
        //check whether removing this supervoxel changes the topology
        // and whether carving would lose information
        if (useNegative)
        {
            if ((!topologyCheckInside(level,top.x,top.y,top.z)) &&
                (!voxel->getFlag(OTFL_CANTMERGE)))
            {
                //If it doesn't, carve this voxel.
                carveVoxelInside(voxel,level,top.x,top.y,top.z,neighborsO,neighborsQ);
                numCarvedNeg++;
            }
        }
        else
        {
            if ((!topologyCheckOutside(level,top.x,top.y,top.z)) &&
                (!voxel->getFlag(OTFL_CANTMERGE)))
            {
                //If it doesn't, carve this voxel.
                carveVoxelOutside(voxel,level,top.x,top.y,top.z,neighborsO,neighborsQ);
                numCarvedPos++;
            }
        }
    }

    cout << numCarvedNeg << " carved from inside, ";
    cout << numCarvedPos << " carved from outside.\n";
}

void volume::carveSimultaneousHighRes(int level)
{
    cout << " Carving... "; cout.flush();
    octree* voxel;
    pqitem top,toppos,topneg;
    pqitem neighborsQ[26];
    octree* neighborsO[26];
    int numCarvedNeg=0,numCarvedPos=0;
    int useNegative;

    //carve without changing topology
    while ((!outerBoundary->empty()) || (!innerBoundary->empty())) //pq not empty
    {
        //pop supervoxel from top of appropriate priority queue
        if (outerBoundary->empty())
        {
            useNegative=1;
            top=innerBoundary->top();
            innerBoundary->pop();
        }
        else if (innerBoundary->empty())
        {
            useNegative=0;
            top=outerBoundary->top();
            outerBoundary->pop();
        }
        else
        {
            toppos=outerBoundary->top();
            topneg=innerBoundary->top();
            if (topneg>toppos)
            {
                useNegative=1;
                top=topneg;
                innerBoundary->pop();
            }
            else
            {
                useNegative=0;
                top=toppos;
                outerBoundary->pop();
            }
        }
        voxel=getVoxel(level,top.x,top.y,top.z);
        voxel->clearFlag(OTFL_BOUNDARY);
        
        //check whether removing this voxel changes the topology
        if (useNegative)
        {
            if (!topologyCheckInside(level,top.x,top.y,top.z))
            {
                //If it doesn't, carve this voxel.
                carveVoxelInside(voxel,level,top.x,top.y,top.z,neighborsO,neighborsQ);
                numCarvedNeg++;
            }
            else
            {
                //Otherwise, save it for opening features later.
                innerForLater->push_back(top);
                //cout << "can't carve\n";
            }
        }
        else
        {
            if (!topologyCheckOutside(level,top.x,top.y,top.z))
            {
                //If it doesn't, carve this voxel.
                carveVoxelOutside(voxel,level,top.x,top.y,top.z,neighborsO,neighborsQ);
                numCarvedPos++;
            }
            else
            {
                //Otherwise, save it for opening features later.
                outerForLater->push_back(top);
                //cout << "can't carve\n";
            }
        }
        //printVolume(level,cout); cout << '\n';
    }

    cout << numCarvedNeg << " carved from inside, ";
    cout << numCarvedPos << " carved from outside.\n";
}

int volume::openFeatureOutside()
{
    cout << " Opening feature... "; cout.flush();
    int level=0,x,y,z;
    int stepSize=1<<level;
    pqvector::const_iterator item;
    octree* voxel;
    pqitem top;
    pqitem neighborsQ[26];
    octree* neighborsO[26];
    float val,max=-BIGNUM;

    //Add the saved voxels back to the boundary queue, skipping over voxels that ended up being carved.
    for (item=outerForLater->begin(); item!=outerForLater->end(); item++)
    {
        voxel=getVoxel(level,item->x,item->y,item->z);
        if ((!voxel->getFlag(OTFL_CARVED)) && (!voxel->getFlag(OTFL_BOUNDARY)))
        {
            voxel->setFlag(OTFL_BOUNDARY);
            outerBoundary->push(*item);
        }
    }

    //Find the global max uncarved positive voxel and add it to the boundary
    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
                if (((voxel=getVoxel(level,x,y,z)) != NULL) && (!voxel->getFlag(OTFL_CARVED)) &&
                    ((val=voxel->getValue())>0) && (val>max))
                {
                    top.x=x; top.y=y; top.z=z;
                    top.priority=val;
                    max=val;
                }
    if (max>-BIGNUM)
    {
        voxel=getVoxel(level,top.x,top.y,top.z);
        if (!voxel->getFlag(OTFL_BOUNDARY))
        {
            voxel->setFlag(OTFL_BOUNDARY);
            outerBoundary->push(top);
        }
    }
    
    //If there were some uncarved voxels,
    if (!outerBoundary->empty())
    {
        //carve the max voxel to open the feature.
        top=outerBoundary->top(); outerBoundary->pop();
        voxel=getVoxel(level,top.x,top.y,top.z);
        voxel->clearFlag(OTFL_BOUNDARY);
        carveVoxelOutside(voxel,level,top.x,top.y,top.z,neighborsO,neighborsQ);
        cout << 1 << " voxel carved from outside.\n";
        return 1;
    }
    else
    {
        cout << "no feature to open.\n";
        return 0;
    }
}

int volume::openFeatureInside()
{
    cout << " Opening feature... "; cout.flush();
    int level=0,x,y,z;
    int stepSize=1<<level;
    pqvector::const_iterator item;
    octree* voxel;
    pqitem top;
    pqitem neighborsQ[26];
    octree* neighborsO[26];
    float val,min=BIGNUM;

    //Add the saved voxels back to the boundary queue, skipping over voxels that ended up being carved.
    for (item=innerForLater->begin(); item!=innerForLater->end(); item++)
    {
        voxel=getVoxel(level,item->x,item->y,item->z);
        if ((!voxel->getFlag(OTFL_CARVED)) && (!voxel->getFlag(OTFL_BOUNDARY)))
        {
            voxel->setFlag(OTFL_BOUNDARY);
            innerBoundary->push(*item);
        }
    }
    
    //Find the global min uncarved negative voxel and add it to the boundary
    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
                if (((voxel=getVoxel(level,x,y,z)) != NULL) && (!voxel->getFlag(OTFL_CARVED)) &&
                    ((val=voxel->getValue())<0) && (val<min))
                {
                    top.x=x; top.y=y; top.z=z;
                    top.priority=val;
                    min=val;
                }
    if (min<BIGNUM)
    {
        voxel=getVoxel(level,top.x,top.y,top.z);
        if (!voxel->getFlag(OTFL_BOUNDARY))
        {
            voxel->setFlag(OTFL_BOUNDARY);
            innerBoundary->push(top);
        }
    }
    
    //If there were some uncarved voxels,
    if (!innerBoundary->empty())
    {
        //carve the min voxel to open the feature.
        top=innerBoundary->top(); innerBoundary->pop();
        voxel=getVoxel(level,top.x,top.y,top.z);
        voxel->clearFlag(OTFL_BOUNDARY);
        carveVoxelInside(voxel,level,top.x,top.y,top.z,neighborsO,neighborsQ);
        cout << 1 << " voxel carved from inside.\n";
        return 1;
    }
    else
    {
        cout << "no feature to open.\n";
        return 0;
    }
}

int volume::changeInsideOrOutside(int level)
{
    //Check the uncarved voxels to see whether there are more negative or positive.
    int x,y,z;
    int stepSize=1<<level;
    octree* voxel;
    float val;
    int numPos=0,numNeg=0;

    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
            {
                voxel=getVoxel(level,x,y,z);
                if ((voxel != NULL) && (!voxel->getFlag(OTFL_CARVED)))
                {
                    val=voxel->getValue();
                    if (val > 0) numPos++;
                    if (val < 0) numNeg++;
                }
            }

    if (numNeg<numPos) return -1;
    else return 1;
}

void volume::fixVolumeOutside(int level)
{
    //Any voxels that are positive but not carved are made slightly negative.
    int x,y,z;
    int stepSize=1<<level;
    octree* voxel;
    int numChanged=0;

    cout << "Fixing volume..."; cout.flush();
    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
            {
                voxel=getVoxel(level,x,y,z);
                if ((voxel != NULL) && (!voxel->getFlag(OTFL_CARVED)))
                {
                    if (voxel->getValue() > 0) numChanged++;
					if (voxel->getValue() > 0) voxel->setValue(-0.5);
					else if (voxel->getFlag(OTFL_UNKNOWN)) voxel->setValue(-fabs(voxel->getValue()));
                }
            }
    
    cout << " " << numChanged << " voxels changed to negative.\n";
}

void volume::fixVolumeInside(int level)
{
    //Any voxels that are negative but not carved are made slightly positive.
    int x,y,z;
    int stepSize=1<<level;
    octree* voxel;
    int numChanged=0;

    cout << "Fixing volume..."; cout.flush();
    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
            {
                voxel=getVoxel(level,x,y,z);
                if ((voxel != NULL) && (voxel->getValue() < 0) && (!voxel->getFlag(OTFL_CARVED)))
                {
                    voxel->setValue(0.5);
                    numChanged++;
                }
            }
    
    //We also change the sign on everything because we switched the topology check
    //changeAllSigns();
    
    cout << " " << numChanged << " voxels changed to positive.\n";
}

void volume::fixVolumeBoth(int level)
{
    fixVolumeOutside(level);
    fixVolumeInside(level);
}

void volume::fixVolumeInsideCarved(int level)
{
    //Any voxels that are negative and carved are made slightly positive.
    int x,y,z;
    int stepSize=1<<level;
    octree* voxel;
    int numChanged=0;

    cout << "Fixing volume..."; cout.flush();
    for (z=0; z<size[2]; z+=stepSize)
        for (y=0; y<size[1]; y+=stepSize)
            for (x=0; x<size[0]; x+=stepSize)
            {
                voxel=getDeepestVoxel(x,y,z);
                if ((voxel->getValue() < 0) && (voxel->getFlag(OTFL_CARVED)))
                {
                    voxel->setValue(0.5);
                    numChanged++;
                }
            }

    cout << " " << numChanged << " voxels changed to positive.\n";
}

void volume::fixTopology()
{
    //We start by filling in the first level completely
    // so that the only null voxels are voxels outside the data range.
    // This way the initial outer boundary will be a topological sphere.
    
    //turnOnAnimation();
    //dataroot->print();
    int feature;
    int level=highestResolutionLevel;
    fillInLevel(level);
    if (animationOn) renderVolume(level);
    cout << "Size: "; cout.flush();
    cout << dataroot->calcMemoryUsage() << " bytes. ";
    cout << dataroot->countNodes() << " nodes in octree.\n";
    for (; level>0; level--)
    {
        cout << "level " << level << ":\n";
        calcDistances(level);
        //printVolume(level,cout);
        constructBoundaryInside(level);
        constructBoundaryOutside(level);
        //printBoundary(level,cout);
        carveSimultaneousLowRes(level);
        //if (!animationOn) cout << " "; renderVolume(level);
        divideLevel(level);
        //checkLevel(level);
    }
    //turnOffAnimation();
    cout << "level " << level << ":\n";
    cout << " Size: "; cout.flush();
    cout << dataroot->calcMemoryUsage() << " bytes. ";
    cout << dataroot->countNodes() << " nodes in octree.\n";
    calcDistances(level);
    //printVolume(level,cout);
    constructBoundaryInside(level);
    //printVolume(level,cout);
    constructBoundaryOutside(level);
    //printVolume(level,cout);
    //printBoundary(level,cout);
    carveSimultaneousHighRes(level);
    //if (!animationOn) cout << " "; renderVolume(level);
    for (feature=0; feature<numFeatures; feature++)
    {
        cout << "feature " << feature+1 << ":\n";
        openFeatureInside();
        openFeatureOutside();
        carveSimultaneousHighRes(level);
    }
#ifdef ONECOMP
    fixVolumeBoth(level);
#else
    int inOrOut=fixStyle;
    //if (inOrOut==0) inOrOut=changeInsideOrOutside(level);
	//If inOrOut is not specified, don't change the volume.
	if (inOrOut!=0)
	{
		if (inOrOut==-1) fixVolumeInside(level);
		else fixVolumeOutside(level);
	}
#endif
}

void volume::turnOnAnimation()
{
    animationOn=1;
}

void volume::turnOffAnimation()
{
    animationOn=0;
}

void volume::renderVolume()
{
    renderVolume(0);
}

//Hook for rendering the volume at every step for animation purposes
void volume::renderVolume(int level)
{
    frameNumber++;
    //if ((level==1) && (frameNumber%3!=0)) return;
    
    int x,y,z,index;
    int coords[3];
    //coords[0]=coords[1]=coords[2]=1<<ceillogbase2(MAX(MAX(size[0],size[1]),size[2]));
    coords[0]=size[0]; coords[1]=size[1]; coords[2]=size[2]; 
    int slicesize=coords[0]*coords[1];
    float* vdata=new float[slicesize];
    
    cout << "Rendering frame " << frameNumber << "...";
    char filename[20];
    sprintf(filename,"frames/frame_%.4d.v",frameNumber);
    ofstream fout(filename);
    
    if (fout)
    {
        fout.write((char*)coords,sizeof(coords));
        for (z=0; z<coords[2]; z++)
        {
            index=0;
            for (y=0; y<coords[1]; y++)
                for (x=0; x<coords[0]; x++)
                {
                    //octree* anc=dataroot->findDeepestAncestor(treeHeight,x,y,z);
                    //if (anc->getFlag(OTFL_CARVED))
                    //if (dataroot->getFlagAnyAncestor(treeHeight,0,x,y,z,OTFL_CARVED))
                    octree* voxel=getVoxel(level,x,y,z);
                    //If the level has been divided, then any null voxel is considered carved.
                    if (voxel==NULL) voxel=getDeepestVoxel(x,y,z);
                    if (fixStyle != 0)
                    {
                        if ((voxel->getFlag(OTFL_CARVED)) && (SGN(voxel->getValue())==fixStyle))
                            vdata[index]=fixStyle*1.0;
                        else
                            vdata[index]=-fixStyle*1.0;
                    }
                    else
                    {
                        if (voxel->getFlag(OTFL_CARVED))
                            vdata[index]=1.0;
                        else
                            vdata[index]=-1.0;
                    }
                    index++;
                }
            fout.write((char*)vdata,slicesize*sizeof(float));
        }
        fout.close();
        cout << "done." << '\n';
    }
    else cout << "output to file \"" << filename << "\" failed!" << '\n';

    delete[] vdata;
}

float trunc(float x)
{
    return floor(1000*x)/1000.0;
}

void volume::printVolume(int level,ostream& out)
{
    int x,y,z;
    int stepSize=1<<level;
    int maxcoord=1<<ceillogbase2(MAX(MAX(size[0],size[1]),size[2]));
    octree* voxel;

    for (x=0; x<size[0]; x+=stepSize)
    {
        out << "----" << '\t';
    }
    out << '\n';

    for (z=0; z<size[2]; z+=stepSize)
    {
        for (y=0; y<size[1]; y+=stepSize)
        {
            for (x=0; x<size[0]; x+=stepSize)
            {
                voxel=getVoxel(level,x,y,z);
                if (voxel != NULL)
                {
                    if (voxel->getFlag(OTFL_CARVED)) out << 'x';
                    else if (voxel->getFlag(OTFL_BOUNDARY)) out << '|';
                    else cout << ' ';
                    out << trunc(voxel->getValue());
                    if (voxel->getFlag(OTFL_CANTMERGE)) out << '*';
                }
                else out << "NULL";
                out << '\t';
            }
            out << '\n';
        }
        out << '\n';
    }
}

int volume::readFile(char* filename)
{
    int result;
    if (dataroot) delete dataroot;
    dataroot=new octree();
    result=dataroot->readFile(filename,size);
    treeHeight=ceillogbase2(MAX(MAX(size[0],size[1]),size[2]));
    if (treeHeight<highestResolutionLevel) highestResolutionLevel=treeHeight;
    return result;
}

int volume::writeFile(char* filename)
{
    int result;
    if (dataroot) result=dataroot->writeFile(filename,size);
    return result;
}

istream& operator>> (istream& in, volume& v)
{
    if (v.dataroot) delete v.dataroot;
    v.dataroot=new octree();
    in.read((char*)v.size,sizeof(v.size));
    v.treeHeight=ceillogbase2(MAX(MAX(v.size[0],v.size[1]),v.size[2]));
    if (v.treeHeight<v.highestResolutionLevel) v.highestResolutionLevel=v.treeHeight;
    clock_t starttime = clock();
    v.dataroot->readData(in,v.size,v.treeHeight);
    clock_t endtime = clock();
    cout << "Octree construction time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";
    return in;
}

ostream& operator<< (ostream& out, const volume& v)
{
    out.write((char*)v.size,sizeof(v.size));
    if (v.dataroot) v.dataroot->writeData(out,v.size,v.treeHeight);
    return out;
}


#ifdef FIXTOP
int main(int argc,char* argv[])
{
    if (argc<=3)
    {
        cerr << "Usage: " << argv[0] << " <input .v file> <output .v file> <number of features> [inside|outside]\n";
        return 1;
    }
    
    int inside=0;
    if (argc>4) if (!strncmp(argv[4],"in",2)) inside=-1;
    if (argc>4) if (!strncmp(argv[4],"out",3)) inside=1;

    int result;

    volume v(atoi(argv[3]),inside);
    result=v.readFile(argv[1]);
    if (result) return result;
    
    octree* root=v.getDataroot();
    result=v.readTopoinfoFile();
    if (result) return result;

    cout << v.getSize()[0] << 'x' << v.getSize()[1] << 'x' << v.getSize()[2] << '=';
    cout << v.getSize()[0]*v.getSize()[1]*v.getSize()[2] << '\n';
    //for (int level=v.getTreeHeight(); level>=0; level--) v.printVolume(level,cout);

    clock_t starttime = clock();
    v.fixTopology();
    clock_t endtime = clock();
    cout << "Topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";

    result=v.writeFile(argv[2]);
    if (result) return result;

    return 0;
}
#endif

#ifdef SLICE
int main(int argc,char* argv[])
{
    if (argc<=2)
    {
        cerr << "Usage: " << argv[0] << " <input .v file> <slice> [...]\n";
        return 1;
    }

    int type=1;

    int result;
    volume v(0,1,0);
    result=v.readFile(argv[1]);
    if (result) return result;
    int* vsize=v.getSize();

    /*v.fillInLevel(0);
    v.calcDistances(0);
    result=v.readTopoinfoFile();
    if (result) return result;
    v.constructBoundaryInside(0);
    v.constructBoundaryOutside(0);
    v.carveSimultaneousHighRes(0);
    v.fixVolumeInside(0);*/
    
    int slicesize=vsize[0]*vsize[1];
    octree** buf=new octree*[slicesize];
    int* levels=new int[slicesize];
    int maxsize=MAX(MAX(vsize[0],vsize[1]),vsize[2]);

    for (int i=2; i<argc; i++)
    {
        int slice=atoi(argv[i]);
        
        char name[128];
        sprintf(name,"slice%03d.ppm",slice);
        ofstream ofs(name);
        if (!ofs)
        {
            cerr << "Cannot open " << name << " for writing.\n";
            return 1;
        }
        
        v.extractSlice(slice,buf);
        v.extractSliceLevels(slice,levels);
        
        ofs << "P6" << endl << vsize[0] << " " << vsize[1] << endl;
        ofs << "255" << endl;

        for (int index=0; index<slicesize; index++)
        {
            int red=0,green=0,blue=0;
            octree* voxel=buf[index];
            assert(voxel != NULL);
            float val=voxel->getValue();

            if (type==1)		//octree
            {
                int color;
                int up=(((index/vsize[0])^(index%vsize[0]))&(1<<levels[index]))>>levels[index];
                if (voxel->getFlag(OTFL_CANTMERGE))
                {
                    color=255;
                }
                else
                {
                    color=126+24*up;
                }
                if (val<0)	//red
                {
                    red=color;
                    //green=0+(!voxel->getFlag(OTFL_UNKNOWN))*16;
                    green=0;
                    blue=0;
                }
                else		//gray
                {
                    red=color/3;
                    //green=color/3+(!voxel->getFlag(OTFL_UNKNOWN))*16;
                    green=color/3;
                    blue=color/3;
                }
            }
            else if (type==2)		//distance field
            {
                float freq=9.0;
                float normalized=val/maxsize;
                if (val<0)	//red
                {
                    red  =255;
                    green=255-(int)floor(-255*normalized*freq);
                    blue =255-(int)floor(-255*normalized*freq);
                }
                else		//gray
                {
                    red  =255-(int)floor(170*normalized*freq);
                    green=255-(int)floor(170*normalized*freq);
                    blue =255-(int)floor(170*normalized*freq);
                }
            }

            ofs.put(red);
            ofs.put(green);
            ofs.put(blue);
        }
        ofs.close();
        cout << "Slice written to " << name << '\n';
    }
    delete[] buf;

    return 0;
}
#endif
