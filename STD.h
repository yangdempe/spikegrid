/// \file
#include "layer.h"
#include "mymath.h"
#include <stdio.h>
STD_data STD_init(const STD_parameters s);
///calculation of STD strength.  In .h file for inlining (might not be required)
static inline Compute_float STD_str (const STD_parameters const s, const int x, const int y,const unsigned int time,const unsigned int lag, STD_data* const d)
{
    const int stdidx=x*grid_size+y;
    if (lag==1) //recalculate after spiking
    {
        const Compute_float spike_interval = ((Compute_float)(time-(d->ftimes[stdidx])))/((Compute_float)1000.0)/Features.Timestep;//calculate inter spike interval in seconds
        d->ftimes[stdidx]=time; //update the time
        const Compute_float prevu=d->U[stdidx]; //need the previous U value
        d->U[stdidx] = s.U + d->U[stdidx]*(One- s.U)*exp(-spike_interval/s.F);
        d->R[stdidx] = One + (d->R[stdidx] - prevu*d->R[stdidx] - One)*exp(-spike_interval/s.D);
    }
    return d->U[stdidx] * d->R[stdidx] * Two; //multiplication by 2 is not in the cited papers, but you could eliminate it by multiplying some other parameters by 2, but multiplying by 2 here enables easier comparison with the non-STD model.  Max has an improvement that calculates a first-order approxiamation that should be included

}
