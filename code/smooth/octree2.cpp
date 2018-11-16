/*
 *  octree.cpp
 *  
 *
 *  Created by James Vanderhyde on Mon Jun 02 2003.
 *
 */

#include <string.h>
#include <stdio.h>
#include <fstream.h>
//#include <iostream.h>

#include <assert.h>

#include "octree.h"

#include "volfill/OccGridRLE.h"
#include "bitc.h"
#include "Vector3D.h"

int numVoxelsChildrenInited;
int numVoxelsChildrenDeleted;
int levelOneVoxelsWithChildren;
int numSupervoxelsCleared;

octree::octree()
{
    value=0;
    children=NULL;
    flags=0;
}

octree::octree(float p_value)
{
    value=p_value;
    children=NULL;
    flags=0;
}

octree::~octree()
{
    deleteChildren();
}

void octree::copy(octree* o)
{
    value=o->value;
    children=o->children;
    flags=o->flags;
}

void octree::clear(float val)
{
    value=val;
    children=NULL;
    flags=0;
}

//Creates the children array and 8 children.
void octree::initChildren(float val)
{
    children=new octree*[8];
    for (int c=0; c<8; c++)
        children[c]=new octree(val);
}

void octree::initChildren()
{
    children=new octree*[8];
    for (int c=0; c<8; c++)
        children[c]=new octree(value);
}

void octree::deleteChildren()
{
    if (children)
    {
        for (int c=0; c<8; c++)
            if (children[c])
			{
				delete children[c];
				children[c]=NULL;
			}
        delete[] children;
    }
    children=NULL;
}

void octree::clearWithChildren(float val)
{
    value=val;
    flags=0;
    if (!children) numSupervoxelsCleared++;
    if (!children) initChildren(val);
    else for (int c=0; c<8; c++)
        children[c]->clear(val);
}

void octree::setChild(int child,octree* node)
{
    children[child]->copy(node);
}

octree* octree::getChild(int child)
{
    assert(children != NULL);
    return children[child];
}

octree** octree::getChildren()
{
    return children;
}

int octree::numChildren()
{
    int r=0;
    if (children != NULL)
        for (int c=0; c<8; c++)
            if (children[c] != NULL)
                r++;
    return r;
}

void octree::copyChildren(octree* o)
{
    for (int c=0; c<8; c++)
        children[c]->copy(o->children[c]);
}

int octree::childIndex(int x,int y,int z)
{
    return x+(y<<1)+(z<<2);
}

int octree::childX(int index)
{
    return index&1;
}

int octree::childY(int index)
{
    return (index>>1)&1;
}

int octree::childZ(int index)
{
    return (index>>2)&1;
}

//x, y, and z are global coordinates. level is the level of the node whose child we want.
int octree::childIndex(int level,int x,int y,int z)
{
    //x, y, and z will be in the range {0, ..., (2^treeHeight)-1}.
    //So to find out which half each is in, we divide by 2^(level-1).
    //Basically we read the bit at position level-1 of x, y, and z
    // and combine these to decide which child to take.
    int cx,cy,cz,l1=level-1;
    cx=(x>>l1)&1;
    cy=(y>>l1)&1;
    cz=(z>>l1)&1;
    return cx+(cy<<1)+(cz<<2);
}

//Returns true if the specified voxel is the last in-range child.
int octree::shouldMerge(int level,int x,int y,int z,const int* vsize)
{
    int cx,cy,cz;
    int mask=(1<<level)-1;
    cx=((x&mask)==mask);
    cy=((y&mask)==mask);
    cz=((z&mask)==mask);
    return (((cx) || (x==vsize[0]-1)) && ((cy) || (y==vsize[1]-1)) && ((cz) || (z==vsize[2]-1)));
}

float octree::getValue()
{
    return value;
}

void octree::setValue(float val)
{
    value=val;
}

int octree::getFlag(int flag)
{
    return ((flags & flag)>0);
}

void octree::setFlag(int flag,int val)
{
    if (val) setFlag(flag);
    else clearFlag(flag);
}

void octree::setFlag(int flag)
{
    flags |= flag;
}

void octree::clearFlag(int flag)
{
    flags &= ~flag;
}

int octree::memsize()
{
    return sizeof(octree);
}

int octree::calcMemoryUsage()
{
    int s=memsize();
    if (children != NULL)
        for (int c=0; c<8; c++)
        {
            s+=sizeof(octree*);
            if (children[c] != NULL)
                s+=children[c]->calcMemoryUsage();
        }
    return s;
}

int octree::countNodes()
{
    int s=1;
    if (children != NULL)
        for (int c=0; c<8; c++)
        {
            if (children[c] != NULL)
                s+=children[c]->countNodes();
        }
    return s;
}

int octree::findMaxDepth(int depth)
{
    int max=depth,d;
    if (children != NULL)
        for (int c=0; c<8; c++)
            if (children[c] != NULL)
            {
                d=children[c]->findMaxDepth(depth+1);
                if (d>max) max=d;
            }
    return max;
}

octree* octree::findParentNode(int level,int x,int y,int z)
{
    if (children==NULL)
    {
        //I don't have children, so you don't belong in the tree.
        return NULL;
    }
    else if (level<=1)
    {
        //I do have children, and you are one of them.
        return this;
    }
    else
    {
        //I do have children, but you are not one, but you may be a descendant.
        // So ask one of my children.
        int c=childIndex(level,x,y,z);
        //cout << c;
        return children[c]->findParentNode(level-1,x,y,z);
    }
}

//Induction hypotheses:
// result is a pointer to an array of lenx*leny*lenz nodes, which will be filled with decendants of this node.
// All nodes in specified range are descendants of this node, or this node.
// All voxels in the specified range are descendants of this node, if they exist in the tree.
//  If a voxel in the range does not exist in the tree, we put its deepest ancestor into result.
//  (If we want NULL instead we will have to add a downToLevel parameter.)
void octree::getRange(int level,int startx,int starty,int startz,int lenx,int leny,int lenz,octree** result)
{
  if (children != NULL)
    {
      int x,y,z;
      int startx2=startx+lenx,starty2=starty+leny,startz2=startz+lenz;
      int lenx2=0,leny2=0,lenz2=0;
      int c,num=0;
      c=childIndex(level,startx,starty,startz);
      for (x=startx+1; x<startx+lenx; x++) if (childIndex(level,x,starty,startz) != c)
	{
	  startx2=x;
	  lenx2=lenx-(startx2-startx);
	  break;
	}
      for (y=starty+1; y<starty+leny; y++) if (childIndex(level,startx,y,startz) != c)
	{
	  starty2=y;
	  leny2=leny-(starty2-starty);
	  break;
	}
      for (z=startz+1; z<startz+lenz; z++) if (childIndex(level,startx,starty,z) != c)
	{
	  startz2=z;
	  lenz2=lenz-(startz2-startz);
	  break;
	}
      children[c]->getRange(level-1,startx,starty,startz,lenx-lenx2,leny-leny2,lenz-lenz2,result+num);
      num+=(lenx-lenx2)*(leny-leny2)*(lenz-lenz2);
      if (lenx2>0)
	{
	  children[childIndex(level,startx2,starty,startz)]->
	    getRange(level-1,startx2,starty,startz,lenx2,leny-leny2,lenz-lenz2,result+num);
	  num+=lenx2*(leny-leny2)*(lenz-lenz2);
	}
      if (leny2>0)
	{
	  children[childIndex(level,startx,starty2,startz)]->
	    getRange(level-1,startx,starty2,startz,lenx-lenx2,leny2,lenz-lenz2,result+num);
	  num+=(lenx-lenx2)*leny2*(lenz-lenz2);
	}
      if (lenz2>0)
	{
	  children[childIndex(level,startx,starty,startz2)]->
	    getRange(level-1,startx,starty,startz2,lenx-lenx2,leny-leny2,lenz2,result+num);
	  num+=(lenx-lenx2)*(leny-leny2)*lenz2;
	}
      if ((lenx2>0) && (leny2>0))
	{
	  children[childIndex(level,startx2,starty2,startz)]->
	    getRange(level-1,startx2,starty2,startz,lenx2,leny2,lenz-lenz2,result+num);
	  num+=lenx2*leny2*(lenz-lenz2);
	}
      if ((leny2>0) && (lenz2>0))
	{
	  children[childIndex(level,startx,starty2,startz2)]->
	    getRange(level-1,startx,starty2,startz2,lenx-lenx2,leny2,lenz2,result+num);
	  num+=(lenx-lenx2)*leny2*lenz2;
	}
      if ((lenz2>0) && (lenx2>0))
	{
	  children[childIndex(level,startx2,starty,startz2)]->
	    getRange(level-1,startx2,starty,startz2,lenx2,leny-leny2,lenz2,result+num);
	  num+=lenx2*(leny-leny2)*lenz2;
	}
      if ((lenx2>0) && (leny2>0) && (lenz2>0))
	{
	  children[childIndex(level,startx2,starty2,startz2)]->
	    getRange(level-1,startx2,starty2,startz2,lenx2,leny2,lenz2,result+num);
	  num+=lenx2*leny2*lenz2;
	}
      assert(num==lenx*leny*lenz);
    }
  else
    {
      //Do something here depending on what we need this function for.
      // For now we just use the current node.
      for (int i=0; i<lenx*leny*lenz; i++) result[i]=this;
    }
}

octree* octree::findDeepestAncestor(int level,int x,int y,int z)
{
    if (children==NULL)
    {
        //I don't have children, so I am your deepest ancestor, or I am you.
        return this;
    }
    else
    {
        //I do have children.
        int c=childIndex(level,x,y,z);
        if (children[c]==NULL)
        {
            //But my child (your ancestor) does not exist, so I am your deepest ancestor.
            return this;
        }
        else
        {
            //My child has children, so you have an ancestor deeper than I.
            // So ask one of my children.
            return children[c]->findDeepestAncestor(level-1,x,y,z);
        }
    }
}

int octree::levelOfDeepestAncestor(int level,int x,int y,int z)
{
    if (children==NULL) return level;
    else
    {
        int c=childIndex(level,x,y,z);
        if (children[c]==NULL) return level;
        else return children[c]->levelOfDeepestAncestor(level-1,x,y,z);
    }
}

octree* octree::getAncestorAtLevel(int level,int downToLevel,int x,int y,int z)
{
    if (level>downToLevel)
    {
        int c=childIndex(level,x,y,z);
        if ((children==NULL) || (children[c]==NULL))
        {
            //I don't have descendants on the requested level!
            return NULL;
        }
        else return children[c]->getAncestorAtLevel(level-1,downToLevel,x,y,z);
    }
    else return this;
}

int octree::getFlagAnyAncestor(int level,int downToLevel,int x,int y,int z,int flag)
{
    if (level>downToLevel)
    {
        int c=childIndex(level,x,y,z);
        if ((children==NULL) || (children[c]==NULL))
        {
            return (this->getFlag(flag));
        }
        else return ((this->getFlag(flag)) || (children[c]->getFlagAnyAncestor(level-1,downToLevel,x,y,z,flag)));
    }
    else return (this->getFlag(flag));
}

void octree::changeSignRecursively()
{
    this->setValue(-this->getValue());
    if (children != NULL)
        for (int c=0; c<8; c++)
            if (children[c] != NULL)
                children[c]->changeSignRecursively();
}

void octree::setFlagRecursively(int flag)
{
    this->setFlag(flag);
    if (children != NULL)
        for (int c=0; c<8; c++)
            if (children[c] != NULL)
                children[c]->setFlagRecursively(flag);
}

void octree::fillInLevels(int level,int downToLevel)
{
    if (this->getFlag(OTFL_CANTMERGE)) this->setFlag(OTFL_VALID);

    if (level>downToLevel)
    {
        int c;
        if (children==NULL) this->initChildren(this->value);
        for (c=0; c<8; c++)
            children[c]->fillInLevels(level-1,downToLevel);
    }
}

//Adds a new voxel to the octree and merges if neseassary.
void octree::addVoxel(int level,int x,int y,int z,octree* leaf,const int* vsize,int addToLevel)
{
    if (children==NULL)
    {
        //I don't have children, but you are a descendant.
        // So I decide to have children.
        this->initChildren(BIGNUM);
        numVoxelsChildrenInited++;
    }

    int c=childIndex(level,x,y,z);

    if (level<=addToLevel+1)
    {
        //You are one of my children.
        this->setChild(c,leaf);
    }
    else
    {
        //You are not one of my children, but you are a descendant.
        // So ask one of my children.
        children[c]->addVoxel(level-1,x,y,z,leaf,vsize,addToLevel);
    }
    if (children[c]->getFlag(OTFL_CANTMERGE)) this->setFlag(OTFL_CANTMERGE);

    if (shouldMerge(level,x,y,z,vsize))
    {
        //I have seen all my children (or we're at the end of the array), so they may have to merge.
        float val,min=BIGNUM,max=-BIGNUM,sum=0.0;
        int i;
	this->setFlag(OTFL_UNKNOWN);
        for (i=0; i<8; i++)
        {
            val=children[i]->getValue();
            if (val<min) min=val;
            if (val>max) max=val;
            sum+=val;
	    if (!children[i]->getFlag(OTFL_UNKNOWN)) this->clearFlag(OTFL_UNKNOWN);
        }
        if (max<0.0) this->value=max;	//an inside node
        if (min>0.0) this->value=min;	//an outside node
        if ((max<0.0) || (min>0.0))
        {
        }
        else this->value=0;		//a mixed node
        if (!this->getFlag(OTFL_CANTMERGE))
        {
            //merge if allowed
            if (addToLevel>0)	//This "if" is a hack to keep from deleting level 0 voxels over and over.
            {
                this->deleteChildren();
                numVoxelsChildrenDeleted++;
            }
	    //If this node had children of both signs but is not can't merge anyway,
	    // all of the children must have unknown sign.
	    // But we don't want any nodes to have value 0 unless they are known
	    // to cross the isosurface.
	    //So in this case we'll use the value of an arbitrary child.
	    // We might get some better results by choosing something a little more intelligent,
	    // such as majority rule. The goal would be to minimize propogation of error.
	    // This situation only exists because of error, but that error can be enlarged
	    // with a poor choice of sign here, and in the end it can have a bad effect on the carving.
	    if (this->value==0) this->value=max;
        }
    }
}

//Adds a new voxel to a fully-constructed octree, no need for a new value
void octree::addNewVoxel(int level,int x,int y,int z,int addToLevel)
{
    if (children==NULL)
    {
        //I don't have children, but you are a descendant.
        // So I decide to have children.
        this->initChildren();
    }
	
    int c=childIndex(level,x,y,z);
	
    if (level<=addToLevel+1)
    {
        //You are one of my children.
        children[c]->setFlag(OTFL_CANTMERGE);
    }
    else
    {
        //You are not one of my children, but you are a descendant.
        // So ask one of my children.
        children[c]->addNewVoxel(level-1,x,y,z,addToLevel);
    }
    if (children[c]->getFlag(OTFL_CANTMERGE)) this->setFlag(OTFL_CANTMERGE);
    if (!children[c]->getFlag(OTFL_UNKNOWN)) this->clearFlag(OTFL_UNKNOWN);
}

//Adds a new voxel to a fully-constructed octree
void octree::addNewVoxel(int level,int x,int y,int z,float val,int addToLevel)
{
    if (children==NULL)
    {
        //I don't have children, but you are a descendant.
        // So I decide to have children.
        this->initChildren();
    }
	
    int c=childIndex(level,x,y,z);
	
    if (level<=addToLevel+1)
    {
        //You are one of my children.
		octree* leaf=new octree(val);
		leaf->setFlag(OTFL_CANTMERGE);
        this->setChild(c,leaf);
		delete leaf;
    }
    else
    {
        //You are not one of my children, but you are a descendant.
        // So ask one of my children.
        children[c]->addNewVoxel(level-1,x,y,z,val,addToLevel);
    }
    if (children[c]->getFlag(OTFL_CANTMERGE)) this->setFlag(OTFL_CANTMERGE);
    if (!children[c]->getFlag(OTFL_UNKNOWN)) this->clearFlag(OTFL_UNKNOWN);
}

//Checks for sign difference between voxel and 26 neighbors.
int octree::checkSignDiff(int x,int y,int z,int index,float* prevSliceData,float* curSliceData,float* nextSliceData,const int vsize[3],int treeHeight)
{
    //cout << ':';
    if ((x==0) || (y==0) || (z==0) || (x==vsize[0]-1) || (y==vsize[1]-1) || (z==vsize[2]-1))
    {
        //We assume values outside the volume grid are positive.
        //Therefore, negative voxels on the grid boundary are adjacent to the isosurface.
        if (curSliceData[index]<0) return 1;
    }

    int sx=-1,sy=-1,ex=1,ey=1;
    if (x==0) sx=0; if (y==0) sy=0;
    if (x==vsize[0]-1) ex=0; if (y==vsize[1]-1) ey=0;
    
    //Check all 26 neighbors
    int s=SGN(curSliceData[index]);
    for (int oy=sy; oy<=ey; oy++)
        for (int ox=sx; ox<=ex; ox++)
        {
            int i=index+ox+oy*vsize[0];
            //cout << ',' << prevSliceData[i] << ',' << curSliceData[i] << ',' << nextSliceData[i];
            if ((s != SGN(prevSliceData[i])) || (s != SGN(curSliceData[i])) || (s != SGN(nextSliceData[i])))
                return 1;
        }
    return 0;
}

//Redid this, so that all 26 neighbors are checked for a sign difference,
// not just 6 neighbors.
// I think we might as well read in 3 slices at a time. Loop over z with z++,
// rearrange the slices, and read in the next slice each time.
// If z is even then process the supervoxel. All the CANTMERGE flags will
// already be set correctly because we checked the next and the prev slices.
// The addSurfaceVoxel() function should be completely phased out.
// Boundary conditions can have slices full of BIGNUM.

void octree::readData(istream& in,const int vsize[3],int treeHeight)
{
    int x,y,z,index,superindex;
    int maxcoord=1<<ceillogbase2(MAX(MAX(vsize[0],vsize[1]),vsize[2]));
    int slicesize=vsize[0]*vsize[1];
    int superxsize=vsize[0]/2+vsize[0]%2;
    int superslicesize=superxsize*(vsize[1]/2+vsize[1]%2);
    float* sliceData0=new float[slicesize];
    float* sliceData1=new float[slicesize];
    float* sliceData2=new float[slicesize];
    float* sliceDataTemp;
    octree* slice1=new octree[slicesize];
    octree* superslice=new octree[superslicesize];
    octree supervoxel;
    int numZeroVoxels=0;

    for (index=0; index<slicesize; index++)
        sliceData0[index]=sliceData1[index]=sliceData2[index]=BIGNUM;
    for (superindex=0; superindex<superslicesize; superindex++) superslice[superindex].clearWithChildren(BIGNUM);

    numVoxelsChildrenInited=0;
    numVoxelsChildrenDeleted=0;
    levelOneVoxelsWithChildren=0;
    numSupervoxelsCleared=0;

    //loop through the slices
    cout << "Reading " << vsize[2] << " slices"; cout.flush();
    in.read((char*)sliceData1,slicesize*sizeof(float));
    for (z=0; z<vsize[2]; z++)
    {
        //read in next slice of data and process current slice
        cout << "."; cout.flush();
        if (z+1<vsize[2]) in.read((char*)sliceData2,slicesize*sizeof(float));
        else for (index=0; index<slicesize; index++) sliceData2[index]=BIGNUM;
        index=0;
        for (y=0; y<vsize[1]; y++)
        {
            for (x=0; x<vsize[0]; x++)
            {
                superindex=(x/2)+superxsize*(y/2);
                
                //We treat 0.0 as a special value, so if necessary shift a little.
                //This may not be important anymore.
                if (ABS(sliceData1[index])<SMALLNUM)
                {
                    sliceData1[index]=(-SMALLNUM); //assume a 0 value meant slightly negative
                    numZeroVoxels++;
                }

                //Init the new octree node
                slice1[index].clear(sliceData1[index]);
                //cout << slice1[index].getValue();
                if (checkSignDiff(x,y,z,index,sliceData0,sliceData1,sliceData2,vsize,treeHeight))
                {
                    slice1[index].setFlag(OTFL_CANTMERGE);
                }
                //if (slice1[index].getFlag(OTFL_CANTMERGE)) cout << '*';
                //cout << '\t';

                //Add voxel (x,y,z) to supervoxel
                superslice[superindex].addVoxel(1,x,y,z,&slice1[index],vsize,0);
                //cout << "(" << x << ',' << y << ',' << z << ")(" << superindex << ")\n";
                //superslice[superindex].print();
                //cout << '\n';
                
                //Check if it's time to add the supervoxel to the tree
                if (shouldMerge(1,x,y,z,vsize))
                {
                    //cout << "---(" << x << ',' << y << ',' << z << ")---\n\n";
                    //Add supervoxel ending with (x,y,z) to the tree and merge if applicable.
                    supervoxel.copy(&superslice[superindex]);
                    if (!supervoxel.getFlag(OTFL_CANTMERGE))
                    {
                        supervoxel.children=NULL;
                    }
                    else
                    {
                        supervoxel.initChildren();
                        supervoxel.copyChildren(&superslice[superindex]);
                    }
                    this->addVoxel(treeHeight,x,y,z,&supervoxel,vsize,1);
                    superslice[superindex].clearWithChildren(BIGNUM);
                }
                
                index++;
            }
            //cout << '\n';
        }

        sliceDataTemp=sliceData0;
        sliceData0=sliceData1;
        sliceData1=sliceData2;
        sliceData2=sliceDataTemp;
    }
    supervoxel.clear(0);
    cout << " done.\n";

    if (numZeroVoxels>0) cout << "Number of voxels adjusted from zero: " << numZeroVoxels << '\n';

    delete[] sliceData0;
    delete[] sliceData1;
    delete[] sliceData2;
    delete[] slice1;
    delete[] superslice;
    //this->print();
}

void octree::writeData(ostream& out,const int vsize[3],int treeHeight)
{
    int x,y,z,index;
    int slicesize=vsize[0]*vsize[1];
    float* vdata=new float[slicesize];

    for (z=0; z<vsize[2]; z++)
    {
        index=0;
        for (y=0; y<vsize[1]; y++)
            for (x=0; x<vsize[0]; x++)
            {
                octree* anc=this->findDeepestAncestor(treeHeight,x,y,z);
                assert(anc != NULL);
                vdata[index++]=anc->getValue();
            }
        out.write((char*)vdata,slicesize*sizeof(float));
    }

    delete[] vdata;
}

//This could be rewritten as a single traversal of the octree. It would be faster.
int octree::writeV2File(ostream& out,const int vsize[3],int treeHeight)
{
    int x,y,z,index;
    int slicesize=vsize[0]*vsize[1];
	int numVoxels=vsize[2]*vsize[1]*vsize[0];
	octree* anc;
	bitc positive(numVoxels);
	bitc carved(numVoxels);
	
	for (z=0; z<vsize[2]; z++)
        for (y=0; y<vsize[1]; y++)
            for (x=0; x<vsize[0]; x++)
			{
                anc=this->findDeepestAncestor(treeHeight,x,y,z);
                assert(anc != NULL);
				index=z*slicesize+y*vsize[0]+x;
				if (anc->getValue()>=0.0)
					positive.setbit(index);
#ifdef CONVERT
				if (!anc->getFlag(OTFL_UNKNOWN))
#else
				if (anc->getFlag(OTFL_CARVED))
#endif
					carved.setbit(index);
			}

    out.write((char*)vsize,3*sizeof(int));
	positive.write(out,numVoxels);
	carved.write(out,numVoxels);
	
	return 0;
}

int octree::readVRI(char* filename,int* vsize,float isovalue)
{
#ifdef _OCC_GRID_RLE_
    //read file into an OccGridRLE
    cout << "Reading " << filename << "..."; cout.flush();
    OccGridRLE *ogSrc;
    ogSrc = new OccGridRLE(1,1,1, CHUNK_SIZE);
    if (!ogSrc->read(filename))
    {
        delete ogSrc;
        return 1;
    }
    OccElement* occSlice;
    cout << "done.\n";

    //construct octree
    vsize[0]=ogSrc->xdim;
    vsize[1]=ogSrc->ydim;
    vsize[2]=ogSrc->zdim;
    int treeHeight=ceillogbase2(MAX(MAX(vsize[0],vsize[1]),vsize[2]));
    int x,y,z,index,superindex;
    int maxcoord=1<<treeHeight;
    int slicesize=vsize[0]*vsize[1];
    int superxsize=vsize[0]/2+vsize[0]%2;
    int superslicesize=superxsize*(vsize[1]/2+vsize[1]%2);
    float* sliceData0=new float[slicesize];
    float* sliceData1=new float[slicesize];
    float* sliceData2=new float[slicesize];
    float* sliceDataTemp;
    octree* slice1=new octree[slicesize];
    octree* superslice=new octree[superslicesize];
    octree supervoxel;
    int numZeroVoxels=0;
    int junk;
    //float pa,pb,pc,pd; pa=pb=pc=0; pd=-1;
    //pa=14076; pb=7521; pc=65867; pd=12882754; //"david-head-1mm-carve.vri"
    //Vector3D polyA(196,409,107),polyB(461,324,53),polyC(570,537,8),polyD(328,623,53);
    //Vector3D polyA(-1,-1,-1),polyB(-1,-2,-2),polyC(-2,-1,-2); //default
    Vector3D polyA(196,409,107),polyB(513,420,38),polyC(394,624,38); //"david-head-1mm-carve.vri"
    Vector3D currentPoint;
    float signedDistToPlane;

    for (index=0; index<slicesize; index++)
        sliceData0[index]=sliceData1[index]=sliceData2[index]=BIGNUM;
    for (superindex=0; superindex<superslicesize; superindex++) superslice[superindex].clearWithChildren(BIGNUM);

    //loop through the slices
    cout << "Processing " << vsize[2] << " slices";
    occSlice=ogSrc->getSlice("z",0,&junk,&junk);
    for (index=0; index<slicesize; index++)
        sliceData1[index]=(isovalue-(float)occSlice[index].value)/(2*isovalue+1);
    for (z=0; z<vsize[2]; z++)
    {
        //read one slice of data from OccGridRLE and copy into sliceData
        cout << "."; cout.flush();
        if (z+1<vsize[2])
        {
            occSlice=ogSrc->getSlice("z",z+1,&junk,&junk);
            for (index=0; index<slicesize; index++)
                sliceData2[index]=(isovalue-(float)occSlice[index].value)/(2*isovalue+1);
        }
        else for (index=0; index<slicesize; index++) sliceData2[index]=BIGNUM;
        index=0;
        for (y=0; y<vsize[1]; y++)
        {
            for (x=0; x<vsize[0]; x++)
            {
                superindex=(x/2)+superxsize*(y/2);
                
                //Init the new octree node
                slice1[index].clear(sliceData1[index]);
                if ((occSlice[index].totalWeight>0) &&
                    (ABS(sliceData1[index])<isovalue))
                    slice1[index].setFlag(OTFL_CANTMERGE);
                if (occSlice[index].totalWeight==0)
                    slice1[index].setFlag(OTFL_UNKNOWN);

                //Insert triangles to plug the big holes
		currentPoint.x=x; currentPoint.y=y; currentPoint.z=z;
		if (pointInTrianglePrism(currentPoint,polyA,polyB,polyC))
		  {
		    signedDistToPlane=signedDistFromPointToTrianglePlane(currentPoint,polyA,polyB,polyC);
		    if (fabs(signedDistToPlane)<1.0)
		      {
			slice1[index].setFlag(OTFL_CANTMERGE);
			slice1[index].clearFlag(OTFL_UNKNOWN);
			slice1[index].setValue(signedDistToPlane);
		      }
		  }
		    
                //Add voxel (x,y,z) to supervoxel
                superslice[superindex].addVoxel(1,x,y,z,&slice1[index],vsize,0);

                //Check if it's time to add the supervoxel to the tree
                if (shouldMerge(1,x,y,z,vsize))
                {
                    //Add supervoxel ending with (x,y,z) to the tree and merge if applicable.
                    supervoxel.copy(&superslice[superindex]);
                    if (!supervoxel.getFlag(OTFL_CANTMERGE))
                    {
                        supervoxel.children=NULL;
                    }
                    else
                    {
                        supervoxel.initChildren();
                        supervoxel.copyChildren(&superslice[superindex]);
                    }
                    this->addVoxel(treeHeight,x,y,z,&supervoxel,vsize,1);
                    superslice[superindex].clearWithChildren(BIGNUM);
                }

                index++;
            }
        }

        sliceDataTemp=sliceData0;
        sliceData0=sliceData1;
        sliceData1=sliceData2;
        sliceData2=sliceDataTemp;
    }
    supervoxel.clear(0);
    cout << " done.\n";

    delete[] sliceData0;
    delete[] sliceData1;
    delete[] sliceData2;
    delete[] slice1;
    delete[] superslice;
    delete ogSrc;
    return 0;
#endif
    cerr << ".vri format not supported. Recompile using volfill code.\n";
    return 1;
}

void octree::readDF(istream& in,int* vsize,int x,int y,int z)
{
    //x, y, and z are the max extent seen so far in each dimension, relative to current level.
    unsigned char fl;
    in.read((char*)&fl,sizeof(unsigned char));
    in.read((char*)&value,sizeof(float));
    if (fl&8) setFlag(OTFL_CARVED);
    if (fl&4) setFlag(OTFL_UNKNOWN);
    if (fl&2) setFlag(OTFL_CANTMERGE);
    if (fl&1)
    {
        //init and read in children
        int ox,oy,oz;
        initChildren();
        for (oz=0; oz<2; oz++) for (oy=0; oy<2; oy++) for (ox=0; ox<2; ox++)
            children[childIndex(ox,oy,oz)]->readDF(in,vsize,(x<<1)+ox,(y<<1)+oy,(z<<1)+oz);
    }
    if (getFlag(OTFL_CANTMERGE))
    {
        if (vsize[0]<x+1) vsize[0]=x+1;
        if (vsize[1]<y+1) vsize[1]=y+1;
        if (vsize[2]<z+1) vsize[2]=z+1;
    }
}

void octree::writeDF(ostream& out)
{
    unsigned char fl=0;
    if (getFlag(OTFL_CARVED)) fl |= 8;
    if (getFlag(OTFL_UNKNOWN)) fl |= 4;
    if (getFlag(OTFL_CANTMERGE)) fl |= 2;
    if (children != NULL) fl |= 1;
    float val=value;
    out.write((char*)&fl,sizeof(unsigned char));
    out.write((char*)&val,sizeof(float));
    if (children != NULL)
    {
        int c;
        for (c=0; c<8; c++)
            children[c]->writeDF(out);
    }
}

int octree::readFile(char* filename,int* vsize)
{
    ifstream fin(filename);
    if (!fin)
    {
        cerr << "Cannot open " << filename << " for reading.\n";
        return 1;
    }

    int result=0;
    char* suffix=strrchr(filename,'.');
    if (!strcmp(suffix,".v"))
    {
        //Read voxel file
        fin.read((char*)vsize,3*sizeof(int));
        int treeHeight=ceillogbase2(MAX(MAX(vsize[0],vsize[1]),vsize[2]));
        this->readData(fin,vsize,treeHeight);
    }
    else if (!strcmp(suffix,".vri"))
    {
        //Read vrip (Michaelangelo format) file
        vsize[0]=vsize[1]=vsize[2]=0;
        result=this->readVRI(filename,vsize,32767.5);
        if (result) cerr << "Error reading file " << filename << '\n';
    }
    else if (!strcmp(suffix,".dfo"))
    {
        //Read DF (depth-first) octree file
        vsize[0]=vsize[1]=vsize[2]=0;
        cout << "Reading octree from " << filename << "..."; cout.flush();
        this->readDF(fin,vsize,0,0,0);
        cout << "done.\n";
    }
    else
    {
        cerr << "File " << filename << " of unknown format.\n";
    }
    fin.close();
    return result;
}

int octree::writeFile(char* filename,int* vsize)
{
    ofstream fout(filename);
    if (!fout)
    {
        cerr << "Cannot open " << filename << " for writing.\n";
        return 1;
    }

    char* suffix=strrchr(filename,'.');
    if (!strcmp(suffix,".v"))
    {
        //Write voxel file
        cout << "Writing " << vsize[2] << " slices to " << filename << "..."; cout.flush();
        fout.write((char*)vsize,3*sizeof(int));
        int treeHeight=ceillogbase2(MAX(MAX(vsize[0],vsize[1]),vsize[2]));
        this->writeData(fout,vsize,treeHeight);
        cout << "done.\n";
    }
    else if (!strcmp(suffix,".v2"))
    {
        //Write 2-bit voxel file
#ifdef CONVERT
		cout << "Writing positive and known info to " << filename << "..."; cout.flush();
#else
		cout << "Writing positive and carved info to " << filename << "..."; cout.flush();
#endif
        int treeHeight=ceillogbase2(MAX(MAX(vsize[0],vsize[1]),vsize[2]));
        this->writeV2File(fout,vsize,treeHeight);
        cout << "done.\n";
    }
    else if (!strcmp(suffix,".dfo"))
    {
        //Write DF (depth-first) octree file
        cout << "Writing octree to " << filename << "..."; cout.flush();
        this->writeDF(fout);
        cout << "done.\n";
    }
    else
    {
        cerr << "File " << filename << " of unknown format.\n";
    }
    fout.close();
    return 0;
}

void octree::print(int depth)
{
    for (int i=0; i<depth; i++) cout << " ";
    cout << value;
    if (getFlag(OTFL_CANTMERGE))
        cout << '*';
    if (children != NULL)
        cout << " (" << numChildren() << ")";
    cout << '\n';
    if (children != NULL)
        for (int c=0; c<8; c++)
            if (children[c] != NULL)
                children[c]->print(depth+1);
}


#ifdef CONVERT
int main(int argc,char* argv[])
{
    if (argc<=2)
    {
        cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
        return 1;
    }

    int result;
    int vsize[3];
    octree* root=new octree();
    result=root->readFile(argv[1],vsize);
    //root->print();
    if (result) return result;
    result=root->writeFile(argv[2],vsize);
    if (result) return result;

    return 0;
}
#endif

int ceillogbase2(int x)
{
    if (x<=1<<0) return 0;
    if (x<=1<<1) return 1;
    if (x<=1<<2) return 2;
    if (x<=1<<3) return 3;
    if (x<=1<<4) return 4;
    if (x<=1<<5) return 5;
    if (x<=1<<6) return 6;
    if (x<=1<<7) return 7;
    if (x<=1<<8) return 8;
    if (x<=1<<9) return 9;
    if (x<=1<<10) return 10;
    if (x<=1<<11) return 11;
    if (x<=1<<12) return 12;
    if (x<=1<<13) return 13;
    if (x<=1<<14) return 14;
    if (x<=1<<15) return 15;
    if (x<=1<<16) return 16;
    if (x<=1<<17) return 17;
    if (x<=1<<18) return 18;
    if (x<=1<<19) return 19;
    if (x<=1<<20) return 20;
    if (x<=1<<21) return 21;
    if (x<=1<<22) return 22;
    if (x<=1<<23) return 23;
    if (x<=1<<24) return 24;
    if (x<=1<<25) return 25;
    if (x<=1<<26) return 26;
    if (x<=1<<27) return 27;
    if (x<=1<<28) return 28;
    if (x<=1<<29) return 29;
    if (x<=1<<30) return 30;
    else return 31;
}
