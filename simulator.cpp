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
        L1->read(i.address, ++prog_counter);
    }
    else if(i.rw_flags == 'w') 
    {
        L1->write(i.address, ++prog_counter);
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
    unsigned int reads;
    unsigned int writes;
    unsigned int read_misses;
    unsigned int write_misses;
    unsigned int write_backs;
    unsigned int mem_ops;
    L1->get_stats(reads, writes, read_misses,
                  write_misses, write_backs, mem_ops);
    L1->dump_cache();
    L2->dump_cache();
    cout << "L1 Reads: " << dec << reads << endl;
    cout << "L1 Read misses: " << dec << read_misses << endl;
    cout << "L1 Writes: " << dec << writes << endl;
    cout << "L1 Write misses: " << dec << write_misses << endl;
    cout << "L1 Write backs: " << dec << write_backs << endl;
    cout << "L1 mem_ops: " << dec << mem_ops << endl;
    L2->get_stats(reads, writes, read_misses,
                  write_misses, write_backs, mem_ops);
    cout << "L2 Reads: " << dec << reads << endl;
    cout << "L2 Read misses: " << dec << read_misses << endl;
    cout << "L2 Writes: " << dec << writes << endl;
    cout << "L2 Write misses: " << dec << write_misses << endl;
    cout << "L2 Write backs: " << dec << write_backs << endl;
    cout << "L2 mem_ops: " << dec << mem_ops << endl;
}
