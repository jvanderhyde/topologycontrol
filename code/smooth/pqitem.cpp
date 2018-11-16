//pqitem.cpp
//James Vanderhyde, 20 May 2005

#include "pqitem.h"

pqitem::pqitem () : priority(0),x(0),y(0),z(0) {}
pqitem::pqitem ( float p, int xx, int yy, int zz ) : priority(p),x(xx),y(yy),z(zz) {}

pqitem::operator float()
{
    return priority;
}

ostream& operator<< (ostream& out, const pqitem& i)
{
    out << '(' << i.x << ',' << i.y << ',' << i.z << ')' << ':' << i.priority;
    return out;
}

