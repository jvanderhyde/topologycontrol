//ContourTree.cpp
//James Vanderhyde, 8 September 2006.

#include <fstream.h>
#include <assert.h>
#include <queue>

//#include "ContourTree.h"
#include "shortvolume.h"

ContourTree::ContourTree()
{
    edgesInitialized=0;
}

//Clones contour tree
ContourTree::ContourTree(ContourTree& ct)
{
    std::map<int,TreeNode>::iterator i;
    std::vector<int>::iterator j;
    for (i=ct.nodes.begin(); i!=ct.nodes.end(); i++)
    {
	TreeNode n;
	for (j=i->second.lowerNeighbors.begin(); j!=i->second.lowerNeighbors.end(); j++)
	    n.lowerNeighbors.push_back(*j);
	for (j=i->second.upperNeighbors.begin(); j!=i->second.upperNeighbors.end(); j++)
	    n.upperNeighbors.push_back(*j);
	nodes[i->first]=n;
    }
    edgesInitialized=0;
}

//Merges the join and split trees
ContourTree::ContourTree(ContourTree jt,ContourTree st)
{
    int verbose=0;
    //add all nodes to CT
    std::map<int,TreeNode>::iterator i;
    std::map<int,TreeNode>::iterator leafJT,leafST,parentNode,childNode;
    std::queue<int> leafQ;
    for (i=jt.nodes.begin(); i!=jt.nodes.end(); i++)
    {
	TreeNode n;
	nodes[i->first]=n;
	leafJT=i;
	leafST=st.nodes.find(i->first);
	if ((leafJT->second.lowerNeighbors.size()==0) && (leafST->second.upperNeighbors.size()==1))
	{
	    //add leaf to queue
	    leafQ.push(i->first);
	    //cout << 'p' << i->first; cout.flush();
	}
	if ((leafJT->second.lowerNeighbors.size()==1) && (leafST->second.upperNeighbors.size()==0))
	{
	    //add leaf to queue
	    leafQ.push(i->first);
	    //cout << 'p' << i->first; cout.flush();
	}
    }
    
    if (verbose)
    {
	cout << nodes.size() << " nodes:\n";
	jt.findJoinRoot();
	jt.printJT();
	st.findSplitRoot();
	st.printST();
    }
    int l=0;
    while (!leafQ.empty())
    {
	//find a leaf in CT
	if (verbose) {cout << "Finding leaf " << l++ << "..."; cout.flush();}
	assert(jt.getNumNodes()==st.getNumNodes());
	int leafInJT=0;
	leafJT=jt.nodes.find(leafQ.front());
	leafST=st.nodes.find(leafQ.front());
	leafQ.pop();
	//cout << 'q'; cout.flush();
	
	if (leafQ.empty())
	{
	    if (verbose) {cout << "moving last leaf " << leafJT->first << "..."; cout.flush();}
	    jt.nodes.erase(leafJT);
	    st.nodes.erase(leafST);
	    assert(jt.getNumNodes()==st.getNumNodes());
	}
	else
	{
	    if (leafJT->second.lowerNeighbors.size()==0) leafInJT=1;
	    else leafInJT=0;
	    
	    if (leafInJT)
	    {
		int nextLeaf=-1;
		if (verbose) {cout << "moving leaf " << leafJT->first << " from JT..."; cout.flush();}
		//add leaf to CT
		int parentIndex=leafJT->second.upperNeighbors[0];
		parentNode=jt.nodes.find(parentIndex);
		nodes[leafJT->first].upperNeighbors.push_back(parentIndex);
		nodes[parentIndex].lowerNeighbors.push_back(leafJT->first);
		
		assert(jt.getNumNodes()==st.getNumNodes());
		//remove leaf from JT
		std::vector<int>::iterator c,leafNode;
		for (c=parentNode->second.lowerNeighbors.begin(); c!=parentNode->second.lowerNeighbors.end(); c++)
		    if (*c==leafJT->first) leafNode=c;
		parentNode->second.lowerNeighbors.erase(leafNode);
		if (parentNode->second.lowerNeighbors.size()==0) nextLeaf=parentIndex;
		jt.nodes.erase(leafJT);
		
		//remove leaf from ST
		int childIndex=leafST->second.upperNeighbors[0];
		childNode=st.nodes.find(childIndex);
		if (leafST->second.lowerNeighbors.size()==0)
		    childNode->second.lowerNeighbors.pop_back();
		else
		{
		    parentIndex=leafST->second.lowerNeighbors[0];
		    parentNode=st.nodes.find(parentIndex);
		    childNode->second.lowerNeighbors[0]=parentIndex;
		    for (c=parentNode->second.upperNeighbors.begin(); c!=parentNode->second.upperNeighbors.end(); c++)
			if (*c==leafST->first) leafNode=c;
		    *leafNode=childIndex;
		}
		st.nodes.erase(leafST);
		assert(jt.getNumNodes()==st.getNumNodes());
		
		if ((nextLeaf>=0) && (st.nodes[nextLeaf].upperNeighbors.size()==1))
		{
		    //add to leaf queue
		    leafQ.push(nextLeaf);
		    //cout << 'p' << nextLeaf; cout.flush();
		}
	    }
	    else
	    {
		int nextLeaf=-1;
		if (verbose) {cout << "moving leaf " << leafST->first << " from ST..."; cout.flush();}
		//add leaf to CT
		int parentIndex=leafST->second.lowerNeighbors[0];
		parentNode=st.nodes.find(parentIndex);
		nodes[leafST->first].lowerNeighbors.push_back(parentIndex);
		nodes[parentIndex].upperNeighbors.push_back(leafST->first);
		
		assert(jt.getNumNodes()==st.getNumNodes());
		//remove leaf from ST
		std::vector<int>::iterator c,leafNode;
		for (c=parentNode->second.upperNeighbors.begin(); c!=parentNode->second.upperNeighbors.end(); c++)
		    if (*c==leafST->first) leafNode=c;
		parentNode->second.upperNeighbors.erase(leafNode);
		if (parentNode->second.upperNeighbors.size()==0) nextLeaf=parentIndex;
		st.nodes.erase(leafST);
		
		//remove leaf from JT
		int childIndex=leafJT->second.lowerNeighbors[0];
		childNode=jt.nodes.find(childIndex);
		if (leafJT->second.upperNeighbors.size()==0)
		    childNode->second.upperNeighbors.pop_back();
		else
		{
		    parentIndex=leafJT->second.upperNeighbors[0];
		    parentNode=jt.nodes.find(parentIndex);
		    childNode->second.upperNeighbors[0]=parentIndex;
		    for (c=parentNode->second.lowerNeighbors.begin(); c!=parentNode->second.lowerNeighbors.end(); c++)
			if (*c==leafJT->first) leafNode=c;
		    *leafNode=childIndex;
		}
		jt.nodes.erase(leafJT);
		assert(jt.getNumNodes()==st.getNumNodes());
		
		if ((nextLeaf>=0) && (jt.nodes[nextLeaf].lowerNeighbors.size()==1))
		{
		    //add to leaf queue
		    leafQ.push(nextLeaf);
		    //cout << 'p' << nextLeaf; cout.flush();
		}
	    }
	}
	if (verbose)
	{
	    cout << "done.\n";
	    if (!leafQ.empty())
	    {
		jt.findJoinRoot();
		jt.printJT();
		st.findSplitRoot();
		st.printST();
	    }
	}
    }
    edgesInitialized=0;
}

void ContourTree::clone(ContourTree& ct,std::vector<int>* nodelist)
{
    std::map<int,TreeNode>::iterator i;
    std::vector<int>::iterator j;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	TreeNode n;
	for (j=i->second.lowerNeighbors.begin(); j!=i->second.lowerNeighbors.end(); j++)
	    n.lowerNeighbors.push_back(*j);
	for (j=i->second.upperNeighbors.begin(); j!=i->second.upperNeighbors.end(); j++)
	    n.upperNeighbors.push_back(*j);
	ct.nodes[i->first]=n;
	if (nodelist) nodelist->push_back(i->first);
    }
    edgesInitialized=0;
}

void ContourTree::clear()
{
    nodes.clear();
    joinRoot=nodes.end();
    splitRoot=nodes.end();
    edgesInitialized=0;
}

void ContourTree::addJoinNode(int index,int parentIndex)
{
    //add parent if necessary
    std::map<int,TreeNode>::iterator i;
    i = nodes.find(parentIndex);
    if (i==nodes.end())
    {
	TreeNode p;
	nodes[parentIndex]=p;
    }
    
    //create new node
    i = nodes.find(index);
    if (i==nodes.end())
    {
	TreeNode n;
	nodes[index]=n;
    }
    
    //form connections
    if (index != parentIndex)
    {
	nodes[parentIndex].lowerNeighbors.push_back(index);
	nodes[index].upperNeighbors.push_back(parentIndex);
    }
}

void ContourTree::addSplitNode(int index,int parentIndex)
{
    //add parent if necessary
    std::map<int,TreeNode>::iterator i;
    i = nodes.find(parentIndex);
    if (i==nodes.end())
    {
	TreeNode p;
	nodes[parentIndex]=p;
    }
    
    //create new node
    i = nodes.find(index);
    if (i==nodes.end())
    {
	TreeNode n;
	nodes[index]=n;
    }
    
    //form connections
    if (index != parentIndex)
    {
	nodes[parentIndex].upperNeighbors.push_back(index);
	nodes[index].lowerNeighbors.push_back(parentIndex);
    }
}

void ContourTree::addNewJoinNode(int index,int parentIndex)
{
    //create new node
    TreeNode n;
    
    //form connections
    if (index != parentIndex)
    {
	nodes[parentIndex].lowerNeighbors.push_back(index);
	n.upperNeighbors.push_back(parentIndex);
    }
    
    //add node to nodes map
    nodes[index]=n;
}

void ContourTree::addNewSplitNode(int index,int parentIndex)
{
    //create new node
    TreeNode n;
    
    //form connections
    if (index != parentIndex)
    {
	nodes[parentIndex].upperNeighbors.push_back(index);
	n.lowerNeighbors.push_back(parentIndex);
    }
    
    //add node to nodes map
    nodes[index]=n;
}

//This needs to be really fast
void ContourTree::addNewJoinNode(int index,int* childIndices,int numChildren)
{
    //create new node
    TreeNode n;
    
    //form connections
    for (int i=0; i<numChildren; i++)
    {
	n.lowerNeighbors.push_back(childIndices[i]);
	nodes[childIndices[i]].upperNeighbors.push_back(index);
    }
    
    //add node to nodes map
    nodes[index]=n;
}

void ContourTree::addNewSplitNode(int index,int* childIndices,int numChildren)
{
    //create new node
    TreeNode n;
    
    //form connections
    for (int i=0; i<numChildren; i++)
    {
	n.upperNeighbors.push_back(childIndices[i]);
	nodes[childIndices[i]].lowerNeighbors.push_back(index);
    }
    
    //add node to nodes map
    nodes[index]=n;
}


//recursive helper function for union-find for merging join/split trees
int findCriticalAncestor(int index, std::map<int,int>& criticalParent)
{
    std::map<int,int>::iterator p=criticalParent.find(index);
    if (index==p->second) return index;
    else return p->second=findCriticalAncestor(p->second,criticalParent);
}

//Pascucci's algorithm doesn't make sense. It implies that we only have to use the union-find
// for the nodes in the intersection of the two regions, thereby making the runtime sublinear.
// But really we also need all ancestors in the two trees of the nodes in the intersection as well.
// Then each child will have to be checked for presence in the union-find. Where's the savings?

void ContourTree::mergeJoinTrees(ContourTree& jt1,ContourTree& jt2,std::vector<int>* process)
{
    std::vector<int>::iterator i;
    std::vector<int>::reverse_iterator j;
    int index,anc;
    std::map<int,int> criticalParent;
    std::vector<int>::iterator c;
    std::map<int,TreeNode>::iterator node1,node2;
    int* children=new int[162]; //I'm assuming 4D or less here
    int numChildren;
    int np,na;
    for (i=process->begin(); i!=process->end(); i++)
    {
	numChildren=0;
	index=*i;
	//cout << ' ' << index;
	criticalParent[index]=index;
	node1=jt1.nodes.find(index);
	node2=jt2.nodes.find(index);
	if (node1!=jt1.nodes.end())
	    for (c=node1->second.lowerNeighbors.begin(); c!=node1->second.lowerNeighbors.end(); c++)
	    {
		na=findCriticalAncestor(*c,criticalParent);
		if (na!=index)
		{
		    criticalParent[na]=index;
		    children[numChildren++]=na;
		}
	    }
	if (node2!=jt2.nodes.end())
	    for (c=node2->second.lowerNeighbors.begin(); c!=node2->second.lowerNeighbors.end(); c++)
	    {
		na=findCriticalAncestor(*c,criticalParent);
		if (na!=index)
		{
		    criticalParent[na]=index;
		    children[numChildren++]=na;
		}
	    }
	if ((node1==jt1.nodes.end()) && (node2==jt2.nodes.end()))
	{
	    //what is this for?
	    //criticalParent[index]=-1;
	}
	else addNewJoinNode(index,children,numChildren);
    }
    delete[] children;
    findJoinRoot();
}

void ContourTree::mergeSplitTrees(ContourTree& st1,ContourTree& st2,std::vector<int>* process)
{
    std::vector<int>::iterator i;
    std::vector<int>::reverse_iterator j;
    int index,anc;
    std::map<int,int> criticalParent;
    std::vector<int>::iterator c;
    std::map<int,TreeNode>::iterator node1,node2;
    int* children=new int[162]; //I'm assuming 4D or less here
    int numChildren;
    int np,na;
    for (i=process->begin(); i!=process->end(); i++)
    {
	numChildren=0;
	index=*i;
	//cout << ' ' << index; cout.flush();
	criticalParent[index]=index;
	node1=st1.nodes.find(index);
	node2=st2.nodes.find(index);
	if (node1!=st1.nodes.end())
	    for (c=node1->second.upperNeighbors.begin(); c!=node1->second.upperNeighbors.end(); c++)
	    {
		//cout << 'a'; cout.flush();
		na=findCriticalAncestor(*c,criticalParent);
		if (na!=index)
		{
		    //cout << 'b'; cout.flush();
		    criticalParent[na]=index;
		    children[numChildren++]=na;
		}
	    }
	if (node2!=st2.nodes.end())
	    for (c=node2->second.upperNeighbors.begin(); c!=node2->second.upperNeighbors.end(); c++)
	    {
		//cout << 'c'; cout.flush();
		na=findCriticalAncestor(*c,criticalParent);
		if (na!=index)
		{
		    //cout << 'd'; cout.flush();
		    criticalParent[na]=index;
		    children[numChildren++]=na;
		}
	    }
	if ((node1==st1.nodes.end()) && (node2==st2.nodes.end()))
	{
	    //what is this for?
	    //criticalParent[index]=-1;
	}
	else addNewSplitNode(index,children,numChildren);
    }
    delete[] children;
    //cout << '\n';
    findSplitRoot();
    //printST();
}

void ContourTree::findJoinRoot()
{
    std::map<int,TreeNode>::iterator i;
    joinRoot=nodes.end();
    int numChildren0=0,numChildren1=0,numChildrenMore=0;
    int numParents0=0,numParents1=0,numParentsMore=0;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if (i->second.upperNeighbors.empty())
	{
	    joinRoot=i;
	    //cout << "Join root = " << i->first << "\n";
	}
	if (i->second.upperNeighbors.size()==0) numParents0++;
	else if (i->second.upperNeighbors.size()==1) numParents1++;
	else numParentsMore++;
	if (i->second.lowerNeighbors.size()==0) numChildren0++;
	else if (i->second.lowerNeighbors.size()==1) numChildren1++;
	else numChildrenMore++;
    }
    if (joinRoot==nodes.end()) cerr << "No root found in join tree!\n";
    assert(numParents0==1);
    assert(numParents1==getNumNodes()-1);
    assert(numParentsMore==0);
}

void ContourTree::findSplitRoot()
{
    std::map<int,TreeNode>::iterator i;
    splitRoot=nodes.end();
    int numChildren0=0,numChildren1=0,numChildrenMore=0;
    int numParents0=0,numParents1=0,numParentsMore=0;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if (i->second.lowerNeighbors.empty())
	{
	    splitRoot=i;
	    //cout << "Split root = " << i->first << "\n";
	}
	if (i->second.lowerNeighbors.size()==0) numParents0++;
	else if (i->second.lowerNeighbors.size()==1) numParents1++;
	else numParentsMore++;
	if (i->second.upperNeighbors.size()==0) numChildren0++;
	else if (i->second.upperNeighbors.size()==1) numChildren1++;
	else numChildrenMore++;
    }
    if (splitRoot==nodes.end()) cerr << "No root found in split tree!\n";
    assert(numParents0==1);
    assert(numParents1==getNumNodes()-1);
    assert(numParentsMore==0);
}

//Assumes node has exactly 1 upper neighbor and 1 lower neighbor.
//Disconnects node from tree and splices upper and lower together.
void ContourTree::removeNodeAndSplice(std::map<int,TreeNode>::iterator node)
{
    int upperIndex=node->second.upperNeighbors[0];
    int lowerIndex=node->second.lowerNeighbors[0];
    std::vector<int>::iterator c;
    for (c=nodes[lowerIndex].upperNeighbors.begin(); c!=nodes[lowerIndex].upperNeighbors.end(); c++)
	if (*c==node->first) *c=upperIndex;
    for (c=nodes[upperIndex].lowerNeighbors.begin(); c!=nodes[upperIndex].lowerNeighbors.end(); c++)
	if (*c==node->first) *c=lowerIndex;
    node->second.upperNeighbors.pop_back();
    node->second.lowerNeighbors.pop_back();
}

void ContourTree::eraseNode(int index)
{
    nodes.erase(index);
}

//Too slow!
void ContourTree::eliminateIsolatedNodes()
{
    std::map<int,TreeNode>::iterator i;
    int found=1;
    while (found)
    {
	found=0;
	i=nodes.begin();
	while ((!found) && (i!=nodes.end()))
	{
	    if ((i->second.lowerNeighbors.size()==0) && (i->second.upperNeighbors.size()==0))
	    {
		found=1;
	    }
	    if (!found) i++;
	}
	if (found)
	{
	    nodes.erase(i);
	}
    }
}

void ContourTree::simplify()
{
    std::map<int,TreeNode>::iterator i;
    
    //remove uninteresting node by splicing neighbor above to neighbor below
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if ((i->second.lowerNeighbors.size()==1) && (i->second.upperNeighbors.size()==1))
	{
	    removeNodeAndSplice(i);
	}
    }
    
    //remove node from nodes map
    eliminateIsolatedNodes();
}

void ContourTree::initEdges()
{
    std::map<int,TreeNode>::iterator i;
    int j;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	while (!i->second.upperEdges.empty()) i->second.upperEdges.pop_back();
	while (!i->second.lowerEdges.empty()) i->second.lowerEdges.pop_back();
	for (j=0; j<i->second.upperNeighbors.size(); j++)
	{
	    TreeEdge e;
	    e.label=0;
	    e.eulerch=0;
	    //e.neighbor=nodes.find(i->second.upperNeighbors[j]);
	    i->second.upperEdges.push_back(e);
	}
	for (j=0; j<i->second.lowerNeighbors.size(); j++)
	{
	    TreeEdge e;
	    e.label=0;
	    e.eulerch=0;
	    e.cavities=0;
	    //e.neighbor=nodes.find(i->second.lowerNeighbors[j]);
	    i->second.lowerEdges.push_back(e);
	}
    }
    edgesInitialized=1;
}

//needs to be optimized with iterators
void ContourTree::labelEdgesWithInducedMap(ContourTree& sdt)
{
    //The sum of the edge labels going up minus the sum of the edge labels going down
    // is the same as the updegree minus the downdegree in the subdomain tree sdt.
    //There is always a node in the full tree with all edge labels except one already computed.
    //Therefore we can calculate the label for each edge, starting with the leaves.
    //To do this we use an auxiliary data structure, the "unlabeled" tree.
    // We remove leaves from this tree as their incident edges are labeled.
    
    ContourTree unlabeled(*this);
    //cout << "Original tree:\n";
    //this->print();
    //cout << "Copied tree:\n";
    //unlabeled.print();
    std::map<int,int> delta;
    std::map<int,TreeNode>::iterator itNode;
    if (!edgesInitialized) initEdges();
    
    //delta is 0 for nodes not involved in the subdomain.
    for (itNode=unlabeled.nodes.begin(); itNode!=unlabeled.nodes.end(); itNode++) delta[itNode->first]=0;
    //Calculate delta, the updegree minus the downdegree in sdt.
    for (itNode=sdt.nodes.begin(); itNode!=sdt.nodes.end(); itNode++)
    {
	delta[itNode->first]=itNode->second.upperNeighbors.size()-itNode->second.lowerNeighbors.size();
    }
    
    //Initialize queue with leaves of the unlabeled tree
    std::queue<int> solvedNodes;
    for (itNode=unlabeled.nodes.begin(); itNode!=unlabeled.nodes.end(); itNode++)
    {
	if ((itNode->second.upperNeighbors.size() + itNode->second.lowerNeighbors.size())==1)
	    solvedNodes.push(itNode->first);
    }
    
    //loop over the queue
    while (!solvedNodes.empty())
    {
	int label,leafIndex,otherIndex,j;
	std::vector<TreeEdge>::iterator i,neighborEdge;
	std::vector<int>::iterator iNeighbor;
	std::map<int,TreeNode>::iterator unlabeledLeaf,thisLeaf,unlabeledOther,thisOther;
	
	//remove leaf from queue
	leafIndex=solvedNodes.front();
	solvedNodes.pop();
	unlabeledLeaf=unlabeled.nodes.find(leafIndex);
	thisLeaf=nodes.find(leafIndex);
	
	//If this is the last node in the tree, both the if and the else if will fail, and the while loop will exit.
	if (unlabeledLeaf->second.lowerNeighbors.size()==1)
	{
	    otherIndex=unlabeledLeaf->second.lowerNeighbors[0];
	    unlabeledOther=unlabeled.nodes.find(otherIndex);
	    thisOther=nodes.find(otherIndex);
	    
	    //calculate label for leaf edge and store in tree
	    label = -delta[leafIndex];
	    for (j=0; j<thisLeaf->second.lowerNeighbors.size(); j++)
	    {
		if (thisLeaf->second.lowerNeighbors[j]==otherIndex)
		    thisLeaf->second.lowerEdges[j].label=label;
	    }
	    for (j=0; j<thisOther->second.upperNeighbors.size(); j++)
	    {
		if (thisOther->second.upperNeighbors[j]==leafIndex)
		    thisOther->second.upperEdges[j].label=label;
	    }
	    
	    //remove leaf from unlabeled tree
	    //cout << "Removing leaf " << leafIndex << "\n";
	    unlabeledLeaf->second.lowerNeighbors.erase(unlabeledLeaf->second.lowerNeighbors.begin());
	    for (iNeighbor=unlabeledOther->second.upperNeighbors.begin(); iNeighbor!=unlabeledOther->second.upperNeighbors.end(); iNeighbor++)
	    {
		//cout << " iNeighbor=" << *iNeighbor << "\n";
		if (*iNeighbor==leafIndex)
		{
		    unlabeledOther->second.upperNeighbors.erase(iNeighbor);
		    iNeighbor=unlabeledOther->second.upperNeighbors.end();
		    break;
		}
	    }
	    //unlabeled.print();
	   
	    //recalculate delta for the other end of the leaf edge
	    delta[otherIndex] -= label;
	    
	    //insert other end into queue if it is now a leaf
	    if ((unlabeledOther->second.upperNeighbors.size() + unlabeledOther->second.lowerNeighbors.size())==1)
		solvedNodes.push(otherIndex);
	}
	else if (unlabeledLeaf->second.upperNeighbors.size()==1)
	{
	    otherIndex=unlabeledLeaf->second.upperNeighbors[0];
	    unlabeledOther=unlabeled.nodes.find(otherIndex);
	    thisOther=nodes.find(otherIndex);
	    
	    //calculate label for leaf edge and store in tree
	    label = delta[leafIndex];
	    for (j=0; j<thisLeaf->second.upperNeighbors.size(); j++)
	    {
		if (thisLeaf->second.upperNeighbors[j]==otherIndex)
		    thisLeaf->second.upperEdges[j].label=label;
	    }
	    for (j=0; j<thisOther->second.lowerNeighbors.size(); j++)
	    {
		if (thisOther->second.lowerNeighbors[j]==leafIndex)
		    thisOther->second.lowerEdges[j].label=label;
	    }
	    
	    //remove leaf from unlabeled tree
	    //cout << "Removing leaf " << leafIndex << "\n";
	    unlabeledLeaf->second.upperNeighbors.erase(unlabeledLeaf->second.upperNeighbors.begin());
	    for (iNeighbor=unlabeledOther->second.lowerNeighbors.begin(); iNeighbor!=unlabeledOther->second.lowerNeighbors.end(); iNeighbor++)
	    {
		//cout << " iNeighbor=" << *iNeighbor << "\n";
		if (*iNeighbor==leafIndex)
		{
		    unlabeledOther->second.lowerNeighbors.erase(iNeighbor);
		    iNeighbor=unlabeledOther->second.lowerNeighbors.end();
		    break;
		}
	    }
	    //unlabeled.print();
	    
	    //recalculate delta for the other end of the leaf edge
	    delta[otherIndex] += label;
	    
	    //insert other end into queue if it is now a leaf
	    if ((unlabeledOther->second.upperNeighbors.size() + unlabeledOther->second.lowerNeighbors.size())==1)
		solvedNodes.push(otherIndex);
	}
    }
}

int ContourTree::countInducedMapManyToOnes(unsigned short* data,int dataLength)
{
    int w,many;
    int total=0;
    std::map<int,TreeNode>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	for (int j=0; j<i->second.upperNeighbors.size(); j++)
	{
	    many=1-i->second.upperEdges[j].label;
	    if (many<0) many=-many;
	    if (data)
	    {
		int d1=65536,d2=65536;
		int neighbor=i->second.upperNeighbors[j];
		if ((0<=i->first) && (i->first<dataLength)) d1=data[i->first];
		if ((0<=neighbor) && (neighbor<dataLength)) d2=data[neighbor];
		w=d2-d1;
	    }
	    else w=1;
	    //cout << "\n" << w << "*" << many << "=" << w*many;
	    total+=w*many;
	}
    }
    //cout << "\nTotal is " << total << "\n";
    return total;
}

void ContourTree::labelJTEdgesWithCT(ContourTree& ct)
{
    //This method is too slow!
    //Something like O(VE) instead of O(V+E)
    
    if (!edgesInitialized) initEdges();
    
    //loop through nodes of CT
    std::map<int,TreeNode>::iterator i;
    std::map<int,TreeNode>::iterator j1,j2,curNode;
    for (i=ct.nodes.begin(); i!=ct.nodes.end(); i++)
    {
	j1=nodes.find(i->first);
	//loop through upper neighbors of CT node
	std::vector<int>::iterator p;
	for (p=i->second.upperNeighbors.begin(); p!=i->second.upperNeighbors.end(); p++)
	{
	    //follow path in JT corresponding to this CT edge
	    j2=nodes.find(*p);
	    curNode=j1;
	    while (curNode != j2)
	    {
		curNode->second.upperEdges[0].label++;
		curNode=nodes.find(curNode->second.upperNeighbors[0]);
	    }
	}
    }
    
    /*std::map<int,TreeNode>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if (i != joinRoot)
	{
	    int lab=i->second.upperEdges[0].label;
	    cout << "Node " << i->first << " has upper label " << lab << "\n";
	}
    }*/
}

void ContourTree::getJTEdgesWithLabelsInRange(int min,int max,std::vector<int>& list)
{
    std::map<int,TreeNode>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if (i != joinRoot)
	{
	    int lab=i->second.upperEdges[0].label;
	    if ((min<=lab) && (lab<=max))
	    {
		list.push_back(i->first);
	    }
	    //cout << "Node " << i->first << " has upper label " << lab << "\n";
	}
    }	
}

void ContourTree::getPathToJoinRoot(int startIndex,std::vector<int>& list)
{
    std::map<int,TreeNode>::iterator i;
    i=nodes.find(startIndex);
    if (i==nodes.end()) return;
    int depth=0;
    while (i != joinRoot)
    {
	//cout << " " << i->first << " ";
	list.push_back(i->first);
	i=nodes.find(i->second.upperNeighbors[0]);
	depth++;
    }
    list.push_back(i->first);
    //cout << " length is " << depth << " ";
}

void ContourTree::calcJTNodeSizesMinima()
{
    std::stack<std::map<int,TreeNode>::iterator> st;
    std::map<int,TreeNode>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {    
	i->second.size=0;
    }
    
    st.push(joinRoot);
    while (!st.empty())
    {
	i=st.top();
	if (i->second.lowerNeighbors.size()==0)
	{
	    //leaf node: set size and pop
	    i->second.size=1;
	    st.pop();
	    //cout << 'o';
	}
	else
	{
	    //internal node
	    std::vector<int>::iterator c;
	    if (nodes[i->second.lowerNeighbors[0]].size==0) //check if children have been pushed yet
	    {
		//cout << '(';
		//push all children onto stack
		for (c=i->second.lowerNeighbors.begin(); c!=i->second.lowerNeighbors.end(); c++)
		    st.push(nodes.find(*c));
	    }
	    else
	    {
		//add up all children and pop
		for (c=i->second.lowerNeighbors.begin(); c!=i->second.lowerNeighbors.end(); c++)
		    i->second.size+=nodes[*c].size;
		st.pop();
		//cout << ')';
	    }
	}
    }
}

void ContourTree::calcJTNodeSizesVoxels(int* criticalParent,int numVoxels)
{
    std::stack<std::map<int,TreeNode>::iterator> st;
    std::map<int,TreeNode>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	i->second.size=0;
    }
    
    for (int v=0; v<numVoxels; v++)
    {
	//We use negative numbers to indicate that the node has not yet been processed.
	nodes[criticalParent[v]].size--;
    }
    
    //add up totals from leaves
    st.push(joinRoot);
    while (!st.empty())
    {
	i=st.top();
	if (i->second.lowerNeighbors.size()==0)
	{
	    //leaf node: add leaf itself and pop
	    i->second.size=-i->second.size;
	    i->second.size++;
	    st.pop();
	    assert(i->second.size>0);
	}
	else
	{
	    //internal node
	    std::vector<int>::iterator c;
	    if (nodes[i->second.lowerNeighbors[0]].size<=0) //check if children have been processed yet
	    {
		//push all children onto stack
		for (c=i->second.lowerNeighbors.begin(); c!=i->second.lowerNeighbors.end(); c++)
		    st.push(nodes.find(*c));
	    }
	    else
	    {
		//add up all children (minus the one counted by criticalParent array) and pop
		i->second.size=-i->second.size;
		for (c=i->second.lowerNeighbors.begin(); c!=i->second.lowerNeighbors.end(); c++)
		    i->second.size+=nodes[*c].size-1;
		st.pop();
		assert(i->second.size>0);
	    }
	}
    }
}

void ContourTree::clearAncestors()
{
    std::map<int,TreeNode>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {    
	i->second.ancestor=-1;
    }
    
}

void ContourTree::calcJTIsovalueAncestors(int voxel,int* order)
{
    int vo=order[voxel];
    std::stack<std::map<int,TreeNode>::iterator> st;
    std::map<int,TreeNode>::iterator i;
    clearAncestors();
    
    st.push(joinRoot);
    while (!st.empty())
    {
	i=st.top();
	st.pop();
	//set ancestor value
	if (order[i->first]<=vo)
	{
	    int parentAncestor;
	    if (i==joinRoot) parentAncestor=-1;
	    else parentAncestor=nodes[i->second.upperNeighbors[0]].ancestor;
	    if (parentAncestor==-1) i->second.ancestor=i->first;
	    else i->second.ancestor=parentAncestor;
	}
	//push children
	std::vector<int>::iterator c;
	for (c=i->second.lowerNeighbors.begin(); c!=i->second.lowerNeighbors.end(); c++)
	    st.push(nodes.find(*c));
    }
}

void ContourTree::labelJTDescendantsWithAncestorNode(int node)
{
    std::map<int,TreeNode>::iterator curNode=nodes.find(node);
    if (curNode->second.ancestor!=-1) return;
    std::stack<std::map<int,TreeNode>::iterator> st;
    st.push(curNode);
    while (!st.empty())
    {
	curNode=st.top();
	st.pop();
	//set ancestor value
	curNode->second.ancestor=node;
	//push children
	std::vector<int>::iterator c;
	for (c=curNode->second.lowerNeighbors.begin(); c!=curNode->second.lowerNeighbors.end(); c++)
	    st.push(nodes.find(*c));
    }
}

void ContourTree::setCriticalParentsJT(int* criticalParent)
{
    std::map<int,TreeNode>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if (i->second.upperNeighbors.size()>0)
	    criticalParent[i->first]=i->second.upperNeighbors[0];
	else
	    criticalParent[i->first]=i->first;
    }
}

int ContourTree::getNumNodes()
{
    return nodes.size();
}

int ContourTree::getJoinRootIndex()
{
    return joinRoot->first;
}

int ContourTree::getNodeSize(int node)
{
    return nodes[node].size;
}

int ContourTree::getNodeAncestor(int node)
{
    return nodes[node].ancestor;
}

int ContourTree::getNodeUpperEdge0Label(int node)
{
    return nodes[node].upperEdges[0].label;
}

int ContourTree::getNodeUpperEdge0EulerCh(int node)
{
    return nodes[node].upperEdges[0].eulerch;
}

int ContourTree::getNodeUpperEdge0Cavities(int node)
{
    return nodes[node].upperEdges[0].cavities;
}

std::map<int,TreeNode>::iterator ContourTree::getNode(int node)
{
    return nodes.find(node);
}

std::map<int,TreeNode>::iterator ContourTree::getBeginNode()
{
    return nodes.begin();
}

std::map<int,TreeNode>::iterator ContourTree::getEndNode()
{
    return nodes.end();
}

int ContourTree::getPositiveCriticals(int* critList)
{
    std::map<int,TreeNode>::iterator i;
    int n=0;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if ((i->second.upperNeighbors.size()==i->second.lowerNeighbors.size()+1) &&
	    (i->second.upperNeighbors.size()<=2))
	{
	    critList[n++]=i->first;
	}
    }
    return n;
}

int ContourTree::getNegativeCriticals(int* critList)
{
    std::map<int,TreeNode>::iterator i;
    int n=0;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if ((i->second.lowerNeighbors.size()==i->second.upperNeighbors.size()+1) &&
	    (i->second.lowerNeighbors.size()<=2))
	{
	    critList[n++]=i->first;
	}
    }
    return n;
}

int ContourTree::getNeutralCriticals(int* critList)
{
    std::map<int,TreeNode>::iterator i;
    int n=0;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if ((i->second.upperNeighbors.size()==1) &&
	    (i->second.lowerNeighbors.size()==1))
	{
	    critList[n++]=i->first;
	}
    }
    return n;
}

int ContourTree::getComplexCriticals(int* critList)
{
    std::map<int,TreeNode>::iterator i;
    int n=0;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	if ((i->second.upperNeighbors.size()>=3) ||
	    (i->second.lowerNeighbors.size()>=3) ||
	    (i->second.upperNeighbors.size()+i->second.lowerNeighbors.size()>=4))
	{
	    critList[n++]=i->first;
	}
    }
    return n;
}

void ContourTree::getUpperEdgeList(int* edgeList,int* labels)
{
    std::map<int,TreeNode>::iterator i;
    int e=0;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	for (int j=0; j<i->second.upperNeighbors.size(); j++)
	{
	    if ((labels) && (edgesInitialized)) labels[e/2]=i->second.upperEdges[j].label;
	    if (edgeList)
	    {
		edgeList[e++]=i->first;
		edgeList[e++]=i->second.upperNeighbors[j];
	    }
	}
    }
}

void ContourTree::printLowerRecursion(ostream& out,int index,int depth,int simplified)
{
    if ((!simplified) || (nodes[index].lowerNeighbors.size() != 1))
    {
	for (int i=0; i<depth; i++) out << '|';
	if (nodes[index].lowerNeighbors.size() > 0) out << '+';
	else out << '-';
	out << index;
	if (depth>0) out << " i" << nodes[index].upperEdges[0].label;
	out << '\n';
	std::vector<int>::iterator it;
	for (it=nodes[index].lowerNeighbors.begin(); it != nodes[index].lowerNeighbors.end(); it++)
	    printLowerRecursion(out,*it,depth+1,simplified);
    }
    else
    {
	printLowerRecursion(out,nodes[index].lowerNeighbors[0],depth,simplified);
    }
}

void ContourTree::printUpperRecursion(ostream& out,int index,int depth,int simplified)
{
    if ((!simplified) || (nodes[index].upperNeighbors.size() != 1))
    {
	for (int i=0; i<depth; i++) out << '|';
	if (nodes[index].upperNeighbors.size() > 0) out << '+';
	else out << '-';
	out << index << '\n';
	std::vector<int>::iterator it;
	for (it=nodes[index].upperNeighbors.begin(); it != nodes[index].upperNeighbors.end(); it++)
	    printUpperRecursion(out,*it,depth+1,simplified);
    }
    else
    {
	printUpperRecursion(out,nodes[index].upperNeighbors[0],depth,simplified);
    }
}

void ContourTree::printJT(ostream& out,int simplified)
{
    printLowerRecursion(out,joinRoot->first,0,simplified);
}

void ContourTree::printST(ostream& out,int simplified)
{
    printUpperRecursion(out,splitRoot->first,0,simplified);
}

void ContourTree::print(ostream& out)
{
    std::map<int,TreeNode>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	//if (i->second.lowerNeighbors.size()>2)
	//{
	    out << i->first << ": ";
	    std::vector<int>::iterator it;
	    if (edgesInitialized)
	    {
		for (int j=0; j<i->second.lowerNeighbors.size(); j++)
		    out << i->second.lowerNeighbors[j] << "[" << i->second.lowerEdges[j].label << "],";
		cout << "( ";
		for (int j=0; j<i->second.upperNeighbors.size(); j++)
		    out << i->second.upperNeighbors[j] << "[" << i->second.upperEdges[j].label << "] ";
		cout << ")\n";
	    }
	    else
	    {
		for (it=i->second.lowerNeighbors.begin(); it != i->second.lowerNeighbors.end(); it++)
		    out << *it << ",";
		cout << "( ";
		for (it=i->second.upperNeighbors.begin(); it != i->second.upperNeighbors.end(); it++)
		    out << *it << " ";
		cout << ")\n";
	    }
	//}
    }
}

void ContourTree::printJT(int simplified)
{
    printJT(cout,simplified);
}

void ContourTree::printST(int simplified)
{
    printST(cout,simplified);
}

void ContourTree::print()
{
    print(cout);
}

void ContourTree::printWithData(ostream& out,unsigned short* data,int dataLength,int* vsize,int isovalue)
{
    volume v(data,vsize);
    v.readTopoinfoFiles();
    std::map<int,TreeNode>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); i++)
    {
	int d=65536;
	if ((0<=i->first) && (i->first<dataLength)) d=data[i->first];
	//if (i->second.ancestor==isovalue)
	//{
	    out << i->first;
	    if (vsize) if (v.voxelCritical(i->first)) out << "*";
	    out << "\t" << d << "\t";
	    //out << "size=" << i->second.size << "\t";
	    std::vector<int>::iterator it;
	    if (edgesInitialized)
	    {
		for (int j=0; j<i->second.lowerNeighbors.size(); j++)
		    out << i->second.lowerNeighbors[j] << "[" << i->second.lowerEdges[j].label << "],";
		cout << "( ";
		for (int j=0; j<i->second.upperNeighbors.size(); j++)
		    out << i->second.upperNeighbors[j] << "[" << i->second.upperEdges[j].label << "] ";
		cout << ")\n";
	    }
	    else
	    {
		for (it=i->second.lowerNeighbors.begin(); it != i->second.lowerNeighbors.end(); it++)
		    out << *it << ",";
		cout << "( ";
		for (it=i->second.upperNeighbors.begin(); it != i->second.upperNeighbors.end(); it++)
		    out << *it << " ";
		cout << ")\n";
	    }
	//}
    }
}

int ContourTree::readFile(char* filename)
{
    ifstream fin(filename);
    if (!fin)
    {
        cerr << "Cannot open " << filename << " for reading.\n";
        return 1;
    }
    //cout << "Reading " << filename << " ..."; cout.flush();
    
    int numNodes,index,numUpperNeighbors,numLowerNeighbors,neighbor;
    numNodes=nodes.size();
    fin.read((char*)&numNodes,sizeof(int));
    
    int i;
    int j;
    for (i=0; i!=numNodes; ++i)
    {
	fin.read((char*)&index,sizeof(int));
	TreeNode n;
	fin.read((char*)&numUpperNeighbors,sizeof(int));
	for (j=0; j!=numUpperNeighbors; ++j)
	{
	    fin.read((char*)&neighbor,sizeof(int));
	    n.upperNeighbors.push_back(neighbor);
	}
	fin.read((char*)&numLowerNeighbors,sizeof(int));
	for (j=0; j!=numLowerNeighbors; ++j)
	{
	    fin.read((char*)&neighbor,sizeof(int));
	    n.lowerNeighbors.push_back(neighbor);
	}
	nodes[index]=n;
    }
    
    fin.close();
    //cout << "done.\n";
    return 0;
}

int ContourTree::writeFile(char* filename)
{
    ofstream fout(filename);
    if (!fout)
    {
        cerr << "Cannot open " << filename << " for writing.\n";
        return 1;
    }
    cout << "Writing to " << filename << " ..."; cout.flush();
    
    int numNodes,index,numUpperNeighbors,numLowerNeighbors,neighbor;
    numNodes=nodes.size();
    fout.write((char*)&numNodes,sizeof(int));
    
    std::map<int,TreeNode>::iterator i;
    std::vector<int>::iterator j;
    for (i=nodes.begin(); i!=nodes.end(); ++i)
    {
	index=i->first;
	fout.write((char*)&index,sizeof(int));
	numUpperNeighbors=i->second.upperNeighbors.size();
	fout.write((char*)&numUpperNeighbors,sizeof(int));
	for (j=i->second.upperNeighbors.begin(); j!=i->second.upperNeighbors.end(); ++j)
	{
	    neighbor=*j;
	    fout.write((char*)&neighbor,sizeof(int));
	}
	numLowerNeighbors=i->second.lowerNeighbors.size();
	fout.write((char*)&numLowerNeighbors,sizeof(int));
	for (j=i->second.lowerNeighbors.begin(); j!=i->second.lowerNeighbors.end(); ++j)
	{
	    neighbor=*j;
	    fout.write((char*)&neighbor,sizeof(int));
	}
    }
	
    fout.close();
    cout << "done.\n";
    return 0;	
}


PostOrderTraversalJT::PostOrderTraversalJT(ContourTree* p_jt)
{
    jt=p_jt;
}

void PostOrderTraversalJT::init()
{
    while (!nodeStack.empty()) nodeStack.pop();
    while (!childStack.empty()) childStack.pop();
    nodeStack.push(jt->getNode(jt->getJoinRootIndex()));
    childStack.push(0);
}

std::map<int,TreeNode>::iterator PostOrderTraversalJT::getCurrent()
{
    if (nodeStack.empty()) return jt->getEndNode();
    return nodeStack.top();
}

void PostOrderTraversalJT::next()
{
    std::map<int,TreeNode>::iterator curNode=nodeStack.top();
    int childrenTraversed=childStack.top();
    if (curNode->second.lowerNeighbors.size()==childrenTraversed)
    {
	//This node is finished
	nodeStack.pop();
	childStack.pop();
    }
    else
    {
	//Increment traversed child count
	childStack.pop();
	childStack.push(childrenTraversed+1);
	
	//Push next child onto stack
	nodeStack.push(jt->getNode(curNode->second.lowerNeighbors[childrenTraversed]));
	childStack.push(0);
    }
    
}

int PostOrderTraversalJT::hasNext()
{
    return !nodeStack.empty();
}

