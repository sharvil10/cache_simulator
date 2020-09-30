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
    dirty = (char **) calloc(sets, sizeof(char *));
    if(repl_pol == LRU)
        sequence = (uint32_t **) calloc(sets, sizeof(uint32_t *));
    for(uint32_t i = 0; i < sets; i++)
        tags[i] = (uint32_t *) calloc(assoc, sizeof(uint32_t));
        dirty[i] = (char *) calloc(assoc, sizeof(char));
        if(repl_pol == LRU)
            sequence[i] = (uint32_t *) calloc(assoc, sizeof(uint32_t));

}

void Cache::add_above(Cache *above)
{
    this->above = above;
}

void Cache::add_below(Cache *below)
{
    this->below = below;
}

static uint32_t Cache::get_tag(uint32_t address)
{
   return (address && tag_mask) >> tag_shift;
}

static uint32_t Cache::get_block(uint32_t address)
{
   return address && block_mask;
}

static uint32_t Cache::get_index(uint32_t address)
{
   return (address && index_mask) >> block_bits;
}

static uint32_t Cache::convert_to_address(uint32_t tag, uint32_t index)
{
    uint32_t address = 0;//Offset of the block is irrelevant
    address |= tag << tag_shift;
    address |= index <<  block_bits;
    return address;
}

void Cache::write(uint32_t address, uint32_t prog_counter)
{
    uint32_t tag = get_tag(address);
    uint32_t index = get_index(address);
    
    for(uint32_t i=0; i < assoc; i++)
    {
        if((tag == tags[index][i]) && (dirty[index][i] != FREE))
        {
            //set_flags is used to distinguish between 
            //0x00000000 and not set
            hits++;
            dirty[index][i] = DIRTY;//Write-back Policy
            sequence[index][i] = prog_counter;
            return;
        }
    }

    //Cache miss so allocate
    misses++;
    allocate(address, tag, index);


}

void Cache::allocate(uint32_t address, uint32_t tag, uint32_t index, uint32_t prog_counter)
{
    for(uint32_t i=0; i < assoc; i++)
    {
        if(dirty[index][i] == FREE) //Free space found
        {
            dirty[index][i] = NOT_DIRTY;//Write-back Policy
            tags[index][i] = tag;
            sequence[index][i] = prog_counter;
            return;
        }
    }

    void replace(uint32_t tag, uint32_t index);
    if(below == NULL)//Nothing below which means last cache so now will get from memory
    {
        mem_ops++;
        return;
    }
    this->below->read(address);
}

void Cache::replace(uint32_t tag, uint32_t index)
{
    uint32_t idx;
    switch(repl_pol)
    {
        case LRU:
            idx = lru(tag, index);
        case PSLRU:
            return;
        case OPT:
            return;
    }

    if(dirty[index][idx] == DIRTY)//Write-back policy
    {
        uint32_t address_to_replace = convert_to_address(tag[index][idx], index);
        if(below == NULL)
            mem_ops++;
        else
            below->write(address_to_replace);
    }
    tags[index][idx] = tag;
}

uint32_t Cache::lru(uint32_t tag_to_write, uint32_t index)
{
    int32_t idx = 0;
    uint32_t min = 0xffffffff;
    for(uint32_t i=0; i < assoc; i++)
    {
        if(sequence[index][i] < min)
        {
            min = sequence[index][i];
            idx = i
        }
    }
    return idx;//Assoc index of the victim block

}

void Cache::read(uint32_t address, uint32_t prog_counter)
{
    uint32_t tag = get_tag(address);
    uint32_t index = get_index(address);
    
    for(uint32_t i=0; i < assoc; i++)
    {
        if((tag == tags[index][i]) && (dirty[index][i] != FREE))
        {
            //set_flags is used to distinguish between 
            //0x00000000 and not set
            hits++;
            sequence[index][i] = prog_counter;
            return;
        }
    }

    //Cache miss so allocate
    misses++;
    allocate(address, tag, index);
}
