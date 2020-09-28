#include <iostream>
#include <fstream>

#ifndef __TRACE_INCLUDED__
#define __TRACE_INCLUDED__

using namespace std;
struct Instruction {
    char rw_flags;
    uint32_t address = 0;
};

class Trace
{
    private:
        size_t len;
        Instruction *trace;
        long long int counter;
        void get_hex(string& inst, uint32_t& address);
    public:
        Trace(string& trace_path);
        //void get_next();
        void clean();
};
#endif
