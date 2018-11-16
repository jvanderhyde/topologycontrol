
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "bitc.h"

bitc::bitc() : size(0), v(NULL) {}

void bitc::freespace()
{
  if (v) free(v);
  size = 0;
  v=NULL;
}

void bitc::setbit ( int n )
{
  assert(n>=0);
  if (n>=size)
    {
	  int newsize=8*((n+8)/8);
      v = (unsigned char *)realloc(v,newsize/8*sizeof(unsigned char));
      memset(v+size/8,0,(newsize/8-size/8)*sizeof(unsigned char));
      size = newsize;
    }
  v[n/8] |= (1<<(n%8));
}

bitc::bitc ( int n )
{
  size = 8*((n+8)/8);  
  v = (unsigned char *)malloc(size/8*sizeof(unsigned char));
  memset(v,0,size/8*sizeof(unsigned char));
}

void bitc::resetbit ( int n )
{
  assert(n>=0 && n<size);
  v[n/8] &= ~(1<<(n%8)); 
}

int bitc::getbit ( int n )
{
  assert(n>=0);
  if (n>=size)
    return 0;
  return !!(v[n/8] & (1<<(n%8)));
}

void bitc::read ( istream &is, int bits )
{
  if (bits>size)
    this->setbit(bits-1);
  is.read((char*)v,(bits+8)/8);
}

void bitc::write ( ostream &os, int bits )
{
	os.write((char*)v,(bits+8)/8);
}

void bitc::clearall()
{
	memset(v,0,size/8*sizeof(unsigned char));
}

void bitc::copy(bitc& src)
{
	if (src.size>size)
		this->setbit(src.size-1);
	memcpy(v,src.v,size/8*sizeof(unsigned char));
}

int bitc::hasdata()
{
	return (size>0);
}
