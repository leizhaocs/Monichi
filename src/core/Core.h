#ifndef CORE_H
#define CORE_H

#include <queue>
#include <stack>
#include "Define.h"
#include "ReorderBuffer.h"
#include "FunctionUnit.h"
#include "BranchPredictor.h"
#include "Cache.h"

/* processor core */
class Core
{
public:
    Emulator *emu;       // function simulator
    INT ID;              // core ID
    Instruction **ib;    // instruction pool
    INT  ibIndex;        // next ib to be used
    INT  renameREG[65];  // R0-R31 F0-F31 FPCR
    ULONG pc;            // pc, may be incorrect
    ULONG cycle;         // current cycle
    ULONG ninsts;        // commited instructions

    INT  stall_issue;    // stop issueing

    FILE *fp;

    // fetch queue
    INT fetchWidth;
    INT fetchQSize;
    std::queue<Instruction *> fetchQ;

    // decode queue
    INT decodeWidth;
    INT decodeQSize;
    std::queue<Instruction *> decodeQ;

    // FU
    INT issueWidth;
    FunctionUnit *FU;

    // ROB
    INT commitWidth;
    ReorderBuffer *ROB;

    // branch predictor
    BranchPredictor *predictor;

    // cache
    Cache *ICache;
    Cache *DCache;

    /* constructor */
    Core(INT argc, CHAR **argv, CHAR **envp);
    /* destructor */
    ~Core();

    /* fetch state */
    void fetch();
    /* decode state */
    void decode();
    /* issue state */
    void issue();
    /* execute state */
    void execute();
    /* memory stage */
    void memory();
    /* commit stage */
    void commit();

    /* simulate one cycle */
    INT run_a_cycle();
    /* update the pipeline when new data is computed */
    void update(INT r);
    /* flush the mispredicted insts in pipeline */
    void flushPipeline(INT binst);
};

#endif
