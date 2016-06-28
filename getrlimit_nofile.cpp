//
//  getrlimit_nofile
//
//  Created by Frédéric Devernay on 28/06/2016.
//  Copyright © 2016 Frédéric Devernay. All rights reserved.
//
// Compiling from the command-line:
// g++ getrlimit_nofile.cpp -o getrlimit_nofile
// g++ -DDOSTDIO getrlimit_nofile.cpp -o getrlimit_nofile_dostdio

// exhibit a bug in OSX: the RLIMIT_NOFILE (max number of open files) cannot be set for fopen after any
// stdio operation (printf, fopen...) was done.
// When executing from Xcode, the initial limit is much higher than when executing from the command-line.

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>     // for getrlimit on linux
#include <sys/resource.h> // for getrlimit
#if defined(__APPLE__)
#include <sys/syslimits.h> // OPEN_MAX
#endif

#define MAXFILES 20000

//#define DOPRINT
#ifdef DOSTDIO
#define PRINT(x) x
#else
#define PRINT(x)
#endif

void setLimit();
void getLimit();

void test_fopen()
{
    char filename[1024];

    FILE* f[MAXFILES];
    unsigned long long i;
    for (i= 0; i < MAXFILES; ++i) {
        snprintf(filename, sizeof(filename), "/tmp/getrlimit_nofile%llu", i);
        f[i] = fopen(filename, "w");
        if (!f[i]) {
            break;
        }
    }
    unsigned long imax = i;
    printf("max number of open files (fopen): %lu\n", imax);
    for (i = 0; i < imax; ++i) {
        fclose(f[i]);
    }

}

void test_open()
{
    char filename[1024];

    int f[MAXFILES];
    unsigned long long i;
    for (i= 0; i < MAXFILES; ++i) {
        snprintf(filename, sizeof(filename), "/tmp/getrlimit_nofile%llu", i);
        f[i] = open(filename, O_WRONLY);
        if (f[i] < 0) {
            break;
        }
    }
    unsigned long imax = i;
    printf("max number of open files (open): %lu\n", imax);
    for (i = 0; i < imax; ++i) {
        close(f[i]);
    }

}

int main( int argc, char* argv[] )
{
#ifdef DOSTDIO
    test_fopen();
    test_open(); 
#endif
    setLimit();
    getLimit();
    test_fopen();
    test_open();
    setLimit();
    getLimit();
    test_fopen();
    test_open();
    return 1;
}

void setLimit( )
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
        if (rl.rlim_max > rl.rlim_cur) {
            rl.rlim_cur = rl.rlim_max;
            int ret = setrlimit(RLIMIT_NOFILE, &rl);
            PRINT(printf( "Setting limit to %llu, %llu\n", (unsigned long long)rl.rlim_cur, (unsigned long long)rl.rlim_max ));
            if (ret != 0) {
                PRINT(printf("failed!\n"));
#             if defined(__APPLE__) && defined(OPEN_MAX)
                // On Mac OS X, setrlimit(RLIMIT_NOFILE, &rl) fails to set
                // rlim_cur above OPEN_MAX even if rlim_max > OPEN_MAX.
                if (rl.rlim_cur > OPEN_MAX) {
                    rl.rlim_cur = OPEN_MAX;
                    PRINT(printf( "Setting limit to %llu, %llu\n", (unsigned long long)rl.rlim_cur, (unsigned long long)rl.rlim_max ));
                    if (setrlimit(RLIMIT_NOFILE, &rl) != 0) {
                        printf("failed!\n");
                    }
                }
#             endif
            }
            printf( "Set limit to %llu, %llu\n", (unsigned long long)rl.rlim_cur, (unsigned long long)rl.rlim_max );
        }
    }

    if (setrlimit(RLIMIT_NOFILE, &rl) != 0) {
        printf("setrlimit() failed with errno=%d\n", errno);
        exit(1);
    }
}

void getLimit()
{
    struct rlimit rl;
    /* Get max number of files. */
    if (getrlimit(RLIMIT_NOFILE, &rl) != 0)
    {
        printf("getrlimit() failed with errno=%d\n", errno);
        exit(1);
    }
    printf("The soft limit is %llu\n", (unsigned long long)rl.rlim_cur);
    printf("The hard limit is %llu\n", (unsigned long long)rl.rlim_max);
}
