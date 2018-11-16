//priorityQueue.h
//James Vanderhyde, 31 march 2006
//Contains all the priority queue structures

#include <queue>
#include <vector>
#include <iostream.h>

class pqitem
{
  friend ostream& operator<< (ostream& out, const pqitem& i);
  friend int operator< (const pqitem& i1,const pqitem& i2);
  friend int operator> (const pqitem& i1,const pqitem& i2);
 public:
  int index;
  int value;
  char requeued;
  pqitem ();
  pqitem ( int i, int val );
};

typedef std::priority_queue < pqitem, std::vector<pqitem>, std::greater<pqitem> > minqueue;
typedef std::priority_queue < pqitem, std::vector<pqitem>, std::less<pqitem> > maxqueue;
typedef std::queue<pqitem> voxelqueue;
typedef std::vector<pqitem> pqvector;
typedef std::vector<int> intvector;

class priorityQueueWrapper
{
 public:
  virtual void push(pqitem i) = 0;
  virtual void pop() = 0;
  virtual pqitem top() = 0;
  virtual int empty() = 0;
};

class minQueueWrapper : public priorityQueueWrapper
{
 protected:
  minqueue q;
 public:
  virtual void push(pqitem i);
  virtual void pop();
  virtual pqitem top();
  virtual int empty();
};

class maxQueueWrapper : public priorityQueueWrapper
{
 protected:
  maxqueue q;
 public:
  virtual void push(pqitem i);
  virtual void pop();
  virtual pqitem top();
  virtual int empty();
};

