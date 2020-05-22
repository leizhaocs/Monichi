#ifndef REORDER_BUFFER_H
#define REORDER_BUFFER_H

#include <vector>
#include "Define.h"

/* reorder buffer element */
class ReorderBufferElem
{
public:
    INT          state; // 0:empty 1:occupied 2:executed 3:memory read
    Instruction *inst;  // current instruction in this entry

    INT Qa;      // ROB index of Ra/Fa operand, -1: no dependency
    INT Qb;      // ROB index of Rb/Fb operand, -1: no dependency
    INT Qc;      // ROB index of Rc/Fc operand, -1: no dependency
    INT Qfpcr;   // ROB index of fpcr operand, -1: no dependency

    INT   destR;  // the register index in renameREG that this instruction is going to write, -1:no need to write
    ULONG data;   // newly computed data by this insturction, in order to serve dependent insts in pendInsts
    INT   done;   // whether data has been produced. For ALU insts: after EXE state. For MEM insts: after MEM stage
    INT   cancel; // wheter this instruction suddenly decide not to write registers, for CMOV* insts
    INT   memLat; // latency caused by access memory/cache

    std::vector<INT> pendInsts; // instructions pending on this one

    /* constructor */
    ReorderBufferElem();
};

/* reorder buffer */
class ReorderBuffer
{
public:
    INT ROBHead; // oldest entry
    INT ROBTail; // next available empty entry
    std::vector<ReorderBufferElem> entries; // circular buffer

    /* constructor */
    ReorderBuffer(INT size);
    /* whether is full */
    INT full();
    /* whether is empty */
    INT empty();
    /* whether an instruction is ready to be executed */
    INT ready(INT r);
    /* clear a ROB entry */
    void clear(INT r);
};

#endif
