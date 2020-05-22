#include "Memory.h"

/* constructor */
MainMemory::MainMemory(Emulator *e, UINT t)
{
    emu = e;
    tag = t;
    for(INT i = 0; i < BLOCK_TABLE_SIZE; i++)
    {
        block_table[i] = NULL;
    }
}

/* allocate a block and initialize to zero */
ULONG *MainMemory::allocblock(ULONG addr)
{
    /* allocate and initialize */
    INT n = BLOCK_SIZE / SUB_BLOCK_SIZE;
    ULONG *ret = new ULONG[n];
    for(INT i = 0; i < n; i++)
        ret[i]=0;

    /* add into block table */
    UINT adr = addr;
    block_table[adr>>BLOCK_MASK_BIT] = ret;

    if(ret == NULL)
        fprintf(emu->LOG, "memory: Error in allocblock.\n");

    return ret;
}

/* load a 8-byte data */
void MainMemory::ld_8byte(ULONG addr, ULONG *data)
{
    UINT adr = addr;
    ULONG *bt = block_table[adr>>BLOCK_MASK_BIT];
    if(bt == NULL)
    {
        *data = 0;
    }
    else
    {
        INT offset = (adr&BLOCK_MASK) >> SUB_BLOCK_BIT;
        *data = bt[offset];
    }
}

/* store a 8-byte data */
void MainMemory::st_8byte(ULONG addr, ULONG *data, ULONG msk)
{
    UINT adr = addr;
    ULONG *bt = block_table[adr>>BLOCK_MASK_BIT];
    if(bt == NULL)
        bt = allocblock(addr);
    INT offset = (adr&BLOCK_MASK) >> SUB_BLOCK_BIT;
    bt[offset] = (msk==0) ? *data : (bt[offset] & msk)|*data;
}

/* constructor */
MemorySystem::MemorySystem(Emulator *e)
{
    emu = e;

    for(INT i = 0; i < NUM_MAIN_MEMORY; i++)
        mm[i] = NULL;
}

/* load an instruction */
void MemorySystem::ld_inst(ULONG addr, UINT *ir)
{
    MainMemory *m = NULL;

    UINT tag = (addr>>32) & MASK32;
    for(INT i = 0; i < NUM_MAIN_MEMORY; i++)
    {
        if(mm[i] == NULL)
            continue;
        if(mm[i]->tag == tag)
        {
            m = mm[i];
            break;
        }
    }

    if(m == NULL)
        *ir = 0;
    else
    {
        ULONG d;
        m->ld_8byte(addr, &d);
        if(addr%8 == 0)
            *ir = d & MASK32;
        if(addr%8 == 4)
            *ir = (d>>32) & MASK32;
    }
}

/* load a 8-byte data */
void MemorySystem::ld_8byte(ULONG addr, ULONG *data)
{
    MainMemory *m = NULL;

    UINT tag = (addr>>32) & MASK32;
    for(INT i = 0; i < NUM_MAIN_MEMORY; i++)
    {
        if(mm[i] == NULL)
            continue;
        if(mm[i]->tag == tag)
        {
            m = mm[i];
            break;
        }
    }

    if(m == NULL)
        *data = 0;
    else
        m->ld_8byte(addr, data);
}

/* store a 8-byte data */
void MemorySystem::st_8byte(ULONG addr, ULONG *data, ULONG msk)
{
    MainMemory *m = NULL;

    UINT tag = (addr>>32) & MASK32;
    for(INT i = 0; i < NUM_MAIN_MEMORY; i++)
    {
        if(mm[i] == NULL)
            continue;
        if(mm[i]->tag == tag)
        {
            m = mm[i];
            break;
        }
    }
    
    if(m == NULL)
    {
        for(INT i = 0; i < NUM_MAIN_MEMORY; i++)
        {
            if(mm[i] == NULL)
            {
                mm[i] = new MainMemory(emu, tag);
                m = mm[i];
                break;
            }
        }
    }

    m->st_8byte(addr, data, msk);
}

/* load a n-byte data */
void MemorySystem::ld_nbyte(INT n, ULONG addr, ULONG *data)
{
    if(addr%n != 0) 
        fprintf(emu->LOG, "memory: ld_nbyte %d miss-align.\n", n);
    if(n!=1 && n!=2 && n!=4 && n!=8)
        fprintf(emu->LOG, "memory: Error in ld_nbyte %d.\n", n);

    ld_8byte(addr, data);
    if(n == 8)
        return;

    INT offset = addr & 7;
    if(n == 4)
    {
        ULONG dt = (*data>>(offset*8)) & MASK32;
        *data = dt;
    }
    else if(n == 2)
    {
        ULONG dt = (*data>>(offset*8)) & MASK16;
        *data = dt;
    }
    else if(n == 1)
    {
        ULONG dt = (*data>>(offset*8)) & MASK08;
        *data = dt;
    }
}

/* store a n-byte data */
void MemorySystem::st_nbyte(INT n, ULONG addr, ULONG *data)
{
    if(addr%n != 0)
        fprintf(emu->LOG, "memory: st_nbyte %d miss-alig.\n", n);
    if(n!=1 && n!=2 && n!=4 && n!=8)
        fprintf(emu->LOG, "memory: Error in st_nbyte %d.\n", n);

    INT offset = addr & 7;
    ULONG mask = 0;

    if(n == 4)
    {
        mask = ~(MASK32 << offset*8);
        ULONG dt = (*data&MASK32) << offset*8;
        *data = dt;
    }
    else if(n == 2)
    {
        mask = ~(MASK16 << offset*8);
        ULONG dt = (*data&MASK16) << offset*8;
        *data = dt;
    }
    else if(n == 1)
    {
        mask = ~(MASK08 << offset*8);
        ULONG dt = (*data&MASK08) << offset*8;
        *data = dt;
    }

    st_8byte(addr, data, mask);
}

/* load an arbitrary number of bytes */
void MemorySystem::load(INT n, ULONG addr, ULONG *data)
{
    CHAR *buf = (CHAR *)data;

    /* load '\0' terminated string */
    if(n == -1)
    {
        INT i = 0;
        while(1)
        {
            ULONG a = addr + i;
            ULONG d;
            ld_nbyte(1, a, &d);
            buf[i++] = (CHAR)d;
            if(d == '\0')
                break;
        }
        return;
    }

    /* load n bytes */
    for(INT i = 0; i < n; i++)
    {
        ULONG a = addr + i;
        ULONG d;
        ld_nbyte(1, a, &d);
        buf[i] = (CHAR)d;
    }
}

/* store an arbitrary number of bytes */
void MemorySystem::store(INT n, ULONG addr, ULONG *data)
{
    CHAR *buf = (CHAR *)data;

    /* store '\0' terminated string */
    if(n == -1)
    {
        INT i = 0;
        while(1)
        {
            ULONG a = addr + i;
            ULONG d = buf[i++];
            st_nbyte(1, a, &d);
            if(d == '\0')
                break;
        }
        return;
    }

    /* store n bytes */
    for(INT i = 0; i < n; i++)
    {
        ULONG a = addr + i;
        ULONG d = buf[i];
        st_nbyte(1, a, &d);
    }
}
