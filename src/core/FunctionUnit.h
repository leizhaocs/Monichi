#ifndef FUNCTION_UNIT_H
#define FUNCTION_UNIT_H

#include <vector>
#include "Define.h"

/* functional unit element */
class FunctionUnitElem
{
public:
    INT type;            // 1:alu 2:ldst 3:bu 4:fp
    INT used;            // count down from latency
    INT latency;         // latency of this FU
    INT ROB;             // current executing instruction
    std::vector<INT> RS; // -1: empty

    /* constructor */
    FunctionUnitElem(INT tp, INT lat, INT size);
};

/* functional unit */
class FunctionUnit
{
public:
    std::vector<FunctionUnitElem> entries; // all function units

    /* constructor */
    FunctionUnit();
};

#endif
