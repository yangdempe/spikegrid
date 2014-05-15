/// \file
#ifndef PARAMHEADER
#define PARAMHEADER
#ifdef FAST
typedef float Compute_float ; //for speed
#else
typedef double Compute_float ; //for accuracy
#endif
///used for storing arrays with their size.  Allows for the matlab_output (and other) function to take both the big and large arrays
typedef struct {
    //we require volatile below as we don't want you to be able to write to an array using the pointer from the tagged array
    //however, other parts of the code could modify the underlying array, so use volatile to force reads
    const volatile Compute_float* const data;
    const unsigned int size;
    const unsigned int offset;
} tagged_array;
///Holds data for sending back to matlab
typedef struct {
    const char const name[10];      ///< a string identifier that is used to identify the output
    const tagged_array data;        ///< the data to return
    const Compute_float minval;     ///< minimum value in array (for a colorbar - currently unused)
    const Compute_float maxval;     ///< maximum value in array (for a colorbar - currently unused)
} output_s; //used so that matlab has string identifiers that correspond to a specific tagged_array


//making OFF 0 will turn off features by default
typedef enum ON_OFF {OFF=0,ON=1} on_off;

typedef enum NORM_TYPE {None=0,TotalArea=1,GlobalMultiplier=2,MultSep=3} Norm_type;
/// The normalization method used in the 2009 paper.  This method normalizes by the total area of Ex and In connections
typedef struct
{
    const Compute_float WE;
    const Compute_float WI;
} Total_area_parameters;
///Normalize by multiplying all connections by a constant
typedef struct
{
    const Compute_float GM; ///< The constant to multiply by
} global_multiplier_parameters;
///Used when normalizing excitatory and inhibitory seperately
typedef struct 
{
    const Compute_float Exfactor;
    const Compute_float Infactor;
} Multsep_parameters;
///Holds the parameters for the decay of a spike
typedef struct decay_parameters{
    const Compute_float R;  ///<rise time constant (units?)
    const Compute_float D;  ///<decay time constant (units?)
} decay_parameters;
typedef enum LayerNumbers {SINGLELAYER=0,DUALLAYER=1} LayerNumbers;
///Parameters for a layer when it is the only one
typedef struct singlelayer_parameters
{
    const Compute_float WE       ;                  ///<excitatory coupling strength
    const Compute_float sigE   ;                    ///<char. length for Ex symapses (int / float?)
    const Compute_float WI     ;                    ///<Inhib coupling strength
    const Compute_float sigI   ;                    ///<char. length for In synapses (int / float?)
    const decay_parameters Ex;                      ///<Parameters for Ex connections
    const decay_parameters In;                      ///<Parameters for In connections
} singlelayer_parameters;
///Layer parameters for when there are two layers
typedef struct duallayer_parameters
{
    const Compute_float     W; //basically as for the singlelayer_properties but with some features missing
    const Compute_float     sigma;
    const decay_parameters  synapse;
} duallayer_parameters;
/// Contains parameters about coupling within either a single or dual layer
typedef struct couple_parameters
{
    const LayerNumbers Layertype;       ///<Whether we are using a single/or dual layer model
    const union
    {
        singlelayer_parameters single;  ///<single layer
        duallayer_parameters   dual;    ///<double layer
    } Layer_parameters;                 
    const Norm_type     norm_type;                  //what normalization method to use
    const union 
    {
        Total_area_parameters total_area;
        global_multiplier_parameters glob_mult;
        Multsep_parameters mult_sep;

    } normalization_parameters;                   //holds data for different normalization methods
    const int tref     ;                    //refractory time
} couple_parameters;

///Contains parameters which control the Voltage dynamics of neurons
typedef struct conductance_parameters
{
    const Compute_float Vrt    ;  ///< reset potential.
    const Compute_float Vth    ;  ///< Threshold potential
    const Compute_float Vlk    ;  ///<leak reversal potential
    const Compute_float Vex    ;  ///<Ex reversal potential
    const Compute_float Vin    ;  ///<In reversal potential
    const Compute_float glk    ;  ///<leak conductance (ms^-1)
    const Compute_float rate   ;  ///<Rate of external input (spikes/neuron/s)
} conductance_parameters;
///Parameters for STDP
typedef struct STDP_parameters
{
    const Compute_float stdp_limit;
    const Compute_float  stdp_tau;
    const Compute_float stdp_strength;
} STDP_parameters;
///Parameters controlling the shape of the STD recovery
typedef struct STD_parameters
{
    const Compute_float U;
    const Compute_float D;
    const Compute_float F;
} STD_parameters;
///Parameters for outputting movies
typedef struct movie_parmeters
{
    const on_off MakeMovie;     ///< Are we making a movie?
    const output_s output;      ///< What will be outputted in the movie
    const unsigned int Delay;   ///< how often to output it
} movie_parameters;
///Parameters for a subthreshold wave (not necersarrily theta)
typedef struct theta_parameters
{
    const Compute_float strength;
	const Compute_float period;
} theta_parameters;
///Global switches to enable/disable features.  Also holds some model-independent parameters
typedef struct model_features
{
	const on_off STDP;
    const on_off STD;
    const on_off Output;
    const on_off Theta;
    const Compute_float Timestep; ///< The timestep in the model
} model_features;

/// procedure for adding new parameters.
/// 1. Add relevant parameter to the parameters struct
/// 2. Add entry to the sweepabletypes enum
/// 3. Update the modparam function in newparam.c to copy your new parameter
/// 4. Add a new default value in parameters.h (this should probably be with the feature off)
typedef struct parameters
{
    const couple_parameters couple;
    const conductance_parameters potential;
    const STDP_parameters STDP;
    const STD_parameters STD;
    const movie_parameters Movie;
    const theta_parameters theta;
    const int skip;
} parameters;
///it is crucial that these parameters have exactly the same names as the various fields in the parameters object.  otherwise you will break the parameter sweep function.
///it might also be a good idea to assign these values that never change with cross compatibilty with matlab
///
///Many parameters are currently not supported by this - need to improve, but basic framework is there
typedef enum {
         //           WE,sigE,WI,sigI,SE,SI,                                      // couple
          //          ExR,ExD,InR,InD,tref,                                     // synapse
                    Vrt,Vth,Vlk,Vex,Vin,glk,                               //potential
                    stdp_limit,stdp_tau,stdp_strength,                          //STDP
           //         U,D,F,                                                      //STD
                   // delay                                                       //movie
                   dummy          //Used for verification that nothing has been missed - DO NOT REMOVE
             } sweepabletypes;
///A struct to specify an attribute to change for a yossarian run
typedef struct Sweepable
{
    const sweepabletypes type;
    const Compute_float minval;
    const Compute_float maxval;
    const unsigned int count;
} sweepable;

//some useful constants
static const Compute_float One = (Compute_float)1; //a useful constant so that you cna get a floating point 1 without needing a cast to float / double.  (the whole idea of compute_float is that it make switching 
static const Compute_float Half = (Compute_float)0.5;
static const Compute_float Two = (Compute_float)2;
static const Compute_float Zero = (Compute_float)0;
#define PARAMETERS 
//get some macros for various sizes
#include "parameters.h" 
//these two get the underlying values from parameters.h and magic
#define conductance_array_size (grid_size + 2*couplerange)
#define couple_array_size (2*couplerange + 1)
#endif
