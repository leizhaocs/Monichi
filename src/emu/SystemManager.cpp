#include "SystemManager.h"

/* constructor */
SystemManager::SystemManager(Emulator *e)
{
    emu = e;
    uniq = 0;
}

/* handle system calls */
void SystemManager::execute_pal(Instruction *p)
{
    UINT ir = p->ir;
    ULONG syscode = emu->as->r[0];

    /* fix for syscall() which uses CALL_PAL CALLSYS for making system calls */
    if(syscode == SYSCALL_OSF_SYSCALL)
        syscode = emu->as->r[16];

    if((ir&0xFFFF) != 0x0083)
    {
        switch((ir&0xFFFF))
        {
            case 0x009e: // rdunique(Read Unique Value)
                emu->as->r[0] = (uniq);
                return;
            case 0x009f: // wrunique(Write Unique Value)
                uniq  = emu->as->r[16];
                return;
            default:
                return; 
        }
    }

    printf("system call %llu\n", syscode);

    /* make sure to set R0 and R19 in every system call */
    /* but in case of mistake, we set default value here */
    emu->as->r[0] = 0;             // return 0
    emu->as->r[19] = ((ULONG)-1);  // unsucceed

    /**********************************************/
    if(syscode == SYSCALL_exit) // 1
    {
        emu->running = 0;

        emu->as->r[0] = 0;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_READ) // 3
    {
        LONG fd = emu->as->r[16];
        ULONG vaddr = emu->as->r[17];
        LONG count = emu->as->r[18];
        LONG len;
        ULONG *buf;

        // alllocate a buffer
        buf = (ULONG *)malloc(count);
        if(!buf)
            fprintf(emu->LOG, "syscall SYSCALL_READ: ran out of memory\n");

        // perform the read
        len = read(fd, buf, count);

        // copy the result to the simulator
        if(len > 0)
        {
            emu->mem->store(len, vaddr, buf);

            emu->as->r[0] = len;
            emu->as->r[19] = 0;
        }
        else if(len == 0) // eof
        {
            emu->as->r[0] = 0;
            emu->as->r[19] = 0;
        }
        else
        {
            emu->as->r[0] = errno;
            emu->as->r[19] = ((ULONG)-1);
        }

        // free the buffer
        free(buf);
    }

    /**********************************************/
    else if(syscode == SYSCALL_WRITE) // 4
    {
        LONG fd = emu->as->r[16];
        ULONG vaddr = emu->as->r[17];
        LONG count = emu->as->r[18];
        LONG len;
        ULONG *buf;

        // allocate a buffer
        buf = (ULONG *)malloc(count);
        if(!buf)
            fprintf(emu->LOG, "syscall SYSCALL_WRITE: ran out of memory\n");

        // copy the buffer from the simulator
        emu->mem->load(count, vaddr, buf);

        // write the buffer out to the fd
        len = write(fd, buf, count);
        fsync(fd);

        // free the buffer
        free(buf);

        // set the result
        if(len >= 0)
        {
            emu->as->r[0] = len;
            emu->as->r[19] = 0;
        }
        else
        {
            emu->as->r[0] = errno;
            emu->as->r[19] = ((ULONG)-1);
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_CLOSE) // 6
    {
        LONG fd = emu->as->r[16];
        INT res;
    
        if(fd > 2)
        {
            res = close(fd);
            if(res != ((LONG)-1))
            {
                emu->as->r[0] = 0;
                emu->as->r[19] = 0;
            }
            else
            {
                emu->as->r[0] = errno;
                emu->as->r[19] = ((ULONG)-1);
            }
        }
        else
        {
            emu->as->r[0] = 0;
            emu->as->r[19] = 0;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_UNLINK) // 10
    {
        CHAR pathname[1024];
        ULONG pathname_ptr = emu->as->r[16];
        LONG result;

        // read path out of simulator memory
        emu->mem->load(-1, pathname_ptr, (ULONG *)pathname);

        result = unlink(pathname);
        if(result == ((LONG)-1))
        {
            emu->as->r[0] = errno;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            emu->as->r[0] = 0;
            emu->as->r[19] = 0;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_BRK) // 17
    {
        LONG delta = emu->as->r[16];
        ULONG addr;

        if(!delta)
            addr = emu->mem->brk_point;
        else 
            addr = delta;

        emu->mem->brk_point = addr;

        emu->as->r[0] = addr;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_LSEEK) // 19
    {
        LONG fd = emu->as->r[16];
        ULONG offset = emu->as->r[17];
        LONG whence = emu->as->r[18];
        LONG result;

        result = lseek(fd, offset, whence);

        if(result == ((LONG)-1))
        {
            emu->as->r[0] = errno;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            emu->as->r[0] = result;
            emu->as->r[19] = 0;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_GETXPID) // 20
    {
        emu->as->r[0] = getpid();
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_GETXUID) // 24
    {
        emu->as->r[20] = geteuid();

        emu->as->r[0] = getuid();
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_KILL) // 37
    {
        fprintf(emu->LOG, "syscall SYSCALL_KILL: kill pid %llu\n", emu->as->r[16]);

        emu->as->r[0] = 0;
        emu->as->r[19] = ((ULONG)-1);
    }

    /**********************************************/
    else if(syscode == SYSCALL_OPEN) // 45
    {
        CHAR  pathname[1024];
        ULONG pathname_ptr = emu->as->r[16];
        ULONG sim_flags = emu->as->r[17];
        ULONG mode = emu->as->r[18];
        INT   local_flags;
        INT   fd;
        
        // read pathname out of virtual memory
        emu->mem->load(-1, pathname_ptr, (ULONG *)pathname);

        // decode sim open flags to local open flags, oct
        local_flags = 0;
        if(sim_flags & 00000000)   local_flags |= O_RDONLY;
        if(sim_flags & 00000001)   local_flags |= O_WRONLY;
        if(sim_flags & 00000002)   local_flags |= O_RDWR;
        if(sim_flags & 00000004)   local_flags |= O_NONBLOCK;
        if(sim_flags & 00000010)   local_flags |= O_APPEND;
        if(sim_flags & 00001000)   local_flags |= O_CREAT;
        if(sim_flags & 00002000)   local_flags |= O_TRUNC;
        if(sim_flags & 00004000)   local_flags |= O_EXCL;
        if(sim_flags & 00010000)   local_flags |= O_NOCTTY;
        if(sim_flags & 00040000)   local_flags |= O_SYNC;
        if(sim_flags & 02000000)   local_flags |= O_DSYNC;
        if(sim_flags & 04000000)   local_flags |= O_RSYNC;

        // open the file
        fd = open(pathname, local_flags, mode);

        if (fd == -1)
        {
          emu->as->r[0] = errno;
          emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
          emu->as->r[0] = fd;
          emu->as->r[19] = 0;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_GETXGID) // 47
    {
        emu->as->r[20] = getegid();

        emu->as->r[0] = getgid();
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_OSF_SIGPROCMASK) // 48
    {
        emu->as->r[0] = 0;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_IOCTL) // 54
    {
        // this is normally a program trying to determine if stdout is a tty
        // if so, tell the program that it's not a tty, so the program does
        // block buffering
        LONG fd = emu->as->r[16];
        LONG request = emu->as->r[17];

        if(fd < 0)
        {
            emu->as->r[0] = EBADF;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            switch(request)
            {
                case 0x40067408: //ALPHA_IOCTL_TIOCGETP:
                case 0x80067409: //ALPHA_IOCTL_TIOCSETP:
                case 0x8006740a: //ALPHA_IOCTL_TIOCSETN:
                case 0x80067411: //ALPHA_IOCTL_TIOCSETC:
                case 0x40067412: //ALPHA_IOCTL_TIOCGETC:
                case 0x2000745e: //ALPHA_IOCTL_TIOCISATTY:
                case 0x402c7413: //ALPHA_IOCTL_TIOCGETS:
                case 0x40127417: //ALPHA_IOCTL_TIOCGETA:
                    emu->as->r[0] = ENOTTY;
                    emu->as->r[19] = ((ULONG)-1);
                    break;
                default:
                    fprintf(emu->LOG, "syscall SYSCALL_IOCTL: unsupported ioctl call: %llx on fd: %lld\n", request, fd);
                    emu->as->r[0] = 0;
                    emu->as->r[19] = ((ULONG)-1);
            }
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_MMAP) // 71
    {
        ULONG addr = emu->as->r[16];
        ULONG length = emu->as->r[17];
        LONG flags = emu->as->r[19];
        LONG fd = emu->as->r[20];

        // verify addr & length are both page aligned
        if(((addr%PAGE_SIZE)!=0) || ((length%PAGE_SIZE)!=0))
        {
            emu->as->r[0] = EINVAL;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            if(addr != 0)
                fprintf(emu->LOG, "syscall SYSCALL_MMAP: mmap ignorning suggested map address\n");

            addr = emu->mem->mmap_end;
            emu->mem->mmap_end += length;

            emu->as->r[0] = addr;
            emu->as->r[19] = 0;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_MUNMAP) // 73
    {
        emu->as->r[0] = 0;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_MPROTECT) // 74
    {
        emu->as->r[0] = 0;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_FCNTL) // 92
    {
        LONG fd = emu->as->r[16];
        LONG cmd = emu->as->r[17];

        if(fd < 0)
        {
            emu->as->r[0] = EBADF;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            switch(cmd)
            {
                case 0: // F_DUPFD
                    emu->as->r[0] = EMFILE;
                case 1: // F_GETFD (get close-on-exec flag)
                case 2: // F_SETFD (set close-on-exec flag)
                    emu->as->r[0] = 0;
                    emu->as->r[19] = 0;
                    break;

                case 3: // F_GETFL (get file flags)
                case 4: // F_SETFL (set file flags)
                    emu->as->r[0] = fcntl(fd, cmd);
                    if(emu->as->r[0] != -1)
                        emu->as->r[19] = 0;
                    else
                        emu->as->r[0] = errno;
                        emu->as->r[19] = ((ULONG)-1);
                    break;

                case 7: // F_GETLK (get lock)
                case 8: // F_SETLK (set lock)
                case 9: // F_SETLKW (set lock and wait)
                default:
                    // pretend that these all worked
                    fprintf(emu->LOG, "syscall SYSCALL_FCNTL: ignored fcntl command %lld on fd %lld\n", cmd, fd);

                    emu->as->r[0] = 0;
                    emu->as->r[19] = 0;
                    break;
            }
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_WRITEV) // 121
    {
        LONG fd = emu->as->r[16];
        ULONG iovptr = emu->as->r[17];
        LONG iovcnt = emu->as->r[18];
        struct iovec *iov;
        struct alpha_iovec a_iov;
        INT i, res;

        if(fd < 0)
        {
            emu->as->r[0] = EBADF;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            iov = (struct iovec *)malloc(sizeof(struct iovec)*iovcnt);
            // for each io vector entry
            for(i = 0; i < iovcnt; i++)
            {
                ULONG tempAddr = iovptr + i*sizeof(a_iov);
                emu->mem->load(sizeof(a_iov), tempAddr, (ULONG *)&a_iov);

                // copy into local iov
                iov[i].iov_len = a_iov.iov_len;
                // allocate local buffer
                iov[i].iov_base = malloc(iov[i].iov_len);

                // copy into local buffer
                emu->mem->load(a_iov.iov_len, a_iov.iov_base, (ULONG *)(iov[i].iov_base));
            }

            // perform writev
            res = writev(fd, iov, iovcnt);

            // cleanup
            for(i = 0; i < iovcnt; i++)
                free(iov[i].iov_base);
            free(iov);

            if(res < 0)
            {
                emu->as->r[0] = errno;
                emu->as->r[19] = ((ULONG)-1);
            }
            else
            {
                emu->as->r[0] = 0;
                emu->as->r[19] = 0;
            }
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_FTRUNCATE) // 130
    {
        LONG fd = emu->as->r[16];
        LONG length = emu->as->r[17];

        if(fd < 0)
        {
            emu->as->r[0] = EBADF;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            emu->as->r[0] = ftruncate(fd, length);
            if(emu->as->r[0] == -1)
            {
                emu->as->r[0] = errno;
                emu->as->r[19] = ((ULONG)-1);
            }
            else
            {
                emu->as->r[0] = 0;
                emu->as->r[19] = 0;
            }
        }
    }
 
    /**********************************************/
    else if(syscode == SYSCALL_GETRLIMIT) // 144
    {
        ULONG resource = emu->as->r[16];
        ULONG vaddr = emu->as->r[17];

        switch(resource)
        {
            case 3: //ALPHA_RLIMIT_STACK:
                emu->as->r[0] = 0;
                emu->as->r[19] = 0;
                break;

            case 2: //ALPHA_RLIMIT_DATA:
                emu->as->r[0] = 0;
                emu->as->r[19] = 0;
                break;

            default:
                fprintf(emu->LOG, "syscall SYSCALL_GETRLIMIT: unimplemented rlimit resource %llu ... failing ...\n", resource);
                emu->as->r[0] = 0;
                emu->as->r[19] = ((ULONG)-1);
                break;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_SETRLIMIT) // 145
    {
        emu->as->r[0] = 0;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_OSF_GETSYSINFO) // 256
    {
        ULONG op = emu->as->r[16];
        ULONG vaddr = emu->as->r[17];
        ULONG data = emu->as->fpcr;
        INT size = sizeof(ULONG);

        switch(op)
        {
            case 45:
                emu->mem->store(size, vaddr, &data);

                emu->as->r[0] = 0;
                emu->as->r[19] = 0;
                break;

            default:
                fprintf(emu->LOG, "syscall SYSCALL_OSF_GETSYSINFO: unsupported operation %llu on system call OSF_GETSYSINFO\n", op);
                emu->as->r[0] = 0;
                emu->as->r[19] = ((ULONG)-1);
                break;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_OSF_SETSYSINFO) // 257
    {
        ULONG op = emu->as->r[16];
        ULONG vaddr = emu->as->r[17];
        ULONG data = emu->as->fpcr;
        INT size = sizeof(ULONG);

        switch(op)
        {
            case 14:
                emu->mem->store(size, vaddr, &data);

                emu->as->r[0] = 0;
                emu->as->r[19] = 0;
                break;

            default:
                fprintf(emu->LOG, "syscall SYSCALL_OSF_SETSYSINFO: unsupported operation %llu on system call OSF_GETSYSINFO\n", op);
                emu->as->r[0] = 0;
                emu->as->r[19] = ((ULONG)-1);
                break;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_TIMES) // 323
    {
        emu->as->r[0] = 0;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_UNAME) // 339
    {
        struct alpha_utsname utbuf;
        ULONG vaddr = emu->as->r[16];
        INT size = sizeof(struct alpha_utsname);
        ULONG *buf = (ULONG *)(&utbuf);

        strcpy(utbuf.sysname,  "Linux");                        // modeling linux OS
        strcpy(utbuf.nodename, "mjdechen");                     // mark's laptop
        strcpy(utbuf.release,  "2.6.27.5-117.fc10");            // fedora 10, why not ...
        strcpy(utbuf.version,  "Mon Dec  8 21:18:29 PST 2008"); // why not
        strcpy(utbuf.machine,  "alpha");                        // modeling alpha isa
        strcpy(utbuf.domainname, "");                           // my laptop had none

        emu->mem->store(size, vaddr, buf);

        emu->as->r[0] = 0;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_MREMAP) // 341
    {
        ULONG addr = emu->as->r[16];
        ULONG old_length = emu->as->r[17];
        ULONG new_length = emu->as->r[18];
        ULONG flags = emu->as->r[19];

        if(new_length > old_length)
        {
            if((addr+old_length) == emu->mem->mmap_end)
            {
                ULONG diff = new_length - old_length;
                emu->mem->mmap_end += diff;

                emu->as->r[0] = addr;
                emu->as->r[19] = 0;
            }
            else
            {
                ULONG *buf;
                buf = (ULONG *)malloc(old_length);
                emu->mem->load(old_length, addr, buf);
                emu->mem->store(old_length, emu->mem->mmap_end, buf);
		
                emu->as->r[0] = emu->mem->mmap_end;
                emu->as->r[19] = 0;

                emu->mem->mmap_end += new_length;
                free(buf);
            }				
        }
        else
        {
            emu->as->r[0] = addr;
            emu->as->r[19] = 0;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_RT_SIGACTION) // 352
    {
        emu->as->r[0] = 0;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_GETTIMEOFDAY) // 359
    {
        // this is a little wonky.  if the benchmark really needs the time
        // to do something useful, then the simulated processor will appear
        // to be running slow (~1MHz).  keeping this in mind, if it becomes
        // necessary to have processors really know something useful about
        // time, the simulator should record time at the beginning of the sim
        // and then fudge all future syscalls to gettimeofday, to something
        // like (orig time + (num_insts / 6.4B))

        struct timeval tv;
        struct timezone tz;
        ULONG tvaddr = emu->as->r[16];
        ULONG tzaddr = emu->as->r[17];
        struct alpha_timeval atv;
        struct alpha_timezone atz;
        LONG result;

        if(tzaddr)
            result = gettimeofday(&tv, &tz);
        else
            result = gettimeofday(&tv, 0);

        if(result == 0)
        {
            atv.tv_sec = tv.tv_sec;
            atv.tv_usec = tv.tv_usec;
            emu->mem->store(sizeof(atv), tvaddr, (ULONG *)&atv);

            if(tzaddr)
            {
                atz.tz_minuteswest = tz.tz_minuteswest;
                atz.tz_dsttime = tz.tz_dsttime;
                emu->mem->store(sizeof(atz), tzaddr, (ULONG *)&atz);
            }

            emu->as->r[0] = 0;
            emu->as->r[19] = 0;
        }
        else
        {
            emu->as->r[0] = errno;
            emu->as->r[19] = ((ULONG)-1);
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_GETCWD) // 367
    {
        ULONG *buf;
        ULONG vaddr = emu->as->r[16];
        ULONG size = emu->as->r[17];
        CHAR *res;

        // allocate a buffer
        buf = (ULONG *)malloc(size);

        res = getcwd((CHAR *)buf, size);

        if(res < 0)
        {
            emu->as->r[0] = errno;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            emu->mem->store(-1, vaddr, buf);

            emu->as->r[0] = strlen(res);
            emu->as->r[19] = 0;
        }

        // clean up after ourselves
        free(buf);
    }

    /**********************************************/
    else if(syscode == SYSCALL_EXIT_GROUP) // 405
    {
        emu->running = 0;

        emu->as->r[0] = 0;
        emu->as->r[19] = 0;
    }

    /**********************************************/
    else if(syscode == SYSCALL_STAT64) // 425
    { 
        CHAR pathname[1024];
        ULONG pathname_ptr = emu->as->r[16];
        ULONG vaddr = emu->as->r[17];
        struct stat64 alpha_buf;
        INT result;

        // read path out of simulator memory
        emu->mem->load(-1, pathname_ptr, (ULONG *)pathname);

        // perform stat
        result = stat64(pathname, &alpha_buf);

        if(result < 0)
        {
            emu->as->r[0] = errno;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            emu->mem->store(sizeof(alpha_buf), vaddr, (ULONG *)&alpha_buf);

            emu->as->r[0] = 0;
            emu->as->r[19] = 0;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_LSTAT64) // 426
    {
        CHAR pathname[1024];
        ULONG pathname_ptr = emu->as->r[16];
        ULONG vaddr = emu->as->r[17];
        struct stat64 alpha_buf;
        INT result;

        // read path out of simulator memory
        emu->mem->load(-1, pathname_ptr, (ULONG *)pathname);

        // perform lstat
        result = lstat64(pathname, &alpha_buf);

        if(result < 0)
        {
            emu->as->r[0] = errno;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            emu->mem->store(sizeof(alpha_buf), vaddr, (ULONG *)&alpha_buf);

            emu->as->r[0] = 0;
            emu->as->r[19] = 0;
        }
    }

    /**********************************************/
    else if(syscode == SYSCALL_FSTAT64) // 427
    {
        struct stat64 alpha_buf;
        LONG fd = emu->as->r[16];
        ULONG vaddr = emu->as->r[17];
        INT size = sizeof(struct stat64);
        ULONG *buf = (ULONG *)(&alpha_buf);
        INT result;
        
        if(fd < 0)
        {
            emu->as->r[0] = 0;
            emu->as->r[19] = ((ULONG)-1);
        }
        else
        {
            result = fstat64(fd, &alpha_buf);

            if(result < 0)
            {
                emu->as->r[0] = errno;
                emu->as->r[19] = ((ULONG)-1);
            }
            else
            {
                emu->mem->store(size, vaddr, buf);

                emu->as->r[0] = 0;
                emu->as->r[19] = 0;
            }
        }
    }

    /**********************************************/
    else
    {
        fprintf(emu->LOG, "syscall not implemented!!: %llu\n", syscode);
        emu->as->r[0] = 0;
        emu->as->r[19] = ((ULONG)-1);
    }

    fflush(emu->LOG);
}
