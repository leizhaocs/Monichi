#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h> 
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include "ArchitectureState.h"
#include "Memory.h"
#include "SystemManager.h"
#include "Instruction.h"
#include "Emulator.h"

/************************************************/
/* 1. Type Definition                           */

typedef char               CHAR;   // 8 bits
typedef short              SHORT;  // 16 bits
typedef int                INT;    // 32 bits
typedef long long          LONG;   // 64 bits
typedef unsigned char      UCHAR;  // unsigned 8 bits
typedef unsigned short     USHORT; // unsigned 16 bits
typedef unsigned int       UINT;   // unsigned 32 bits
typedef unsigned long long ULONG;  // unsigned 64 bits
typedef float              FLOAT;  // 32 bits fp
typedef double             DOUBLE; // 64 bits fp

/************************************************/
/* 2. Constant Definition                       */

#define BIT0    0x0000000000000001ull
#define BIT1    0x0000000000000002ull
#define BIT2    0x0000000000000004ull
#define BIT3    0x0000000000000008ull
#define BIT4    0x0000000000000010ull
#define BIT5    0x0000000000000020ull
#define BIT6    0x0000000000000040ull
#define BIT7    0x0000000000000080ull
#define BIT8    0x0000000000000100ull
#define BIT9    0x0000000000000200ull
#define BIT10   0x0000000000000400ull
#define BIT11   0x0000000000000800ull
#define BIT12   0x0000000000001000ull
#define BIT15   0x0000000000008000ull
#define BIT20   0x0000000000100000ull
#define BIT31   0x0000000080000000ull
#define BIT63   0x8000000000000000ull

#define EXTND08 0xffffffffffffff00ull
#define EXTND16 0xffffffffffff0000ull
#define EXTND21 0xffffffffffe00000ull
#define EXTND32 0xffffffff00000000ull

#define MASK08  0x00000000000000ffull
#define MASK16  0x000000000000ffffull
#define MASK21  0x00000000001fffffull
#define MASK32  0x00000000ffffffffull

/** by Alpha Architecture Handbook V.4 **/

/* unimplemented instruction */
#define UNIMPLEMENTED  -1

/* 0x0 */
#define _CALL_PAL_  0x000000
#define _OPC01_     0x010000
#define _OPC02_     0x020000
#define _OPC03_     0x030000
#define _OPC04_     0x040000
#define _OPC05_     0x050000
#define _OPC06_     0x060000
#define _OPC07_     0x070000
#define _LDA_       0x080000
#define _LDAH_      0x090000
#define _LDBU_      0x0a0000
#define _LDQ_U_     0x0b0000
#define _LDWU_      0x0c0000
#define _STW_       0x0d0000
#define _STB_       0x0e0000
#define _STQ_U_     0x0f0000

/* 0x10 */
#define _ADDL_    0x100000
#define _S4ADDL_  0x100002
#define _SUBL_    0x100009
#define _S4SUBL_  0x10000b
#define _CMPBGE_  0x10000f
#define _S8ADDL_  0x100012
#define _S8SUBL_  0x10001b
#define _CMPULT_  0x10001d
#define _ADDQ_    0x100020
#define _S4ADDQ_  0x100022
#define _SUBQ_    0x100029
#define _S4SUBQ_  0x10002b
#define _CMPEQ_   0x10002d
#define _S8ADDQ_  0x100032
#define _S8SUBQ_  0x10003b
#define _CMPULE_  0x10003d
#define _ADDLv_   0x100040
#define _SUBLv_   0x100049
#define _CMPLT_   0x10004d
#define _ADDQv_   0x100060
#define _SUBQv_   0x100069
#define _CMPLE_   0x10006d

/* 0x11 */
#define _AND_     0x110000
#define _BIC_     0x110008
#define _CMOVLBS_ 0x110014
#define _CMOVLBC_ 0x110016
#define _BIS_     0x110020
#define _CMOVEQ_  0x110024
#define _CMOVNE_  0x110026
#define _ORNOT_   0x110028
#define _XOR_     0x110040
#define _CMOVLT_  0x110044
#define _CMOVGE_  0x110046
#define _EQV_     0x110048
#define _AMASK_   0x110061
#define _CMOVLE_  0x110064
#define _CMOVGT_  0x110066
#define _IMPLVER_ 0x11006c

/* 0x12 */
#define _MSKBL_   0x120002
#define _EXTBL_   0x120006
#define _INSBL_   0x12000b
#define _MSKWL_   0x120012
#define _EXTWL_   0x120016
#define _INSWL_   0x12001b
#define _MSKLL_   0x120022
#define _EXTLL_   0x120026
#define _INSLL_   0x12002b
#define _ZAP_     0x120030
#define _ZAPNOT_  0x120031
#define _MSKQL_   0x120032
#define _SRL_     0x120034
#define _EXTQL_   0x120036
#define _SLL_     0x120039
#define _INSQL_   0x12003b
#define _SRA_     0x12003c
#define _MSKWH_   0x120052
#define _INSWH_   0x120057
#define _EXTWH_   0x12005a
#define _MSKLH_   0x120062
#define _INSLH_   0x120067
#define _EXTLH_   0x12006a
#define _MSKQH_   0x120072
#define _INSQH_   0x120077
#define _EXTQH_   0x12007a

/* 0x13 */
#define _MULL_   0x130000
#define _MULQ_   0x130020
#define _UMULH_  0x130030
#define _MULLv_  0x130040
#define _MULQv_  0x130060

/* 0x14 */
#define _ITOFS_  0x140004
#define _ITOFF_  0x140014
#define _ITOFT_  0x140024
#define _SQRTF_  0x14000a  /* and 0xff002f */
#define _SQRTS_  0x14000b  /* and 0xff002f */
#define _SQRTG_  0x14002a  /* and 0xff002f */
#define _SQRTT_  0x14002b  /* and 0xff002f */

/* 0x16 */
#define _ADDS_    0x160000  /* and 0xff003f */
#define _SUBS_    0x160001  /* and 0xff003f */
#define _MULS_    0x160002  /* and 0xff003f */
#define _DIVS_    0x160003  /* and 0xff003f */
#define _ADDT_    0x160020  /* and 0xff003f */
#define _SUBT_    0x160021  /* and 0xff003f */
#define _MULT_    0x160022  /* and 0xff003f */
#define _DIVT_    0x160023  /* and 0xff003f */
#define _CMPTUN_  0x160024  /* and 0xff003f */
#define _CMPTEQ_  0x160025  /* and 0xff003f */
#define _CMPTLT_  0x160026  /* and 0xff003f */
#define _CMPTLE_  0x160027  /* and 0xff003f */
#define _CVTTS_   0x16002c  /* and 0xff003f */
#define _CVTTQ_   0x16002f  /* and 0xff003f */
#define _CVTQS_   0x16003c  /* and 0xff003f */
#define _CVTQT_   0x16003e  /* and 0xff003f */
#define _CVTST_   0x1600ac  /* and 0xff00ff for 0x1602ac 0x1606ac */

/* 0x17 */
#define _CVTLQ_    0x170010
#define _CPYS_     0x170020
#define _CPYSN_    0x170021
#define _CPYSE_    0x170022
#define _MT_FPCR_  0x170024
#define _MF_FPCR_  0x170025
#define _FCMOVEQ_  0x17002a
#define _FCMOVNE_  0x17002b
#define _FCMOVLT_  0x17002c
#define _FCMOVGE_  0x17002d
#define _FCMOVLE_  0x17002e
#define _FCMOVGT_  0x17002f
#define _CVTQL_    0x170030
#define _CVTQLv_   0x170130
#define _CVTQLsv_  0x170530

/* 0x18 */
#define _TRAPB_    0x180000
#define _EXCB_     0x180400
#define _MB_       0x184000
#define _WMB_      0x184400
#define _FETCH_    0x188000
#define _FETCH_M_  0x18a000
#define _RPCC_     0x18c000
#define _RC_       0x18e000
#define _ECB_      0x18e800
#define _RS_       0x18f000
#define _WH64_     0x18f800

/* 0x1a */
#define _JMP_    0x1a0000
#define _JSR_    0x1a0001
#define _RET_    0x1a0002
#define _JSR_C_  0x1a0003

/* 0x1c */
#define _SEXTB_   0x1c0000
#define _SEXTW_   0x1c0001
#define _CTPOP_   0x1c0030
#define _PERR_    0x1c0031
#define _CTLZ_    0x1c0032
#define _CTTZ_    0x1c0033
#define _UNPKBW_  0x1c0034
#define _UNPKBL_  0x1c0035
#define _PKWB_    0x1c0036
#define _PKLB_    0x1c0037
#define _MINSB8_  0x1c0038
#define _MINSW4_  0x1c0039
#define _MINUB8_  0x1c003a
#define _MINUW4_  0x1c003b
#define _MAXUB8_  0x1c003c
#define _MAXUW4_  0x1c003d
#define _MAXSB8_  0x1c003e
#define _MAXSW4_  0x1c003f
#define _FTOIT_   0x1c0070
#define _FTOIS_   0x1c0078

/* 0x2 */
#define _LDF_    0x200000
#define _LDG_    0x210000
#define _LDS_    0x220000
#define _LDT_    0x230000
#define _STF_    0x240000
#define _STG_    0x250000
#define _STS_    0x260000
#define _STT_    0x270000
#define _LDL_    0x280000
#define _LDQ_    0x290000
#define _LDL_L_  0x2a0000
#define _LDQ_L_  0x2b0000
#define _STL_    0x2c0000
#define _STQ_    0x2d0000
#define _STL_C_  0x2e0000
#define _STQ_C_  0x2f0000

/* 0x3 */
#define _BR_    0x300000
#define _FBEQ_  0x310000
#define _FBLT_  0x320000
#define _FBLE_  0x330000
#define _BSR_   0x340000
#define _FBNE_  0x350000
#define _FBGE_  0x360000
#define _FBGT_  0x370000
#define _BLBC_  0x380000
#define _BEQ_   0x390000
#define _BLT_   0x3a0000
#define _BLE_   0x3b0000
#define _BLBS_  0x3c0000
#define _BNE_   0x3d0000
#define _BGE_   0x3e0000
#define _BGT_   0x3f0000

/************************************************/
/* 3. class declaration                         */

class ArchitectureState;
class MainMemory;
class MemorySystem;
class SystemManager;
class Emulator;
class Instruction;

/************************************************/
/* 4. function prototypes                       */

/* (sing) extend 8bit data into 64 bit data */
extern ULONG sext8(ULONG d);
/* (sing) extend 16bit data into 64 bit data */
extern ULONG sext16(ULONG d);
/* (sing) extend 21bit data into 64 bit data */
extern ULONG sext21(ULONG d);
/* (sing) extend 32bit data into 64 bit data */
extern ULONG sext32(ULONG d);
/* 64 bit multiply Input of A and B, output of High 64b and Low 64b */
void mul128(ULONG a, ULONG b, ULONG *rh, ULONG *rl);
/* return 24bit unique instruction ID */
extern INT get_code(UINT ir);
