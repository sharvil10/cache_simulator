#include "trace.hpp"

#ifndef __CACHE_INLCUDED__
#define __CACHE_INLCUDED__

#define TRUE 0
#define FALSE 1

#define NOT_DIRTY 2
#define DIRTY 1
#define FREE 0 

#define LRU 0
#define PLRU 1
#define OPT 2

#define NON_INCLUSIVE 0
#define INCLUSIVE 1

using namespace std;

class Cache
{
    private:

        unsigned int size;
        unsigned int block_size;
        unsigned int assoc;
        unsigned int sets;
        unsigned int reads=0;
        unsigned int writes=0;
        unsigned int read_misses=0;
        unsigned int write_misses=0;
        unsigned int write_backs=0;
        unsigned int hits=0;
        unsigned int mem_ops=0;
        unsigned int **tags;
        unsigned int **sequence; //For LRU
        unsigned int seq_counter=0;
        char **dirty; //Set if the specific tag is already used. 
                          //To differentiate between 0x00000000 and not available.
        unsigned int tag_mask;
        unsigned int index_mask;
        unsigned int block_mask;
        unsigned int tag_shift;
        unsigned int index_bits;
        unsigned int block_bits;

        char replacement_policy;
        char inclusion_policy;
        Cache *below;
        Cache *above;

        unsigned int allocate(unsigned int address, unsigned int tag, unsigned int index, unsigned int prog_counter);
        unsigned int replace(unsigned int tag, unsigned int index, unsigned int prog_counter);
        unsigned int lru(unsigned int tag_to_write, unsigned int index);
        unsigned int get_tag(unsigned int address);
        unsigned int get_index(unsigned int address);
        unsigned int get_block(unsigned int address);
        unsigned int convert_to_address(unsigned int tag, unsigned int index);
    public:
        //Cache();
        Cache(unsigned int size, unsigned int block_size, unsigned int assoc, char repl_pol, char incl_pol);
        void add_above(Cache *above);
        void add_below(Cache *below);
        void read(unsigned int address, unsigned int prog_counter);
        void write(unsigned int address, unsigned int prog_counter);
        void get_stats(unsigned int& reads, unsigned int& writes, unsigned int& read_misses,
                       unsigned int& write_misses, unsigned int& write_backs, unsigned int& mem_ops);
        void dump_cache();
};

#endif
