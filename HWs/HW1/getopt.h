#ifndef _GETOPT
#define _GETOPT

extern char* fname, *fprmns;
extern char* spath, *ftype;

extern int fsize, fnlinks;

extern int wfnd, ffnd, bfnd;
extern int tfnd, pfnd, lfnd;

int set_opts_and_args(int, char *[]);

#endif /* _GETOPT */