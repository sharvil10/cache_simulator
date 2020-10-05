#include "cache.hpp"
#include <cmath>

using namespace std;


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
    unsigned long int nodes; 
    if((replacement_policy == LRU) | (replacement_policy == OPT))
        sequence = (unsigned int **) calloc(sets, sizeof(unsigned int *));
    else if(replacement_policy == PLRU)
    {
        tree = (char **) calloc(sets, sizeof(char *));
        levels = log2(assoc); 
        nodes = (unsigned long int) pow(2, levels) - 1;
#ifdef DEBUG
        cout << "Number of nodes in the tree: " << nodes << endl;
#endif
    }
    for(unsigned int i = 0; i < sets; i++)
    {
        tags[i] = (unsigned int *) calloc(assoc, sizeof(unsigned int));
        dirty[i] = (char *) calloc(assoc, sizeof(char));
        if((replacement_policy == LRU) | (replacement_policy == OPT))
            sequence[i] = (unsigned int *) calloc(assoc, sizeof(unsigned int));
        else if(replacement_policy == PLRU)
            tree[i] = (char *) calloc(nodes, sizeof(char));
    }

}

void Cache::add_above(Cache *above)
{
    this->above = above;
}

void Cache::add_below(Cache *below)
{
    this->below = below;
    this->below->add_above(this);
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

void Cache::access_plru(unsigned int index, unsigned int idx_way)
{
    unsigned long int curr_idx = idx_way;
    curr_idx += (unsigned long int) pow(2, levels) - 1;
    char tmp_flag = 0;
    while(curr_idx != 0)
    {
        tmp_flag = 0;
        if((curr_idx % 2) == 0)
            tmp_flag = 1;
        curr_idx = (curr_idx - 1) / 2;
        tree[index][curr_idx] = tmp_flag;
    }
}

unsigned int Cache::plru_get_replace_idx(unsigned int index)
{
    unsigned long int curr_idx = 0;
    unsigned long int curr_lev = 0;
    while(curr_lev < levels)
    {
        if(tree[index][curr_idx] == 0)
        {
            tree[index][curr_idx] = 1;
            curr_idx = 2 * (curr_idx + 1); //Go right.
        }
        else
        {
            tree[index][curr_idx] = 0;
            curr_idx = 2 * (curr_idx + 1) - 1;//Go left

        }
        curr_lev++;
    }
    curr_idx -= (unsigned long int) pow(2, levels) - 1;
    return (unsigned int) curr_idx;
}

void Cache::set_access_ids(unsigned int index, unsigned int idx_way, unsigned int next_idx)
{
    switch(replacement_policy)
    {
        case LRU:
            sequence[index][idx_way] = seq_counter - 1;
            break;
        case PLRU:
            access_plru(index, idx_way);
            break;
        case OPT:
            sequence[index][idx_way] = next_idx;
            break;
    }
}

void Cache::write(unsigned int address, unsigned int next_idx)
{
    writes++;
    seq_counter++;
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
            set_access_ids(index, i, next_idx);
            return;
        }
    }

    //Cache miss so allocate
    write_misses++;
    unsigned int idx = allocate(address, tag, index, next_idx);
    dirty[index][idx] = DIRTY; //After reading block from next level we are also going to update it.
    if(below == NULL)//Nothing below which means last cache so now will get from memory
    {
        mem_ops++;
        return;
    }
    this->below->read(address, next_idx);
}

unsigned int Cache::allocate(unsigned int address, unsigned int tag, unsigned int index, unsigned int next_idx)
{

    for(unsigned int i=0; i < assoc; i++)
    {
        if(dirty[index][i] == FREE) //Free space found
        {
            dirty[index][i] = NOT_DIRTY;//Write-back Policy
            tags[index][i] = tag;
            set_access_ids(index, i, next_idx);
            return i;
        }
    }

    return replace(tag, index, next_idx);
}

unsigned int Cache::replace(unsigned int tag, unsigned int index, unsigned int next_idx)
{

    unsigned int idx;
    switch(replacement_policy)
    {
        case LRU:
            idx = lru(tag, index);
            break;
        case PLRU:
            idx = plru_get_replace_idx(index);
            break;
        case OPT:
            idx = opt(index);
            break;
    }
#ifdef DEBUG
    cout << "Existing tag " << hex << tags[index][idx] << 
        " at " << idx << " will be replaced with " << hex << tag << endl;
#endif
    unsigned int address_to_replace = convert_to_address(tags[index][idx], index);
    if(dirty[index][idx] == DIRTY)//Write-back policy
    {
        write_backs++;
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
            below->write(address_to_replace, next_idx);
    }
    tags[index][idx] = tag;
    dirty[index][idx] = NOT_DIRTY;
    set_access_ids(index, idx, next_idx);
    if((inclusion_policy == INCLUSIVE) & (this->above != NULL))
        this->above->evict(address_to_replace);

    return idx;
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
#ifdef DEBUG
        if((sequence[index][i] == min) & (sequence[index][i] != 0))
            cout << "WARNING: PC same " << sequence[index][i] << endl;
#endif
    }
    return idx;//Assoc index of the victim block

}

unsigned int Cache::opt(unsigned int index)
{
    unsigned int idx = 0;
    unsigned int max = 0;
    for(unsigned int i=0; i < assoc; i++)
    {
#ifdef DEBUG
        if((sequence[index][i] == max) & (sequence[index][i] != 0xffffffff))
            cout << "WARNING: OPT PC same " << sequence[index][i] << endl;
#endif
        if(sequence[index][i] > max)
        {
            max = sequence[index][i];
            idx = i;
        }

    }
    return idx;//Assoc index of the victim block

}

void Cache::read(unsigned int address, unsigned int next_idx)
{
    reads++;
    seq_counter++;
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
            set_access_ids(index, i, next_idx);
            return;
        }
    }

    //Cache miss so allocate
    read_misses++;
#ifdef DEBUG
    cout << "Missed" << endl;
#endif
    allocate(address, tag, index, next_idx);
    
    if(below == NULL)//Nothing below which means last cache so now will get from memory
    {
        mem_ops++;
        return;
    }
    below->read(address, next_idx); 
}

void Cache::evict(unsigned int address)
{
#ifdef DEBUG
    cout << "***** Eviction Started *****" << endl;
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

    for(unsigned int i = 0; i < assoc; i++)
    {
        if(dirty[index][i] == FREE)
            continue;
        if(tags[index][i] == tag)
        {
#ifdef DEBUG
            cout << "Found tag at: " << dec << i << endl;
#endif
            if(dirty[index][i] == DIRTY)
                mem_ops++;
            dirty[index][i] = FREE;
        }
    }
}


void Cache::get_stats(unsigned int& reads, unsigned int& writes, unsigned int& read_misses,
                      unsigned int& write_misses, unsigned int& write_backs, unsigned int& mem_ops)
{
    reads = this->reads;
    writes = this->writes;
    read_misses = this->read_misses;
    write_misses = this->write_misses;
    write_backs = this->write_backs;
    mem_ops = this->mem_ops;
}

void Cache::dump_cache()
{
    for(unsigned int i = 0; i < sets; i++)
    {
        printf("Set     %d:", i);
        for(unsigned int j = 0; j < assoc; j++)
        {
            printf("\t%-6x ", tags[i][j]);
            switch(dirty[i][j])
            {
                case DIRTY:
                    printf("D");
                    break;
                case NOT_DIRTY:
                    printf(" ");
                    break;
                case FREE:
                    printf(" ");
                    break;
            }
        }
        printf("\n");
    }
}
