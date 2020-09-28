#include "trace.hpp"

#ifndef __CACHE_INLCUDED__
#define __CACHE_INLCUDED__

using namespace std;

class Cache
{
    private:

        uint32_t size;
        uint32_t block_size;
        uint32_t assoc;
        uint32_t sets;
        uint32_t miss;
        uint32_t hit;
        uint32_t **tags;
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
        void read(uint32_t address);
        void write(uint32_t address);
        uint32_t get_tag(uint32_t address);
        uint32_t get_index(uint32_t address);
        uint32_t get_block(uint32_t address);
    public:
        Cache(uint32_t size, uint32_t block_size, uint32_t assoc);
};

#endif
