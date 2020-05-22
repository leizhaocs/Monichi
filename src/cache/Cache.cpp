#include "Cache.h"

/* constructor */
CacheBlock::CacheBlock(ULONG t, INT c_id, INT lru, CacheState c_state, INT b_size)
{
    tag = t;
    coreID = c_id;
    LRU = lru;
    state = c_state;
    block_size = b_size;
    data = (CHAR *)malloc(block_size);
}

/* move this block to MRU position */
void CacheBlock::moveToMRU(CacheSet *set)
{
    for(INT i = 0; i < set->assoc; i++)
    {
        if(set->blocks[i].LRU > LRU)
            set->blocks[i].LRU--;
    }
    LRU = set->assoc-1;
}

/* constructor */
CacheSet::CacheSet(INT a, INT b_size)
{
    assoc = a;
    block_size = b_size;
    for(INT i = 0; i < assoc; i++)
        blocks.push_back(CacheBlock(-1, -1, i, INVALID, block_size));
}

/* constructor */
Cache::Cache(const CHAR *nam, INT s, INT a, INT b_size, INT r_lat, INT w_lat, INT realdata)
{
    strcpy(name, nam);
    real_data = realdata;
    size = s;
    assoc = a;
    block_size = b_size;
    num_sets = size / (block_size * assoc);
    read_latency = r_lat;
    write_latency = w_lat;
    lower_level = NULL;
    mem = NULL;
    for(INT i = 0; i < num_sets; i++)
        sets.push_back(CacheSet(assoc, block_size));
}

/* add upper level cache */
void Cache::add_upper_level(Cache *up)
{
    upper_level.push_back(up);
}

/* set lower level cache */
void Cache::set_lower_level(Cache *low)
{
    lower_level = low;
}

/* set main memory */
void Cache::set_memory(MemorySystem *m)
{
    mem = m;
}

/* probe the cache */
INT Cache::probe(INT c_id, ULONG addr, INT *set, INT *block)
{
    // get set index and tag
    INT block_bits = log2(block_size);
    INT set_bits = log2(num_sets);
    ULONG set_mask = ((ULONG)num_sets) - 1;
    INT set_index = (addr>>block_bits) & set_mask;
    ULONG tag = addr >> (block_bits+set_bits);

    for(INT i = 0; i < assoc; i++)
    {
        if(sets[set_index].blocks[i].state != INVALID)
        {
            if(sets[set_index].blocks[i].tag == tag)
            {
                if(sets[set_index].blocks[i].coreID == c_id)
                {
                    *set = set_index;
                    *block = i;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* get victim block */
void Cache::getVictim(INT c_id, ULONG addr, INT *set, INT *block)
{
    // get set index
    INT block_bits = log2(block_size);
    INT set_bits = log2(num_sets);
    ULONG set_mask = ((ULONG)num_sets) - 1;
    INT set_index = (addr>>block_bits) & set_mask;

    for(INT i = 0; i < assoc; i++)
    {
        if(sets[set_index].blocks[i].state == INVALID)
        {
            *set = set_index;
            *block = i;
            break;
        }
        if(sets[set_index].blocks[i].LRU == 0)
        {
            *set = set_index;
            *block = i;
        }
    }
}

/* invalide upper level caches */
void Cache::invalidate(INT set, INT block)
{
    // get victim information
    INT block_bits = log2(block_size);
    INT set_bits = log2(num_sets);
    ULONG addr = ((sets[set].blocks[block].tag<<set_bits)|(ULONG)(set)) << block_bits;
    INT coreID = sets[set].blocks[block].coreID;
    CacheState state = sets[set].blocks[block].state;
    CHAR *data = sets[set].blocks[block].data;

    // invalidate upper caches
    for(INT i = 0; i < upper_level.size(); i++)
    {
        INT temps, tempb;
        Cache *temp_cache = upper_level[i];
        INT present = temp_cache->probe(coreID, addr, &temps, &tempb);
        if(present)
            temp_cache->invalidate(temps, tempb);
    }

    // write back
    if(state == DIRTY)
    {
        if(lower_level != NULL)
            lower_level->access(coreID, addr, 1, data, block_size);
        else if(real_data)
                mem->store(block_size, addr, (ULONG *)data);
    }

    // invalidate itselt
    sets[set].blocks[block].state = INVALID;
}

/* access the cache */
ULONG Cache::access(INT c_id, ULONG addr, INT write, CHAR *data, INT length)
{
    ULONG lat = 0;
    ULONG block_mask = ((ULONG)block_size) - 1;
    ULONG aligned_addr = addr & (~block_mask);
    ULONG offset = addr & block_mask;

    /* check cache line boundary */
    if((offset+length) > 64)
        fprintf(stderr, "cache access cross cache line\n");

    /* probe */
    INT s, b;
    INT hit = probe(c_id, aligned_addr, &s, &b);

    /* hit */
    if(hit)
    {
        CacheBlock *block = &(sets[s].blocks[b]);

        /* update block */
        block->moveToMRU(&(sets[s]));

        if(write == 0)
        {
            // read into data
            if(real_data)
                memcpy(data, block->data+offset, length);
            lat += read_latency;
        }
        else
        {
            // write data
            block->state = DIRTY;
            if(real_data)
                memcpy(block->data+offset, data, length);
            lat += write_latency;
        }
    }
    /* miss */
    else
    {
        INT vs, vb;
        getVictim(c_id, aligned_addr, &vs, &vb);
        CacheBlock *victim = &(sets[vs].blocks[vb]);
        if(victim->state != INVALID)
            invalidate(vs, vb);

        /* load missed data */
        if(lower_level != NULL)
            lat += lower_level->access(c_id, aligned_addr, 0, victim->data, block_size);
        else
        {
            if(real_data)
                mem->load(block_size, aligned_addr, (ULONG *)(victim->data));
            lat += 200;
        }

        /* update block */
        victim->tag = addr >> (INT)(log2(block_size)+log2(num_sets));
        victim->coreID = c_id;
        victim->moveToMRU(&(sets[vs]));

        if(write == 0)
        {
            // read into data
            victim->state = CLEAN;
            if(real_data)
                memcpy(data, victim->data+offset, length);
            lat += read_latency;
        }
        else
        {
            // write data
            victim->state = DIRTY;
            if(real_data)
                memcpy(victim->data+offset, data, length);
            lat += write_latency;
        }
    }

    return lat;
}
