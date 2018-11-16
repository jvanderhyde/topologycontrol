/*
 *  octree.h
 *  
 *
 *  Created by James Vanderhyde on Mon Jun 02 2003.
 *
 */

#include <iostream.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)<(b))?(b):(a))
//#define SGN(x) (((x)<0)?-1:1)
#define SGN(x) (((x)<0)?-1:(((x)>0)?1:0))
#define ABS(x) (((x)<0)?-(x):(x))
#define BIGNUM 1e20
#define SMALLNUM 1e-15

enum
{
    OTFL_CANTMERGE=1,	//voxel can't merge into its parent
    OTFL_VALID=2,	//voxel has a calculated distance value
    OTFL_CARVED=4,	//voxel has been carved, either from inside or outside
    OTFL_BOUNDARY=8,	//voxel is part of the carving boundary
    OTFL_QUEUED=16,	//voxel is in a priority queue
    OTFL_UNKNOWN=32,	//voxel sign is unknown
    OTFL_INITED=64	//voxel has been initialized
};

//The OTFL_CANTMERGE flag has several meanings.
// On the highest resolution, it means merging the voxel would throw away information.
//  This meaning is passed up to ancestors, clearly.
// On any lower resolution, it also means the supervoxel has children.
//  If CANTMERGE is set, then it must have a CANTMERGE descendant.
//  If CANTMERGE is cleared, then it can't have a CANTMERGE descendant.
// We have also decided to let it indicate the starting unknown status of highest-res voxels.
//  The carving of voxels that are not CANTMERGE should proceed without regard to sign.
//  The only voxels with CANTMERGE set that can be carved are on the highest res.
//  Any voxel on the highest res with CANTMERGE set has a known sign.
//  Any voxel on any lower res with CANTMERGE set has value 0 if mixed or has known sign.
//  Any voxel on any res that is not CANTMERGE can be carved from either direction,
//   regardless of 0 value. CANTMERGE indicates children, 0 value does not.
//  Therefore, CANTMERGE implies known, on every resolution level, and UNKNOWN => !CANTMERGE.
//   Note that the status of UNKNOWN changes during carving, from unknown to known,
//   but CANTMERGE stays constant. However, the implication remains.
// We need to use the UNKNOWN flag during carving.
//  Clearly any carved voxel is known.
//  However, we need to mark voxels as known as soon as they're pushed onto a boundary queue.
//  Without this, one surface can leak through a hole into a patch of uncarved voxels.

class octree
{
protected:
    octree** children;
    float value;
    unsigned int flags;
    void calcInternalNodeValues();
public:
    octree();
    octree(float p_val);
    ~octree();
    void copy(octree* o);
    void clear(float val);
    void initChildren(float val);		//uses val for children's values
    void initChildren();			//uses this->value for children's values
    void deleteChildren();
    void clearWithChildren(float val);  	//calls clear(val) on children and sets value to val
    void setChild(int child,octree* node);
    octree* getChild(int child);
    octree** getChildren();
    int numChildren();
    void copyChildren(octree* o);
    static int childIndex(int x,int y,int z);
    static int childX(int index);
    static int childY(int index);
    static int childZ(int index);
    static int childIndex(int level,int x,int y,int z);
    static int shouldMerge(int level,int x,int y,int z,const int* vsize);
    float getValue();
    void setValue(float val);
    int getFlag(int flag);
    void setFlag(int flag,int val);
    void setFlag(int flag);
    void clearFlag(int flag);
    int memsize();
    int calcMemoryUsage();
    int countNodes();
    int findMaxDepth(int depth);
    octree* findParentNode(int level,int x,int y,int z);
    void getRange(int level,int startx,int starty,int startz,int lenx,int leny,int lenz,octree** result)
    octree* findDeepestAncestor(int level,int x,int y,int z);
    int levelOfDeepestAncestor(int level,int x,int y,int z);
    octree* getAncestorAtLevel(int level,int downToLevel,int x,int y,int z);
    int getFlagAnyAncestor(int level,int downToLevel,int x,int y,int z,int flag);
    void changeSignRecursively();
    void setFlagRecursively(int flag);
    void fillInLevels(int level,int downToLevel);
    void addVoxel(int level,int x,int y,int z,octree* leaf,const int* vsize,int addToLevel=0);
	void addNewVoxel(int level,int x,int y,int z,int addToLevel=0);
	void addNewVoxel(int level,int x,int y,int z,float val,int addToLevel=0);
    int checkSignDiff(int x,int y,int z,int index,float* prevSliceData,float* curSliceData,float* nextSliceData,const int vsize[3],int treeHeight);
    void readData(istream& in,const int vsize[3],int treeHeight);
    void writeData(ostream& out,const int vsize[3],int treeHeight);
    int writeV2File(ostream& out,const int vsize[3],int treeHeight);
    int readVRI(char* filename,int* vsize,float isovalue);
    void readDF(istream& in,int* vsize,int x,int y,int z);
    void writeDF(ostream& out);
    int readFile(char* filename,int* vsize);
    int writeFile(char* filename,int* vsize);
    void print(int depth=0);
};

int floorlogbase2(int x);
int ceillogbase2(int x);

