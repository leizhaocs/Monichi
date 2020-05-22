#ifndef EMULATOR_H
#define EMULATOR_H

#include "Define.h"

/* file header */
typedef struct
{
    UCHAR e_ident[16];  /* ELF identification */
    USHORT e_type;      /* Object file type */
    USHORT e_machine;   /* Machine type */
    UINT e_version;     /* Object file version */
    ULONG e_entry;      /* Entry point address */
    ULONG e_phoff;      /* Program header offset */
    ULONG e_shoff;      /* Section header offset */
    UINT e_flags;       /* Processor-specific flags */
    USHORT e_ehsize;    /* ELF header size */
    USHORT e_phentsize; /* Size of program header entry */
    USHORT e_phnum;     /* Number of program header entries */
    USHORT e_shentsize; /* Size of section header entry */
    USHORT e_shnum;     /* Number of section header entries */
    USHORT e_shstrndx;  /* Section name string table index */
}Elf64_Ehdr;

/* section header */
typedef struct
{
    UINT  sh_name;      /* Section name */
    UINT  sh_type;      /* Section type */
    ULONG sh_flags;     /* Section attributes */
    ULONG sh_addr;      /* Virtual address in memory */
    ULONG sh_offset;    /* Offset in file */
    ULONG sh_size;      /* Size of section */
    UINT  sh_link;      /* Link to other section */
    UINT  sh_info;      /* Miscellaneous information */
    ULONG sh_addralign; /* Address alignment boundary */
    ULONG sh_entsize;   /* Size of entries, if section has table */
}Elf64_Shdr;

/* program header */
typedef struct
{
    UINT  p_type;   /* Type of segment */
    UINT  p_flags;  /* Segment attributes */
    ULONG p_offset; /* Offset in file */
    ULONG p_vaddr;  /* Virtual address in memory */
    ULONG p_paddr;  /* Reserved */
    ULONG p_filesz; /* Size of segment in file */
    ULONG p_memsz;  /* Size of segment in memory */
    ULONG p_align;  /* Alignment of segment */
}Elf64_Phdr;

/* function simulator */
class Emulator
{
public:
    CHAR  program_name[512];  // program name
    INT   running;            // whether this emulator is running
    FILE  *LOG;               // log file
    FILE  *STATS;             // statistics file

    ArchitectureState  *as;   // all the registers
    MemorySystem       *mem;  // simulated memory
    SystemManager      *sys;  // system call
    Instruction        **ib;  // instruction pool
    Instruction        *pt;   // current instruction

    /* load information */
    ULONG ld_text_size;
    ULONG ld_text_base;
    ULONG ld_data_size;
    ULONG ld_data_base;
    ULONG ld_bss_size;
    ULONG ld_bss_base;
    ULONG stack_base;
    ULONG stack_min;
    ULONG argc_value;
    ULONG argv_data_size;
    ULONG envp_data_size;
    ULONG argv_array_size;
    ULONG envp_array_size;
    ULONG argv_array_base;
    ULONG envp_array_base;
    ULONG argv_data_base;
    ULONG envp_data_base;

    /* constructor and destructor */
    Emulator(INT argc, CHAR **argv, CHAR **envp);
    /* destructor */
    ~Emulator();
    /* loader */
    void load(INT argc, CHAR **argv, CHAR **envp);
    /* get true pc */
    ULONG getTruePC();
    /* main loop for functional simulation */
    void loop();
};

#endif
