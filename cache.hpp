#include "trace.hpp"

#ifndef __CACHE_INLCUDED__
#define __CACHE_INLCUDED__

#define TRUE 0
#define FALSE 1

#define NOT_DIRTY 0
#define DIRTY 1
#define FREE 2

#define LRU 0
#define PLRU 1
#define OPT 2

using namespace std;

class Cache
{
    private:

        uint32_t size;
        uint32_t block_size;
        uint32_t assoc;
        uint32_t sets;
        uint32_t misses=0;
        uint32_t hits=0;
        uint32_t mem_ops=0;
        uint32_t **tags;
        uint32_t **sequence; //For LRU
        char **dirty; //Set if the specific tag is already used. 
                          //To differentiate between 0x00000000 and not available.
        uint32_t tag_mask;
        uint32_t index_mask;
        uint32_t block_mask;
        uint32_t tag_shift;
        uint32_t index_bits;
        uint32_t block_bits;
        Cache *below;
        Cache *above;

        void add_above(Cache *above)
        void add_below(Cache *below)
        void read(uint32_t address, uint32_t prog_counter);
        void write(uint32_t address, uint32_t prog_counter);
        void allocate(uint32_t address, uint32_t tag, uint32_t index, uint32_t prog_counter);
        void replace(uint32_t tag, uint32_t index);
        static uint32_t get_tag(uint32_t address);
        static uint32_t get_index(uint32_t address);
        static uint32_t get_block(uint32_t address);
    public:
        Cache(uint32_t size, uint32_t block_size, uint32_t assoc, char repl_pol, char incl_pol);
};

#endif
