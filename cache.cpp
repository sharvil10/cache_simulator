#include "cache.hpp"
#include <cmath>

using namespace std;

Cache::Cache(uint32_t size, uint32_t block_size, uint32_t assoc)
{
    this->size = size;
    this-> block_size = block_size;
    this->assoc = assoc;
    this->sets = (uint32_t) size / (block_size * assoc);

    //All sub addresses masks can be set by using size
    //and shifting them by the previous mask sizes.
    // ------------------------------------------------
    // | Address bits | Index bits | Block Offset bits|
    // ------------------------------------------------

    block_mask = block_size - 1;
    block_bits = log2(block_size);
    //index mask should be shifted by block bits
    //as block bits is on the right side of it.
    index_mask = (sets - 1) << block_bits; 
    index_bits = log2(sets);
    tag_shift = block_bits + index_bits;
    tag_mask = ((uint32_t) pow(2, 32 - tag_shift) - 1) << tag_shift;

    #ifdef DEBUG
        cout << hex << tag_mask <<endl;
        cout << hex << index_mask <<endl;
        cout << hex << block_mask <<endl;
        cout << tag_shift <<endl;
        cout << index_bits <<endl;
        cout << block_bits <<endl;
    #endif
    tags = (uint32_t **) calloc(sets, sizeof(uint32_t *));
    for(uint32_t i = 0; i < sets; i++)
        tags[i] = (uint32_t *) calloc(assoc, sizeof(uint32_t));

}

void Cache::add_above(Cache *above)
{
    this->above = above;
}

void Cache::add_below(Cache *below)
{
    this->below = below;
}

uint32_t Cache::get_tag(uint32_t address)
{
   return (address && tag_mask) >> tag_shift;
}

uint32_t Cache::get_block(uint32_t address)
{
   return address && block_mask;
}

uint32_t Cache::get_index(uint32_t address)
{
   return (address && index_mask) >> index_bits;
}
