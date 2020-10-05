#include "trace.hpp"
#include "iostream"
#include "fstream"
#include <stdlib.h>
#include <cinttypes>

using namespace std;

Trace::Trace(string& trace_path)
{
    ifstream trace_file(trace_path);
    counter = 0;
    len = 0;
    string curr_line;
    Instruction *tmp_trace;
    while (getline(trace_file, curr_line)) {
        if( curr_line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            curr_line = curr_line.substr(3);

        if((curr_line[0] != 'r') && (curr_line[0] != 'w'))
            continue;
        if(counter == 0)
        {
            trace = (Instruction *) calloc(1, sizeof(Instruction));
            counter++;
        }
        else
        {
            tmp_trace = (Instruction *) realloc(trace, (++counter) * sizeof(Instruction));
            if(tmp_trace == NULL)
            {
                printf("WARNING: Not able to realloc memory\n");
                trace_file.close();
                exit(EXIT_FAILURE);
            }
            trace = tmp_trace;
        }
        trace[counter - 1].rw_flags = curr_line[0];
        get_hex(curr_line, trace[counter - 1].address);
        len++;
    }
#ifdef DEBUG
    printf("%u\n", len);
#endif
    trace_file.close();
}


void Trace::clean()
{
    free(trace);
}

/*void Trace::get_next()
{
    
}*/

void Trace::get_hex(string& inst, unsigned int& address)
{
    size_t N = inst.length();
    int shift = 0;
    for(int i = N-1; i >= 2; i--)
    {
        if(inst[i] >= 'a' && inst[i] <= 'f')
            address = address | ((inst[i] - 'a' + 10) << shift);
        else if(inst[i] >= 'A' && inst[i] <= 'F')
            address = address | ((inst[i] - 'A' + 10) << shift);
        else if(inst[i] >= '0' && inst[i] <= '9')
            address = address | ((inst[i] - '0') << shift);
        else
            break;
        shift += 4;
    }

    #ifdef DEBUG
        //printf("Input address is: %x\n", address);
    #endif
}

unsigned int Trace::get_next_idx(unsigned int curr_idx)
{
    unsigned int query_address = trace[curr_idx].address;
    for(unsigned int i = curr_idx + 1; i < len; i++)
        if(trace[i].address == query_address)
            return i;
    return len;
}

unsigned int Trace::get_length()
{
    return len;
}
