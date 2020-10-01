#include "cache.hpp"
#include <cmath>

using namespace std;

/*Cache::Cache()
{
    return;
}*/

Cache::Cache(unsigned int size, unsigned int block_size, unsigned int assoc,  char replacement_policy, char inclusion_policy)
{
    this->size = size;
    this-> block_size = block_size;
    this->assoc = assoc;
    this->sets = (unsigned int) size / (block_size * assoc);
    this->replacement_policy = replacement_policy;
    this->inclusion_policy = inclusion_policy;
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
    this->tag_mask = ((unsigned int) pow(2, 32 - tag_shift) - 1) << tag_shift;

    #ifdef DEBUG
        cout << "Tag Mask: " << tag_mask << endl;
        cout << "Index Mask: " <<  index_mask <<endl;
        cout << "Block Mask: " << block_mask <<endl;
        cout << "Tag shift: " << tag_shift <<endl;
        cout << "Index bits: " << index_bits <<endl;
        cout << "Block bits: " << block_bits <<endl;
    #endif
    tags = (unsigned int **) calloc(sets, sizeof(unsigned int *));
    dirty = (char **) calloc(sets, sizeof(char *));
    if(replacement_policy == LRU)
        sequence = (unsigned int **) calloc(sets, sizeof(unsigned int *));
    for(unsigned int i = 0; i < sets; i++)
    {
        tags[i] = (unsigned int *) calloc(assoc, sizeof(unsigned int));
        dirty[i] = (char *) calloc(assoc, sizeof(char));
        if(replacement_policy == LRU)
            sequence[i] = (unsigned int *) calloc(assoc, sizeof(unsigned int));
    }

}

void Cache::add_above(Cache *above)
{
    this->above = above;
}

void Cache::add_below(Cache *below)
{
    this->below = below;
}

unsigned int Cache::get_tag(unsigned int address)
{
    return (address & this->tag_mask) >> tag_shift;
}


unsigned int Cache::get_block(unsigned int address)
{
   return address & block_mask;
}

unsigned int Cache::get_index(unsigned int address)
{
   return (address & index_mask) >> block_bits;
}

unsigned int Cache::convert_to_address(unsigned int tag, unsigned int index)
{
    unsigned int address = 0;//Offset of the block is irrelevant
    address |= tag << tag_shift;
    address |= index <<  block_bits;
    return address;
}

void Cache::write(unsigned int address, unsigned int prog_counter)
{
#ifdef DEBUG
    cout << "Input Address: " << address <<endl;
#endif
    unsigned int tag = get_tag(address);
#ifdef DEBUG
    cout << "Input Tag: " << tag <<endl;
#endif
    unsigned int index = get_index(address);
#ifdef DEBUG
    cout << "Input Index: " << index <<endl;
#endif
     
    for(unsigned int i=0; i < assoc; i++)
    {
        if((tag == tags[index][i]) & (dirty[index][i] != FREE))
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
    allocate(address, tag, index, prog_counter);


}

void Cache::allocate(unsigned int address, unsigned int tag, unsigned int index, unsigned int prog_counter)
{
    cout << "Tag placed: " << tag << endl;

    for(unsigned int i=0; i < assoc; i++)
    {
        if(dirty[index][i] == FREE) //Free space found
        {
            dirty[index][i] = NOT_DIRTY;//Write-back Policy
            tags[index][i] = tag;
            sequence[index][i] = prog_counter;
            return;
        }
    }

    replace(tag, index, prog_counter);
    if(below == NULL)//Nothing below which means last cache so now will get from memory
    {
        mem_ops++;
        return;
    }
    this->below->read(address, prog_counter);
}

void Cache::replace(unsigned int tag, unsigned int index, unsigned int prog_counter)
{
    unsigned int idx;
    switch(replacement_policy)
    {
        case LRU:
            idx = lru(tag, index);
        case PLRU:
            return;
        case OPT:
            return;
    }

    if(dirty[index][idx] == DIRTY)//Write-back policy
    {
        unsigned int address_to_replace = convert_to_address(tags[index][idx], index);
        if(below == NULL)
            mem_ops++;
        else
            below->write(address_to_replace, prog_counter);
    }
    tags[index][idx] = tag;
}

unsigned int Cache::lru(unsigned int tag_to_write, unsigned int index)
{
    int32_t idx = 0;
    unsigned int min = 0xffffffff;
    for(unsigned int i=0; i < assoc; i++)
    {
        if(sequence[index][i] < min)
        {
            min = sequence[index][i];
            idx = i;
        }
    }
    return idx;//Assoc index of the victim block

}

void Cache::read(unsigned int address, unsigned int prog_counter)
{
    unsigned int tag = get_tag(address);
    unsigned int index = get_index(address);
    
    for(unsigned int i=0; i < assoc; i++)
    {
        if((tag == tags[index][i]) & (dirty[index][i] != FREE))
        {
            //set_flags is used to distinguish between 
            //0x00000000 and not set
            hits++;
            sequence[index][i] = prog_counter;
            return;
        }
    }

    //Cache miss so allocate
    this->misses++;
    cout << "Missed" << endl;
    allocate(address, tag, index, prog_counter);
}

void Cache::get_stats(unsigned int& hits, unsigned int& misses, unsigned int& mem_ops)
{
    hits = this->hits;
    misses = this->misses;
    mem_ops = this->mem_ops;
}


