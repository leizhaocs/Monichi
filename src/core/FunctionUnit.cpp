#include "FunctionUnit.h"

/* constructor */
FunctionUnitElem::FunctionUnitElem(INT tp, INT lat, INT size)
{
    type = tp;
    used = 0;
    latency = lat;
    ROB = -1;
    for(INT i = 0; i < size; i++)
        RS.push_back(-1);
}

/* constructor */
FunctionUnit::FunctionUnit()
{
    entries.push_back(FunctionUnitElem(1, 2, 4));
    entries.push_back(FunctionUnitElem(2, 4, 4));
    entries.push_back(FunctionUnitElem(3, 2, 4));
    entries.push_back(FunctionUnitElem(4, 8, 4));
}
