///\file
#include <string.h> //memset
#include <stdio.h>  //useful when we need to print things
#include "STDP.h"
#include "model.h"
#include "theta.h"
#include "evolvegen.h"
#include "STD.h"
#include "mymath.h"
#include "paramheader.h"
#include "localstim.h"
#include "animal.h"
#include "randconns.h"
#include "lagstorage.h"
#ifdef ANDROID
    #define APPNAME "myapp"
    #include <android/log.h>
#endif

void RandSpikes(const unsigned int x,const unsigned int y,const layer L,Compute_float* __restrict__ gE, Compute_float* __restrict__ gI,const Compute_float str)
{
    unsigned int norand;
    const randomconnection* rcs = GetRandomConnsLeaving(x,y,*L.rcinfo,&norand);
    for (unsigned int i=0;i<norand;i++)
    {
        const int condindex = Conductance_index(rcs[i].destination.x,rcs[i].destination.y);
        if (L.Layer_is_inhibitory) {gI[condindex] += str * (rcs[i].strength + rcs[i].stdp_strength);}
        else                       {gE[condindex] += str * (rcs[i].strength + rcs[i].stdp_strength);}
    }
}

///Adds the effect of the spikes that have fired in the past to the gE and gI arrays as appropriate
/// currently, single layer doesn't work (correctly)
void AddSpikes(layer L, Compute_float* __restrict__ gE, Compute_float* __restrict__ gI,const unsigned int time)
{
    for (unsigned int y=0;y<grid_size;y++)
    {
        for (unsigned int x=0;x<grid_size;x++)
        {
            Compute_float str = Zero;
            const unsigned int lagidx = LagIdx(x,y,L.firinglags);
            unsigned int newlagidx = lagidx;
            while (L.firinglags->lags[newlagidx] != -1)  //Note: I think perf might be overstating the amount of time on this line - although, if it isn't massive potential for perf improvement
            {
                Compute_float this_str =L.Mytimecourse[L.firinglags->lags[newlagidx]];
                if (Features.STD == ON)
                {
                    this_str = this_str * STD_str(L.P->STD,x,y,time,L.firinglags->lags[newlagidx],L.std);
                }
                newlagidx++;
                str += this_str;
            }
            if (L.Layer_is_inhibitory) {str = (-str);} //invert strength for inhib conns.
            if (newlagidx != lagidx) //only fire if we had a spike.
            {
                if (Features.STDP==OFF)
                {
                    evolvept_duallayer((int)x,(int)y,L.connections,str,(L.Layer_is_inhibitory?gI:gE)); //side note evolvegen doesn't currently work with singlelayer - should probably fix
                }
                else
                {
                    evolvept_duallayer_STDP((int)x,(int)y,L.connections,L.STDP_data->connections,str,(L.Layer_is_inhibitory?gI:gE));
                }
            }
            if (Features.Random_connections == ON )
            {
               RandSpikes(x,y,L,gE,gI,str);
            }
        }
    }
}

///This function adds in the overlapping bits back into the original matrix.  It is slightly opaque but using pictures you can convince yourself that it works
///To keep the evolvept code simple we use an array like this:
///~~~~
///     +-––––––––––––––––+
///     |Extra for Overlap|
///     |  +––––––––––+   |
///     |  |Actual    |   |
///     |  |matrix    |   |
///     |  +––––––––––+   |
///     +–––––––––––––––––+
///~~~~
void fixboundary(Compute_float* __restrict gE, Compute_float* __restrict gI)
{   //theoretically the two sets of loops could be combined but that would be incredibly confusing
    //top + bottom
    for (int i=0;i<couplerange;i++)
	{
        for (int j=0;j<conductance_array_size;j++)
		{
            gE[(grid_size+i)*conductance_array_size + j] += gE[i*conductance_array_size+j]; //add to bottom
            gE[(i+couplerange)*conductance_array_size+j] += gE[(grid_size+couplerange+i)*conductance_array_size+j];//add to top
            gI[(grid_size+i)*conductance_array_size + j] += gI[i*conductance_array_size+j]; //add to bottom
            gI[(i+couplerange)*conductance_array_size+j] += gI[(grid_size+couplerange+i)*conductance_array_size+j];//add to top
		}
	}
    //left + right boundary condition fix
    for (int i=couplerange;i<couplerange+grid_size;i++)
	{
        for (int j=0;j<couplerange;j++)
		{
             gE[i*conductance_array_size +grid_size+j ]  += gE[i*conductance_array_size+j];//left
             gE[i*conductance_array_size +couplerange+j] += gE [i*conductance_array_size + grid_size+couplerange+j];//right
             gI[i*conductance_array_size +grid_size+j ]  += gI[i*conductance_array_size+j];//left
             gI[i*conductance_array_size +couplerange+j] += gI [i*conductance_array_size + grid_size+couplerange+j];//right
		}
	}
}

///rhs_func used when integrating the neurons forward through time.  The actual integration is done using the midpoint method
Compute_float __attribute__((const,pure)) rhs_func  (const Compute_float V,const Compute_float ge,const Compute_float gi,const conductance_parameters p)
{
    switch (p.type.type)
    {
        case LIF:
            return -(p.glk*(V-p.Vlk) + ge*(V-p.Vex) + gi*(V-p.Vin));
        case QIF:
            return -(p.glk*(V-p.Vlk)*(p.type.extra.QIF.Vth-V) + ge*(V-p.Vex) + gi*(V-p.Vin));
        case EIF:
            return -(p.glk*(V-p.Vlk) - p.glk*p.type.extra.EIF.Dpk*exp((V-p.type.extra.EIF.Vth)/p.type.extra.EIF.Dpk) + ge*(V-p.Vex) + gi*(V-p.Vin));
        default: return One; //avoid -Wreturn-type error which is probably wrong anyway
    }
}

///Uses precalculated gE and gI to integrate the voltages forward through time.
///Uses eulers method
void CalcVoltages(const Compute_float* const __restrict__ Vinput,
        const Compute_float* const __restrict__ gE,
        const Compute_float* const __restrict__ gI,
        const conductance_parameters C,
        Compute_float* const __restrict__ Vout)
{
    for (int x=0;x<grid_size;x++)
    {
        for (int y=0;y<grid_size;y++)
        {
            const int idx =Conductance_index(x,y);
            const int idx2=  x*grid_size+y;
            const Compute_float rhs = rhs_func(Vinput[idx2],gE[idx],gI[idx],C);
            Vout[idx2]=Vinput[idx2]+Features.Timestep*rhs;
        }
    }
}
///Uses precalculated gE and gI to integrate the voltages and recoverys forward through time. This uses the Euler method
//this should probably take a struct as input - way too many arguments
void CalcRecoverys(const Compute_float* const __restrict__ Vinput,
        const Compute_float* const __restrict__ Winput,
        const Compute_float* const __restrict__ gE,
        const Compute_float* const __restrict__ gI,
        const conductance_parameters C,
        const recovery_parameters R,
        Compute_float* const __restrict__ Vout,
        Compute_float* const __restrict__ Wout)
{    // Adaptive quadratic integrate-and-fire
    for (int x=0;x<grid_size;x++)
    {
        for (int y=0;y<grid_size;y++)
        {   //step all neurons through time - use Euler method
            const int idx = Conductance_index(x,y);
            const int idx2 = x*grid_size+y;       //index for voltage/recovery
            const Compute_float rhsV=rhs_func(Vinput[idx2],gE[idx],gI[idx],C)-Winput[idx2];
            const Compute_float rhsW=R.Wcv*(R.Wir*(Vinput[idx2]-C.Vlk) - Winput[idx2]);
            Vout[idx2] = Vinput[idx2] + Features.Timestep*rhsV;
            Wout[idx2] = Winput[idx2] + Features.Timestep*rhsW;
        }
    }
}
//detect if a neuron is active - may be useful elsewhere
int IsActiveNeuron (const int x, const int y,const int step)
{
    const int test = (x % step) == 0 && (y % step) ==0;
    return (test && step > 0) || (!test && step < 0);
}
///Store current firing spikes also apply random spikes
///TODO: make faster - definitely room for improvement here
void StoreFiring(layer* L)
{
    for (unsigned int x=0;x<grid_size;x++)
    {
        for (unsigned int y=0;y<grid_size;y++)
        {
            if (IsActiveNeuron(x,y,L->P->skip) ) //check if this is an active neuron - here reversing the sign of "step" changes what becomes the active neurons
            {
                const unsigned int baseidx=LagIdx(x,y,L->firinglags);
                modifyLags(L->firinglags,baseidx);
                if (Features.STDP==ON) {modifyLags(L->STDP_data->lags,LagIdx(x,y,L->STDP_data->lags));} //question - would it be better to use a single lagstorage here with limits in appropriate places?
                //now - add in new spikes
                if (L->voltages_out[x*grid_size + y]  >= L->P->potential.Vpk)
                {
                    if (Features.Recovery==ON) //reset recovery if needed
                    {
                        L->voltages_out[x*grid_size+y]=L->P->potential.Vrt;                    //does voltage also need to be reset like this?
                        L->recoverys_out[x*grid_size+y]+=L->P->recovery.Wrt;
                    }
                    AddnewSpike(L->firinglags,baseidx);
                    if (Features.STDP==ON && L->STDP_data->RecordSpikes==ON /*We are sometimes not recording spikes */) {AddnewSpike(L->STDP_data->lags,LagIdx(x,y,L->STDP_data->lags));}
                }//add random spikes
                else if (L->P->potential.rate > 0 && //this check is because the compiler doesn't optimize the call to random() otherwise
                            (((Compute_float)(random()))/((Compute_float)RAND_MAX) <
                            (L->P->potential.rate*((Compute_float)0.001)*Features.Timestep)))
                {
                    L->voltages_out[x*grid_size+y]=L->P->potential.Vpk+(Compute_float)0.1;//make sure it fires - the neuron will actually fire next timestep
                }
            }
            else //non-active neurons never get to fire
            {
                    L->voltages_out[x*grid_size+y]=-1000; //skipped neurons set to -1000 - probably not required but perf impact should be minimal - also ensures they will never be >Vpk
            }
        }
    }
}
///Cleans up voltages for neurons that are in the refractory state
void ResetVoltages(Compute_float* const __restrict Vout,const couple_parameters C,const lagstorage* const  l,const conductance_parameters CP)
{
    const int trefrac_in_ts =(int) ((Compute_float)C.tref / Features.Timestep);
    for (unsigned int i=0;i<grid_size*grid_size;i++)
    {
        unsigned int baseidx = i*l->lagsperpoint;
        if (CurrentShortestLag(l,baseidx) <= trefrac_in_ts)
        {
            Vout[i] = CP.Vrt;
        }
    }
}
#include "imread/imread.h"
///This function takes up way too much time in the code - mostly in storefiring - slightly annoying as this is essentially all pure overhead.  It would be really nice to significantly reduce the amount of time this function takes.
void tidylayer (layer* l,const Compute_float timemillis,const Compute_float* const gE,const Compute_float* const gI)
{
    if (Features.Recovery==OFF)
    {
        CalcVoltages(l->voltages,gE ,gI,l->P->potential,l->voltages_out);
        ResetVoltages(l->voltages_out,l->P->couple,l->firinglags,l->P->potential);
    }
    else
    {
        CalcRecoverys(l->voltages,l->recoverys,gE,gI,l->P->potential,l->P->recovery,l->voltages_out,l->recoverys_out);
    }
    StoreFiring(l);
    if (Features.Theta==ON)
    {
        dotheta(l->voltages_out,l->P->theta,timemillis);
    }
    if (Features.ImageStim==ON)
    {
        ApplyStim(l->voltages_out,timemillis,l->P->Stim,l->P->potential.Vpk,l->STDP_data);
    }
}
///Steps a model through 1 timestep - quite high-level function
///This is the only function in the file that needs model.h
void step1(model* m)
{
    const Compute_float timemillis = ((Compute_float)m->timesteps) * Features.Timestep ;
    memset(m->gE,0,sizeof(Compute_float)*conductance_array_size*conductance_array_size); //zero the gE/gI matrices so they can be reused for this timestep
    memset(m->gI,0,sizeof(Compute_float)*conductance_array_size*conductance_array_size);
    if (Features.LocalStim==ON)
    {
        if (m->timesteps %1000 < 250) {ApplyLocalBoost(m->gE,20,20);}
        else if (m->timesteps % 1000 < 500) {ApplyLocalBoost(m->gE,20,60);}
        else if (m->timesteps % 1000 < 750) {ApplyLocalBoost(m->gE,60,20);}
        else  {ApplyLocalBoost(m->gE,60,60);}
    }
    if(Features.UseAnimal==ON)
    {
        MoveAnimal(m->animal,timemillis);
        AnimalEffects(*m->animal,m->gE,timemillis);
    }
    // Add spiking input to the conductances
    AddSpikes(m->layer1,m->gE,m->gI,m->timesteps);
    if (m->NoLayers==DUALLAYER) {AddSpikes(m->layer2,m->gE,m->gI,m->timesteps);}
    if (Features.Disablewrapping==OFF)
    {
        fixboundary(m->gE,m->gI);
    }
    // Add constant input to the conductances
    for (int i = 0;i < conductance_array_size*conductance_array_size;i++)
    {
        m->gE[i] += Extinput.gE0;
        m->gI[i] += Extinput.gI0;
    }
    //from this point the GE and GI are actually fixed - as a result there is no more layer interaction - so do things sequentially to each layer

    tidylayer(&m->layer1,timemillis,m->gE,m->gI);
    if (m->NoLayers==DUALLAYER){tidylayer(&m->layer2,timemillis,m->gE,m->gI);}
    if (Features.STDP==ON)
    {
        DoSTDP(m->layer1.connections,m->layer2.connections,m->layer1.STDP_data,m->layer1.P->STDP, m->layer2.STDP_data,m->layer2.P->STDP,m->layer1.rcinfo);
        DoSTDP(m->layer2.connections,m->layer1.connections,m->layer2.STDP_data,m->layer2.P->STDP, m->layer1.STDP_data,m->layer1.P->STDP,m->layer2.rcinfo);
    }
}
