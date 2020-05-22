#include <unistd.h> 
#include "Define.h"

/* (sing) extend 8bit data into 64 bit data */
ULONG sext8(ULONG d)
{
    return (d & BIT7) ? d | EXTND08 : d & MASK08;
}

/* (sing) extend 16bit data into 64 bit data */
ULONG sext16(ULONG d)
{
    return (d & BIT15) ? d | EXTND16 : d & MASK16;
}

/* (sing) extend 21bit data into 64 bit data */
ULONG sext21(ULONG d)
{
    return (d & BIT20) ? d | EXTND21 : d & MASK21;
}

/* (sing) extend 32bit data into 64 bit data */
ULONG sext32(ULONG d)
{
    return (d & BIT31) ? d | EXTND32 : d & MASK32;
}

/* 64 bit multiply Input of A and B, output of High 64b and Low 64b */
void mul128(ULONG a, ULONG b, ULONG *rh, ULONG *rl)
{
    ULONG ah,al, bh, bl;
    ULONG x,y,z;
    INT carry = 0;

    ah = a >> 32;
    al = a & 0xFFFFFFFF;

    bh = b >> 32;
    bl = b & 0xFFFFFFFF;

    x = al*bl;
    y = (al*bh << 32);
    z = x + y;
    if(z<x || z<y)
        carry ++;
    x = z;
    y = (ah*bl << 32);
    z = x + y;
    if(z<x || z<y)
        carry ++;
    *rl = z;

    *rh = ah*bh+(al*bh >> 32)+(ah*bl >> 32)+carry;
}

/* return 24bit unique instruction ID */
INT get_code(UINT ir)
{
    INT id = 0;
    INT op6    = (ir>>26) & 0x003f;  // oo
    INT jmp_op = (ir>>14) & 0x0003;  // for Mbr: h
    INT func7  = (ir>>5 ) & 0x007f;  // for Opr: ff
    INT func11 = (ir>>5 ) & 0x07ff;  // for F-P: fff
    INT func16 = ir & 0xffff;        // for Mfc: ffff

    switch(op6)
    {
        case 0x00: id =  op6 << 16;           break;
        case 0x01: id =  op6 << 16;           break;
        case 0x02: id =  op6 << 16;           break;
        case 0x03: id =  op6 << 16;           break;
        case 0x04: id =  op6 << 16;           break;
        case 0x05: id =  op6 << 16;           break;
        case 0x06: id =  op6 << 16;           break;
        case 0x07: id =  op6 << 16;           break;
        case 0x08: id =  op6 << 16;           break;
        case 0x09: id =  op6 << 16;           break;
        case 0x0a: id =  op6 << 16;           break;
        case 0x0b: id =  op6 << 16;           break;
        case 0x0c: id =  op6 << 16;           break;
        case 0x0d: id =  op6 << 16;           break;
        case 0x0e: id =  op6 << 16;           break;
        case 0x0f: id =  op6 << 16;           break;
        case 0x10: id = (op6 << 16) | func7;  break;
        case 0x11: id = (op6 << 16) | func7;  break;
        case 0x12: id = (op6 << 16) | func7;  break;
        case 0x13: id = (op6 << 16) | func7;  break;
        case 0x14:
            id = (op6 << 16) | func11;
            id = id & 0xff002f;
            break;
        case 0x15: id = UNIMPLEMENTED;        break; // unimplemented instruction
        case 0x16:
            id = (op6 << 16) | func11;
            if(id == 0x1602ac || id == 0x1606ac)
                id = id & 0xff00ff;
            else
                id = id & 0xff003f;
            break;
        case 0x17: id = (op6 << 16) | func11; break;
        case 0x18: id = (op6 << 16) | func16; break;
        case 0x19: id = UNIMPLEMENTED;        break; // unimplemented instruction
        case 0x1a: id = (op6 << 16) | jmp_op; break;
        case 0x1b: id = UNIMPLEMENTED;        break; // unimplemented instruction
        case 0x1c: id = (op6 << 16) | func7;  break;
        case 0x1d: id = UNIMPLEMENTED;        break; // unimplemented instruction
        case 0x1e: id = UNIMPLEMENTED;        break; // unimplemented instruction
        case 0x1f: id = UNIMPLEMENTED;        break; // unimplemented instruction
        case 0x20: id =  op6 << 16;           break;
        case 0x21: id =  op6 << 16;           break;
        case 0x22: id =  op6 << 16;           break;
        case 0x23: id =  op6 << 16;           break;
        case 0x24: id =  op6 << 16;           break;
        case 0x25: id =  op6 << 16;           break;
        case 0x26: id =  op6 << 16;           break;
        case 0x27: id =  op6 << 16;           break;
        case 0x28: id =  op6 << 16;           break;
        case 0x29: id =  op6 << 16;           break;
        case 0x2a: id =  op6 << 16;           break;
        case 0x2b: id =  op6 << 16;           break;
        case 0x2c: id =  op6 << 16;           break;
        case 0x2d: id =  op6 << 16;           break;
        case 0x2e: id =  op6 << 16;           break;
        case 0x2f: id =  op6 << 16;           break;
        case 0x30: id =  op6 << 16;           break;
        case 0x31: id =  op6 << 16;           break;
        case 0x32: id =  op6 << 16;           break;
        case 0x33: id =  op6 << 16;           break;
        case 0x34: id =  op6 << 16;           break;
        case 0x35: id =  op6 << 16;           break;
        case 0x36: id =  op6 << 16;           break;
        case 0x37: id =  op6 << 16;           break;
        case 0x38: id =  op6 << 16;           break;
        case 0x39: id =  op6 << 16;           break;
        case 0x3a: id =  op6 << 16;           break;
        case 0x3b: id =  op6 << 16;           break;
        case 0x3c: id =  op6 << 16;           break;
        case 0x3d: id =  op6 << 16;           break;
        case 0x3e: id =  op6 << 16;           break;
        case 0x3f: id =  op6 << 16;           break;
        default:
            id = UNIMPLEMENTED; // unimplemented instruction
    }

    return id;
}
