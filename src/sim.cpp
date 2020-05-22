#include <sys/time.h>
#include "Define.h"
#include "Core.h"

INT main(INT argc, CHAR **argv, CHAR **envp)
{
    struct  timeval  start;
    struct  timeval  end;
    ULONG timer;

    gettimeofday(&start, NULL);

#if 1
    if(argc==1)
        printf("please specify executable name\n");
    else
    {
        Emulator *c = new Emulator(argc, argv, envp);

        c->loop();

        delete c;
    }
#else
    if(argc==1)
        printf("please specify executable name\n");
    else
    {
        Core *core = new Core(argc, argv, envp);

        while(core->run_a_cycle())
        {
        }

        delete core;
    }
#endif

    gettimeofday(&end, NULL);
    timer = end.tv_sec-start.tv_sec;
    printf("Total time: %llu seconds\n", timer);

    return 0;
}
