/// \file
#include <stddef.h> //offsetof
//these first few parameters actually escape into the paramheader file through magic
#define grid_size 300 // fix to look at small step sizes!
///Total size of the grid
///Coupling range
#define couplerange 30
#ifndef PARAMETERS  //DO NOT REMOVE
///include guard
#define PARAMETERS  //DO NOT REMOVE

//the following typedef must be before the include to get the right compute types
///Whether we are using the single or double layer model
static const LayerNumbers ModelType = DUALLAYER;

//Fun note - with the right optimisations GCC actually will pull these constants inline (for example disassemble evolvept_STDP with STDP off)
///Parameters for the single layer model
static const parameters OneLayerModel = {.couple={0}};
// Potential parameters
#define potparams .potential =          \
{                                       \
    .type    =                          \
    {                                   \
        .type = LIF,                    \
    },                                  \
    .Vrt     = -70e-3,                     \
    .Vpk    = -55e-3,                      \
    .Vlk     = -70e-3,                     \
    .Vex     = 0e-3,                       \
    .Vin     = -80e-3,                     \
    .glk     = 25e-9,                    \
    .rate = 0,                          \
}

#define stimparams .Stim =                          \
    {                                               \
        .ImagePath  = "input_maps/S20_N300_C0.png", \
        .timeperiod=1e6,                            \
        .lag=1e6,                                   \
        .PreconditioningTrials=0,                   \
        .NoUSprob=0,                                \
        .Testing=OFF,                               \
        .Periodic=OFF,                              \
        .I2 = 1.2e-9,                                 \
        .I1 = 0,                                    \
        .I0 = 0.4e-9,                                 \
        .mu = 0.04,                                 \
    }

///parameters for the inhibitory layer of the double layer model
static const parameters DualLayerModelIn =
{
    .couple =
    {
        .Layertype = DUALLAYER,
        .Layer_parameters = 
        {
            .dual = 
            {
                .connectivity = HOMOGENEOUS,   
                .W            = -5.0e-12, //vart W? 
                .sigma        = 0, 
                .synapse      = {.R=0,.D=2e-3},
            }
        },
        .tref       = 5e-3,
        .norm_type = GlobalMultiplier,
        .normalization_parameters = {.glob_mult = {.GM=1}},
    },
    potparams,
    stimparams,    
    .skip = 2,
};
///parameters for the excitatory layer of the double layer model
static const parameters DualLayerModelEx =
{
    .couple =   
    {
        .Layertype = DUALLAYER,
        .Layer_parameters = 
        {
            .dual =     
            {
                .connectivity = EXPONENTIAL,   
                .W            = 7.5e-12, 
                .sigma        = 30,
                .synapse      = {.R=0,.D=2e-3},
            }
        },
        .tref       = 5e-3,
        .norm_type = GlobalMultiplier,
        .normalization_parameters = {.glob_mult = {.GM=1}},
    },
    potparams,
    stimparams,
    .skip = -2, //3 out of 4?
};
///Constant external input to conductances
static const extinput Extinput =
{
    .gE0 = 0,
    .gI0 = 0,
};
///Some global features that can be turned on and off
static const model_features Features = 
{
    .Recovery   = OFF,
    .STDP       = OFF, //Question - some of these do actually make more sense as a per-layer feature - just about everything that isn't the timestep - 
    .STD        = OFF,  //if we need any of these features we can make the changes then.
    .Theta      = OFF,
    .Timestep   = 0.05e-3, // Works in like with 0.1 for midpoint. But if gE too small should addition be smaller too???
    .ImageStim = ON,
    .Simlength  = 70000, //50000
    .job        = {.initcond = RAND_JOB, .Voltage_or_count = 1},
    .Outprefix = "checkbiol",
    .output = {
        // { .method=SPIKES,.Output="Spike1" ,.Delay=1}, // Exc. spikes
        // { .method=SPIKES,.Output="Spike2" ,.Delay=1}, // Inh. spikes
        // { .method=TEXT,.Output="V1",.Delay=50},    // Exc. voltage 
        // { .method=TEXT,.Output="V2",.Delay=50},    // Inh. voltage
        // { .method=TEXT,.Output="gE",.Delay=50},
        // { .method=TEXT,.Output="gI",.Delay=50},
        {.method=GUI,.Output="V2",.Delay=1,.Overlay="Timestep"},
    },                                            
};
///Parameters for conducting a parameter sweep.
static const sweepable Sweep =
{
    // .offset=offsetof(parameters,couple.Layer_parameters.dual.W),
    // .minval = -0.010,
    // .maxval = -0.001,
    // .count = 1,
    // .SweepEx = OFF,
    // .SweepIn = ON,
    // 
    .offset=offsetof(parameters,couple.normalization_parameters.glob_mult.GM),
    .minval = 1, 
    .maxval = 1,
    .count = 499,  
    .SweepEx = ON,
    .SweepIn = ON,
    //
    // .offset=offsetof(parameters,Stim.I2),
    // .minval = 0,
    // .maxval = 2.40,
    // .count = 24,
    // .SweepEx = ON,
    // .SweepIn = ON,
};


#endif //DO NOT REMOVE
