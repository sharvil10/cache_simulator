#ifndef __SIMULATOR_INCLUDED__
#define __SIMULATOR_INCLUDED__

#include "trace.hpp"
#include "cache.hpp"
#include <stdlib.h>
#include <cinttypes>

#ifdef DEBUG
    #define print_debug(x) cout << x;
#else
    #define print_debug(x) do {} while(0)
#endif

using namespace std;

class Simulator
{
    private:
        Cache *L1 = NULL;
        Cache *L2 = NULL;
        unsigned int prog_counter = 0;
        Trace *trace; 
    public:
        Simulator(unsigned int blocksize,
                  unsigned int l1_size,
                  unsigned int l1_assoc,
                  unsigned int l2_size,
                  unsigned int l2_assoc,
                  char replacement_policy,
                  char inclusion_policy,
                  string& trace_file);
        void execute();
        void execute_instruction(Instruction& i);
        void get_stats();
        

};
#endif
