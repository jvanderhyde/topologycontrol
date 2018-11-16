//TopoCheck4D.h
//James Vanderhyde, 14 May 2007

#include "TopoCheck.h"

class TopoCheck4D
{
 protected:
  TopoCheck4D();
 public:
  virtual ~TopoCheck4D();
  virtual int topologyChange(bool* neighborhood) = 0;
  virtual int getDimension();
  virtual void printNeighborhood(bool* neighborhood);
};

class TopoCheck4DLowerDim : public TopoCheck4D
{
protected:
    TopoCheck* topoLowD;
public:
    TopoCheck4DLowerDim(TopoCheck* new_topoLowD);
    virtual ~TopoCheck4DLowerDim();
    virtual int topologyChange(bool* neighborhood);
    virtual int getDimension();
};

class TopoCheck4DRestrictZ : public TopoCheck4D
{
protected:
    TopoCheck4D* topoCheck;
    int zBits[54];
public:
    TopoCheck4DRestrictZ(TopoCheck4D* new_topoCheck);
    virtual ~TopoCheck4DRestrictZ();
    virtual int topologyChange(bool* neighborhood); //clobbers 54 bits
    virtual int getDimension();
};

class TopoCheckGraph4D : public TopoCheck4D
{
protected:
    int dimension,numNodes;
    int* nodeConnectionLengths;
    int** nodeConnections;
    int* nodeConnectionLDLengths;
    int** nodeConnectionsLD;
    int numHiDimNodes;
    int* hiDimNodes;
    int* stack;
    int stackPointer; //index to first empty slot in stack, i.e. num elements in stack
    TopoCheckGraph4D();
    virtual void fixForConnectivityModel(bool* neighborhood) = 0;
public:
    virtual ~TopoCheckGraph4D();
    virtual int topologyChange(bool* neighborhood);
    int calcEulerCh(bool* neighborhood);
    //virtual int saveTopoinfoFile(char* filename);
    virtual void printNeighborhood(bool* neighborhood);
};

class TopoCheckStrict4D : public TopoCheckGraph4D
{
 protected:
    virtual void fixForConnectivityModel(bool* neighborhood);
 public:
    TopoCheckStrict4D();
};

class TopoCheck3D4DCombo : public TopoCheck4D
{
 protected:
  TopoCheckStrict topo3D;
  TopoCheckStrict4D topo4D;
 public:
  TopoCheck3D4DCombo();
  virtual int topologyChange(bool* neighborhood);
};

class TopoCheckThick3D4DDouble : public TopoCheck4D
{
protected:
    TopoCheckStrict4D topo4D;
public:
    TopoCheckThick3D4DDouble();
    virtual int topologyChange(bool* neighborhood); //first 27 bits are ignored (clobbered)
};

/*class TopoCheckThick3D4D : public TopoCheckGraph4D
{
protected:
    virtual void fixForConnectivityModel(bool* neighborhood);
public:
    TopoCheckThick3D4D();
};
*/

