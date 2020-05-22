#include "Instruction.h"

/* constructor */
Instruction::Instruction(Emulator *e)
{
    emu = e;
    valid = 0;
}

/* fetch the instruction */
void Instruction::fetch(ULONG pc)
{
    valid = 1;
    emu->mem->ld_inst(pc, &ir);
    Cpc = pc;
    Npc = pc + 4;
    Ppc = Npc;
}

/* decode the instruction */
void Instruction::decode()
{ 
    CI = get_code(ir);      // Code ID
    Op = (ir>>26) & 0x3f;   // Op field
    Ra = (ir>>21) & 0x1f;   // Ra or Fa field
    Rb = (ir>>16) & 0x1f;   // Rb or Fb field
    Rc = ir & 0x1f;         // Rc or Fc field
    Mdisp = ir & 0xffff;    // Memory displacement field
    Bdisp = ir & 0x1fffff;  // Branch displacement field
    SBZ = (ir>>13) & 0x7;   // SBZ field
    LIT = (ir>>13) & 0xff;  // LIT field
    RI = (ir>>12) & 0x1;    // Choose Rb or LIT
    PALF = ir & 0x3ffffff;  // PAL function field

    /* initialize */
    Adr = 0;
    Memv = 0;
    Rav = 0;
    Rbv = 0;
    Rcv = 0;
    Fpcr = 0;
    Target = 0;

    /* decode */
    Syscall=0; Bra=0; FU=0;
    RRa=0; WRa=0; RRb=0; WRb=0; RRc=0; WRc=0; RMem=0; WMem=0; Rfpcr=0; Wfpcr=0;
    switch(CI)
    {
        /* unimplemented instruction, do nothing only report at commit time */
        case UNIMPLEMENTED:                           break;

        /***********************************************************/
        case _CALL_PAL_:  FU=1; Syscall=1;            break;
        case _OPC01_:     CI = UNIMPLEMENTED;         break; //printf("Unrecognized Instruction  _OPC01_\n"); exit(1); break;
        case _OPC02_:     CI = UNIMPLEMENTED;         break; //printf("Unrecognized Instruction  _OPC02_\n"); exit(1); break;
        case _OPC03_:     CI = UNIMPLEMENTED;         break; //printf("Unrecognized Instruction  _OPC03_\n"); exit(1); break;
        case _OPC04_:     CI = UNIMPLEMENTED;         break; //printf("Unrecognized Instruction  _OPC04_\n"); exit(1); break;
        case _OPC05_:     CI = UNIMPLEMENTED;         break; //printf("Unrecognized Instruction  _OPC05_\n"); exit(1); break;
        case _OPC06_:     CI = UNIMPLEMENTED;         break; //printf("Unrecognized Instruction  _OPC06_\n"); exit(1); break;
        case _OPC07_:     CI = UNIMPLEMENTED;         break; //printf("Unrecognized Instruction  _OPC07_\n"); exit(1); break;
        case _LDA_:       FU=1; WRa=1; RRb=1;         break;
        case _LDAH_:      FU=1; WRa=1; RRb=1;         break;
        case _LDBU_:      FU=2; WRa=1; RRb=1; RMem=1; break;
        case _LDQ_U_:     FU=2; WRa=1; RRb=1; RMem=8; break;
        case _LDWU_:      FU=2; WRa=1; RRb=1; RMem=2; break;
        case _STW_:       FU=2; RRa=1; RRb=1; WMem=2; break;
        case _STB_:       FU=2; RRa=1; RRb=1; WMem=1; break;
        case _STQ_U_:     FU=2; RRa=1; RRb=1; WMem=8; break;

        /***********************************************************/
        case _ADDL_:    FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _S4ADDL_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _SUBL_:    FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _S4SUBL_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMPBGE_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _S8ADDL_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _S8SUBL_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMPULT_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _ADDQ_:    FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _S4ADDQ_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _SUBQ_:    FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _S4SUBQ_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMPEQ_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _S8ADDQ_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _S8SUBQ_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMPULE_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _ADDLv_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _SUBLv_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMPLT_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _ADDQv_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _SUBQv_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMPLE_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;

        /***********************************************************/
        case _AND_:      FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _BIC_:      FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMOVLBS_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMOVLBC_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _BIS_:      FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMOVEQ_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMOVNE_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _ORNOT_:    FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _XOR_:      FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMOVLT_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMOVGE_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _EQV_:      FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _AMASK_:    FU=1; RRb=RI?0:1; WRc=1;        break;
        case _CMOVLE_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _CMOVGT_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _IMPLVER_:  FU=1; WRc=1;                    break;

        /***********************************************************/
        case _MSKBL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _EXTBL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _INSBL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MSKWL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _EXTWL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _INSWL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MSKLL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _EXTLL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _INSLL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _ZAP_:     FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _ZAPNOT_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MSKQL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _SRL_:     FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _EXTQL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _SLL_:     FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _INSQL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _SRA_:     FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MSKWH_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _INSWH_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _EXTWH_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MSKLH_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _INSLH_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _EXTLH_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MSKQH_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _INSQH_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _EXTQH_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;

        /***********************************************************/
        case _MULL_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MULQ_:   FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _UMULH_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MULLv_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MULQv_:  FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;

        /***********************************************************/
        case _ITOFS_: FU=1; RRa=1; WRc=2; break;
        case _ITOFF_: CI = UNIMPLEMENTED; break; //printf("Unrecognized Instruction  _ITOFF_\n"); exit(1); break;
        case _ITOFT_: FU=1; RRa=1; WRc=2; break;
        case _SQRTF_: CI = UNIMPLEMENTED; break; //printf("Unrecognized Instruction  _SQRTF_\n"); exit(1); break;
        case _SQRTS_: FU=4; RRb=2; WRc=2; break;
        case _SQRTG_: CI = UNIMPLEMENTED; break; //printf("Unrecognized Instruction  _SQRTG_\n"); exit(1); break;
        case _SQRTT_: FU=4; RRb=2; WRc=2; break;

        /***********************************************************/
        case _ADDS_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _SUBS_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _MULS_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _DIVS_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _ADDT_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _SUBT_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _MULT_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _DIVT_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _CMPTUN_: FU=4; RRa=2; RRb=2; WRc=2; break;
        case _CMPTEQ_: FU=4; RRa=2; RRb=2; WRc=2; break;
        case _CMPTLT_: FU=4; RRa=2; RRb=2; WRc=2; break;
        case _CMPTLE_: FU=4; RRa=2; RRb=2; WRc=2; break;
        case _CVTTS_:  FU=4; RRb=2; WRc=2;        break;
        case _CVTTQ_:  FU=4; RRb=2; WRc=2;        break;
        case _CVTQS_:  FU=4; RRb=2; WRc=2;        break;
        case _CVTQT_:  FU=4; RRb=2; WRc=2;        break;
        case _CVTST_:  FU=4; RRb=2; WRc=2;        break;

        /***********************************************************/
        case _CVTLQ_:   FU=1; RRb=2; WRc=2;        break;
        case _CPYS_:    FU=4; RRa=2; RRb=2; WRc=2; break;
        case _CPYSN_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _CPYSE_:   FU=4; RRa=2; RRb=2; WRc=2; break;
        case _MT_FPCR_: FU=1; RRa=2; Wfpcr=1;      break;
        case _MF_FPCR_: FU=1; WRc=2; Rfpcr=1;      break;
        case _FCMOVEQ_: FU=1; RRa=2; RRb=2; WRc=2; break;
        case _FCMOVNE_: FU=1; RRa=2; RRb=2; WRc=2; break;
        case _FCMOVLT_: FU=1; RRa=2; RRb=2; WRc=2; break;
        case _FCMOVGE_: FU=1; RRa=2; RRb=2; WRc=2; break;
        case _FCMOVLE_: FU=1; RRa=2; RRb=2; WRc=2; break;
        case _FCMOVGT_: FU=1; RRa=2; RRb=2; WRc=2; break;
        case _CVTQL_:   FU=1; RRb=2; WRc=2;        break;
        case _CVTQLv_:  FU=1; RRb=2; WRc=2;        break;
        case _CVTQLsv_: FU=1; RRb=2; WRc=2;        break;

        /***********************************************************/
        case _TRAPB_:                break;
        case _EXCB_:                 break;
        case _MB_:                   break;
        case _WMB_:                  break;
        case _FETCH_:                break;
        case _FETCH_M_:              break;
        case _RPCC_:    FU=1; WRa=1; break;
        case _RC_:                   break;
        case _ECB_:                  break;
        case _RS_:                   break;
        case _WH64_:                 break;

        /***********************************************************/
        case _JMP_:    FU=3; Bra=1; WRa=1; RRb=1; break;
        case _JSR_:    FU=3; Bra=1; WRa=1; RRb=1; break;
        case _RET_:    FU=3; Bra=1; WRa=1; RRb=1; break;
        case _JSR_C_:  FU=3; Bra=1; WRa=1; RRb=1; break;

        /***********************************************************/
        case _SEXTB_:  FU=1; RRb=RI?0:1; WRc=1;        break;
        case _SEXTW_:  FU=1; RRb=RI?0:1; WRc=1;        break;
        case _CTPOP_:  FU=1; RRb=1; WRc=1;             break;
        case _PERR_:   FU=1; RRa=1; RRb=1; WRc=1;      break;
        case _CTLZ_:   FU=1; RRb=1; WRc=1;             break;
        case _CTTZ_:   FU=1; RRb=1; WRc=1;             break;
        case _UNPKBW_: FU=1; RRb=1; WRc=1;             break;
        case _UNPKBL_: FU=1; RRb=1; WRc=1;             break;
        case _PKWB_:   FU=1; RRb=1; WRc=1;             break;
        case _PKLB_:   FU=1; RRb=1; WRc=1;             break;
        case _MINSB8_: FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MINSW4_: FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MINUB8_: FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MINUW4_: FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MAXUB8_: FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MAXUW4_: FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MAXSB8_: FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _MAXSW4_: FU=1; RRa=1; RRb=RI?0:1; WRc=1; break;
        case _FTOIT_:  FU=1; RRa=2; WRc=1;             break;
        case _FTOIS_:  FU=1; RRa=2; WRc=1;             break;

        /***********************************************************/
        case _LDF_:    CI = UNIMPLEMENTED;                break; //printf("Unrecognized Instruction  _LDF_\n"); exit(1); break;
        case _LDG_:    CI = UNIMPLEMENTED;                break; //printf("Unrecognized Instruction  _LDG_\n"); exit(1); break;
        case _LDS_:    FU=2; WRa=2; RRb=1; RMem=4;        break;
        case _LDT_:    FU=2; WRa=2; RRb=1; RMem=8;        break;
        case _STF_:    CI = UNIMPLEMENTED;                break; //printf("Unrecognized Instruction  _STF_\n"); exit(1); break;
        case _STG_:    CI = UNIMPLEMENTED;                break; //printf("Unrecognized Instruction  _STG_\n"); exit(1); break;
        case _STS_:    FU=2; RRa=2; RRb=1; WMem=4;        break;
        case _STT_:    FU=2; RRa=2; RRb=1; WMem=8;        break;
        case _LDL_:    FU=2; WRa=1; RRb=1; RMem=4;        break;
        case _LDQ_:    FU=2; WRa=1; RRb=1; RMem=8;        break;
        case _LDL_L_:  FU=2; WRa=1; RRb=1; RMem=4;        break;
        case _LDQ_L_:  FU=2; WRa=1; RRb=1; RMem=8;        break;
        case _STL_:    FU=2; RRa=1; RRb=1; WMem=4;        break;
        case _STQ_:    FU=2; RRa=1; RRb=1; WMem=8;        break;
        case _STL_C_:  FU=2; RRa=1; WRa=1; RRb=1; WMem=4; break;
        case _STQ_C_:  FU=2; RRa=1; WRa=1; RRb=1; WMem=8; break;

        /***********************************************************/
        case _BR_:    FU=3; Bra=1; WRa=1; break;
        case _FBEQ_:  FU=3; Bra=1; RRa=2; break;
        case _FBLT_:  FU=3; Bra=1; RRa=2; break;
        case _FBLE_:  FU=3; Bra=1; RRa=2; break;
        case _BSR_:   FU=3; Bra=1; WRa=1; break;
        case _FBNE_:  FU=3; Bra=1; RRa=2; break;
        case _FBGE_:  FU=3; Bra=1; RRa=2; break;
        case _FBGT_:  FU=3; Bra=1; RRa=2; break;
        case _BLBC_:  FU=3; Bra=1; RRa=1; break;
        case _BEQ_:   FU=3; Bra=1; RRa=1; break;
        case _BLT_:   FU=3; Bra=1; RRa=1; break;
        case _BLE_:   FU=3; Bra=1; RRa=1; break;
        case _BLBS_:  FU=3; Bra=1; RRa=1; break;
        case _BNE_:   FU=3; Bra=1; RRa=1; break;
        case _BGE_:   FU=3; Bra=1; RRa=1; break;
        case _BGT_:   FU=3; Bra=1; RRa=1; break;

        default: CI = UNIMPLEMENTED; //printf("Unrecognized Instruction  %x\n", CI); exit(1); break;
    }

    /* R31 is always 0, so no need to write it */
    if((WRa==1) && (Ra==31))
        WRa = 0;
    if((WRb==1) && (Rb==31))
        WRb = 0;
    if((WRc==1) && (Rc==31))
        WRc = 0;
}

/* execute the instruction */
void Instruction::execute()
{
    switch(CI)
    {
        /* unimplemented instruction, do nothing only report at commit time */
        case -1: break;

        /***********************************************************/
        case _CALL_PAL_:
        {
            break;
        }
        case _OPC01_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _OPC02_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _OPC03_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _OPC04_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _OPC05_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _OPC06_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _OPC07_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _LDA_:
        {
            Rav = Rbv + sext16((ULONG)Mdisp);
            break;
        }
        case _LDAH_:
        {
            Rav = Rbv + (sext16((ULONG)Mdisp) << 16);
            break;
        }
        case _LDBU_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            break;
        }
        case _LDQ_U_:
        {
            Adr = (Rbv + sext16((ULONG)Mdisp)) & (~(0x7ull));
            break;
        }
        case _LDWU_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            break;
        }
        case _STW_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            Memv = Rav;
            break;
        }
        case _STB_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            Memv = Rav;
            break;
        }
        case _STQ_U_:
        {
            Adr = (Rbv + sext16((ULONG)Mdisp)) & (~(0x7ull));
            Memv = Rav;
            break;
        }

        /***********************************************************/
        case _ADDL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)(Rav+tempb));
            break;
        }
        case _S4ADDL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)((Rav<<2)+tempb));
            break;
        }
        case _SUBL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)(Rav-tempb));
            break;
        }
        case _S4SUBL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)((Rav<<2)-tempb));
            break;
        }
        case _CMPBGE_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = 0;
            if((Rav&(~0xffffffffffffff00ull)) >= (tempb&(~0xffffffffffffff00ull))) Rcv |= 0x01ull;
            if((Rav&(~0xffffffffffff00ffull)) >= (tempb&(~0xffffffffffff00ffull))) Rcv |= 0x02ull;
            if((Rav&(~0xffffffffff00ffffull)) >= (tempb&(~0xffffffffff00ffffull))) Rcv |= 0x04ull;
            if((Rav&(~0xffffffff00ffffffull)) >= (tempb&(~0xffffffff00ffffffull))) Rcv |= 0x08ull;
            if((Rav&(~0xffffff00ffffffffull)) >= (tempb&(~0xffffff00ffffffffull))) Rcv |= 0x10ull;
            if((Rav&(~0xffff00ffffffffffull)) >= (tempb&(~0xffff00ffffffffffull))) Rcv |= 0x20ull;
            if((Rav&(~0xff00ffffffffffffull)) >= (tempb&(~0xff00ffffffffffffull))) Rcv |= 0x40ull;
            if((Rav&(~0x00ffffffffffffffull)) >= (tempb&(~0x00ffffffffffffffull))) Rcv |= 0x80ull;
            break;
        }
        case _S8ADDL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)((Rav<<3)+tempb));
            break;
        }
        case _S8SUBL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)((Rav<<3)-tempb));
            break;
        }
        case _CMPULT_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav<tempb ? 1 : 0;
            break;
        }
        case _ADDQ_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav + tempb;
            break;
        }
        case _S4ADDQ_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav<<2) + tempb;
            break;
        }
        case _SUBQ_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav - tempb;
            break;
        }
        case _S4SUBQ_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav<<2) - tempb;
            break;
        }
        case _CMPEQ_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (LONG)Rav==(LONG)tempb ? 1 : 0;
            break;
        }
        case _S8ADDQ_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav<<3) + tempb;
            break;
        }
        case _S8SUBQ_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav<<3) - tempb;
            break;
        }
        case _CMPULE_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav<=tempb ? 1 : 0;
            break;
        }
        case _ADDLv_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)(Rav+tempb));
            break;
        }
        case _SUBLv_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)(Rav-tempb));
            break;
        }
        case _CMPLT_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (LONG)Rav<(LONG)tempb ? 1 : 0;
            break;
        }
        case _ADDQv_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav + tempb;
            break;
        }
        case _SUBQv_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav - tempb;
            break;
        }
        case _CMPLE_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (LONG)Rav<=(LONG)tempb ? 1 : 0;
            break;
        }

        /***********************************************************/
        case _AND_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav & tempb;
            break;
        }
        case _BIC_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav & (~tempb);
            break;
        }
        case _CMOVLBS_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if(Rav&0x1ull)
                Rcv = tempb;
            else
                WRc = 0;
            break;
        }
        case _CMOVLBC_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if(!(Rav&0x1ull))
                Rcv = tempb;
            else
                WRc = 0;
            break;
        }
        case _BIS_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav | tempb;
            break;
        }
        case _CMOVEQ_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((LONG)Rav==0)
                Rcv = tempb;
            else
                WRc = 0;
            break;
        }
        case _CMOVNE_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((LONG)Rav!=0)
                Rcv = tempb;
            else
                WRc = 0;
            break;
        }
        case _ORNOT_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav | (~tempb);
            break;
        }
        case _XOR_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav ^ tempb;
            break;
        }
        case _CMOVLT_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((LONG)Rav<0)
                Rcv = tempb;
            else
                WRc = 0;
            break;
        }
        case _CMOVGE_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((LONG)Rav>=0)
                Rcv = tempb;
            else
                WRc = 0;
            break;
        }
        case _EQV_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav ^ (~tempb);
            break;
        }
        case _AMASK_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = tempb & (~(0x17ull));
            break;
        }
        case _CMOVLE_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((LONG)Rav<=0)
                Rcv = tempb;
            else
                WRc = 0;
            break;
        }
        case _CMOVGT_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((LONG)Rav>0)
                Rcv = tempb;
            else
                WRc = 0;
            break;
        }
        case _IMPLVER_:
        {
            Rcv = 0x2ull;
            break;
        }

        /***********************************************************/
        case _MSKBL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav & ~(0xffull<<((tempb&0x7ull)*8));
            break;
        }
        case _EXTBL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav>>((tempb&0x7ull)*8)) & 0xffull;
            break;
        }
        case _INSBL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav&0xffull) << ((tempb&0x7ull)*8);
            break;
        }
        case _MSKWL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav & ~(0xffffull<<((tempb&0x7ull)*8));
            break;
        }
        case _EXTWL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav>>((tempb&0x7ull)*8)) & 0xffffull;
            break;
        }
        case _INSWL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav&0xffffull) << ((tempb&0x7ull)*8);
            break;
        }
        case _MSKLL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav & ~(0xffffffffull<<((tempb&0x7ull)*8));
            break;
        }
        case _EXTLL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav>>((tempb&0x7ull)*8)) & 0xffffffffull;
            break;
        }
        case _INSLL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav&0xffffffffull) << ((tempb&0x7ull)*8);
            break;
        }
        case _ZAP_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav;
            if(tempb & 0x01ull) Rcv &= 0xffffffffffffff00ull;
            if(tempb & 0x02ull) Rcv &= 0xffffffffffff00ffull;
            if(tempb & 0x04ull) Rcv &= 0xffffffffff00ffffull;
            if(tempb & 0x08ull) Rcv &= 0xffffffff00ffffffull;
            if(tempb & 0x10ull) Rcv &= 0xffffff00ffffffffull;
            if(tempb & 0x20ull) Rcv &= 0xffff00ffffffffffull;
            if(tempb & 0x40ull) Rcv &= 0xff00ffffffffffffull;
            if(tempb & 0x80ull) Rcv &= 0x00ffffffffffffffull;
            break;
        }
        case _ZAPNOT_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav;
            if(~tempb & 0x01ull) Rcv &= 0xffffffffffffff00ull;
            if(~tempb & 0x02ull) Rcv &= 0xffffffffffff00ffull;
            if(~tempb & 0x04ull) Rcv &= 0xffffffffff00ffffull;
            if(~tempb & 0x08ull) Rcv &= 0xffffffff00ffffffull;
            if(~tempb & 0x10ull) Rcv &= 0xffffff00ffffffffull;
            if(~tempb & 0x20ull) Rcv &= 0xffff00ffffffffffull;
            if(~tempb & 0x40ull) Rcv &= 0xff00ffffffffffffull;
            if(~tempb & 0x80ull) Rcv &= 0x00ffffffffffffffull;
            break;
        }
        case _MSKQL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav& ~(0xffffffffffffffffull<<((tempb&0x7ull)*8));
            break;
        }
        case _SRL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav >> (tempb&0x3full);
            break;
        }
        case _EXTQL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav>>((tempb&0x7ull)*8));
            break;
        }
        case _SLL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav << (tempb&0x3full);
            break;
        }
        case _INSQL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = Rav << ((tempb&0x7ull)*8);
            break;
        }
        case _SRA_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (LONG)Rav >> (tempb&0x3full);
            break;
        }
        case _MSKWH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((tempb&0x7ull) != 0)
                Rcv = Rav & ~(0xffffull>>(64-(tempb&0x7ull)*8));
            else
                Rcv = Rav;
            break;
        }
        case _INSWH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((tempb&0x7ull) != 0)
                Rcv = (Rav&0xffffull) >> (64-(tempb&0x7ull)*8);
            else
                Rcv = 0ull;
            break;
        }
        case _EXTWH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav<<(64-(tempb&0x7ull)*8)) & 0xffffull;
            break;
        }
        case _MSKLH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((tempb&0x7ull) != 0)
                Rcv = Rav & ~(0xffffffffull>>(64-(tempb&0x7ull)*8));
            else
                Rcv = Rav;
            break;
        }
        case _INSLH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((tempb&0x7ull) != 0)
                Rcv = (Rav&0xffffffffull) >> (64-(tempb&0x7ull)*8);
            else
                Rcv = 0ull;
            break;
        }
        case _EXTLH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav<<(64-(tempb&0x7ull)*8)) & 0xffffffffull;
            break;
        }
        case _MSKQH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((tempb&0x7ull) != 0)
                Rcv = Rav & ~(0xffffffffffffffffull>>(64-(tempb&0x7ull)*8));
            else
                Rcv = Rav;
            break;
        }
        case _INSQH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            if((tempb&0x7ull) != 0)
                Rcv = Rav >> (64-(tempb&0x7ull)*8);
            else
                Rcv = 0ull;
            break;
        }
        case _EXTQH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = (Rav<<(64-(tempb&0x7ull)*8));
            break;
        }

        /***********************************************************/
        case _MULL_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)(Rav*tempb));
            break;
        }
        case _MULQ_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            ULONG unused;
            mul128(Rav, tempb, &unused, &Rcv);
            break;
        }
        case _UMULH_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            ULONG unused;
            mul128(Rav, tempb, &Rcv, &unused);
            break;
        }
        case _MULLv_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext32((ULONG)(Rav*tempb));
            break;
        }
        case _MULQv_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            ULONG unused;
            mul128(Rav, tempb, &unused, &Rcv);
            break;
        }

        /***********************************************************/
        case _ITOFS_:
        {
            ULONG temp1 = 0, temp2 = 0;

            UINT i8 = (UINT)((Rav >> 23) & 0xffull);
            if(i8 == 0x0)
                temp1 = 0x0ull;
            else if(i8 & 0x80)
                temp1 = 0x400ull | (i8 & 0x7full);
            else
                temp1 = 0x380ull | (i8 & 0x7full);

            temp2 = ((Rav & 0x80000000ull) << 32) | (temp1 << 52) | ((Rav & 0x7fffffull) << 29);
            memcpy(&Rcv, &temp2, 8);
            break;
        }
        case _ITOFF_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _ITOFT_:
        {
            Rcv = Rav;
            break;
        }
        case _SQRTF_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _SQRTS_:
        {
            DOUBLE fb = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE temp = (DOUBLE)sqrt(fb);
            memcpy(&Rcv, &temp, sizeof(ULONG));
            break;
        }
        case _SQRTG_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _SQRTT_:
        {
            DOUBLE db = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE temp = (DOUBLE)sqrt(db);
            memcpy(&Rcv, &temp, sizeof(ULONG));
            break;
        }

        /***********************************************************/
        case _ADDS_:
        {
            DOUBLE fa = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE fb = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE fc = fa + fb;
            memcpy(&Rcv, &fc, sizeof(ULONG));
            break;
        }
        case _SUBS_:
        {
            DOUBLE fa = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE fb = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE fc = fa - fb;
            memcpy(&Rcv, &fc, sizeof(ULONG));
            break;
        }
        case _MULS_:
        {
            DOUBLE fa = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE fb = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE fc = fa * fb;
            memcpy(&Rcv, &fc, sizeof(ULONG));
            break;
        }
        case _DIVS_:
        {
            DOUBLE fa = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE fb = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE fc = fa / fb;
            memcpy(&Rcv, &fc, sizeof(ULONG));
            break;
        }
        case _ADDT_:
        {
            DOUBLE da = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE db = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE dc = da + db;
            memcpy(&Rcv, &dc, sizeof(ULONG));
            break;
        }
        case _SUBT_:
        {
            DOUBLE da = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE db = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE dc = da - db;
            memcpy(&Rcv, &dc, sizeof(ULONG));
            break;
        }
        case _MULT_:
        {
            DOUBLE da = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE db = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE dc = da * db;
            memcpy(&Rcv, &dc, sizeof(ULONG));
            break;
        }
        case _DIVT_:
        {
            DOUBLE da = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE db = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            DOUBLE dc = da / db;
            memcpy(&Rcv, &dc, sizeof(ULONG));
            break;
        }
        case _CMPTUN_:
        {
            DOUBLE d;
            DOUBLE da = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE db = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            if(!(da < db) && !(da == db) && !(da > db)) 
                d = 2.0;
            else
                d = 0.0;
            memcpy(&Rcv, &d, sizeof(ULONG));
            break;
        }
        case _CMPTEQ_:
        {
            DOUBLE d;
            DOUBLE da = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE db = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            if(da == db)
                d = 2.0;
            else
                d = 0.0;
            memcpy(&Rcv, &d, sizeof(ULONG));
            break;
        }
        case _CMPTLT_:
        {
            DOUBLE d;
            DOUBLE da = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE db = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            if(da < db)
                d = 2.0;
            else
                d = 0.0;
            memcpy(&Rcv, &d, sizeof(ULONG));
            break;
        }
        case _CMPTLE_:
        {
            DOUBLE d;
            DOUBLE da = (DOUBLE)(*((DOUBLE *)(&Rav)));
            DOUBLE db = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            if(da <= db)
                d = 2.0;
            else
                d = 0.0;
            memcpy(&Rcv, &d, sizeof(ULONG));
            break;
        }
        case _CVTTS_:
        {
            FLOAT f = (FLOAT)(*((DOUBLE *)(&Rbv)));
            DOUBLE d = (DOUBLE)f;
            memcpy(&Rcv, &d, sizeof(ULONG));
            break;
        }
        case _CVTTQ_:
        {
            DOUBLE d = (DOUBLE)(*((DOUBLE *)(&Rbv)));
            Rcv = (LONG)d;
            break;
        }
        case _CVTQS_:
        {
            DOUBLE d = (DOUBLE)((LONG)Rbv);
            memcpy(&Rcv, &d, sizeof(ULONG));
            break;
        }
        case _CVTQT_:
        {
            DOUBLE d = (DOUBLE)((LONG)Rbv);
            memcpy(&Rcv, &d, sizeof(ULONG));
            break;
        }
        case _CVTST_:
        {
            FLOAT f = (FLOAT)(*((DOUBLE *)(&Rbv)));
            DOUBLE d = (DOUBLE)f;
            memcpy(&Rcv, &d, sizeof(ULONG));
            break;
        }

        /***********************************************************/
        case _CVTLQ_:
        {
            ULONG  temp;
            temp = ((Rbv>>32)&0xc0000000ull) | ((Rbv>>29)&0x3fffffffull);
            Rcv = sext32((ULONG)temp);
            break;
        }
        case _CPYS_:
        {
            Rcv = (Rav&0x8000000000000000ull) | (Rbv&0x7fffffffffffffffull);
            break;
        }
        case _CPYSN_:
        {
            Rcv = ((~Rav)&0x8000000000000000ull) | (Rbv&0x7fffffffffffffffull);
            break;
        }
        case _CPYSE_:
        {
            Rcv = (Rav&(0xfffull<<52)) | (Rbv&0xfffffffffffffull);
            break;
        }
        case _MT_FPCR_:
        {
            Fpcr = Rav;
            break;
        }
        case _MF_FPCR_:
        {
            Rcv = Fpcr;
            break;
        }
        case _FCMOVEQ_:
        {
            if((Rav<<1) == 0ull)
                Rcv = Rbv;
            else
                WRc = 0;
            break;
        }
        case _FCMOVNE_:
        {
            if((Rav<<1) != 0ull)
                Rcv = Rbv;
            else
                WRc = 0;
            break;
        }
        case _FCMOVLT_:
        {
            if(((Rav<<1)!=0ull) && (Rav>>63))
                Rcv = Rbv;
            else
                WRc = 0;
            break;
        }
        case _FCMOVGE_:
        {
            if(((Rav<<1)==0ull) || ((Rav>>63)==0ull))
                Rcv = Rbv;
            else
                WRc = 0;
            break;
        }
        case _FCMOVLE_:
        {
            if(((Rav<<1)==0ull) || (Rav>>63))
                Rcv = Rbv;
            else
                WRc = 0;
            break;
        }
        case _FCMOVGT_:
        {
            if(((Rav<<1)!=0ull) && ((Rav>>63)==0ull))
                Rcv = Rbv;
            else
                WRc = 0;
            break;
        }
        case _CVTQL_:
        {
            Rcv = ((Rbv<<32)&0xc000000000000000ull) | ((Rbv&0x3fffffffull)<<29);
            break;
        }
        case _CVTQLv_:
        {
            Rcv = ((Rbv<<32)&0xc000000000000000ull) | ((Rbv&0x3fffffffull)<<29);
            break;
        }
        case _CVTQLsv_:
        {
            Rcv = ((Rbv<<32)&0xc000000000000000ull) | ((Rbv&0x3fffffffull)<<29);
            break;
        }

        /***********************************************************/
        case _TRAPB_:
        {
            break;
        }
        case _EXCB_:
        {
            break;
        }
        case _MB_:
        {
            break;
        }
        case _WMB_:
        {
            break;
        }
        case _FETCH_:
        {
            break;
        }
        case _FETCH_M_:
        {
            break;
        }
        case _RPCC_:
        {
            Rav = 0x0ull;
            break;
        }
        case _RC_:
        {
            break;
        }
        case _ECB_:
        {
            break;
        }
        case _RS_:
        {
            break;
        }
        case _WH64_:
        {
            break;
        }

        /***********************************************************/
        case _JMP_:
        {
            Rav = Cpc + 4;
            Target = Rbv & (~(0x3ull));
            Npc = Target;
            break;
        }
        case _JSR_:
        {
            Rav = Cpc + 4;
            Target = Rbv & (~(0x3ull));
            Npc = Target;
            break;
        }
        case _RET_:
        {
            Rav = Cpc + 4;
            Target = Rbv & (~(0x3ull));
            Npc = Target;
            break;
        }
        case _JSR_C_:
        {
            Rav = Cpc + 4;
            Target = Rbv & (~(0x3ull));
            Npc = Target;
            break;
        }

        /***********************************************************/
        case _SEXTB_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext8((ULONG)tempb);
            break;
        }
        case _SEXTW_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            Rcv = sext16((ULONG)tempb);
            break;
        }
        case _CTPOP_:
        {
            ULONG temp = 0;
            for(INT i = 0; i < 64; i++)
            {
                if((Rbv>>i)&0x1ull)
                    temp++;
            }
            Rcv = temp;
            break;
        }
        case _PERR_:
        {
            for(INT i = 0; i <= 7; i++)
            {
                UCHAR a = (Rav>>(i*8)) & 0xff;
                UCHAR b = (Rbv>>(i*8)) & 0xff;
                if(a >= b)
                    Rcv += (a - b);
                else
                    Rcv += (b - a);
            }
            break;
        }
        case _CTLZ_:
        {
            ULONG temp = 0;
            for(INT i = 63; i >=0 ; i--)
            {
                if((Rbv>>i)&0x1ull)
                    break;
                else
                    temp++;
            }
            Rcv = temp;
            break;
        }
        case _CTTZ_:
        {
            ULONG temp = 0;
            for(INT i = 0; i < 64; i++)
            {
                if((Rbv>>i)&0x1ull)
                    break;
                else
                    temp++;
            }
            Rcv = temp;
            break;
        }
        case _UNPKBW_:
        {
            Rcv |= (Rbv & 0xff);
            Rcv |= (((Rbv>>8)&0xff) << 16);
            Rcv |= (((Rbv>>16)&0xff) << 32);
            Rcv |= (((Rbv>>24)&0xff) << 48);
            break;
        }
        case _UNPKBL_:
        {
            Rcv |= (Rbv & 0xff);
            Rcv |= (((Rbv>>8)&0xff) << 32);
            break;
        }
        case _PKWB_:
        {
            Rcv |= (Rbv & 0xff);
            Rcv |= (((Rbv>>16)&0xff) << 8);
            Rcv |= (((Rbv>>32)&0xff) << 16);
            Rcv |= (((Rbv>>48)&0xff) << 24);
            break;
        }
        case _PKLB_:
        {
            Rcv |= (Rbv & 0xff);
            Rcv |= (((Rbv>>32)&0xffull) << 8);
            break;
        }
        case _MINSB8_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            for(INT i = 0; i <= 7; i++)
            {
                CHAR a = (Rav>>(i*8)) & 0xff;
                CHAR b = (tempb>>(i*8)) & 0xff;
                CHAR c = (a<b) ? a : b;
                Rcv = (Rcv<<8) | (UCHAR)c;
            }
            break;
        }
        case _MINSW4_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            for(INT i = 0; i <= 3; i++)
            {
                SHORT a = (Rav>>(i*16)) & 0xffff;
                SHORT b = (tempb>>(i*16)) & 0xffff;
                SHORT c = (a<b) ? a : b;
                Rcv = (Rcv<<16) | (USHORT)c;
            }
            break;
        }
        case _MINUB8_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            for(INT i = 0; i <= 7; i++)
            {
                UCHAR a = (Rav>>(i*8)) & 0xff;
                UCHAR b = (tempb>>(i*8)) & 0xff;
                UCHAR c = (a<b) ? a : b;
                Rcv = (Rcv<<8) | (UCHAR)c;
            }
            break;
        }
        case _MINUW4_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            for(INT i = 0; i <= 3; i++)
            {
                USHORT a = (Rav>>(i*16)) & 0xffff;
                USHORT b = (tempb>>(i*16)) & 0xffff;
                USHORT c = (a<b) ? a : b;
                Rcv = (Rcv<<16) | (USHORT)c;
            }
            break;
        }
        case _MAXUB8_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            for(INT i = 0; i <= 7; i++)
            {
                UCHAR a = (Rav>>(i*8)) & 0xff;
                UCHAR b = (tempb>>(i*8)) & 0xff;
                UCHAR c = (a>b) ? a : b;
                Rcv = (Rcv<<8) | (UCHAR)c;
            }
            break;
        }
        case _MAXUW4_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            for(INT i = 0; i <= 3; i++)
            {
                USHORT a = (Rav>>(i*16)) & 0xffff;
                USHORT b = (tempb>>(i*16)) & 0xffff;
                USHORT c = (a>b) ? a : b;
                Rcv = (Rcv<<16) | (USHORT)c;
            }
            break;
        }
        case _MAXSB8_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            for(INT i = 0; i <= 7; i++)
            {
                CHAR a = (Rav>>(i*8)) & 0xff;
                CHAR b = (tempb>>(i*8)) & 0xff;
                CHAR c = (a>b) ? a : b;
                Rcv = (Rcv<<8) | (UCHAR)c;
            }
            break;
        }
        case _MAXSW4_:
        {
            ULONG tempb = RI ? (ULONG)LIT : Rbv;
            for(INT i = 0; i <= 3; i++)
            {
                SHORT a = (Rav>>(i*16)) & 0xffff;
                SHORT b = (tempb>>(i*16)) & 0xffff;
                SHORT c = (a>b) ? a : b;
                Rcv = (Rcv<<16) | (USHORT)c;
            }
            break;
        }
        case _FTOIT_:
        {
            Rcv = Rav;
            break;
        }
        case _FTOIS_:
        {
            Rcv = ((Rav>>32) & 0xc0000000ull) | ((Rav >> 29) & 0x3fffffffull);
            Rcv = sext32((ULONG)Rcv);
            break;
        }

        /***********************************************************/
        case _LDF_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _LDG_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _LDS_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            break;
        }
        case _LDT_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            break;
        }
        case _STF_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _STG_: fprintf(emu->LOG, "instruction: Unrecognized Instruction\n"); break;
        case _STS_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            Memv = Rav;
            break;
        }
        case _STT_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            Memv = Rav;
            break;
        }
        case _LDL_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            break;
        }
        case _LDQ_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            break;
        }
        case _LDL_L_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            break;
        }
        case _LDQ_L_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            break;
        }
        case _STL_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            Memv = Rav;
            break;
        }
        case _STQ_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            Memv = Rav;
            break;
        }
        case _STL_C_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            Memv = Rav;
            Rav = 0x1ull;
            break;
        }
        case _STQ_C_:
        {
            Adr = Rbv + sext16((ULONG)Mdisp);
            Memv = Rav;
            Rav = 0x1ull;
            break;
        }

        /***********************************************************/
        case _BR_:
        {
            Rav = Cpc + 4;
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            Npc = Target;
            break;
        }
        case _FBEQ_:
        {
            DOUBLE d = (DOUBLE)(*((DOUBLE *)(&Rav)));
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if(d == 0.0)
                Npc = Target;
            break;
        }
        case _FBLT_:
        {
            DOUBLE d = (DOUBLE)(*((DOUBLE *)(&Rav)));
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if(d < 0.0)
                Npc = Target;
            break;
        }
        case _FBLE_:
        {
            DOUBLE d = (DOUBLE)(*((DOUBLE *)(&Rav)));
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if(d <= 0.0)
                Npc = Target;
            break;
        }
        case _BSR_:
        {
            Rav = Cpc + 4;
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            Npc = Target;
            break;
        }
        case _FBNE_:
        {
            DOUBLE d = (DOUBLE)(*((DOUBLE *)(&Rav)));
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if(d != 0.0)
                Npc = Target;
            break;
        }
        case _FBGE_:
        {
            DOUBLE d = (DOUBLE)(*((DOUBLE *)(&Rav)));
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if(d >= 0.0)
                Npc = Target;
            break;
        }
        case _FBGT_:
        {
            DOUBLE d = (DOUBLE)(*((DOUBLE *)(&Rav)));
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if(d > 0.0)
                Npc = Target;
            break;
        }
        case _BLBC_:
        {
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if(!(Rav & 0x1ull))
                Npc = Target;
            break;
        }
        case _BEQ_:
        {
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if((LONG)Rav == 0)
                Npc = Target;
            break;
        }
        case _BLT_:
        {
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if((LONG)Rav < 0)
                Npc = Target;
            break;
        }
        case _BLE_:
        {
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if((LONG)Rav <= 0)
                Npc = Target;
            break;
        }
        case _BLBS_:
        {
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if(Rav & 0x1ull)
                Npc = Target;
            break;
        }
        case _BNE_:
        {
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if((LONG)Rav != 0)
                Npc = Target;
            break;
        }
        case _BGE_:
        {
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if((LONG)Rav >= 0)
                Npc = Target;
            break;
        }
        case _BGT_:
        {
            Target = Cpc + 4 + (sext21((ULONG)Bdisp)<<2);
            if((LONG)Rav > 0)
                Npc = Target;
            break;
        }

        default:
        {
            fprintf(emu->LOG, "instruction: Unrecognized Instruction\n");
            break;
        }
    }
}

/* read register */
void Instruction::readRegister()
{
    if(RRa == 1)
        Rav = emu->as->r[Ra];
    else if(RRa == 2)
        Rav = emu->as->f[Ra];
    if(RRb == 1)
        Rbv = emu->as->r[Rb];
    else if(RRb == 2)
        Rbv = emu->as->f[Rb];
    if(RRc == 1)
        Rcv = emu->as->r[Rc];
    else if(RRc == 2)
        Rcv = emu->as->f[Rc];
    if(Rfpcr == 1)
        Fpcr = emu->as->fpcr;
}

/* write register */
void Instruction::writeRegister()
{
    if(WRa == 1)
        emu->as->r[Ra] = Rav;
    else if(WRa == 2)
        emu->as->f[Ra] = Rav;
    if(WRb == 1)
        emu->as->r[Rb] = Rbv;
    else if(WRb == 2)
        emu->as->f[Rb] = Rbv;
    if(WRc == 1)
        emu->as->r[Rc] = Rcv;
    else if(WRc == 2)
        emu->as->f[Rc] = Rcv;
    if(Wfpcr == 1)
        emu->as->fpcr = Fpcr;
}

/* load from memory */
void Instruction::readMemory()
{
    if(RMem)
    {
        emu->mem->load(RMem, Adr, &Memv);

        if(CI == _LDL_ || CI == _LDL_L_)
            Memv = sext32((ULONG)Memv);
        else if(CI == _LDS_)
        {
            ULONG e1 = Memv & 0x40000000;
            ULONG e2 = (Memv >> 23) & 0x7f;
            if(e1)
            {
                if(e2 == 0x3f800000)
                    e2 = 0x7ff;
                else
                    e2 |= 0x400;
            }
            else
            {
                if(e2 == 0)
                    e2 = 0;
                else
                    e2 |= 0x380;
            }					
            Memv = (Memv & 0x80000000) << 32 | e2 << 52 | (Memv & 0x7fffff) << 29;
        }
        Rav = Memv;
    }
}

/* store to memory */
void Instruction::writeMemory()
{
    if(WMem)
    {
        if(CI == _STS_)
        {
            ULONG temp = 0;
            temp |= ((Memv >> 29) & 0x3fffffff);
            temp |= ((Memv >> 32) & 0xc0000000);
            Memv = temp;
        }

        emu->mem->store(WMem, Adr, &Memv);
    }
}

/* execute system call */
void Instruction::syscall()
{
    if(Syscall == 1)
        emu->sys->execute_pal(this);
}

/* commit, update pc */
void Instruction::commit()
{
    /* unimplemented instruction trying to commit, report error */
    if(CI == UNIMPLEMENTED)
    {
        fprintf(emu->LOG, "instruction: Unimplemented instruction trying to commit\n");
        exit(1);
    }
    emu->as->pc = Npc;
    valid = 0;
}

/* set Cpc Npc Ppc */
void Instruction::setPC(ULONG pc)
{
    valid = 1;
    Cpc = pc;
    Npc = pc + 4;
    Ppc = Npc;
}

/* read Rav */
void Instruction::readRav()
{
    if(RRa == 1)
        Rav = emu->as->r[Ra];
    else if(RRa == 2)
        Rav = emu->as->f[Ra];
}

/* read Rbv */
void Instruction::readRbv()
{
    if(RRb == 1)
        Rbv = emu->as->r[Rb];
    else if(RRb == 2)
        Rbv = emu->as->f[Rb];
}

/* read Rcv */
void Instruction::readRcv()
{
    if(RRc == 1)
        Rcv = emu->as->r[Rc];
    else if(RRc == 2)
        Rcv = emu->as->f[Rc];
}

/* read Fpcr */
void Instruction::readFpcr()
{
    if(Rfpcr == 1)
        Fpcr = emu->as->fpcr;
}

/* set Rav value */
void Instruction::setRav(ULONG rav)
{
    Rav = rav;
}

/* set Rbv value */
void Instruction::setRbv(ULONG rbv)
{
    Rbv = rbv;
}

/* set Rcv value */
void Instruction::setRcv(ULONG rcv)
{
    Rcv = rcv;
}

/* set Fpcr value */
void Instruction::setFpcr(ULONG fpcr)
{
    Fpcr = fpcr;
}

/* manipulate data read from memory */
void Instruction::loadData()
{
    if(RMem)
    {
        if(CI == _LDL_ || CI == _LDL_L_)
            Memv = sext32((ULONG)Memv);
        else if(CI == _LDS_)
        {
            ULONG e1 = Memv & 0x40000000;
            ULONG e2 = (Memv >> 23) & 0x7f;
            if(e1)
            {
                if(e2 == 0x3f800000)
                    e2 = 0x7ff;
                else
                    e2 |= 0x400;
            }
            else
            {
                if(e2 == 0)
                    e2 = 0;
                else
                    e2 |= 0x380;
            }					
            Memv = (Memv & 0x80000000) << 32 | e2 << 52 | (Memv & 0x7fffff) << 29;
        }
        Rav = Memv;
    }
}

/* manipulate data write to memory */
void Instruction::writeData()
{
    if(WMem)
    {
        if(CI == _STS_)
        {
            ULONG temp = 0;
            temp |= ((Memv >> 29) & 0x3fffffff);
            temp |= ((Memv >> 32) & 0xc0000000);
            Memv = temp;
        }
    }
}
