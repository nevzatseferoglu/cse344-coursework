#ifndef _ARG
#define _ARG

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* arguments */
extern char* homeworkFilePath;
extern char* studentsFilePath;
extern int money;

/* sets the argument variable */
void set_opts_and_args(int, char *[]);

#endif /* _ARG */