//priorityQueue.cpp
//James Vanderhyde, 31 March 2006

#include "priorityQueue.h"

pqitem::pqitem () : index(0),value(0),requeued(0) {}

pqitem::pqitem ( int i, int val ) : index(i),value(val),requeued(0) {}

ostream& operator<< (ostream& out, const pqitem& i)
{
  out << '(' << i.index << ')' << ':' << i.value;
  return out;
}

int operator< (const pqitem& i1,const pqitem& i2)
{
  if (i1.value != i2.value)
    return i1.value<i2.value;
  return i1.index<i2.index;
}

int operator> (const pqitem& i1,const pqitem& i2)
{
  if (i1.value != i2.value)
    return i1.value>i2.value;
  return i1.index>i2.index;
}


void minQueueWrapper::push(pqitem i)
{
  q.push(i);
}

void minQueueWrapper::pop()
{
  q.pop();
}

pqitem minQueueWrapper::top()
{
  return q.top();
}

int minQueueWrapper::empty()
{
  return q.empty();
}


void maxQueueWrapper::push(pqitem i)
{
  q.push(i);
}

void maxQueueWrapper::pop()
{
  q.pop();
}

pqitem maxQueueWrapper::top()
{
  return q.top();
}

int maxQueueWrapper::empty()
{
  return q.empty();
}

