/// \file
#ifndef LAGSTORAGE
#define LAGSTORAGE
#include <stdint.h>
#include "sizes.h"
/// TODO: Much of the time in the code appears to be in bracnch mispredicts caused by while loops in lagstorage
/// One solution would be to also add a count of the spikes in here - then we can count down to 0
/// it would also make adding more new spikes simpler.
/// There is a small memory cost so do some benchmarking
typedef struct lagstorage
{
    int16_t*    lags; //since the maximum time delay should be short, don't bother with a full int, using int16 will halve memory usage.  In the no-STDP case, lagstorage is a pretty significant amount of RAM usage
    const int   cap;
    const int   lagsperpoint;
} lagstorage;

lagstorage* lagstorage_init(const int flagcount,const int cap);
void AddnewSpike(lagstorage* L,const int baseidx);
void modifyLags(lagstorage* L,int baseidx);
int16_t __attribute__((const,pure)) CurrentShortestLag(const lagstorage* const L,const int baseidx);
///A small helper function to calculate the base idx for the lags at a given x and y coordinate.  Mostly just for convenience
inline static int __attribute__((const,pure)) LagIdx(const int x,const int y,const lagstorage* L) {return (x*grid_size+y)*L->lagsperpoint;}
#endif
