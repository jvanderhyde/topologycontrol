//FilterVolume.h
//James Vanderhyde, 18 June 2007.

#include "shortvolume.h"

class FloatComparator : public std::binary_function<int,int,int>
{
private:
    float* data;
    
public:
    FloatComparator(float* p_data);
    int operator()(int a,int b);
};

class FilterVolume : public volume
{
protected:
    
public:
    FilterVolume(unsigned short p_featureSize=65535,int p_fixStyle=0);
    virtual ~FilterVolume();
    unsigned short floatToShort(float val);
    float filter(int x,int y,int z);
    int fixTopologyByFiltering();

};

