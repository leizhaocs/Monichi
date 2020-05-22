#ifndef MEMORY_H
#define MEMORY_H

#include "Define.h"

#define NUM_MAIN_MEMORY  10         /* number of main memories  */
#define BLOCK_TABLE_SIZE 0x00080000 /* block table size         */
#define BLOCK_SIZE       0x00002000 /* block size (8KB)         */
#define BLOCK_MASK       0x00001fff /* block offset             */
#define BLOCK_MASK_BIT   13         /* # of bits of BLOCK_MASK  */
#define SUB_BLOCK_SIZE   8          /* size of a sub block      */ 
#define SUB_BLOCK_BIT    3          /* # bits of SUB_BLOCK_SIZE */

/* stores the actual data */
class MainMemory
{
public:
    Emulator  *emu;                           // emulator
    UINT       tag;                           // the upper 32 bit of the address
    ULONG     *block_table[BLOCK_TABLE_SIZE]; // actual data

    /* constructor */
    MainMemory(Emulator *e, UINT t);
    /* allocate a block and initialize to zero */
    ULONG *allocblock(ULONG addr);
    /* load a 8-byte data */
    void ld_8byte(ULONG addr, ULONG *data);
    /* store a 8-byte data */
    void st_8byte(ULONG addr, ULONG *data, ULONG msk);
};

/* memory system */
class MemorySystem
{
public:
    Emulator   *emu;                 // emulator
    MainMemory *mm[NUM_MAIN_MEMORY]; // the actual memory data

    ULONG brk_point;  // break point defines the end of the data segment
    ULONG mmap_end;   // end of mmap range, used to fake mmap

    /* constructor */
    MemorySystem(Emulator *e);
    /* load an instruction */
    void ld_inst(ULONG addr, UINT *ir);
    /* load a 8-byte data */
    void ld_8byte(ULONG addr, ULONG *data);
    /* store a 8-byte data */
    void st_8byte(ULONG addr, ULONG *data, ULONG msk);
    /* load a n-byte data */
    void ld_nbyte(INT n, ULONG addr, ULONG *data);
    /* store a n-byte data */
    void st_nbyte(INT n, ULONG addr, ULONG *data);
    /* load an arbitrary number of bytes */
    void load(INT n, ULONG addr, ULONG *data);
    /* store an arbitrary number of bytes */
    void store(INT n, ULONG addr, ULONG *data);
};

#endif
