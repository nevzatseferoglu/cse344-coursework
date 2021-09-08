#ifndef _LIB
#define _LIB

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/file.h>
#include <string.h>
#include <math.h>

#define POLY_NUMBER 8
#define CHILD_PAIR_NUMBER 8
#define MAXLINE 256

char* isValidArgs (int argc, char* argv[]);
int setRowPairs(FILE* fptr, int rowId, char line[]);
int childRound(FILE* fptr, int cindex, int round);
double Li(int i, int n, double x[], double X);
double Pn(int n, double x[], double y[], double X);
void calcErrPrint(FILE* fptr, int round);
void setCoefs(double coefs[], double x[], double y[]);

#endif