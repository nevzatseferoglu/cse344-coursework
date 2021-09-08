#ifndef _GETOPT
#define _GETOPT

extern int numOfNurses;
extern int numOfVaccinators;
extern int numOfCitizens;
extern int sizeOftheBuffer;
extern int numOfDose;
extern char* inputFile;

extern int nFnd, vFnd, cFnd, bFnd, dFnd, fFnd;
void set_opts_and_args(int, char *[]);

#endif /* _GETOPT */