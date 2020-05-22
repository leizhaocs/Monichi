#include "Core.h"

#define IBSIZE 5000  /* size of inst pool */

/* constructor */
Core::Core(INT argc, CHAR **argv, CHAR **envp)
{
    emu = new Emulator(argc, argv, envp);
    ID = 0;
    ib = new Instruction*[IBSIZE];
    for(INT i = 0; i < IBSIZE; i++)
        ib[i] = new Instruction(emu);
    ibIndex = 0;
    for(INT i = 0; i < 65; i++)
        renameREG[i] = -1;
    stall_issue = 0;
    pc = emu->getTruePC();
    cycle = 0;
    ninsts = 0;

    fetchWidth = 4;
    fetchQSize = 32;

    decodeWidth = 4;
    decodeQSize = 32;

    issueWidth = 4;
    FU = new FunctionUnit();

    commitWidth = 4;
    ROB = new ReorderBuffer(32);

    predictor = new BranchPredictor(32);

    ICache = new Cache("core0-icache", 32768, 4, 64, 2, 2, 0);
    DCache = new Cache("core0-dcache", 32768, 4, 64, 2, 2, 0);
    Cache *L2Cache = new Cache("core0-l2cache", 2097152, 8, 64, 20, 20, 0);
    ICache->set_lower_level(L2Cache);
    DCache->set_lower_level(L2Cache);
    L2Cache->add_upper_level(ICache);
    L2Cache->add_upper_level(DCache);
    L2Cache->set_memory(emu->mem);

    //fp = fopen("sc_inst1", "w");
}

/* destructor */
Core::~Core()
{
    fprintf(emu->STATS, "IPC: %f\n", DOUBLE(ninsts)/cycle);
    delete FU;
    delete ROB;
    delete predictor;
    delete ICache;
    delete DCache;
    //fclose(fp);
}

/* simulate one cycle */
INT Core::run_a_cycle()
{
    cycle++;
    commit();
    memory();
    execute();
    issue();
    decode();
    fetch();
    return emu->running;
}

/* update the pipeline when new data is computed */
void Core::update(INT r)
{
    if(ROB->entries[r].inst->RMem && (ROB->entries[r].state<3))
        return;
    if(ROB->entries[r].state < 2)
        return;

    /* get new computed data */
    if(ROB->entries[r].done == 0)
    {
        ROB->entries[r].done = 1;
        if(ROB->entries[r].inst->WRa > 0)
            ROB->entries[r].data = ROB->entries[r].inst->Rav;
        else if(ROB->entries[r].inst->WRb > 0)
            ROB->entries[r].data = ROB->entries[r].inst->Rbv;
        else if(ROB->entries[r].inst->WRc > 0)
            ROB->entries[r].data = ROB->entries[r].inst->Rcv;
        else if(ROB->entries[r].inst->Wfpcr > 0)
            ROB->entries[r].data = ROB->entries[r].inst->Fpcr;
        else if(ROB->entries[r].destR != -1)
            ROB->entries[r].cancel = 1;
    }

    /* sometimes the instruction suddenly decides not to write registers after (CMOV* insts) */
    if(ROB->entries[r].cancel == 1)
    {
        /* roll back renaming registers, so that following insts will not depend on this one any more */
        INT found = 0;
        INT target = r;
        while(1)
        {
            if((target!=r) && (ROB->entries[target].destR==ROB->entries[r].destR) && (ROB->entries[target].cancel==0))
            {
                found = 1;
                renameREG[ROB->entries[r].destR] = target;
                break;
            }
            if(target == ROB->ROBHead)
                break;
            target--;
            if(target < 0)
                target = ROB->entries.size() - 1;
        }
        if(!found)
        {
            renameREG[ROB->entries[r].destR] = -1;
        }

        /* update pending instructions */
        INT totalPendInsts = ROB->entries[r].pendInsts.size();
        for(INT i = 0; i < totalPendInsts; i++)
        {
            INT cand = ROB->entries[r].pendInsts.back();
            ROB->entries[r].pendInsts.pop_back();
            if(ROB->entries[cand].Qa == r)
            {
                if(target == r)
                {
                    ROB->entries[cand].Qa = -1;
                    ROB->entries[cand].inst->readRav();
                }
                else
                {
                    ROB->entries[cand].Qa = target;
                    ROB->entries[target].pendInsts.push_back(cand);
                    update(target);
                }
            }
            if(ROB->entries[cand].Qb == r)
            {
                if(target == r)
                {
                    ROB->entries[cand].Qb = -1;
                    ROB->entries[cand].inst->readRbv();
                }
                else
                {
                    ROB->entries[cand].Qb = target;
                    ROB->entries[target].pendInsts.push_back(cand);
                    update(target);
                }
            }
            if(ROB->entries[cand].Qc == r)
            {
                if(target == r)
                {
                    ROB->entries[cand].Qc = -1;
                    ROB->entries[cand].inst->readRcv();
                }
                else
                {
                    ROB->entries[cand].Qc = target;
                    ROB->entries[target].pendInsts.push_back(cand);
                    update(target);
                }
            }
            if(ROB->entries[cand].Qfpcr == r)
            {
                if(target == r)
                {
                    ROB->entries[cand].Qfpcr = -1;
                    ROB->entries[cand].inst->readFpcr();
                }
                else
                {
                    ROB->entries[cand].Qfpcr = target;
                    ROB->entries[target].pendInsts.push_back(cand);
                    update(target);
                }
            }
        }
        return;
    }

    /* update pending instructions */
    INT totalPendInsts = ROB->entries[r].pendInsts.size();
    for(INT i = 0; i < totalPendInsts; i++)
    {
        INT cand = ROB->entries[r].pendInsts.back();
        ROB->entries[r].pendInsts.pop_back();
        if(ROB->entries[cand].Qa == r)
        {
            ROB->entries[cand].Qa = -1;
            ROB->entries[cand].inst->setRav(ROB->entries[r].data);
        }
        if(ROB->entries[cand].Qb == r)
        {
            ROB->entries[cand].Qb = -1;
            ROB->entries[cand].inst->setRbv(ROB->entries[r].data);
        }
        if(ROB->entries[cand].Qc == r)
        {
            ROB->entries[cand].Qc = -1;
            ROB->entries[cand].inst->setRcv(ROB->entries[r].data);
        }
        if(ROB->entries[cand].Qfpcr == r)
        {
            ROB->entries[cand].Qfpcr = -1;
            ROB->entries[cand].inst->setFpcr(ROB->entries[r].data);
        }
    }
}

/* flush the mispredicted insts in pipeline */
void Core::flushPipeline(INT binst)
{
    /* empty fetch queue */
    while(!fetchQ.empty())
    {
        Instruction *pt = fetchQ.front();
        pt->valid = 0;
        fetchQ.pop();
    }

    /* empty decode queue */
    while(!decodeQ.empty())
    {
        Instruction *pt = decodeQ.front();
        pt->valid = 0;
        decodeQ.pop();
    }

    /* there cannot be correct syscall or ECB insts issued */
    stall_issue = 0;

    /* for instructions already issued, scan from the back of ROB */
    INT r = (ROB->ROBTail==0) ? (ROB->entries.size()-1) : (ROB->ROBTail-1);
    while(1)
    {
        if(r == binst)
            break;

        /* dummy instruction, delete from ROB directly */
        if(ROB->entries[r].inst->FU == 0)
        {
            ROB->clear(r);
            ROB->ROBTail = r;
            if(r == 0)
                r = ROB->entries.size() - 1;
            else
                r = r - 1;
            continue;
        }

        /* remove from function unit */
        for(INT i = 0; i < FU->entries.size(); i++)
        {
            if(FU->entries[i].type == ROB->entries[r].inst->FU)
            {
                /* if currently executing */
                if((FU->entries[i].used>0) && (FU->entries[i].ROB==r))
                {
                    FU->entries[i].used = 0;
                    break;
                }
                /* remove from reservation station */
                INT found = 0;
                for(INT j = 0; j < FU->entries[i].RS.size(); j ++)
                {
                    if(FU->entries[i].RS[j] == r)
                    {
                        FU->entries[i].RS[j] = -1;
                        found = 1;
                        break;
                    }
                }
                if(found)
                    break;
            }
        }

        /* roll back renaming register and delete pending information */
        INT t = binst;
        while(1)
        {
            /* need to roll back renaming register */
            if((ROB->entries[r].destR!=-1) && (renameREG[ROB->entries[r].destR]==r) && (ROB->entries[r].cancel==0))
            {
                if((ROB->entries[t].destR==ROB->entries[r].destR) && (ROB->entries[t].cancel==0))
                {
                    renameREG[ROB->entries[r].destR] = t;
                }
            }

            /* delete the pending information */
            for(INT i = 0; i < ROB->entries[t].pendInsts.size(); i++)
            {
                if(ROB->entries[t].pendInsts[i] == r)
                {
                    ROB->entries[t].pendInsts.resize(i);
                    break; // all following insts must also be predicted
                }
            }

            if(t == ROB->ROBHead)
            {
                /* check whether roll back register is done */
                if((ROB->entries[r].destR!=-1) && (renameREG[ROB->entries[r].destR]==r) && (ROB->entries[r].cancel==0))
                    renameREG[ROB->entries[r].destR] = -1;
                break;
            }
            if(t == 0)
                t = ROB->entries.size() - 1;
            else
                t = t - 1;
        }

        /* remove from ROB */
        ROB->entries[r].inst->valid = 0;
        ROB->clear(r);
        ROB->ROBTail = r;

        if(r == 0)
            r = ROB->entries.size() - 1;
        else
            r = r - 1;
    }
}

/* fetch state */
void Core::fetch()
{
    for(INT i = 0; i < fetchWidth; i++)
    {
        if(fetchQ.size() >= fetchQSize)
            break;

        /* get an instruction from instruction pool */
        Instruction *pt = ib[ibIndex++];
        if(ibIndex >= IBSIZE)
            ibIndex = 0;
        while(pt->valid == 1)
        {
            pt = ib[ibIndex++];
            if(ibIndex >= IBSIZE)
                ibIndex = 0;
        }

        /* fetch */
        //ICache->access(ID, pc, 0, (CHAR *)&(pt->ir), sizeof(pt->ir));
        //pt->setPC(pc);
        INT lat = ICache->access(ID, pc, 0, NULL, sizeof(pt->ir));
        pt->fetch(pc);

        /* branch prediction */
        ULONG target_pc;
        INT taken = predictor->predict(pc, &target_pc);
        if(taken == 0)
        {
            pc = pc + 4;
            pt->Ppc = pc;
        }
        else if(taken == 1)
        {
            pc = target_pc;
            pt->Ppc = pc;
        }
        else
        {
            pc = pc + 4;
        }

        fetchQ.push(pt);
    }
}

/* decode state */
void Core::decode()
{
    for(INT i = 0; i < decodeWidth; i++)
    {
        if(fetchQ.size() <= 0)
            break;
        if(decodeQ.size() >= decodeQSize)
            break;

        Instruction *pt = fetchQ.front();
        fetchQ.pop();
        pt->decode();
        decodeQ.push(pt);
    }
}

/* issue state */
void Core::issue()
{
    for(INT i = 0; i < issueWidth; i++)
    {
        if(decodeQ.size() <= 0)
            break;
        if(ROB->full())
            break;

        Instruction *pt = decodeQ.front();

        /* stop issuing until pipeline is empty */
        if(pt->CI == _EXCB_)
            if(!ROB->empty())
                break;

        /* system call instructions will stall issuing */
        if(pt->Syscall)
            stall_issue = 1;
        if(stall_issue)
        {
            if(!pt->Syscall)
                break;
            if(!ROB->empty())
                break;
        }

        /* do not need any function unit */
        if(pt->FU == 0)
        {
            ROB->clear(ROB->ROBTail);
            ROB->entries[ROB->ROBTail].state = 2;
            ROB->entries[ROB->ROBTail].inst = pt;
            ROB->ROBTail = (ROB->ROBTail+1) % ROB->entries.size();
            decodeQ.pop();
            continue;
        }

        /* try to issue to corresponding function unit */
        INT success = 0;
        for(INT j = 0; j < FU->entries.size(); j++)
        {
            if(FU->entries[j].type == pt->FU)
            {
                for(INT k = 0; k < FU->entries[j].RS.size(); k++)
                {
                    if(FU->entries[j].RS[k] == -1)
                    {
                        FU->entries[j].RS[k] = ROB->ROBTail;
                        success = 1;
                        break;
                    }
                }
            }
            if(success)
                break;
        }

        /* can not issue to function unit */
        if(success == 0)
            break;

        /* put into ROB */
        ROB->clear(ROB->ROBTail);
        ROB->entries[ROB->ROBTail].state = 1;
        ROB->entries[ROB->ROBTail].inst = pt;

        /* read dependency */
        if(pt->RRa > 0)
        {
            UCHAR renameIndex = (pt->RRa==1) ? pt->Ra : (pt->Ra+32);
            if(renameREG[renameIndex] != -1)
            {
                ROB->entries[ROB->ROBTail].Qa = renameREG[renameIndex];
                ROB->entries[renameREG[renameIndex]].pendInsts.push_back(ROB->ROBTail);
                update(renameREG[renameIndex]);
            }
            else
                pt->readRav();
        }
        if(pt->RRb > 0)
        {
            UCHAR renameIndex = (pt->RRb==1) ? pt->Rb : (pt->Rb+32);
            if(renameREG[renameIndex] != -1)
            {
                ROB->entries[ROB->ROBTail].Qb = renameREG[renameIndex];
                ROB->entries[renameREG[renameIndex]].pendInsts.push_back(ROB->ROBTail);
                update(renameREG[renameIndex]);
            }
            else
            {
                pt->readRbv();
            }
        }
        if(pt->RRc > 0)
        {
            UCHAR renameIndex = (pt->RRc==1) ? pt->Rc : (pt->Rc+32);
            if(renameREG[renameIndex] != -1)
            {
                ROB->entries[ROB->ROBTail].Qc = renameREG[renameIndex];
                ROB->entries[renameREG[renameIndex]].pendInsts.push_back(ROB->ROBTail);
                update(renameREG[renameIndex]);
            }
            else
                pt->readRcv();
        }
        if(pt->Rfpcr > 0)
        {
            if(renameREG[64] != -1)
            {
                ROB->entries[ROB->ROBTail].Qfpcr = renameREG[64];
                ROB->entries[renameREG[64]].pendInsts.push_back(ROB->ROBTail);
                update(renameREG[64]);
            }
            else
                pt->readFpcr();
        }

        /* write dependency */
        if(pt->WRa == 1)
        {
            ROB->entries[ROB->ROBTail].destR = pt->Ra;
            renameREG[pt->Ra] = ROB->ROBTail;
        }
        else if(pt->WRa == 2)
        {
            ROB->entries[ROB->ROBTail].destR = pt->Ra+32;
            renameREG[pt->Ra+32] = ROB->ROBTail;
        }
        if(pt->WRb == 1)
        {
            ROB->entries[ROB->ROBTail].destR = pt->Rb;
            renameREG[pt->Rb] = ROB->ROBTail;
        }
        else if(pt->WRb == 2)
        {
            ROB->entries[ROB->ROBTail].destR = pt->Rb+32;
            renameREG[pt->Rb+32] = ROB->ROBTail;
        }
        if(pt->WRc == 1)
        {
            ROB->entries[ROB->ROBTail].destR = pt->Rc;
            renameREG[pt->Rc] = ROB->ROBTail;
        }
        else if(pt->WRc == 2)
        {
            ROB->entries[ROB->ROBTail].destR = pt->Rc+32;
            renameREG[pt->Rc+32] = ROB->ROBTail;
        }
        if(pt->Wfpcr == 1)
        {
            ROB->entries[ROB->ROBTail].destR = 64;
            renameREG[64] = ROB->ROBTail;
        }

        ROB->ROBTail = (ROB->ROBTail+1) % ROB->entries.size();
        decodeQ.pop();
    }
}

/* execute state */
void Core::execute()
{
    for(INT i = 0; i < FU->entries.size(); i++)
    {
        if(FU->entries[i].used > 0)
        {
            FU->entries[i].used--;
            if(FU->entries[i].used == 0)
            {
                // execute
                ROB->entries[FU->entries[i].ROB].inst->execute();
                ROB->entries[FU->entries[i].ROB].state = 2;

                /* branch instruction */
                if(ROB->entries[FU->entries[i].ROB].inst->Bra)
                {
                    /* predicted wrong */
                    if(ROB->entries[FU->entries[i].ROB].inst->Npc != ROB->entries[FU->entries[i].ROB].inst->Ppc)
                    {
                        flushPipeline(FU->entries[i].ROB);
                        pc = ROB->entries[FU->entries[i].ROB].inst->Npc;
                    }
                    /* taken */
                    if(ROB->entries[FU->entries[i].ROB].inst->Npc != (ROB->entries[FU->entries[i].ROB].inst->Cpc+4))
                        predictor->update(ROB->entries[FU->entries[i].ROB].inst->Cpc, 1, ROB->entries[FU->entries[i].ROB].inst->Target);
                    /* not taken */
                    if(ROB->entries[FU->entries[i].ROB].inst->Npc == (ROB->entries[FU->entries[i].ROB].inst->Cpc+4))
                        predictor->update(ROB->entries[FU->entries[i].ROB].inst->Cpc, 0, ROB->entries[FU->entries[i].ROB].inst->Target);
                }
                /* read cycle instruction */
                if(ROB->entries[FU->entries[i].ROB].inst->CI == _RPCC_)
                    ROB->entries[FU->entries[i].ROB].inst->Rav = cycle;

                // update other insts
                update(FU->entries[i].ROB);
            }
        }
        /* peak a ready instruction */
        if(FU->entries[i].used == 0)
        {
            for(INT j = 0; j < FU->entries[i].RS.size(); j++)
            {
                if((FU->entries[i].RS[j]!=-1) && ROB->ready(FU->entries[i].RS[j]))
                {
                    FU->entries[i].ROB = FU->entries[i].RS[j];
                    FU->entries[i].used = FU->entries[i].latency;
                    FU->entries[i].RS[j] = -1;
                    break;
                }
            }
        }
    }
}

/* memory stage */
void Core::memory()
{
    /* at most one memory request each cycle */
    for(INT i = 0, temp = ROB->ROBHead; i <= ROB->entries.size(); i++, temp=(temp+1)%ROB->entries.size())
    {
        /* no more instructions */
        if(ROB->entries[temp].state == 0)
            break;
        /* load instruction */
        if(ROB->entries[temp].inst->RMem)
        {
            if(ROB->entries[temp].state == 1)
            {
                break;
            }
            else if(ROB->entries[temp].state == 2)
            {
                if(ROB->entries[temp].memLat > 0)
                {
                    ROB->entries[temp].memLat--;
                    if(ROB->entries[temp].memLat == 0)
                    {
                        ROB->entries[temp].state = 3;
                        update(temp);
                    }
                    break;
                }
                else
                {
                    //INT lat = DCache->access(ID, ROB->entries[temp].inst->Adr, 0, (CHAR *)&(ROB->entries[temp].inst->Memv), ROB->entries[temp].inst->RMem);
                    //ROB->entries[temp].inst->loadData();
                    INT lat = DCache->access(ID, ROB->entries[temp].inst->Adr, 0, NULL, ROB->entries[temp].inst->RMem);
                    ROB->entries[temp].inst->readMemory();
                    ROB->entries[temp].memLat = lat;
                    break;
                }
            }
            else if(ROB->entries[temp].state == 3)
            {
                continue;
            }
        }
        /* store instruction */
        if(ROB->entries[temp].inst->WMem)
        {
            if(ROB->entries[temp].state == 1)
            {
                break;
            }
            else if(ROB->entries[temp].state == 2)
            {
                ROB->entries[temp].state = 3;
                break;
            }
            else if(ROB->entries[temp].state == 3)
            {
                break;
            }
        }
        /* other instructions */
        else
        {
            if(ROB->entries[temp].state == 1)
            {
                continue;
            }
            else if(ROB->entries[temp].state == 2)
            {
                ROB->entries[temp].state = 3;
                continue;
            }
            else if(ROB->entries[temp].state == 3)
            {
                continue;
            }
        }
    }
}

/* commit stage */
void Core::commit()
{
    for(INT i = 0; i < commitWidth; i++)
    {
        /* no more commitable instructions */
        if(ROB->entries[ROB->ROBHead].state < 3)
            break;

        /* write memory */
        if(ROB->entries[ROB->ROBHead].inst->WMem)
        {
            if(ROB->entries[ROB->ROBHead].memLat > 0)
            {
                ROB->entries[ROB->ROBHead].memLat--;
                if(ROB->entries[ROB->ROBHead].memLat == 0)
                    break;
            }
            else
            {
                //ROB->entries[ROB->ROBHead].inst->writeData();
                //INT lat = DCache->access(ID, ROB->entries[ROB->ROBHead].inst->Adr, 1, (CHAR *)&(ROB->entries[ROB->ROBHead].inst->Memv), ROB->entries[ROB->ROBHead].inst->WMem);
                INT lat = DCache->access(ID, ROB->entries[ROB->ROBHead].inst->Adr, 1, NULL, ROB->entries[ROB->ROBHead].inst->WMem);
                ROB->entries[ROB->ROBHead].inst->writeMemory();
                ROB->entries[ROB->ROBHead].memLat = lat;
                break;
            }
        }
        /* write back register */
        ROB->entries[ROB->ROBHead].inst->writeRegister();
        /* perform system call */
        ROB->entries[ROB->ROBHead].inst->syscall();
        /* commit it */
        ROB->entries[ROB->ROBHead].inst->commit();

        /* update renaming register */
        for(INT j = 0; j < 65; j++)
        {
            if(renameREG[j] == ROB->ROBHead)
            {
                renameREG[j] = -1;
            }
        }

        /* resume issuing */
        if(ROB->entries[ROB->ROBHead].inst->Syscall)
            stall_issue = 0;

        //fprintf(fp, "pc: %llx\n", ROB->entries[ROB->ROBHead].inst->Cpc);
        //for(int i = 0; i < 32; i++)
        //    fprintf(fp, "  %llx", emu->as->r[i]);
        //for(int i = 0; i < 32; i++)
        //    fprintf(fp, "  %llx", emu->as->f[i]);
        //fprintf(fp, "  %llx", emu->as->fpcr);
        //fprintf(fp, "\n");

        ROB->clear(ROB->ROBHead);
        ROB->ROBHead = (ROB->ROBHead+1) % ROB->entries.size();

        ninsts++;
    }
}
