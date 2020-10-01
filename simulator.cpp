#include "simulator.hpp"
#include "cache.hpp"
#include "trace.hpp"

using namespace std;

Simulator::Simulator(unsigned int blocksize, unsigned int l1_size,
                    unsigned int l1_assoc, unsigned int l2_size,
                    unsigned int l2_assoc, char replacement_policy,
                    char inclusion_policy, std::string& trace_file)
{
    /*Create caches*/
    Cache L1(l1_size, blocksize, l1_assoc, replacement_policy, inclusion_policy);
    hierarchy = &L1;
    Cache *L2;
    if(l2_size > 0)
    {
        L2 = new Cache(l2_size, blocksize, l2_assoc, replacement_policy, inclusion_policy);
        L1.add_below(L2);
    }

    trace = new Trace(trace_file);
}

void Simulator::execute()
{
    unsigned int N = trace->get_length();
    for(int i = 0; i < N; i++)
    {
       execute_instruction(trace->trace[i]);
    }
}

void Simulator::execute_instruction(Instruction& i)
{
    if(i.rw_flags == 'r')
    {
        hierarchy->read(i.address, prog_counter++);
    }
    else if(i.rw_flags == 'w') 
    {
        hierarchy->write(i.address, prog_counter++);
    }
    else
    {
        cout << "The instruction's flag was:" << i.rw_flags << endl;
        exit(EXIT_FAILURE);
    }

}
