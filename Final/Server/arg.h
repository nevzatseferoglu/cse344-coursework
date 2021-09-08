#ifndef _ARG
#define _ARG

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#define VALID_LOWER_PORT_LIMIT 1000
#define VALID_UPPER_PORT_LIMIT 65535

/* server arguments */
extern unsigned int PORT;
extern char* pathToLogFile;
extern unsigned int poolSize;
extern char* datasetPath;

/* sets the argument variable */
void set_opts_and_args(int, char *[]);

#endif /* _ARG */