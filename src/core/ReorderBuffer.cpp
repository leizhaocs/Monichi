#include "ReorderBuffer.h"

/* constructor */
ReorderBufferElem::ReorderBufferElem()
{
    state = 0;
    inst = NULL;
    Qa = -1;
    Qb = -1;
    Qc = -1;
    Qfpcr = -1;
    destR = -1;
    data = 0;
    done = 0;
    cancel = 0;
    memLat = 0;
    pendInsts.clear();
}

/* constructor */
ReorderBuffer::ReorderBuffer(INT size)
{
    for(INT i = 0; i < size; i++)
        entries.push_back(ReorderBufferElem());
    ROBHead = 0;
    ROBTail = 0;
}

/* whether is full */
INT ReorderBuffer::full()
{
    if(entries[ROBTail].state > 0)
        return 1;
    return 0;
}

/* whether is empty */
INT ReorderBuffer::empty()
{
    if(entries[ROBHead].state == 0)
        return 1;
    return 0;
}

/* whether an instruction is ready to be executed */
INT ReorderBuffer::ready(INT r)
{
    if((entries[r].Qa==-1) && (entries[r].Qb==-1) && (entries[r].Qc==-1) && (entries[r].Qfpcr==-1))
        return 1;
    return 0;
}

/* clear a ROB entry */
void ReorderBuffer::clear(INT r)
{
    entries[r].state = 0;
    entries[r].inst = NULL;
    entries[r].Qa = -1;
    entries[r].Qb = -1;
    entries[r].Qc = -1;
    entries[r].Qfpcr = -1;
    entries[r].destR = -1;
    entries[r].data = 0;
    entries[r].done = 0;
    entries[r].cancel = 0;
    entries[r].memLat = 0;
    entries[r].pendInsts.clear();
}
