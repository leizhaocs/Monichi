#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <math.h>
#include "Define.h"

class CacheSet;

/* cache state */
enum CacheState
{
    INVALID,
    CLEAN,
    DIRTY
};

/* one cache block */
class CacheBlock
{
public:
    ULONG       tag;        // tag
    INT         coreID;     // the core id of the data in this block
    INT         LRU;        // used for LRU replacement policy 0: LRU assoc-1: MRU
    CacheState  state;      // state
    INT         block_size; // block size
    CHAR       *data;       // real data

    /* constructor */
    CacheBlock(ULONG t, INT c_id, INT lru, CacheState c_state, INT b_size);
    /* move this block to MRU position */
    void moveToMRU(CacheSet *set);
};

/* one cache set */
class CacheSet
{
public:
    INT                     assoc;      // associativity
    INT                     block_size; // block_size
    std::vector<CacheBlock> blocks;     // all the blocks in this set

    /* constructor */
    CacheSet(INT a, INT b_size);
};

/* one cache */
class Cache
{
public:
    CHAR name[100];    // name of this cache
    INT real_data;      // whether store real data in cache

    INT size;          // cache size (in byte)
    INT assoc;         // associativity
    INT block_size;    // block size (in byte)
    INT num_sets;      // number of sets

    INT read_latency;  // read latency
    INT write_latency; // write latency

    std::vector<Cache *> upper_level; // all upper level caches
    Cache               *lower_level; // the lower level cache
    MemorySystem        *mem;         //  main memory

    std::vector<CacheSet> sets; // all the sets in this cache

    /* constructor */
    Cache(const CHAR *nam, INT s, INT a, INT b_size, INT r_lat, INT w_lat, INT realdata);
    /* add upper level cache */
    void add_upper_level(Cache *up);
    /* set lower level cache */
    void set_lower_level(Cache *low);
    /* set main memory */
    void set_memory(MemorySystem *m);
    /* probe the cache */
    INT probe(INT c_id, ULONG addr, INT *set, INT *block);
    /* get victim block */
    void getVictim(INT c_id, ULONG addr, INT *set, INT *block);
    /* invalide upper level caches */
    void invalidate(INT set, INT block);
    /* access the cache, return latency */
    ULONG access(INT c_id, ULONG addr, INT write, CHAR *data, INT length);
};

#endif