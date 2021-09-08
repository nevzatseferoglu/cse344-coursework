#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "getopt.h"

char* nameofsharedmemory = NULL;
char* filewithfifonames = NULL;
char* namedsemaphore = NULL;
int haspotatoornot = -1;

int pfnd = 0;
int mfnd = 0; 
int ffnd = 0;
int sfnd = 0;

void xargserror(char* argv[]);
int is_not_a_number(const char* str);

void set_opts_and_args(int argc, char *argv[]) {

    int opt;
    
    while ((opt = getopt(argc, argv, "b:s:f:m:")) != -1) {
        switch (opt) {
            case 'b':
                pfnd = 1;
                haspotatoornot = atoi(optarg);
                if (is_not_a_number(optarg))
                    xargserror(argv);
                break;

            case 's':
                mfnd = 1;
                nameofsharedmemory = optarg;
                break;

            case 'f':
                ffnd = 1;
                filewithfifonames = optarg;
                break;

            case 'm':
                sfnd = 1;
                namedsemaphore = optarg;
                break;

            default:
                /* precise message given by getopt */
                break;
        }
    }

    if (!pfnd || !mfnd || !ffnd || !sfnd || (optind < argc)) {
        xargserror(argv);
    }
}

void xargserror(char* argv[]) {
    fprintf(stderr, "Usage: %s "
                "[-b haspotatoornot] "
                "[-s nameofsharedmemory] "
                "[-f filewithfifonames] "
                "[-m namedsemaphore]\n\n"
                "-> haspotatoornot     : Initial potatoes number\n"
                "-> nameofsharedmemory : Name of shared memory\n"
                "-> filewithfifonames  : Name of file that includes fifos\n"
                "-> namedsemaphore     : Name of semaphore\n", *argv);
    exit(EXIT_FAILURE);
}

int is_not_a_number(const char* str) {
    if (str == NULL)
        return -1;

    while(!haspotatoornot && *str) {
        if(!(*str >= '0' && *str <= '9'))
            return -1;
        ++str;
    }
    return 0;
}
