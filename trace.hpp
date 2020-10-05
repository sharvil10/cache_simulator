#ifndef __TRACE_INCLUDED__
#define __TRACE_INCLUDED__
#include <iostream>
#include <fstream>
#include "cache.hpp"
#define NOT_USED_AGAIN 0xffffffff; 

using namespace std;
struct Instruction {
    char rw_flags;
    unsigned int address = 0;
};

class Cache;

class Trace
{
    private:
        unsigned int len;
        long long int counter;
        void get_hex(string& inst, unsigned int& address);
    public:
        Trace(string& trace_path);
        void clean();
        unsigned int get_length();
        unsigned int get_next_idx(unsigned int curr_idx, Cache *cache);
        Instruction *trace;

};
#endif
