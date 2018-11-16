//pqitem.h
//James Vanderhyde, 20 May 2005

#include <queue>
#include <vector>
#include <iostream.h>

class pqitem
{
    friend ostream& operator<< (ostream& out, const pqitem& i);
public:
    float priority;
    int x,y,z;
    pqitem ();
    pqitem ( float p, int xx, int yy, int zz );
    operator float();
};

typedef std::priority_queue < pqitem, std::vector<pqitem>, std::greater<float> > minqueue;
typedef std::priority_queue < pqitem, std::vector<pqitem>, std::less<float> > maxqueue;
typedef std::vector<pqitem> pqvector;

