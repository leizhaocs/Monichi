#include "Emulator.h"

#define IMSK 0x0ffff  /* mask of inst pool */

/* constructor */
Emulator::Emulator(INT argc, CHAR **argv, CHAR **envp)
{
    strcpy(program_name, argv[1]);
    running = 1;
    LOG = fopen("MonichiLog", "w");
    STATS = fopen("MonichiStats", "w");

    as  = new ArchitectureState(this);
    mem = new MemorySystem(this);
    sys = new SystemManager(this);
    ib = new Instruction*[IMSK+1];
    for(INT i = 0; i < IMSK+1; i++)
    {
        ib[i] = new Instruction(this);
    }

    ld_text_size = 0;
    ld_text_base = 0;
    ld_data_size = 0;
    ld_data_base = 0;
    ld_bss_size = 0;
    ld_bss_base = 0;
    stack_base = 0;
    stack_min = 0;
    argc_value = 0;
    argv_data_size = 0;
    envp_data_size = 0;
    argv_array_size = 0;
    envp_array_size = 0;
    argv_array_base = 0;
    envp_array_base = 0;
    argv_data_base = 0;
    envp_data_base = 0;

    load(argc-1, argv+1, envp);
}

/* destructor */
Emulator::~Emulator()
{
    fclose(LOG);
    fclose(STATS);
}

/* loader */
void Emulator::load(INT argc, CHAR **argv, CHAR **envp)
{
    FILE *fp;
    Elf64_Ehdr fhr;
    Elf64_Phdr phr;
    Elf64_Shdr shr;
    CHAR *shdr_strs;
    ULONG zero = 0;

    /* open file */
    if ((fp = fopen(program_name, "r")) == NULL)
    {
        fprintf(LOG, "emulator: Bad file name: %s\n", program_name);
        exit(1);
    }

    fread(&fhr, 1, sizeof(fhr), fp);

    /* load */
    for(INT i = 0; i < fhr.e_phnum; i++)
    {
        fseek(fp, fhr.e_phoff+fhr.e_phentsize*i, SEEK_SET);
        fread(&phr, 1, sizeof(phr), fp);

        if(phr.p_type != 1 && phr.p_type != 2 && phr.p_type != 3 && phr.p_type != 6)
            continue;

        fseek(fp, phr.p_offset, SEEK_SET);
        
        CHAR *buf = (CHAR *)calloc(phr.p_filesz, sizeof(CHAR));
        fread(buf, phr.p_filesz, 1, fp);

        ULONG *dat = (ULONG *)buf;
        ULONG adr = phr.p_vaddr;
        mem->store(phr.p_filesz, adr, dat);
        free(buf);
    }

    fseek(fp, fhr.e_shoff+(fhr.e_shstrndx*fhr.e_shentsize), SEEK_SET);
    fread(&shr, 1, sizeof(shr), fp);

    fseek(fp, shr.sh_offset, SEEK_SET);
    shdr_strs = (CHAR *)calloc(shr.sh_size, sizeof(CHAR));
    fread(shdr_strs, 1, shr.sh_size, fp);

    /* locate labels */
    for(INT i = 0; i < fhr.e_shnum; i++)
    {
        fseek(fp, fhr.e_shoff+(i*fhr.e_shentsize), SEEK_SET);
        fread(&shr, 1, sizeof(shr), fp);

        if(!strcmp(".text", shdr_strs + shr.sh_name))
        {
            ld_text_size = shr.sh_size;
            ld_text_base = shr.sh_addr;
        }

        if(!strcmp(".data", shdr_strs + shr.sh_name))
        {
            ld_data_size = shr.sh_size;
            ld_data_base = shr.sh_addr;
        }

        if(!strcmp(".bss", shdr_strs + shr.sh_name))
        {
            ld_bss_size = shr.sh_size;
            ld_bss_base = shr.sh_addr;
        }
    }

    free(shdr_strs);
    fclose(fp);

    stack_base = (ld_text_base - 1024*1024*8) & 0xffffffffffff0000ull;

    // calculate size of argv and envp pointer and data arrays
    for(argv_array_size = 0; argv[argv_array_size]; argv_array_size++)
        argv_data_size += strlen(argv[argv_array_size]) + 1;
    for(envp_array_size = 0; envp[envp_array_size]; envp_array_size++)
        envp_data_size += strlen(envp[envp_array_size]) + 1;

    argc_value = argv_array_size;
    argv_array_size = (argv_array_size + 1) * 8; // null terminated pointer array
    envp_array_size = (envp_array_size + 1) * 8; // null terminated pointer array

    // push stack to allow for argc, argv, envp, argv data, envp data
    stack_min -= (8 + // argc is 64-bits
                  argv_array_size +
                  envp_array_size +
                  argv_data_size +
                  envp_data_size);
    // align the stack min, arbitrarily to 8kb
    stack_min &= 0xffffffffffffe000ull;

    // map argv and envp
    argv_array_base = stack_min + 8; // allow for argc
    envp_array_base = argv_array_base + argv_array_size;
    argv_data_base = envp_array_base + envp_array_size;
    envp_data_base = argv_data_base + argv_data_size;

    // write argc to memory
    mem->store(8, stack_min, &argc_value);

    ULONG addr;
    UINT i;

    // write argv array and data to memory
    addr = argv_data_base;
    for(i = 0; argv[i]; i++)
    {
        // write argv pointer
        ULONG adr = argv_array_base + (i*8);
        mem->store(8, adr, &addr);
        // write argv data
        ULONG *dat = (ULONG *)(argv[i]);
        mem->store(strlen(argv[i])+1, addr, dat);
        // increment data address
        addr += strlen(argv[i]) + 1;
    }
    // null terminate argv array
    addr = argv_array_base + (i*8);
    mem->store(8, addr, &zero);

    // write envp array and data to memory
    addr = envp_data_base;
    for(i = 0; envp[i]; i++)
    {
        // write envp pointer
        ULONG adr = envp_array_base + (i*8);
        mem->store(8, adr, &addr);
        // write envp data
        ULONG *dat = (ULONG *)(envp[i]);
        mem->store(strlen(envp[i]) + 1, addr, dat);
        // increment data address
        addr += strlen(envp[i]) + 1;
    }
    // null terminate argv array
    addr = envp_array_base + (i * 8);
    mem->store(8, addr, &zero);

    as->pc    = fhr.e_entry;
    as->r[30] = stack_min;
    as->r[16] = argc_value;
    as->r[17] = argv_array_base;
    as->r[18] = envp_array_base;
    as->r[31] = 0;
    
    mem->brk_point = (ld_bss_base + ld_bss_size + 0xffffLL) & 0xffffffffffff0000LL;
    mem->mmap_end = 0x10000;
}

/* get true pc */
ULONG Emulator::getTruePC()
{
    return as->pc;
}       

/* function simulation loop */
void Emulator::loop()
{
    ULONG insts = 0;

    while(running)
    {
        INT index = (as->pc>>2) & IMSK;
        pt = ib[index];

        pt->fetch(as->pc);
        pt->decode();
        pt->readRegister();
        pt->execute();
        pt->readMemory();
        pt->writeMemory();
        pt->writeRegister();
        pt->syscall();
        pt->commit();

        insts++;
    }

    fprintf(STATS, "Total number of instructions: %llu\n", insts);
}
