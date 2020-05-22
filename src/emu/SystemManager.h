#ifndef SYSTEM_MANAGER_M
#define SYSTEM_MANAGER_M

#include "Define.h"

#define PAGE_SIZE 8192 // 8192 bytes in a virtual memory page

/* for systemcall implementation */
#define SYSCALL_OSF_SYSCALL      0
#define SYSCALL_exit             1
#define SYSCALL_READ             3
#define SYSCALL_WRITE            4
#define SYSCALL_CLOSE            6
#define SYSCALL_UNLINK          10
#define SYSCALL_BRK             17
#define	SYSCALL_LSEEK	        19
#define SYSCALL_GETXPID         20
#define SYSCALL_GETXUID         24
#define SYSCALL_KILL            37
#define SYSCALL_OPEN            45
#define SYSCALL_GETXGID         47
#define SYSCALL_OSF_SIGPROCMASK 48
#define SYSCALL_IOCTL           54
#define SYSCALL_MMAP            71
#define SYSCALL_MUNMAP          73
#define SYSCALL_MPROTECT        74
#define SYSCALL_FCNTL           92
#define SYSCALL_WRITEV         121
#define SYSCALL_FTRUNCATE      130
#define	SYSCALL_GETRLIMIT      144
#define	SYSCALL_SETRLIMIT      145
#define SYSCALL_OSF_GETSYSINFO 256
#define SYSCALL_OSF_SETSYSINFO 257
#define SYSCALL_TIMES          323
#define SYSCALL_UNAME          339
#define SYSCALL_MREMAP         341
#define SYSCALL_RT_SIGACTION   352
#define SYSCALL_GETTIMEOFDAY   359
#define SYSCALL_GETCWD         367
#define SYSCALL_EXIT_GROUP     405
#define SYSCALL_STAT64         425
#define SYSCALL_LSTAT64        426
#define SYSCALL_FSTAT64        427

/* used in ASYS_fstat */
struct osf_statbuf
{
  UINT   a_st_dev;
  UINT   a_st_ino;
  UINT   a_st_mode;
  USHORT a_st_nlink;
  USHORT a_pad0;
  UINT   a_st_uid;
  UINT   a_st_gid;
  UINT   a_st_rdev;
  UINT   a_pad1;
  ULONG  a_st_size;
  UINT   a_st_atime;
  UINT   a_st_spare1;
  UINT   a_st_mtime;
  UINT   a_st_spare2;
  UINT   a_st_ctime;
  UINT   a_st_spare3;
  UINT   a_st_blksize;
  UINT   a_st_blocks;
  UINT   a_st_gennum;
  UINT   a_st_spare4;
};

/* used in SYSCALL_UNAME */
struct alpha_utsname
{
    CHAR sysname[65];
    CHAR nodename[65];
    CHAR release[65];
    CHAR version[65];
    CHAR machine[65];
    CHAR domainname[65];
};

/* used in SYSCALL_WRITEV */
struct alpha_iovec
{
  ULONG iov_base;
  ULONG iov_len;
};

/* used in SYSCALL_GETTIMEOFDAY */
struct alpha_timeval
{
  LONG tv_sec;  // seconds
  LONG tv_usec; // microseconds
};

/* used in SYSCALL_GETTIMEOFDAY */
struct alpha_timezone
{
  LONG tz_minuteswest;
  LONG tz_dsttime;
};

/* manage system call */
class SystemManager
{
public:
    Emulator *emu;                  // emulator
    ULONG uniq;                     // unique Data for rduniq & wruniq inst

    /* constructor */
    SystemManager(Emulator *e);
    /* handle system calls */
    void execute_pal(Instruction *p);
};

#endif
