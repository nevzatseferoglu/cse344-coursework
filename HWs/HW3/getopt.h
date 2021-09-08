#ifndef _GETOPT
#define _GETOPT

extern char* nameofsharedmemory;
extern char* filewithfifonames;
extern char* namedsemaphore;
extern int haspotatoornot;

extern int pfnd, mfnd, ffnd, sfnd;

void set_opts_and_args(int, char *[]);


#endif /* _GETOPT */