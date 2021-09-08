#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "getopt.h"

char* fname = NULL, *fprmns = NULL;
char* spath = NULL, *ftype = NULL;

int fsize = 0, fnlinks = 0;

int wfnd = 0, ffnd = 0, bfnd = 0;
int tfnd = 0, pfnd = 0, lfnd = 0;

int is_valid_filename(char*);
int is_valid_filesize(int);
int is_valid_filetype(char*);
int is_valid_noflinks(int);
int does_contain_char(const char*);


int set_opts_and_args(int argc, char *argv[]) {

    int opt;
    
    while ((opt = getopt(argc, argv, "f:b:t:p:l:w:")) != -1) {
        switch (opt) {
            case 'f':
                ffnd = 1;
                fname = optarg;
                break;

            case 'b':
                bfnd = 1;
                fsize = atoi(optarg);
                if(does_contain_char(optarg)) {
                    fprintf(stderr, "ERROR: invalid argument for -b option\n");
                    return  -1;
                }
                break;
            
            case 't':
                tfnd = 1;
                ftype = optarg;
                break;

            case 'p':
                pfnd = 1;
                fprmns = optarg;
                break;

            case 'l':
                lfnd = 1;
                fnlinks = atoi(optarg);
                break;

            case 'w':
                wfnd = 1;
                spath = optarg;
                break;

            default:
                /* precise message given by getopt */
                return -1;
                break;
        }
    }

    if (wfnd != 1) {
        fprintf(stderr, "ERROR: -w is a mandatory option, usage: -w searchpath\n");
        return -1;
    }

    if (!(ffnd || bfnd || tfnd || pfnd  || lfnd)) {
        fprintf(stderr, "ERROR: at least one search criteria is mandatory, usage: -f, -b, -t, -p, -l\n");
        return -1;
    }

    if (ffnd && (is_valid_filename(fname) == -1)) {
        fprintf(stderr, "ERROR: filename regex cannot start with \'+\'\n");
        return -1;
    }

    if (bfnd && (is_valid_filesize(fsize) == -1)) {
        fprintf(stderr, "ERROR: Invalid file size as an argument, usage: -b positive_integer\n");
        return -1;
    }

    if (tfnd && is_valid_filetype(ftype) == -1) {
        fprintf(stderr, "ERROR: Invalid file type as an argument, usage: -t (d,s,b,c,f,l)\n");
        return -1;
    }

    if (lfnd && is_valid_noflinks(fnlinks) == -1) {
        fprintf(stderr, "ERROR: Invalid number of links as an argument, usage: -l positive_integer\n");
        return -1;
    }

    if (optind < argc) {
        fprintf(stderr, "ERROR: Unexpected argument after options\n");
        return -1;
    }

    return 0;
}

int does_contain_char(const char* str) {
    if (str == NULL)
        return 0;

    while(!fsize && *str) {
        if(!(*str >= '0' && *str <= '9'))
            return -1;
        ++str;
    }
    return 0;
}

int is_valid_filename(char* fn) {

    if(fn != NULL && fn[0] == '+')
        return -1;
    return 1;
}

int is_valid_filesize(int size) {
    if(size < 0)
        return -1;
    return 0;
}

int is_valid_filetype(char* type) {
    
    char c;

    if (strlen(type) != 1)
        return -1;
    
    switch (c = *type) {
        case 'd':
        case 's':
        case 'b':
        case 'c':
        case 'f':
        case 'p':
        case 'l':
            return 1;

        default:
            return -1;
    }
}

int is_valid_noflinks(int nlink) {
    if (nlink < 1)
        return -1;
    return 1;
}
