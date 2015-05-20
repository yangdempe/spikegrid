/// \file
#include <stdio.h>
#include <math.h>
#include "../sizes.h" //so we get the correct parameter values
int __attribute__((const)) min(const int a,const int b) {if (a<b) {return a;} else {return b;}}
//calculate the circle
int __attribute__((const)) getoffset(const int range, const int i)
{
    const double drange = (double)range;
    const double di     = (double)(i-range);
    if (drange < di) {return (int)drange;}
    return min(range,(int) ((sqrt(drange*drange-di*di)) ));
}
//virtually identical to below - please keep both methods in sync - need to improve
void withSTDP()
{
    //some initial setup lines
    printf("void evolvept_duallayer_STDP (const int x,const int y,const Compute_float* const __restrict connections,const Compute_float* const __restrict STDP_connections,const Compute_float strmod, Compute_float* __restrict condmat)\n");
    printf("{\n");
    //start the loop
    for (int i = 0; i < couple_array_size;i++)
    {
        printf("    const int outoff%i = ((x+%i)*%i + y);\n",i,i,conductance_array_size);
        printf("    const int conoff%i = %i*couple_array_size;\n",i,i);
        printf("    const int STDPoff%i = (x*grid_size+y)*couple_array_size*couple_array_size + conoff%i;\n" ,i,i);
        char buf [1000];
        sprintf(buf,"((x+%i)*%i + y)",i,conductance_array_size);
        const int off = getoffset(couplerange,i);
        printf("    for (int kk=(couplerange-%i);kk<=(couplerange+%i);kk++)\n",off,off); //this is the key part - offsets are now known at compile time
        printf("    {\n");
        printf("        condmat[outoff%i + kk] += (connections[conoff%i+kk] + STDP_connections[STDPoff%i+kk])*strmod;\n",i,i,i); //include base connection and STDP-modified connection
        printf("    }\n");
    }
    printf("}\n");

}
void checkfn()
{
    printf("void check()\n");
    printf("{\n");
    printf("    if (conductance_array_size != %i || couplerange != %i || couple_array_size != %i)\n",conductance_array_size,couplerange,couple_array_size);
    printf("    {\n");
    printf("        printf(\"You need to regenerate maskgen.c\");\n");
    printf("        exit(EXIT_FAILURE);\n");
    printf("    }\n");
    printf("}");

}
void RDAdd()
{
    printf("void AddRD (const int x,const int y,const Compute_float* const __restrict connections, Compute_float* __restrict Rmat,Compute_float* __restrict Dmat, const Compute_float R,const Compute_float D)\n");
    printf("{\n");
    printf("c++;");
    printf("const Compute_float Rstr = 1/(D-R);// * exp(-1*Features.Timestep/R) ;\n");
    printf("const Compute_float Dstr = 1/(D-R);// * exp(-1*Features.Timestep/D);\n");
    //start the loop
    for (int i = 0; i < couple_array_size;i++)
    {
        printf("    const int outoff%i = ((x+%i)*%i + y);\n",i,i,conductance_array_size);
        printf("    const int conoff%i = %i*couple_array_size;\n",i,i);
        char buf [1000];
        sprintf(buf,"((x+%i)*%i + y)",i,conductance_array_size);
        const int off = getoffset(couplerange,i);
        printf("    for (int kk=(couplerange-%i);kk<=(couplerange+%i);kk++)\n",off,off); //this is the key part - offsets are now known at compile time
        printf("    {\n");
        printf("        Rmat[outoff%i + kk] += connections[conoff%i+kk]*Rstr;\n",i,i);
        printf("        Dmat[outoff%i + kk] += connections[conoff%i+kk]*Dstr;\n",i,i);
        printf("    }\n");
    }
    printf("}\n");
}
int main()
{
    //some initial setup lines
    printf("//autogenerated code - do not modify - see maskgen\n");
    printf("//parameters: conductance_array_size %i, couplerange %i, couple_array_size %i\n",conductance_array_size,couplerange,couple_array_size);
    printf("#include <stdio.h>\n");
    printf("#include \"sizes.h\"\n");
    printf("#include \"mymath.h\"\n");
    printf("#include \"paramheader.h\"\n");
    printf("long long int c  = 0;\n");
    printf("void evolvept_duallayer (const int x,const int y,const Compute_float* const __restrict connections,const Compute_float strmod, Compute_float* __restrict condmat)\n");
    printf("{\n");
    printf("c++;");
    //start the loop
    for (int i = 0; i < couple_array_size;i++)
    {
        printf("    const int outoff%i = ((x+%i)*%i + y);\n",i,i,conductance_array_size);
        printf("    const int conoff%i = %i*couple_array_size;\n",i,i);
        char buf [1000];
        sprintf(buf,"((x+%i)*%i + y)",i,conductance_array_size);
        const int off = getoffset(couplerange,i);
        printf("    for (int kk=(couplerange-%i);kk<=(couplerange+%i);kk++)\n",off,off); //this is the key part - offsets are now known at compile time
        printf("    {\n");
        printf("        condmat[outoff%i + kk] += connections[conoff%i+kk]*strmod;\n",i,i);
        printf("    }\n");
    }
    printf("}\n");
    withSTDP();
    RDAdd();
    checkfn();
}
