/// Testing adaptability in a t-maze based network model - here the probability of association will vary gradually over time
#include <stddef.h> //offsetof
//these first few parameters actually escape into the paramheader file through magic
#define grid_size 200
///Total size of the grid
///Coupling range
#define couplerange 25
#ifndef PARAMETERS  //DO NOT REMOVE
///include guard
#define PARAMETERS  //DO NOT REMOVE
//disable warnings about float conversion in this file only
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
//the following typedef must be before the include to get the right compute types
///Whether we are using the single or double layer model
static const LayerNumbers ModelType = DUALLAYER;

//Fun note - with the right optimisations GCC actually will pull these constants inline (for example disassemble evolvept_STDP with STDP off)
///Parameters for the single layer model
static const parameters OneLayerModel = {.couple={0}}; //since unused - shortes possible definition that produces no warnings

#define potparams .potential =     \
{                                  \
    .type    =                     \
    {                           \
        .type = LIF,            \
    },                          \
    .Vrt     = -70,             \
    .Vpk    = -55,              \
    .Vlk     = -70,             \
    .Vex     = 0,               \
    .Vin     = -80,             \
    .glk     = 0.05,            \
    .rate = 0,                  \
}

#define STDPparams .STDP=   \
{                       \
    .stdp_limit=0.07,    \
    .stdp_tau=20,       \
    .stdp_strength=0.005,  \
    .STDP_on=ON,\
    .STDP_decay_factor=0.9985,\
    .STDP_decay_frequency=100,\
}

#define Stimparams .Stim=\
{\
    .ImagePath  = "input_maps/stoch_interact_Tmaze.png",\
    .timeperiod=150,\
    .lag=55,\
    .PreconditioningTrials=0,\
    .NoUSprob=0,\
    .Testing = OFF,\
    .TestPathChoice = ON,\
    .Oscillating_path = ON,\
    .path_osc_freq = 100,\
    .Periodic = ON,\
    .Gradual_stim_swap=ON,\
    .Gradual_swap_period=800,\
}
#define Rparams .random=\
{ \
    .numberper = grid_size*grid_size, \
    .str=1,\
    .Specials=0,\
    .FancySpecials=ON,\
    .SpecialAInd=300,\
    .SpecialBInd=200,\
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
                .W          = -0.5, //-0.40 //-0.57 //-0.70 //-1.25,
                .sigma      = 90,
                .synapse    = {.R=0.5,.D=2.0},
            }
        },

        .norm_type = GlobalMultiplier,
        .normalization_parameters = {.glob_mult = {.GM=1.0}},
        .tref       = 5,
    },
    STDPparams,
    potparams,
    Stimparams,
    Rparams,
    .skip=2,
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
                .W          =  0.25,
                .sigma      = 20,
                .synapse    = {.R=0.5,.D=2.0},
            }
        },
        .tref       = 5,
        .norm_type = GlobalMultiplier,
        .normalization_parameters = {.glob_mult = {.GM=1.0}},
    },
    STDPparams,
    potparams,
    .skip=-2,
    Stimparams,
    Rparams,
};
///Some global features that can be turned on and off
static const model_features Features =
{
    .STD        = OFF,
    .STDP		= ON,
    .Random_connections = ON,
    .Timestep   = 0.1,
    .Simlength  = 10000000,
    .ImageStim  = ON,
    .job        = {.initcond = RAND_JOB, .Voltage_or_count = 1},
    .Disablewrapping = ON,
    .output = {
        {.method = VIDEO,.Output="V2",.Delay=40, .Overlay="Trialno"},
        {.method=GUI,.Output="V2",.Delay=10,.Overlay="Timestep"},
        {.method=TEXT,.Output="RC2",.Delay=10000},
//        {.method=TEXT,.Output="V2",.Delay=10},
        {.method=TEXT,.Output="STDP_bias2",.Delay=10000}
    }
};
///Constant external input to conductances
static const extinput Extinput =
{
    .gE0 = 0.000,
    .gI0 = 0.15,
};
///Parameters for conducting a parameter sweep.
static const sweepable Sweep =
{
    .offset=offsetof(parameters,Stim.Prob1) ,
    .minval = 0.000,
    .maxval = 0,
    .count = 100,
    .SweepEx = ON,
    .SweepIn = ON,
};

#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif
#endif //DO NOT REMOVE
