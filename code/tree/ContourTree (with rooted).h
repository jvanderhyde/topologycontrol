//ContourTree.h
//James Vanderhyde, 8 September 2006.

#include <vector>
#include <map>
#include <iostream.h>

class TreeEdge
{
public:
    int label;
};

class TreeNode
{
public:
    std::vector<int> upperNeighbors; //A node in a join tree will have only 1 upper neighbor (except root, 0)
    std::vector<int> lowerNeighbors; //A node in a split node will have only 1 lower neighbor (except root, 0)
    
    std::vector<TreeEdge> upperEdges;
    std::vector<TreeEdge> lowerEdges;
    
    int size; //number of voxels or minima in the component below this node
    int ancestor; //index of highest ancestor less than an isovalue
};

class ContourTree
{
protected:
    std::map<int,TreeNode> nodes;
    int edgesInitialized;
public:
    ContourTree();
    ContourTree(ContourTree& ct);
    ContourTree(JoinTree jt,SplitTree st);
    void clone(ContourTree& ct,std::vector<int>* nodelist=NULL);
    void clear();
    void removeNodeAndSplice(std::map<int,TreeNode>::iterator node);
    void eraseNode(int index);
    void eliminateIsolatedNodes();
    void simplify();
    void initEdges();
    void labelEdgesWithInducedMap(ContourTree& ct);
    int countInducedMapManyToOnes(unsigned short* data=NULL,int dataLength=0);
    int getNumNodes();
    int getNodeSize(int node);
    int getNodeAncestor(int node);
    std::map<int,TreeNode>::iterator getBeginNode();
    std::map<int,TreeNode>::iterator getEndNode();
    int getPositiveCriticals(int* critList);
    int getNegativeCriticals(int* critList);
    int getNeutralCriticals(int* critList);
    int getComplexCriticals(int* critList);
    void getUpperEdgeList(int* edgeList,int* labels=NULL);
    void print(ostream& out);
    void print();
    void printWithData(ostream& out,unsigned short* data,int dataLength,int* vsize=NULL,int isovalue=65536);
};

class RootedContourTree : public ContourTree
{
protected:
    std::map<int,TreeNode>::iterator root;

    void printLowerRecursion(ostream& out,int index,int depth,int simplified=0);
    void printUpperRecursion(ostream& out,int index,int depth,int simplified=0);
    
public:
    int getRootIndex();
    
    virtual void addNode(int index,int parentIndex) = 0; //for constructing tree haphazardly
    virtual void addNewNode(int index,int parentIndex) = 0; //for constructing tree top down
    virtual void addNewNode(int index,int* childIndices,int numChildren) = 0; //for constructing tree bottom up
    virtual void findRoot() = 0;
    virtual void mergeOverlappingTrees(ContourTree& rt1,ContourTree& rt2,std::vector<int>* process) = 0;
    
    virtual void getEdgesWithLabelsInRange(int min,int max,std::vector<int>& list) = 0;
    virtual void calcNodeSizesMinima() = 0;
    virtual void calcIsovalueAncestors(int isovalue,unsigned short* data) = 0;
    
    virtual void printRooted(ostream& out,int simplified=0) = 0;
    virtual void printRooted(int simplified=0) = 0;
};

class JoinTree : public RootedContourTree
{
public:
    virtual void addNode(int index,int parentIndex); //for constructing tree haphazardly
    virtual void addNewNode(int index,int parentIndex); //for constructing tree top down
    virtual void addNewNode(int index,int* childIndices,int numChildren); //for constructing tree bottom up
    virtual void findRoot();
    virtual void mergeOverlappingTrees(ContourTree& rt1,ContourTree& rt2,std::vector<int>* process);
    virtual void getEdgesWithLabelsInRange(int min,int max,std::vector<int>& list);
    virtual void calcNodeSizesMinima();
    virtual void calcIsovalueAncestors(int isovalue,unsigned short* data);
    virtual void printRooted(ostream& out,int simplified=0);
    virtual void printRooted(int simplified=0);
};

class SplitTree : public RootedContourTree
{
public:
    virtual void addNode(int index,int parentIndex); //for constructing tree haphazardly
    virtual void addNewNode(int index,int parentIndex); //for constructing tree top down
    virtual void addNewNode(int index,int* childIndices,int numChildren); //for constructing tree bottom up
    virtual void findRoot();
    virtual void mergeOverlappingTrees(ContourTree& rt1,ContourTree& rt2,std::vector<int>* process);
    virtual void getEdgesWithLabelsInRange(int min,int max,std::vector<int>& list);
    virtual void calcNodeSizesMinima();
    virtual void calcIsovalueAncestors(int isovalue,unsigned short* data);
    virtual void printRooted(ostream& out,int simplified=0);
    virtual void printRooted(int simplified=0);
};

