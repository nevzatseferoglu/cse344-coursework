#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "getopt.h"

int numOfNurses       = -1,   nFnd = 0;
int numOfVaccinators  = -1,   vFnd = 0;
int numOfCitizens     = -1,   cFnd = 0;
int sizeOftheBuffer   = -1,   bFnd = 0;
int numOfDose         = -1,   dFnd = 0;
char* inputFile = NULL;
int fFnd = 0;

void xargserror(char* argv[]);
int isNotaNumber(const char* str);

void set_opts_and_args(int argc, char *argv[]) {

    int opt;
    while ((opt = getopt(argc, argv, "n:v:c:b:t:i:")) != -1) {
        switch (opt) {
            case 'n':
                if (isNotaNumber(optarg))   xargserror(argv);
                else numOfNurses = atoi(optarg);
                nFnd = 1;
                break;
            
            case 'v':
                if (isNotaNumber(optarg))   xargserror(argv);
                else numOfVaccinators = atoi(optarg);
                vFnd = 1;
                break;

            case 'c':
                if (isNotaNumber(optarg))   xargserror(argv);
                else numOfCitizens = atoi(optarg);
                cFnd = 1;
                break;

            case 'b':
                if (isNotaNumber(optarg))   xargserror(argv);
                else sizeOftheBuffer = atoi(optarg);
                bFnd = 1;
                break;
            
            case 't':
                if (isNotaNumber(optarg))   xargserror(argv);
                else numOfDose = atoi(optarg);
                dFnd = 1;
                break;

            case 'i':
                inputFile = optarg;
                break;

            default:
                /* precise message given by getopt */
                break;
        }
    }

    if (!nFnd || !vFnd || !cFnd || !bFnd || !dFnd || (inputFile == NULL) || (optind < argc))
        xargserror(argv);

    if (numOfNurses < 2 || numOfVaccinators < 2 || numOfCitizens < 3 || numOfDose < 1
         || sizeOftheBuffer < (numOfDose*numOfCitizens + 1))
        xargserror(argv);
}

void xargserror(char* argv[]) {
    fprintf(stderr, "Usage:\n%s [-n n] [-v v] [-c c] [-b b] [-t t] [-i file]\n\n"
                "-n -> number of nurses         (integer >= 2)\n"
                "-v -> number of vaccinators    (integer >= 2)\n"
                "-c -> number of citizens       (integer >= 3)\n"
                "-b -> size of buffer           (integer >= (tc+1))\n"
                "-t -> dose number per citizen  (integer >= 1)\n"
                "-i -> path name of the input file\n", *argv);
    exit(EXIT_FAILURE);
}

int isNotaNumber(const char* str) {
    if (str == NULL)
        return -1;
        
    while(*str) {
        if(!(*str >= '0' && *str <= '9'))
            return -1;
        ++str;
    }
    return 0;
}
