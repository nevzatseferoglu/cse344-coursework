#ifndef _ARG
#define _ARG

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#define VALID_LOWER_PORT_LIMIT 1000
#define VALID_UPPER_PORT_LIMIT 65535

/* client arguments */
extern int id;
extern char* IPv4;
extern unsigned int PORT;
extern char* pathToQueryFile;

/* sets the argument variable */
void set_opts_and_args(int, char *[]);

#endif /* _ARG */