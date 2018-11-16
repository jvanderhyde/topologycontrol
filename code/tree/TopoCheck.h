//TopoCheck.h
//James Vanderhyde, 29 November 2005

class TopoCheck
{
 protected:
  TopoCheck();
 public:
  virtual ~TopoCheck();
  virtual int topologyChange(int neighborhood) = 0;
  virtual int getDimension() = 0;
};

class TopoCheckNever : public TopoCheck
{
 public:
  TopoCheckNever();
  ~TopoCheckNever();
  int topologyChange(int neighborhood);
  virtual int getDimension();
};

class TopoCheckFromFile : public TopoCheck
{
 protected:
  unsigned char* lookupTable;
  TopoCheckFromFile();
  virtual int readTopoinfoFile(char* filename);
 public:
  virtual ~TopoCheckFromFile();
  virtual int topologyChange(int neighborhood);
  virtual int getDimension();
};

class TopoCheckStrict : public TopoCheckFromFile
{
 public:
  TopoCheckStrict();
};

class TopoCheckPatch : public TopoCheckFromFile
{
 public:
  TopoCheckPatch();
};

class TopoCheckThread : public TopoCheckFromFile
{
 public:
  TopoCheckThread();
};

class TopoCheck2DFromFile : public TopoCheckFromFile
{
 protected:
  TopoCheck2DFromFile();
  virtual int readTopoinfoFile(char* filename);
  virtual int getDimension();
};

class TopoCheck2DVertex : public TopoCheck2DFromFile
{
 public:
  TopoCheck2DVertex();
};

class TopoCheck2D3DCombo : public TopoCheck
{
 protected:
  TopoCheck2DVertex topo2D;
  TopoCheckStrict topo3D;
 public:
  TopoCheck2D3DCombo();
  virtual int topologyChange(int neighborhood);
  virtual int getDimension();
};

class TopoCheckGraph : public TopoCheck
{
protected:
    int dimension,numNodes;
    int* nodeConnectionLengths;
    int** nodeConnections;
    int* stack;
    int stackPointer; //index to first empty slot in stack, i.e. num elements in stack
    TopoCheckGraph();
    virtual int fixForConnectivityModel(int neighborhood);
public:
    virtual ~TopoCheckGraph();
    virtual int topologyChange(int neighborhood);
    virtual int getDimension() = 0;
    virtual int saveTopoinfoFile(char* filename);
};

class TopoCheckThick2D3D : public TopoCheckGraph
{
protected:
    virtual int fixForConnectivityModel(int neighborhood);
public:
    TopoCheckThick2D3D();
    virtual int getDimension();
};

class TopoCheckThick2D3DTable : public TopoCheckFromFile
{
public:
    TopoCheckThick2D3DTable();
};

