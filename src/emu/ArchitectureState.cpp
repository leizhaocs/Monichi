#include "ArchitectureState.h"

/* constructor */
ArchitectureState::ArchitectureState(Emulator *e)
{
    emu = e;
    INT i;
    for(i = 0; i < 32; i++)
        r[i] = 0;
    for(i = 0; i < 32; i++)
        f[i] = 0;
    fpcr = 0;
    pc = 0;
}
