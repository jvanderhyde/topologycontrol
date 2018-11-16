//ContourTree.h
//James Vanderhyde, 8 September 2006.

#include <vector>
#include <map>
#include <stack>
#include <iostream.h>

class TreeEdge
{
public:
    //std::map<int,TreeNode>::iterator neighbor;
    int label;
    int eulerch;
    int cavities;
    int leftSliceOverlap,rightSliceOverlap;
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
    std::map<int,TreeNode>::iterator joinRoot,splitRoot;
    int edgesInitialized;
public:
    ContourTree();
    ContourTree(ContourTree& ct);
    ContourTree(ContourTree jt,ContourTree st);
    void clone(ContourTree& ct,std::vector<int>* nodelist=NULL);
    void clear();
    void addJoinNode(int index,int parentIndex); //for constructing tree haphazardly
    void addSplitNode(int index,int parentIndex);
    void addNewJoinNode(int index,int parentIndex); //for constructing tree top down
    void addNewSplitNode(int index,int parentIndex);
    void addNewJoinNode(int index,int* childIndices,int numChildren); //for constructing tree bottom up
    void addNewSplitNode(int index,int* childIndices,int numChildren);
    void mergeJoinTrees(ContourTree& jt1,ContourTree& jt2,std::vector<int>* process);
    void mergeSplitTrees(ContourTree& st1,ContourTree& st2,std::vector<int>* process);
    void findJoinRoot();
    void findSplitRoot();
    void removeNodeAndSplice(std::map<int,TreeNode>::iterator node);
    void eraseNode(int index);
    void eliminateIsolatedNodes();
    void simplify();
    void initEdges();
    void labelEdgesWithInducedMap(ContourTree& sdt);
    int countInducedMapManyToOnes(unsigned short* data=NULL,int dataLength=0);
    void labelJTEdgesWithCT(ContourTree& ct);
    void getJTEdgesWithLabelsInRange(int min,int max,std::vector<int>& list);
    void getPathToJoinRoot(int startIndex,std::vector<int>& list);
    void calcJTNodeSizesMinima();
    void calcJTNodeSizesVoxels(int* criticalParent,int numVoxels);
    void clearAncestors();
    void calcJTIsovalueAncestors(int voxel,int* order);
    void labelJTDescendantsWithAncestorNode(int node); //skips if already labeled
    void setCriticalParentsJT(int* criticalParent);
    int getNumNodes();
    int getJoinRootIndex();
    int getNodeSize(int node);
    int getNodeAncestor(int node);
    int getNodeUpperEdge0Label(int node);
    int getNodeUpperEdge0EulerCh(int node);
    int getNodeUpperEdge0Cavities(int node);
    std::map<int,TreeNode>::iterator getNode(int node);
    std::map<int,TreeNode>::iterator getBeginNode();
    std::map<int,TreeNode>::iterator getEndNode();
    int getPositiveCriticals(int* critList);
    int getNegativeCriticals(int* critList);
    int getNeutralCriticals(int* critList);
    int getComplexCriticals(int* critList);
    void getUpperEdgeList(int* edgeList,int* labels=NULL);
    void printLowerRecursion(ostream& out,int index,int depth,int simplified=0);
    void printUpperRecursion(ostream& out,int index,int depth,int simplified=0);
    void printJT(ostream& out,int simplified=0);
    void printST(ostream& out,int simplified=0);
    void print(ostream& out);
    void printJT(int simplified=0);
    void printST(int simplified=0);
    void print();
    void printWithData(ostream& out,unsigned short* data,int dataLength,int* vsize=NULL,int isovalue=65536);
    int readFile(char* filename);
    int writeFile(char* filename);
};

class PostOrderTraversalJT
{
protected:
    ContourTree* jt;
    std::stack<std::map<int,TreeNode>::iterator> nodeStack;
    std::stack<int> childStack;
public:
    PostOrderTraversalJT(ContourTree* p_jt);
    void init();
    std::map<int,TreeNode>::iterator getCurrent();
    void next();
    int hasNext();
};


