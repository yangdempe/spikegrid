#include "STD.h"
#include "paramheader.h"
#include "helpertypes.h"
STD_data STD_init(const STD_parameters* s)
{
    STD_data ret = {.P=s};
    for(int i=0;i < grid_size*grid_size;i++)
    {
        ret.ftimes[i]=0; //probably not required
        ret.U[i] = s->U;
        ret.R[i] = One;
    }
    return ret;
}


