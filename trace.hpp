#include <iostream>
#include <fstream>

#ifndef __TRACE_INCLUDED__
#define __TRACE_INCLUDED__

using namespace std;
struct Instruction {
    char rw_flags;
    unsigned int address = 0;
};

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
        Instruction *trace;

};
#endif
