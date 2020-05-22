#ifndef ARCHITECTURE_STATE_H
#define ARCHITECTURE_STATE_H

#include "Define.h"

/* all the architectural registers */
class ArchitectureState
{
public:
    Emulator *emu;   // emulator

    ULONG pc;        // program counter
    ULONG r[32];     // general purpose regs
    ULONG f[32];     // floating point  regs
    ULONG fpcr;      // floating point control register

    /* constructor */
    ArchitectureState(Emulator *e);
};

#endif
