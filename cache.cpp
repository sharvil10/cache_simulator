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
        cout << "Tag Mask: " << hex << tag_mask << endl;
        cout << "Index Mask: " <<  hex << index_mask <<endl;
        cout << "Block Mask: " << hex << block_mask <<endl;
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
    cout << "***** Writing Started *****" << endl;
    cout << "Input Address: " << hex << address <<endl;
#endif
    unsigned int tag = get_tag(address);
#ifdef DEBUG
    cout << "Input Tag: " << hex << tag <<endl;
#endif
    unsigned int index = get_index(address);
#ifdef DEBUG
    cout << "Input Index: " << hex << index <<endl;
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
    write_misses++;
    allocate(address, tag, index, prog_counter);

    if(below == NULL)//Nothing below which means last cache so now will get from memory
    {
        mem_ops++;
        return;
    }
    this->below->read(address, prog_counter);
}

void Cache::allocate(unsigned int address, unsigned int tag, unsigned int index, unsigned int prog_counter)
{

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
}

void Cache::replace(unsigned int tag, unsigned int index, unsigned int prog_counter)
{

    unsigned int idx;
    switch(replacement_policy)
    {
        case LRU:
            idx = lru(tag, index);
            break;
        case PLRU:
            return;
            break;
        case OPT:
            return;
            break;
    }
#ifdef DEBUG
    cout << "Existing tag " << hex << tags[index][idx] << 
        " at " << idx << " will be replaced with " << hex << tag << endl;
#endif
    if(dirty[index][idx] == DIRTY)//Write-back policy
    {

        unsigned int address_to_replace = convert_to_address(tags[index][idx], index);
#ifdef DEBUG
        cout << "Write-back for address " << address_to_replace << endl;
#endif
        if(below == NULL)
        {
#ifdef DEBUG
            cout << "Memory below" << endl;
#endif            
            mem_ops++;
        }
        else
            below->write(address_to_replace, prog_counter);
    }
    tags[index][idx] = tag;
    dirty[index][idx] = NOT_DIRTY;
    if(replacement_policy == LRU)
        sequence[index][idx] = prog_counter;
}

unsigned int Cache::lru(unsigned int tag_to_write, unsigned int index)
{
    unsigned int idx = 0;
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
#ifdef DEBUG
    cout << "***** Reading Started *****" << endl;
    cout << "Input Address: " << hex << address <<endl;
#endif
    unsigned int tag = get_tag(address);
#ifdef DEBUG
    cout << "Input Tag: " << hex << tag <<endl;
#endif
    unsigned int index = get_index(address);
#ifdef DEBUG
    cout << "Input Index: " << hex << index <<endl;
#endif
    
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
    read_misses++;
#ifdef DEBUG
    cout << "Missed" << endl;
#endif
    allocate(address, tag, index, prog_counter);
    
    if(below == NULL)//Nothing below which means last cache so now will get from memory
    {
        mem_ops++;
        return;
    }
    below->read(address, prog_counter); 
}

void Cache::get_stats(unsigned int& hits, unsigned int& read_misses, unsigned int& write_misses, unsigned int& mem_ops)
{
    hits = this->hits;
    read_misses = this->read_misses;
    write_misses = this->write_misses;
    mem_ops = this->mem_ops;
}

void Cache::dump_cache()
{
    cout << "===== Cache Dump =====" << endl;
    for(unsigned int i = 0; i < sets; i++)
    {
        printf("Set %5d: ", i);
        for(unsigned int j = 0; j < assoc; j++)
        {
            printf("\t%x ", tags[i][j]);
            switch(dirty[i][j])
            {
                case DIRTY:
                    printf("D");
                    break;
                case NOT_DIRTY:
                    printf("N");
                    break;
                case FREE:
                    printf("F");
                    break;
            }
        }
        printf("\n");
    }
    cout << "===== Cache dump ended =====" << endl;
}
