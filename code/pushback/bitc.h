
#include <iostream.h>

class bitc {
  int size;
  unsigned char *v;
 public:
  bitc();
  void freespace();
  bitc ( int n );
  void setbit ( int n );
  void resetbit ( int n );
  int getbit ( int n );
  void read ( istream &is, int bits );
  void write ( ostream &os, int bits );
  void clearall();
  void copy(bitc& src);
  int hasdata();
};

extern istream & operator>> ( istream &is, bitc &bc );
