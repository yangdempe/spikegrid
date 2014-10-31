/// \file
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#ifdef OPENCV
    #include "openCVAPI/api.h"
#endif
#include "STD.h"
#include "output.h"
#include "paramheader.h" //needed because we can define output_parameters structs in the config file
#include "STDP.h"
#include "model.h"
///Total number of things to be output - occasionally needs to be incremented
#define output_count  19
///Holds the outputtable objects for the current model
///Holds file* for the output types that output to a consistent file over time to save repeatedly calling fopen/fclose - mainly useful for ouputting ringbuffer stuff
FILE* outfiles[output_count];
///Extracts the actual information out of a tagged array and converts it to a simple square matrix



///Send an outputtable to a text file
///@param input     the outputtable object to output
///@param idx       the index of the outputtable so that if multiple objects are output, files have consistent naming
void outputToText(const output_s input,const int idx)
{
    if (outfiles[idx]==NULL)
    {
        char buf[100];
        sprintf(buf,"%s/%i.txt",outdir,idx);
        outfiles[idx]=fopen(buf,"w");
        if (outfiles[idx]==NULL) {printf("fopen failed on %s error is %s\n",buf,strerror(errno));}
    }
    switch (input.datatype)
    {
        case SPIKE_DATA:
        {
            for (int i=0;i<grid_size;i++)
            {
                for (int j=0;j<grid_size;j++)
                {
                    if (CurrentShortestLag(input.data.Lag_data,(i*grid_size+j)*input.data.Lag_data->lagsperpoint) == 1)
                    {
                        fprintf(outfiles[idx],"%i,%i:",i,j);
                    }
                }
            }
            fprintf(outfiles[idx],"\n");
            fflush(outfiles[idx]);
            break;
        }
        default:
        printf("I don't know how to output this data\n");

    }
}

///Finds an output which matches the given name - case sensitive
///@param name the name of the outputtable
output_s __attribute__((pure)) getOutputByName(const char* const name)
{
    int outidx=0;
    while (strlen(Outputtable[outidx].name) != 0)
    {
        if (!strcmp(Outputtable[outidx].name,name))
        {
            return Outputtable[outidx];
        }
        outidx++;
    }
    printf("tried to get unknown thing to output called -%s-\n",name);
    exit(EXIT_FAILURE);
}

///Set up the outputtables for a given model
///@param m the model we are going to output stuff from
void output_init(const model* const m)
{
    //WHEN YOU ADD SOMETHING - INCREASE OUTPUT_COUNT AT TOP OF FILE;
    //ALSO - only add things to the end of the array
    output_s* outdata=(output_s[]){ //note - neat feature - missing elements initailized to 0
        //Name          data type                  actual data                size                    offset     8bz634
        //subgrid,minval,maxval
        {"gE",          FLOAT_DATA, .data.TA_data={m->gE,                     conductance_array_size, couplerange,   1,0,2}}, //gE is a 'large' matrix - as it wraps around the edges
        {"gI",          FLOAT_DATA, .data.TA_data={m->gI,                     conductance_array_size, couplerange,   1,0,2}}, //gI is a 'large' matrix - as it wraps around the edges
        {"Coupling1",   FLOAT_DATA, .data.TA_data={m->layer1.connections,     couple_array_size,      0,             1,-0.5,0.5}}, //return the coupling matrix of layer 1 //TODO: fix min and max values
        {"Coupling2",   FLOAT_DATA, .data.TA_data={m->layer2.connections,     couple_array_size,      0,             1,-0.5,0.5}}, //return the coupling matrix of layer 2
        {"V1",          FLOAT_DATA, .data.TA_data={m->layer1.voltages_out,    grid_size,              0,             1,m->layer1.P->potential.Vin,m->layer1.P->potential.Vpk}},
        {"V2",          FLOAT_DATA, .data.TA_data={m->layer2.voltages_out,    grid_size,              0,             1,m->layer2.P->potential.Vin,m->layer2.P->potential.Vpk}},
        {"Recovery1",   FLOAT_DATA, .data.TA_data={m->layer1.recoverys_out,   grid_size,              0,             1,0,100}}, //TODO: ask adam for max and min recovery values
        {"Recovery2",   FLOAT_DATA, .data.TA_data={m->layer2.recoverys_out,   grid_size,              0,             1,0,100}}, //TODO: ask adam for max and min recovery values
        {"STDU1",       FLOAT_DATA, .data.TA_data={Features.STD==ON?m->layer1.std->U:NULL, grid_size, 0,             1,0,1}},
        {"STDR1",       FLOAT_DATA, .data.TA_data={Features.STD==ON?m->layer1.std->R:NULL, grid_size, 0,             1,0,1}},
        {"STDU2",       FLOAT_DATA, .data.TA_data={Features.STD==ON?m->layer2.std->U:NULL, grid_size, 0,             1,0,1}},
        {"STDR2",       FLOAT_DATA, .data.TA_data={Features.STD==ON?m->layer2.std->R:NULL, grid_size, 0,             1,0,1}},
        {"STDP1",       FLOAT_DATA, .data.TA_data={Features.STDP==ON?m->layer1.STDP_data->connections:NULL,grid_size,0,couple_array_size,-0.01,0.01}},
        {"STDP2",       FLOAT_DATA, .data.TA_data={Features.STDP==ON?m->layer2.STDP_data->connections:NULL,grid_size,0,couple_array_size,-0.01,0.01}},
        {"Spike1",      SPIKE_DATA, .data.Lag_data=&m->layer1.firinglags},
        {"Spike2",      SPIKE_DATA, .data.Lag_data=&m->layer2.firinglags},
        {.name={0}}};         //a marker that we are at the end of the outputabbles list
    output_s* malloced = malloc(sizeof(output_s)*output_count);
    memcpy(malloced,outdata,sizeof(output_s)*output_count);
    Outputtable = malloced;
}
///Cleans up memory and file handles that are used by the outputtables object
void CleanupOutput()
{
    for (int i=0;i<output_count;i++)    
    {
        if (outfiles[i] != NULL)
        {
            fclose(outfiles[i]);
            outfiles[i]=NULL;
        }
    }
    free(Outputtable);//also cleanup outputtables
    Outputtable=NULL;
}
