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
    trace = new Trace(trace_file);

    L1 = new Cache(l1_size, blocksize, l1_assoc, replacement_policy, inclusion_policy);
    if(l2_size > 0)
    {
        L2 = new Cache(l2_size, blocksize, l2_assoc, replacement_policy, inclusion_policy);
        L1->add_below(L2);
    }

}

void Simulator::execute()
{
    unsigned int N = trace->get_length();
    for(unsigned int i = 0; i < N; i++)
    {
        #ifdef DEBUG
            cout << "===== Executing instruction =====" << dec << i << endl;
        #endif
        execute_instruction(trace->trace[i]);
    }
    this->get_stats();
}

void Simulator::execute_instruction(Instruction& i)
{
    unsigned int next_idx = this->trace->get_next_idx(prog_counter);
#ifdef DEBUG
    cout << "Next index: " << dec << next_idx << endl;
#endif
    prog_counter++;
    if(i.rw_flags == 'r')
    {
        L1->read(i.address, next_idx);
    }
    else if(i.rw_flags == 'w') 
    {
        L1->write(i.address, next_idx);
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
    unsigned int mem_ops1 = 0;
    unsigned int mem_ops2 = 0;

    L1->get_stats(reads, writes, read_misses,
                  write_misses, write_backs, mem_ops1);

    cout << "===== L1 contents =====" << endl;
    L1->dump_cache();
    if(L2 != NULL)
    {
        cout << "===== L2 contents =====" << endl;
        L2->dump_cache();
    }
    cout << "===== Simulation results (raw) =====" << endl;
    printf("a. %-26s %u\n", "number of L1 reads:", reads);
    printf("b. %-26s %u\n", "number of L1 read misses:", read_misses);
    printf("c. %-26s %u\n", "number of L1 writes:", writes);
    printf("d. %-26s %u\n", "number of L1 write misses:", write_misses);
    printf("e. %-26s %.6f\n", "L1 miss rate:", (float)(read_misses + write_misses)/(reads + writes));
    printf("f. %-26s %u\n", "number of L1 writebacks:", write_backs);

    reads = 0;
    writes = 0;
    read_misses = 0;
    write_misses = 0;
    write_backs = 0;
    if(L2 != NULL)
        L2->get_stats(reads, writes, read_misses,
                      write_misses, write_backs, mem_ops2);

    printf("g. %-26s %u\n", "number of L2 reads:", reads);
    printf("h. %-26s %u\n", "number of L2 read misses:", read_misses);
    printf("i. %-26s %u\n", "number of L2 writes:", writes);
    printf("j. %-26s %u\n", "number of L2 write misses:", write_misses);
    if(L2 != NULL)
        printf("k. %-26s %.6f\n", "L2 miss rate:", (float)(read_misses)/(reads));
    else
        printf("k. %-26s %d\n", "L2 miss rate:", 0);

    printf("l. %-26s %u\n", "number of L2 writebacks:", write_backs);
    printf("m. %-26s %u\n", "total memory traffic:", mem_ops1 + mem_ops2);
}
