//contoursearch.cpp
//James Vanderhyde, 10 January 2007.

#include "ContourQuery.h"

int verbose=0;

int main(int argc,char* argv[])
{
    if (argc<=1)
    {
	cerr << "Usage: " << argv[0] << " <input .v file> [<tree file prefix> [threshold | <.vo order file>]]\n";
	return 1;
    }
    
    int useTreeFiles=0;
    if (argc>2) useTreeFiles=1;
    
    int simplifyTopology=0;
    unsigned short threshold=65535;
    char* orderfilename=NULL;
    if (argc>3)
    {
	char* suffix=strrchr(argv[3],'.');
	if ((suffix) && (!strcmp(suffix,".vo"))) orderfilename=argv[3];
	else
	{
	    threshold=atoi(argv[3]);
	    simplifyTopology=1;
	}
    }
    
    int result;
    
    Volume2DplusT v(threshold);
    
    v.readTopoinfoFiles();
    
    result=v.readFile(argv[1]);
    if (result) return result;
    
    cout << v.getSize()[0] << 'x' << v.getSize()[1] << 'x' << v.getSize()[2] << '=';
    cout << v.getSize()[0]*v.getSize()[1]*v.getSize()[2] << '\n';
    
    int signsNeedToBeChanged=v.dataShouldBeNegated();
    signsNeedToBeChanged=0;
    if (signsNeedToBeChanged) v.changeAllSigns();
    
    unsigned short min=65535,max=0;
    for (int i=0; i<v.getSize()[2]*v.getSize()[1]*v.getSize()[0]; i++)
    {
	if (v.getVoxel(i)<min) min=v.getVoxel(i);
	if (v.getVoxel(i)>max) max=v.getVoxel(i);
    }
    cout << "Min: " << min << ", Max: " << max << "\n";
    
    if (simplifyTopology)
    {
	clock_t starttime = clock();
	v.fixTopologyStrict();
	clock_t endtime = clock();
	cout << "Topology fixing time: " << (endtime-starttime)/(double)CLOCKS_PER_SEC << " sec\n";
	//v.saveOrder("carvingorder.vo");
    }
    else
    {
	v.sortVoxels();
	//v.saveOrder("sortedorder.vo");
	if (orderfilename) v.readCarvedOrder(orderfilename);
	//v.countCriticalsInThickSlices();
    }
    
    int windowSize=10;
    int numWindows=(v.getNumSlices()-windowSize-1)/(windowSize/2)+1;
    
    char* treeFilenameJT;
    char* treeFilenameST;
    char* treeFilenameCT;
    if (useTreeFiles)
    {
	treeFilenameJT=new char[strlen(argv[2])+10+4+1];
	treeFilenameST=new char[strlen(argv[2])+10+4+1];
	treeFilenameCT=new char[strlen(argv[2])+10+4+1];
    }
    
    int isovalue=2350;
    //isovalue=65535;
    int lastVoxel=v.findLastVoxelBelowIsovalue(isovalue);
    //cout << "Last voxel below isovalue " << isovalue << " is " << lastVoxel << "(value " << v.getVoxel(lastVoxel) << ")" << "\n";
    
    //v.buildTrees(v.getDefaultOrder(),1);
    //v.buildFullJoinTree();
    
    int** allResults=new int*[numWindows];
    for (int i=0; i<numWindows; i++) allResults[i]=NULL;
    int* allNumResults=new int[numWindows];
    
    int win=0;
    int numResults;
    int* results;
    int totalResults=0;
    for (int window=0; window<v.getNumSlices()-windowSize; window+=/*3**/(windowSize/2))
    {
	ContourQuery q(&v);
	if (useTreeFiles)
	{
	    sprintf(treeFilenameJT,"%s_jt%03d-%03d.ctb",argv[2],window,window+windowSize);
	    sprintf(treeFilenameST,"%s_st%03d-%03d.ctb",argv[2],window,window+windowSize);
	    sprintf(treeFilenameCT,"%s_ct%03d-%03d.ctb",argv[2],window,window+windowSize);
	    q.defineWindow(window,windowSize,treeFilenameJT,treeFilenameST,treeFilenameCT);
	}
	else
	{
	    q.defineWindow(window,windowSize);
	}
	
	if (0)
	{
	    std::vector<int> questionPath;
	    int questionBranch=q.findVoxelPathInTree(236293/*v.getVoxelIndex(52,50,27)*/,questionPath);
	    q.questionVoxel=questionBranch;
	}
	
	if (verbose) {cout << "Pruning..."; cout.flush();}
	q.calcCompsBelowIsovalueOf(lastVoxel);
	if (verbose) cout << "done: " << q.getNumResults() << "\n";
	if (verbose) {cout << "Finding comps with low boundary overlap..."; cout.flush();}
	q.keepCompsWithBoundarySliceOverlapInRange(1,3);
	if (verbose) cout << "done: " << q.getNumResults() << "\n";
	/*if (verbose) {cout << "Finding comps with low inner slice overlap..."; cout.flush();}
	for (int is=1; is<windowSize; is++) q.keepCompsWithSliceOverlapInRange(window+is,1,3);
	if (verbose) cout << "done: " << q.getNumResults() << "\n";*/
	//q.keepSmallComps(150);
	/*if (verbose) {cout << "Decreasing isovalue..."; cout.flush();}
	q.keepOnlyContainedSublevelSets();
	if (verbose) cout << "done: " << q.getNumResults() << "\n";*/
	if (verbose) {cout << "Collapsing..."; cout.flush();}
	q.keepOnlyContainingSublevelSets();
	if (verbose) cout << "done: " << q.getNumResults() << "\n";
	if (verbose) {cout << "Finding comps with low number of cavities..."; cout.flush();}
	q.keepCompsWithSmallNumberOfCavities(1);
	if (verbose) cout << "done: " << q.getNumResults() << "\n";
	if (verbose) {cout << "Finding comps with low number of handles..."; cout.flush();}
	q.keepCompsWithSmallNumberOfHandles(0);
	if (verbose) cout << "done: " << q.getNumResults() << "\n";
	
	numResults=q.getNumResults();
	results=new int[numResults];
	q.getResults(results);
	
	if (verbose)
	{
	    //v.buildFullJoinTree();
	    //q.getWindowTree()->calcJTIsovalueAncestors(isovalue,v.getData());
	    cout << "Calculating node sizes..."; cout.flush();
	    q.getWindowTree()->calcJTNodeSizesMinima();
	    //q.getWindowTree()->calcJTNodeSizesVoxels(v.getCriticalParents(),v.getNumVoxels());
	    cout << "done.\n";
	    /*cout << "Calculating Euler characterisitic..."; cout.flush();
	    q.computeEulerCh();
	    cout << "done.\n";
	    cout << "Counting cavities..."; cout.flush();
	    q.computeCavities();
	    cout << "done.\n";*/
	    if (1)
	    {
		cout << "Calculating mid slice induced map..."; cout.flush();
		ContourTree sliceJT;
		v.buildJoinTreeForSlice(v.getDefaultOrder(),window+windowSize/2,sliceJT);
		q.getWindowTree()->labelEdgesWithInducedMap(sliceJT);
		cout << "done.\n";
	    }
	    
	}
	
	for (int i=0; i<numResults; i++)
	{
	    cout << results[i] << "\t";
	    cout << window << " " << window+windowSize << "\t";
	    if (verbose)
	    {
		cout << "value=" << v.getVoxel(results[i]) << "  \t";
		cout << "size=" << q.getWindowTree()->getNodeSize(results[i]) << "   \t";
		cout << "cavities=" << q.getWindowTree()->getNodeUpperEdge0Cavities(results[i]) << "\t";
		//cout << "chi=" << q.getWindowTree()->getNodeUpperEdge0EulerCh(results[i]) << "    \t";
		cout << "handles=" << 1+q.getWindowTree()->getNodeUpperEdge0Cavities(results[i])-q.getWindowTree()->getNodeUpperEdge0EulerCh(results[i]) << "\t";
		cout << "inncomps=" << q.getWindowTree()->getNodeUpperEdge0Label(results[i]) << "\t";
	    }
	    /*cout << "iso ancestor=" << q.getWindowTree()->getNodeAncestor(results[i]) << "\t";
	    
	    q.getWindowTree()->calcJTIsovalueAncestors(v.getVoxel(results[i]),v.getData());
	    v.labelFullJoinTree(v.getVoxel(results[i]));
	    //windowJT.printWithData(cout,v.getData(),v.getNumVoxels(),NULL,*i);
	    //v.getFullJoinTree()->printWithData(cout,v.getData(),v.getNumVoxels(),NULL,*i);
	    
	    for (int index=window; index<(window+windowSize)*v.getSliceSize(); index++)
	    {
		//if (windowJT.getNodeAncestor(v.traceLowerCriticalAncestor(index))==*i)
		if (v.voxelBelowJTEdge(index,results[i])) 
		{
		    cout << " " << index;
		    //cout << " " << v.getCriticalParent(index);
		    //cout << "\t";
		}
	    
	    }
	    */
	    cout << "\n";
	    totalResults++;
	}
	assert(win<numWindows);
	allNumResults[win]=numResults;
	allResults[win]=results;
	win++;
    }
    
    cout << "\nTotal number of results is " << totalResults << "\n\n";
    
    if (0) for (int f=0; f<allNumResults[0]; f++)
    {
	//maybe this should actually be a tree, not a chain
	cout << "Chain of results starting from first window\n";
	int index=allResults[0][f],index1,index2;
	for (int i=1; i<numWindows; i++)
	{
	    
	    cout << index << "\n";
	    int change=0;
	    index1=index;
	    for (int j=0; j<allNumResults[i]; j++)
	    {
		index2=allResults[i][j];
		if (v.getDefaultOrder()[index1]>v.getDefaultOrder()[index2])
		{
		    v.markVoxelsBelowVoxelInSlices(index1,(i-1)*windowSize/2,(i+2)*windowSize/2);
		    //cout << index1 << " > " << index2 << ", ";
		}
		else
		{
		    v.markVoxelsBelowVoxelInSlices(index2,(i-1)*windowSize/2,(i+2)*windowSize/2);
		    //cout << index1 << " < " << index2 << ", ";
		}
		//cout << "range is " << ((i-1)*windowSize/2)*v.getSliceSize() << "..." << ((i+2)*windowSize/2)*v.getSliceSize()-1 << ", ";
		if ((v.voxelCarved(index1)) && (v.voxelCarved(index2)))
		{
		    index=index2;
		    change=1;
		    //cout << "both are marked.";
		}
		//cout << "\n";
	    }
	    if (!change) break;
	}
	cout << "\n";
    }
    
    for (int i=0; i<numWindows; i++) if (allResults[i]) delete[] allResults[i];
    delete[] allResults;
    delete[] allNumResults;
    if (useTreeFiles)
    {
	delete[] treeFilenameJT;
	delete[] treeFilenameST;
	delete[] treeFilenameCT;
    }
    
    if (0) //count violations of 1-to-1
    {
	cout << "Checking contour trees for thick slices"; cout.flush();
	
	int vio0=0,vio1=0;
	ContourTree jt0,st0,jt1,st1,jt,st;
	v.buildJoinTreeForSlice(v.getDefaultOrder(),0,jt0);
	v.buildSplitTreeForSlice(v.getDefaultOrder(),0,st0);
	ContourTree ct0(jt0,st0);
	
	int s;
	for (s=0; s<v.getNumSlices()-1; s++)
	{
	    jt1.clear();
	    st1.clear();
	    v.buildJoinTreeForSlice(v.getDefaultOrder(),s+1,jt1);
	    v.buildSplitTreeForSlice(v.getDefaultOrder(),s+1,st1);
	    ContourTree ct1(jt1,st1);
	    jt.clear();
	    st.clear();
	    v.buildJoinTreeForThickSlice(v.getDefaultOrder(),s,jt,1);
	    v.buildSplitTreeForThickSlice(v.getDefaultOrder(),s,st,1);
	    ContourTree ct(jt,st);
	    ct.labelEdgesWithInducedMap(ct0);
	    //vio0+=ct.countInducedMapManyToOnes(v.getData(),v.getNumVoxels());
	    vio0+=ct.countInducedMapManyToOnes();
	    cout << ct.countInducedMapManyToOnes() << " ";
	    ct.labelEdgesWithInducedMap(ct1);
	    //vio1+=ct.countInducedMapManyToOnes(v.getData(),v.getNumVoxels());
	    vio1+=ct.countInducedMapManyToOnes();
	    cout << ct.countInducedMapManyToOnes() << "\n";
	    
	    ct0=ct1;
	    cout << "."; cout.flush();
	}
	cout << "done: " << vio0 << " left violations and " << vio1 << " right violations." << "\n";
    }
    
    if (0) //merge thick slice join trees
    {
	ContourTree jt1,jt2;
	v.buildJoinTreeForThickSlice(v.getDefaultOrder(),0,jt1,1);
	v.buildJoinTreeForThickSlice(v.getDefaultOrder(),1,jt2,1);
	
	/*cout << "Join tree for thick slice 0-1\n";
	jt1.printWithData(cout,v.getData(),v.getNumVoxels());
	cout << "Join tree for thick slice 1-2\n";
	jt2.printWithData(cout,v.getData(),v.getNumVoxels());*/
	
	ContourTree jt;
	std::vector<int>* nodes=new std::vector<int>;
	//v.mergeJoinTrees(v.getDefaultOrder(),0,2,jt,nodes);
	delete nodes;
	/*cout << "Merged join tree for slices 0-2\n";
	jt.printWithData(cout,v.getData(),v.getNumVoxels());*/
	
	ContourTree fullJT;
	nodes=new std::vector<int>;
	cout << "\nConstructing merged join tree for all slices"; cout.flush();
	v.mergeJoinTrees(v.getDefaultOrder(),0,v.getNumSlices(),fullJT,nodes);
	delete nodes;
	cout << "done:\n";
	fullJT.printWithData(cout,v.getData(),v.getNumVoxels());
    }
    
    
    return 0;
}
