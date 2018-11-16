//ContourQuery.cpp
//James Vanderhyde, 6 February 2007.

#include "ContourQuery.h"

void ContourQuery::sortedListIntersection(std::vector<int>& v1,std::vector<int>& v2,std::vector<int>& v)
{
    std::vector<int>::iterator i1,i2;
    
    //cout << "Intersection of\n";
    //for (i1=v1.begin(); i1!=v1.end(); i1++) cout << " " << *i1;
    //cout << "\nand\n";
    //for (i2=v2.begin(); i2!=v2.end(); i2++) cout << " " << *i2;
    //cout << "\nis\n";
    
    i1=v1.begin();
    i2=v2.begin();
    while ((i1!=v1.end()) && (i2!=v2.end()))
    {
	if (*i1==*i2)
	{
	    v.push_back(*i1);
	    i1++;
	    i2++;
	}
	else if (*i1<*i2) i1++;
	else i2++;
    }

    //for (i2=v.begin(); i2!=v.end(); i2++) cout << " " << *i2;
    //cout << "\n\n";
}

ContourQuery::ContourQuery(Volume2DplusT* v)
{
    vol=v;
    windowStart=0;
    windowLength=vol->getNumSlices();
    questionVoxel=-1;
    cavitiesComputed=0;
    eulerComputed=0;
}

ContourQuery::~ContourQuery()
{
}

int ContourQuery::getNumResults()
{
    return queryResult.size();
}

void ContourQuery::getResults(int* results)
{
    std::vector<int>::iterator i;
    int r=0;
    for (i=queryResult.begin(); i!=queryResult.end(); i++) results[r++]=*i;
}

ContourTree* ContourQuery::getWindowTree()
{
    return &windowJT;
}

ContourTree* ContourQuery::getWindowCT()
{
    return &windowCT;
}

int ContourQuery::containsVoxel(int index)
{
    for (std::vector<int>::iterator i=queryResult.begin(); i!=queryResult.end(); i++)
    {
	if (*i==index) return 1;
    }
    return 0;
}

void ContourQuery::defineWindow(int startSlice,int numSlices,char* filenameJT,char* filenameST,char* filenameCT)
{
    windowStart=startSlice;
    windowLength=numSlices;
    windowJT.clear();
    windowST.clear();
    windowCT.clear();
    std::vector<int>* nodes;
    int verbose=0;
    
    if (filenameJT)
    {
	int result=windowJT.readFile(filenameJT);
	if (result) filenameJT=NULL;
	else windowJT.findJoinRoot();
    }
    if (filenameST)
    {
	int result=windowST.readFile(filenameST);
	if (result) filenameST=NULL;
	else windowST.findSplitRoot();
    }
    if (filenameCT)
    {
	int result=windowCT.readFile(filenameCT);
	if (result) filenameCT=NULL;
    }
    
    if (!filenameJT)
    {
	if (verbose)
	{
	    cout << "\nConstructing merged  join tree for window " << windowStart << "-" << windowStart+windowLength;
	    cout.flush();
	}
	nodes=new std::vector<int>;
	vol->mergeJoinTrees(vol->getDefaultOrder(),windowStart,windowStart+windowLength,windowJT,nodes);
	delete nodes;
	if (verbose)
	{
	    cout << "done.\n";
	}
    }
    if (!filenameST)
    {
	if (verbose)
	{
	    cout << "Constructing merged split tree for window " << windowStart << "-" << windowStart+windowLength; 
	    cout.flush();
	}
	nodes=new std::vector<int>;
	vol->mergeSplitTrees(vol->getDefaultOrder(),windowStart,windowStart+windowLength,windowST,nodes);
	delete nodes;
	if (verbose)
	{
	    cout << "done.\n";
	}
    }
    if (!filenameCT)
    {
	if (verbose)
	{
	    cout << "Constructing contour tree for window " << windowStart << "-" << windowStart+windowLength << "..."; 
	    cout.flush();
	}
	ContourTree mergedCT(windowJT,windowST);
	mergedCT.clone(windowCT);
	if (verbose)
	{
	    cout << "done.\n";
	}	
    }
    
    windowJT.initEdges();
    windowST.initEdges();
    windowCT.initEdges();
    
    if (questionVoxel!=-1)
    {
	int x,y,z;
	vol->getVoxelLocFromIndex(questionVoxel,&x,&y,&z);
	cout << "Voxel " << questionVoxel << " is " << ((vol->voxelCriticalInVolume(questionVoxel))?"":"not ") << "critical in volume.\n";
	cout << "Voxel " << questionVoxel << " is " << ((vol->voxelCriticalInSlice(questionVoxel))?"":"not ") << "critical in slice.\n";
	cout << "Voxel " << questionVoxel << " is " << ((vol->voxelCriticalInThickSlice(questionVoxel,0))?"":"not ") << "critical in left thick slice.\n";
	cout << "Voxel " << questionVoxel << " is " << ((vol->voxelCriticalInThickSlice(questionVoxel,1))?"":"not ") << "critical in right thick slice.\n";
    }
}

void ContourQuery::checkQuestionVoxel()
{
    if (questionVoxel!=-1)
    {
	cout << "Question voxel ";
	if (containsVoxel(questionVoxel)) cout << "in results.\n";
	else cout << "not in results.\n";
    }
}

void ContourQuery::computeBoundarySliceOverlaps()
{
    //build slice trees
    ContourTree jt0,jt1;
    vol->buildJoinTreeForSlice(vol->getDefaultOrder(),windowStart,jt0);
    vol->buildJoinTreeForSlice(vol->getDefaultOrder(),windowStart+windowLength,jt1);
    
    //label edges
    windowJT.labelEdgesWithInducedMap(jt0);
    
    //copy label into storage
    std::map<int,TreeNode>::iterator curNode;
    for (curNode=windowJT.getBeginNode(); curNode!=windowJT.getEndNode(); ++curNode)
    {
	if (!(curNode->second.upperEdges.empty())) 
	    curNode->second.upperEdges[0].leftSliceOverlap=curNode->second.upperEdges[0].label;
    }
    
    //label edges
    windowJT.labelEdgesWithInducedMap(jt1);
    
    //copy label into storage
    for (curNode=windowJT.getBeginNode(); curNode!=windowJT.getEndNode(); ++curNode)
    {
	if (!(curNode->second.upperEdges.empty())) 
	    curNode->second.upperEdges[0].rightSliceOverlap=curNode->second.upperEdges[0].label;
    }
}

void ContourQuery::calcCompsWithBoundarySliceOverlapInRange(int min,int max)
{
    //Calc overlap with boundary
    ContourTree jt0,jt1;
    vol->buildJoinTreeForSlice(vol->getDefaultOrder(),windowStart,jt0);
    vol->buildJoinTreeForSlice(vol->getDefaultOrder(),windowStart+windowLength,jt1);
    std::vector<int> leftEdges,rightEdges;
    windowJT.labelEdgesWithInducedMap(jt0);
    windowJT.getJTEdgesWithLabelsInRange(min,max,leftEdges);
    if (questionVoxel>=0) printTreeBelow(questionVoxel);
    //cout << "Left join tree:\n";
    //windowJT.printWithData(cout,vol->getData(),vol->getNumVoxels(),vol->getSize(),2000);
    windowJT.labelEdgesWithInducedMap(jt1);
    windowJT.getJTEdgesWithLabelsInRange(min,max,rightEdges);
    if (questionVoxel>=0) printTreeBelow(questionVoxel);
    //cout << "Right join tree:\n";
    //windowJT.printWithData(cout,vol->getData(),vol->getNumVoxels(),vol->getSize(),2000);
    
    queryResult.clear();
    sortedListIntersection(leftEdges,rightEdges,queryResult);
    checkQuestionVoxel();
}

void ContourQuery::keepCompsWithBoundarySliceOverlapInRange(int min,int max)
{
    computeBoundarySliceOverlaps();
    
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	std::map<int,TreeNode>::iterator node=windowJT.getNode(*i);
	if (!(node->second.upperEdges.empty()))
	{
	    int comps0=node->second.upperEdges[0].leftSliceOverlap;
	    int comps1=node->second.upperEdges[0].rightSliceOverlap;
	    if ((min<=comps0) && (comps0<=max) &&
		(min<=comps1) && (comps1<=max))
		queryResult.push_back(*i);
	    
	}
    }
    checkQuestionVoxel();
}

void ContourQuery::keepCompsWithSliceOverlapInRange(int slice,int min,int max)
{
    ContourTree jt;
    vol->buildJoinTreeForSlice(vol->getDefaultOrder(),slice,jt);
    std::vector<int> edges;
    windowJT.labelEdgesWithInducedMap(jt);
    windowJT.getJTEdgesWithLabelsInRange(min,max,edges);
    
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    sortedListIntersection(oldResults,edges,queryResult);
    checkQuestionVoxel();
}

void ContourQuery::keepCompsWithSliceOverlapInBoundaryRange(int slice)
{
    ContourTree jt;
    vol->buildJoinTreeForSlice(vol->getDefaultOrder(),slice,jt);
    windowJT.labelEdgesWithInducedMap(jt);
    
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	std::map<int,TreeNode>::iterator node=windowJT.getNode(*i);
	if (!(node->second.upperEdges.empty()))
	{
	    int comps0=node->second.upperEdges[0].leftSliceOverlap;
	    int comps1=node->second.upperEdges[0].rightSliceOverlap;
	    int comps=node->second.upperEdges[0].label;
	    if (((comps0<=comps) && (comps<=comps1)) ||
		((comps1<=comps) && (comps<=comps0)))
		queryResult.push_back(*i);
	}
    }
    checkQuestionVoxel();
}

void ContourQuery::keepSmallComps(int maxSize)
{
    //Keep small size
    windowJT.calcJTNodeSizesMinima();
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	if (windowJT.getNodeSize(*i)<=maxSize)
	    queryResult.push_back(*i);
    }
    checkQuestionVoxel();
}

void ContourQuery::calcCompsBelowIsovalueOf(int voxel)
{
    queryResult.clear();
    
    //Keep good isovalue range
    int vo=vol->getVoxelOrder(voxel);
    std::map<int,TreeNode>::iterator curNode;
    for (curNode=windowJT.getBeginNode(); curNode!=windowJT.getEndNode(); ++curNode)
    {
	if (vol->getVoxelOrder(curNode->first)<=vo)
	    queryResult.push_back(curNode->first);
    }
    checkQuestionVoxel();
}

void ContourQuery::keepCompsBelowIsovalue(int voxel)
{
    //Keep good isovalue range
    int vo=vol->getVoxelOrder(voxel);
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	if (vol->getVoxelOrder(*i)<=vo)
	    queryResult.push_back(*i);
    }
    checkQuestionVoxel();
}

void ContourQuery::keepOnlyContainingSublevelSets(int voxel)
{
    //Throw away any component contained in another component in the list.
    //Uses given voxel as a cut off.
    windowJT.calcJTIsovalueAncestors(voxel,vol->getDefaultOrder());
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	//cout << "Node " << *i << " has ancestor " << windowJT.getNodeAncestor(*i) << "\n";
	if (windowJT.getNodeAncestor(*i)==*i)
	    queryResult.push_back(*i);
    }
    checkQuestionVoxel();
}

void ContourQuery::keepOnlyContainingSublevelSets()
{
    //Throw away any component contained in another component in the list.
    windowJT.clearAncestors();
    std::vector<int>::iterator i;
    for (i=queryResult.begin(); i!=queryResult.end(); i++)
    {
	windowJT.labelJTDescendantsWithAncestorNode(*i);
    }
    if (questionVoxel>=0)
    {
	int anc=windowJT.getNodeAncestor(questionVoxel);
	cout << "Node " << questionVoxel << " has ancestor " << anc << "\n";
	cout << "Node " << anc << " has ancestor " << windowJT.getNodeAncestor(anc) << "\n";
    }
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	//cout << "Node " << *i << " has ancestor " << windowJT.getNodeAncestor(*i) << "\n";
	if (windowJT.getNodeAncestor(*i)==*i)
	    queryResult.push_back(*i);
    }
    checkQuestionVoxel();
}

void ContourQuery::keepOnlyContainedSublevelSets()
{
    //Throw away any component containing in another component in the list.
    windowJT.clearAncestors();
    std::vector<int>::iterator i;
    for (i=queryResult.begin(); i!=queryResult.end(); i++)
    {
	windowJT.labelJTDescendantsWithAncestorNode(*i);
    }
    if (questionVoxel>=0)
    {
	int anc=windowJT.getNodeAncestor(questionVoxel);
	cout << "Node " << questionVoxel << " has ancestor " << anc << "\n";
	cout << "Node " << anc << " has ancestor " << windowJT.getNodeAncestor(anc) << "\n";
    }
    
    std::map<int,int> ancs;
    for (std::vector<int>::iterator i=queryResult.begin(); i!=queryResult.end(); i++)
	if (windowJT.getNodeAncestor(*i)==*i)
	    ancs[*i]=0;
    for (std::vector<int>::iterator i=queryResult.begin(); i!=queryResult.end(); i++)
    {
	int anc=windowJT.getNodeAncestor(*i);
	if (anc!=*i)
	    ancs[anc]=1;
    }
    
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	//cout << "Node " << *i << " has ancestor " << windowJT.getNodeAncestor(*i) << "\n";
	if (windowJT.getNodeAncestor(*i)!=*i)
	    queryResult.push_back(*i);
	else if (ancs[*i]==0)
	    queryResult.push_back(*i);
    }
    checkQuestionVoxel();
}

void ContourQuery::keepWindowInterior()
{
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	int slice=*i/vol->getSliceSize();
	if ((windowStart<slice) && (slice<windowStart+windowLength))
	    queryResult.push_back(*i);
    }
    checkQuestionVoxel();
}

void ContourQuery::computeCavities()
{
    //label edges
    windowJT.labelEdgesWithInducedMap(windowCT);
    
    //copy into cavities
    std::map<int,TreeNode>::iterator curNode;
    PostOrderTraversalJT trav(&windowJT);
    trav.init();
    while (trav.hasNext())
    {
	curNode=trav.getCurrent();
	if (!(curNode->second.upperEdges.empty())) 
	    curNode->second.upperEdges[0].cavities=curNode->second.upperEdges[0].label-1;
	trav.next();
    }
    cavitiesComputed=1;
}

void ContourQuery::keepCompsWithSmallNumberOfCavities(int max)
{
    if (!cavitiesComputed) computeCavities();
    
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	std::map<int,TreeNode>::iterator node=windowJT.getNode(*i);
	int cav;
	if (!(node->second.upperEdges.empty())) cav=node->second.upperEdges[0].cavities;
	else cav=0;
	//cout << "Node " << node->first << " has " << cav << " cavities\n";
	if ((0<=cav) && (cav<=max))
	    queryResult.push_back(*i);
    }
    checkQuestionVoxel();
}

void ContourQuery::computeEulerCh()
{
    int xmin=0,ymin=0,zmin=windowStart;
    int xmax=vol->getSize()[0]-1,ymax=vol->getSize()[1]-1,zmax=windowStart+windowLength;
    std::map<int,TreeNode>::iterator curNode;
    for (curNode=windowJT.getBeginNode(); curNode!=windowJT.getEndNode(); ++curNode)
	if (!(curNode->second.upperEdges.empty())) curNode->second.upperEdges[0].eulerch=0;
    PostOrderTraversalJT trav(&windowJT);
    trav.init();
    rootChi=0;
    while (trav.hasNext())
    {
	curNode=trav.getCurrent();
	std::vector<int>::iterator c;
	int eulerCh=1; //add 1 for this voxel
	for (c=curNode->second.lowerNeighbors.begin(); c!=curNode->second.lowerNeighbors.end(); ++c)
	    eulerCh+=windowJT.getNode(*c)->second.upperEdges[0].eulerch; //add euler of children
	eulerCh-=vol->getBdEulerChBelowVoxelInRange(curNode->first,xmin,xmax,ymin,ymax,zmin,zmax); //subtract euler of voxel boundary intersection
	if (!(curNode->second.upperEdges.empty())) curNode->second.upperEdges[0].eulerch=eulerCh;
	else rootChi=eulerCh;
	trav.next();
    }
    eulerComputed=1;
}

void ContourQuery::keepCompsWithEulerChInRange(int min,int max)
{
    if (!eulerComputed) computeEulerCh();
    
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	std::map<int,TreeNode>::iterator node=windowJT.getNode(*i);
	int chi;
	if (!(node->second.upperEdges.empty())) chi=node->second.upperEdges[0].eulerch;
	else chi=rootChi;
	//cout << "Node " << node->first << " has Euler characteristic " << chi << "\n";
	if ((min<=chi) && (chi<=max))
	    queryResult.push_back(*i);
    }
    checkQuestionVoxel();
}

void ContourQuery::keepCompsWithSmallNumberOfHandles(int max)
{
    if (!cavitiesComputed) computeCavities();
    if (!eulerComputed) computeEulerCh();
    
    std::vector<int> oldResults(queryResult);
    queryResult.clear();
    for (std::vector<int>::iterator i=oldResults.begin(); i!=oldResults.end(); i++)
    {
	std::map<int,TreeNode>::iterator node=windowJT.getNode(*i);
	int handles;
	if (!(node->second.upperEdges.empty())) handles=1+node->second.upperEdges[0].cavities-node->second.upperEdges[0].eulerch;
	else handles=1+0-rootChi;
	//cout << "Node " << node->first << " has " << handles << " handles\n";
	if ((0<=handles) && (handles<=max))
	    queryResult.push_back(*i);
    }
    checkQuestionVoxel();
}

int ContourQuery::findVoxelPathInTree(int voxel,std::vector<int>& path)
{
    //cout << "Tracing voxel to local min..."; cout.flush();
    int localMin=vol->traceToLocalMinInRange(voxel,0,vol->getSize()[0]-1,0,vol->getSize()[1]-1,
					     windowStart,windowStart+windowLength);
    //cout << "done.\n";
    //cout << "Tree has " << windowJT.getNumNodes() << " nodes.\n";
    //cout << "Tracing min to root in tree..."; cout.flush();
    windowJT.getPathToJoinRoot(localMin,path);
    //cout << "done.\n";
    
    int* order=vol->getDefaultOrder();
    int x,y,z;
    vol->getVoxelLocFromIndex(voxel,&x,&y,&z);
    cout << "Finding voxel " << voxel << "(" << x << "," << y << "," << z << ")";
    cout << "[" << order[voxel] << "]" << " in path:\n";
    vol->getVoxelLocFromIndex(localMin,&x,&y,&z);
    cout << " Local min is " << localMin << "(" << x << "," << y << "," << z << ")";
    cout << "[" << order[localMin] << "]" << " in path:\n";
    cout.flush();
    assert(windowJT.getNode(localMin)!=windowJT.getEndNode());
    int criticalBelow=-1;
    std::vector<int>::iterator f=path.begin();
    vol->getVoxelLocFromIndex(*f,&x,&y,&z);
    cout << " " << *f << "(" << x << "," << y << "," << z << ")";
    cout << "[" << order[*f] << "]" << "\n";
    while ((f!=path.end()) && (order[*f]<=order[voxel]))
    {
	criticalBelow=*f;
	f++;
	vol->getVoxelLocFromIndex(*f,&x,&y,&z);
	cout << " " << *f << "(" << x << "," << y << "," << z << ")";
	cout << "[" << order[*f] << "]" << "\n";
    }
    cout << "done.\n";
    
    /*cout << "Path in join tree containing voxel " << voxel << ":\n";
    std::vector<int>::reverse_iterator i;
    for (i=path.rbegin(); i!=path.rend(); i++)
    {
	if (*i==criticalBelow) cout << "**  ";
	cout << *i << "  ";
    }*/
    
    //cout << "\n";
    
    return criticalBelow;
}

void ContourQuery::printTreeBelow(int critVoxel)
{
    windowJT.printLowerRecursion(cout,critVoxel,2,0);
    cout << "\n";
}

void ContourQuery::printResult()
{
    std::vector<int>::iterator i;
    for (i=queryResult.begin(); i!=queryResult.end(); i++)
	cout << *i << '\n';
}


