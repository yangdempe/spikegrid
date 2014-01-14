#include "paramheader.h"
#include "helpertypes.h"
#include <tgmath.h>
STD_data STD_init(const STD_parameters* s);
static inline Compute_float STD_str (const STD_parameters* const s, const int x, const int y,const int time,const int lag, STD_data* const d)
{
    const int stdidx=x*grid_size+y;
    if (lag==1) //recalculate after spiking
    {
        const Compute_float dt = ((Compute_float)(time-(d->ftimes[stdidx])))/1000.0/Param.time.dt;//calculate inter spike interval in seconds
        d->ftimes[stdidx]=time; //update the time
        const Compute_float prevu=d->U[stdidx]; //need the previous U value
        d->U[stdidx] = s->U + d->U[stdidx]*(One- s->U)*exp(-dt/s->F);
        d->R[stdidx] = One + (d->R[stdidx] - prevu*d->R[stdidx] - One)*exp(-dt/s->D);
    }
    return d->U[stdidx] * d->R[stdidx] * 2.0; //multiplication by 2 is not in the cited papers, but you could eliminate it by multiplying some other parameters by 2, but multiplying by 2 here enables easier comparison with the non-STD model.  Max has an improvement that calculates a first-order approxiamation that should be included

}
