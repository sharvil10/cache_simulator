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
    L1 = new Cache(l1_size, blocksize, l1_assoc, replacement_policy, inclusion_policy);
    if(l2_size > 0)
    {
        L2 = new Cache(l2_size, blocksize, l2_assoc, replacement_policy, inclusion_policy);
        L1->add_below(L2);
    }

    trace = new Trace(trace_file);
}

void Simulator::execute()
{
    unsigned int N = trace->get_length();
    for(unsigned int i = 0; i < N; i++)
    {
        #ifdef DEBUG
            cout << "===== Executing instruction =====" << i << endl;
        #endif
        execute_instruction(trace->trace[i]);
    }
    this->get_stats();
}

void Simulator::execute_instruction(Instruction& i)
{
    if(i.rw_flags == 'r')
    {
        L1->read(i.address, prog_counter++);
    }
    else if(i.rw_flags == 'w') 
    {
        L1->write(i.address, prog_counter++);
    }
    else
    {
        cout << "The instruction's flag was:" << i.rw_flags << endl;
        exit(EXIT_FAILURE);
    }
#ifdef DUMP
    L1->dump_cache();
#endif
}

void Simulator::get_stats()
{
    unsigned int hits;
    unsigned int write_misses;
    unsigned int read_misses;
    unsigned int mem_ops;

    L1->get_stats(hits, read_misses, write_misses, mem_ops);

    cout << "Hits: " << dec << hits << endl;
    cout << "Read misses: " << dec << read_misses << endl;
    cout << "Write misses: " << dec << write_misses << endl;
    cout << dec << mem_ops << endl;
    //L2->get_stats(hits, misses, mem_ops);
    //cout << dec << hits << endl;
    //cout << dec << misses << endl;
    //cout << dec << mem_ops << endl;     
}
