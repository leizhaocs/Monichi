#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <vector>
#include "Define.h"

/* branch history table */
class BHT
{
public:
	INT   used;         // whether this entry is used
	ULONG pc;           // pc of the branch instruction
	ULONG target_pc;    // target pc
	INT   predict;      // 0: nt 1: nt 2: t 3: t

    /* constructor */
    BHT();
};

/* branch predictor */
class BranchPredictor
{
public:
    std::vector<BHT> entries; // branch history table

    /* constructor */
    BranchPredictor(INT size);
    /* predict 0:not taken  1:taken  -1:not branch instruction */
    INT predict(ULONG pc, ULONG *target_pc);
    /* update BHT */
    void update(ULONG pc, INT taken, ULONG target_pc);
};

#endif
