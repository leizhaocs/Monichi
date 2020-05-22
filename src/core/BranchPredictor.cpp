#include "BranchPredictor.h"

/* constructor */
BHT::BHT()
{
    used = 0;
    pc = 0;
    target_pc = 0;
    predict = 0;
}

/* constructor */
BranchPredictor::BranchPredictor(INT size)
{
    for(INT i = 0; i < size; i++)
        entries.push_back(BHT());
}

/* predict 0:not taken  1:taken  -1:not branch instruction */
INT BranchPredictor::predict(ULONG pc, ULONG *target_pc)
{
    INT index = pc % entries.size();
    if(entries[index].used && (entries[index].pc==pc))
    {
        *target_pc = entries[index].target_pc;
        if(entries[index].predict > 1)
            return 1;
        else
            return 0;
    }
    return -1;
}

/* update BHT */
void BranchPredictor::update(ULONG pc, INT taken, ULONG target_pc)
{
    INT index = pc % entries.size();
    if(entries[index].used && (entries[index].pc==pc))
    {
        if(taken)
            entries[index].predict++;
        else
            entries[index].predict--;
        if(entries[index].predict > 3)
            entries[index].predict = 3;
        if(entries[index].predict < 0)
            entries[index].predict = 0;
    }
    else
    {
        entries[index].used = 1;
        entries[index].pc = pc;
        entries[index].target_pc = target_pc;
        if(taken)
            entries[index].predict = 2;
        else
            entries[index].predict = 1;
    }
}
