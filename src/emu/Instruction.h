#ifndef INSTRUCTION_H
#define INSTRUCTION_H

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* 1. When adding new insructions, make sure one instruction only write           */
/*    at most one regist, otherwise the detailed simulation will be incorrect     */
/* 2. If an instruction does not need any function unit, it must not read or      */
/*    write any register or memory                                                */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#include "Define.h"

class Instruction
{
public:
    Emulator *emu;   // emulator
    INT       valid; // currently in used

    UINT   ir;       // 32b: Raw instruction
    UINT   CI;       // 24b: Code ID
    UCHAR  Op;       //  6b: Opcode field
    UCHAR  Ra;       //  5b: Ra or Fa field
    UCHAR  Rb;       //  5b: Rb or Fb field
    UCHAR  Rc;       //  5b: Rc or Fc field
    USHORT Mdisp;    // 16b: Memory displacement field
    UINT   Bdisp;    // 21b: Branch displacement field
    UCHAR  SBZ;      //  3b: SBZ field
    UCHAR  LIT;      //  8b: LIT field
    UCHAR  RI;       //  1b: Choose Rb or LIT
    UINT   PALF;     // 26b: PAL function field

    ULONG  Cpc;      // PC of the instruction
    ULONG  Npc;      // PC of the next instruction, Calculated in Exe. stage
    ULONG  Ppc;      // predicted PC (only used for branch instructions)

    INT    RRa;   // whether need to read Ra or Fa register, 1: Ra 2: Fa
    INT    WRa;   // whether need to write Ra or Fa register, 1: Ra 2: Fa
    INT    RRb;   // whether need to read Rb or Fb register, 1: Rb, 2: Fb
    INT    WRb;   // whether need to write Rb or Fb register, 1: Rb, 2: Fb
    INT    RRc;   // whether need to read Rc or Fc register, 1: Rc 2: Fc
    INT    WRc;   // whether need to write Rc or Fc register, 1: Rc 2: Fc
    INT    RMem;  // whether need to load from memory, if not 0, Data width of memory value (Bytes)
    INT    WMem;  // Whether need to store to memory, if not 0, Data width of memory value (Bytes)
    INT    Rfpcr; // whether need to read fpcr register
    INT    Wfpcr; // whether need to write fpcr register

    ULONG  Adr;      // Memory address
    ULONG  Memv;     // Memory value
    ULONG  Rav;      // Ra or Fa value
    ULONG  Rbv;      // Rb or Fb value
    ULONG  Rcv;      // Rc or Fc value
    ULONG  Fpcr;     // Fpcr value
    ULONG  Target;   // Target address in instruction

    INT    Syscall;  // this is a syscall instruction
    INT    Bra;      // whether this is a branch instruction
    INT    FU;       // 0:none 1:alu 2:ldst 3:bu 4:fp

    /* constructor */
    Instruction(Emulator *e);

    /* fetch the instruction */
    void fetch(ULONG pc);
    /* decode the instruction */
    void decode();
    /* execute the instruction */
    void execute();
    /* read register */
    void readRegister();
    /* write register */
    void writeRegister();
    /* load from memory */
    void readMemory();
    /* store to memory */
    void writeMemory();
    /* execute system call */
    void syscall();
    /* commit, update pc */
    void commit();

    /* pipeline specific functions */

    /* set Cpc Npc Ppc */
    void setPC(ULONG pc);
    /* read Rav */
    void readRav();
    /* read Rbv */
    void readRbv();
    /* read Rcv */
    void readRcv();
    /* read Fpcr */
    void readFpcr();
    /* set Rav value */
    void setRav(ULONG rav);
    /* set Rbv value */
    void setRbv(ULONG rbv);
    /* set Rcv value */
    void setRcv(ULONG rcv);
    /* set Fpcr value */
    void setFpcr(ULONG fpcr);
    /* manipulate data read from memory */
    void loadData();
    /* manipulate data write to memory */
    void writeData();
};

#endif
