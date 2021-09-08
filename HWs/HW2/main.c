
#include <stdio.h>
#include "lib.h"

sig_atomic_t vSIGUSR1 = 0;
sig_atomic_t vSIGUSR2 = 0;
sig_atomic_t vSIGINT = 0;

void handlerSIGUSE1(int signo) {
    vSIGUSR1 = signo + 1;
}

void handlerSIGUSE2(int signo) {
    vSIGUSR2 = signo + 1;
}

void handlerSIGINT(int signo) {
    vSIGINT = signo + 1;
}


int
main(int argc, char* argv[]) {

    FILE* fptr;
    char* filename;
    pid_t childs[POLY_NUMBER];
    pid_t dchild;
    sigset_t blockSig, pReceive, cReceive;

    struct sigaction saUSR1;
    struct sigaction saUSR2;
    struct sigaction saSIGINT;
    memset(&saUSR1, 0, sizeof(saUSR1));
    memset(&saUSR2, 0, sizeof(saUSR2));
    memset(&saSIGINT, 0, sizeof(saSIGINT));

    saUSR1.sa_handler = handlerSIGUSE1;
    sigaction(SIGUSR1, &saUSR1, NULL);

    saUSR2.sa_handler = handlerSIGUSE2;
    sigaction(SIGUSR2, &saUSR2, NULL);

    saSIGINT.sa_handler = handlerSIGINT;
    sigaction(SIGINT, &saSIGINT, NULL);

    
    if (!(filename = isValidArgs(argc, argv))) {
        fprintf(stderr, "invalid argument, usage: %s 'filename'\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (sigemptyset(&pReceive) || sigfillset(&pReceive) || sigdelset(&pReceive, SIGUSR2)) {
        perror("pReceive");
        exit(EXIT_FAILURE);
    }

    if (sigemptyset(&cReceive) || sigfillset(&cReceive) || sigdelset(&cReceive, SIGUSR1)) {
        perror("cReceive");
        exit(EXIT_FAILURE);
    }

    if (sigemptyset(&blockSig) || sigfillset(&blockSig) || sigdelset(&blockSig, SIGINT)) {
        perror("blockSig");
        exit(EXIT_FAILURE);
    }
    
    if(sigprocmask(SIG_BLOCK, &blockSig, NULL) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    
    fptr = fopen(filename, "r+");
    if (!fptr) {
        fprintf(stderr, "File cannot be opened");
        exit(EXIT_FAILURE);
    }
    
    for (int i=0; i<POLY_NUMBER; ++i) {
        childs[i] = fork();
        switch (childs[i])
        {
            case 0:
                if (childRound(fptr, i, 1) == -1) {
                    fprintf(stderr, "Error: childFirstRound\n");
                    if (fclose(fptr) == EOF)
                        perror("fclose");
                    exit(EXIT_FAILURE);
                }

                if(sigsuspend(&cReceive) && errno == EFAULT) {
                    perror("sigsuspend");
                    if (fclose(fptr) == EOF)
                        perror("fclose");
                    exit(EXIT_FAILURE);
                }

                if(kill(getppid(), SIGUSR2) == -1) {
                    perror("kill");
                    if (fclose(fptr) == EOF)
                        perror("fclose");
                    exit(EXIT_FAILURE);
                }

                if(sigsuspend(&cReceive) && errno == EFAULT) {
                    perror("sigsuspend");
                    if (fclose(fptr) == EOF)
                        perror("fclose");
                    exit(EXIT_FAILURE);
                }
                
                if (childRound(fptr, i, 2) == -1) {
                    fprintf(stderr, "Error: childSecondRound\n");
                    if (fclose(fptr) == EOF)
                        perror("fclose");
                    exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS);

            case -1:
                perror("fork");
                if (fclose(fptr) == EOF)
                    perror("fclose");
                exit(EXIT_FAILURE);

            default:
                break;
        }
    }

    if (vSIGINT == (SIGINT + 1)) {
        if (fclose(fptr) == EOF)
            perror("fclose");
        fprintf(stderr, "CTRL-C is pressed\n");
        exit(EXIT_FAILURE);
    }

    for (int i=0; i<POLY_NUMBER; ++i) {
        if(kill(childs[i], SIGUSR1) == -1) {
            perror("kill");
            if (fclose(fptr) == EOF)
                perror("fclose");
            exit(EXIT_FAILURE);
        }
        if(sigsuspend(&pReceive) && errno == EFAULT) {
            perror("sigsuspend");
            if (fclose(fptr) == EOF)
                perror("fclose");
            exit(EXIT_FAILURE);
        }
    }

    calcErrPrint(fptr, 1);

    if (vSIGINT == (SIGINT + 1)) {
        if (fclose(fptr) == EOF)
            perror("fclose");
        fprintf(stderr, "CTRL-C is pressed\n");
        exit(EXIT_FAILURE);
    }

    for (int i=0; i<POLY_NUMBER; ++i) {
        if(kill(childs[i], SIGUSR1) == -1) {
            perror("kill");
            if (fclose(fptr) == EOF)
                perror("fclose");
            exit(EXIT_FAILURE);
        }
    }

	while((dchild = wait(NULL)) != -1);
    if (errno != ECHILD) {
        if (fclose(fptr) == EOF)
            perror("fclose");
        perror("wait");
    }

    if (vSIGINT == (SIGINT + 1)) {
        if (fclose(fptr) == EOF)
            perror("fclose");
        fprintf(stderr, "CTRL-C is pressed\n");
        exit(EXIT_FAILURE);
    }

    calcErrPrint(fptr, 2);

    if (fclose(fptr) == EOF) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}