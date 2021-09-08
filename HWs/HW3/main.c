
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include "lib.h"
#include "getopt.h"

#define MAXNAME 255

typedef struct Process {
    pid_t id;
    unsigned int coolDown;
    char fifoname[MAXNAME];
    int fd;
} Process;

Process* init_shared_mem(int procNum, int* fd);
void get_certain_fifo(char* filepath, int counter, char line[]);
int get_fifos_number(char* filepath);
void releaseMem(Process* proc, int procNum, int shrMemFd);
void openFifos(Process* proc, int procNum);
int getProcessIndex(Process* proc, int procNum);
int gpiWithPid(Process* proc, int procNum, pid_t rec);
void startWithPotato(Process* proc, int procNum);
void noPotatoStart(Process* proc, int procNum);
int checkPotatoFinish(Process* proc, int procNum);

sig_atomic_t vSIGINT = 0;

void handlerSIGINT(int signo) {
    vSIGINT = signo + 1;
}

int
main(int argc, char *argv[]) {

    int procNum, fd, index;
    Process* proc;
    struct sigaction saSIGINT;

    memset(&saSIGINT, 0, sizeof(saSIGINT));
    saSIGINT.sa_handler = handlerSIGINT;
    sigaction(SIGUSR1, &saSIGINT, NULL);

    srand(getpid());

    set_opts_and_args(argc, argv);

    if (vSIGINT == (SIGINT + 1)) {
        fprintf(stdout, "Exit, pressed\n");
        exit(EXIT_FAILURE);
    }

    procNum = get_fifos_number(filewithfifonames);
    proc = init_shared_mem(procNum, &fd);

    openFifos(proc, procNum);

    if (haspotatoornot == 0) {
        for (int i=0; i<procNum; ++i) {
            fprintf(stdout, "\nPid: %ld\n", (long)proc[i].id);
            fprintf(stdout, "coolDown: %d\n", proc[i].coolDown);
            fprintf(stdout, "FifoName: %s\n", proc[i].fifoname);
            fprintf(stdout, "FD: %d\n", proc[i].fd);
            fprintf(stdout, "\n");
            fflush(stdout);
        }
        fflush(stdout);
    }

    if (vSIGINT == (SIGINT + 1)) {
        fprintf(stdout, "Exit, pressed\n");
        exit(EXIT_FAILURE);
    }

    index = getProcessIndex(proc, procNum);
    if (proc[index].coolDown > 0)
        startWithPotato(proc, procNum);
    else
        noPotatoStart(proc, procNum);

    if (vSIGINT == (SIGINT + 1)) {
        fprintf(stdout, "Exit, pressed\n");
        exit(EXIT_FAILURE);
    }

    releaseMem(proc ,procNum, fd);
    exit(EXIT_SUCCESS);
}

void noPotatoStart(Process* proc, int procNum) {
    
    int index, stat, pIndex;
    pid_t rec;
    sem_t *mySem;

    index = getProcessIndex(proc, procNum);
    mySem = x_sem_open("noPotato", (unsigned int)1);
    
    while (!checkPotatoFinish(proc, procNum)) {
        x_sem_wait(mySem);
        if ((stat = read(proc[index].fd, &rec, sizeof(pid_t))) == sizeof(pid_t)) {
            fprintf(stdout, "pid=%ld receiving potato number %ld to aliveFifo\n"
                ,(long)getpid(), (long)rec);

            pIndex = gpiWithPid(proc, procNum, rec);
            proc[pIndex].coolDown -= 1;
            if (proc[pIndex].coolDown == 0)
                fprintf(stdout, "pid=%ld; potato number %ld has cooled dow\n"
                ,(long)getpid(), (long)rec);
            
        } else if (stat == -1) {
            x_error(__func__, "Potato cannot be received");
        }
        x_sem_post(mySem);
    }
}

void startWithPotato(Process* proc, int procNum) {
    int rFifo, index, stat, pIndex;
    pid_t rec;
    sem_t *mySem;

    index = getProcessIndex(proc, procNum);
    mySem = x_sem_open("mySemWithPotato", (unsigned int)1);

    while ((rFifo = rand()%10) == index);

    if (write(rFifo, &(proc[index].id), sizeof(pid_t)) == -1) {
        x_error(__func__, "Potato cannot be send properly");
    } else {
        fprintf(stdout, "pid=%ld sending potato number %ld to aliveFifo; this is switch number 1\n"
            ,(long)getpid(), (long)getpid());
    }

    while (!checkPotatoFinish(proc, procNum)) {
        x_sem_wait(mySem);
        if ((stat = read(proc[index].fd, &rec, sizeof(pid_t))) == sizeof(pid_t)) {
            fprintf(stdout, "pid=%ld receiving potato number %ld to aliveFifo\n"
                ,(long)getpid(), (long)rec);

            pIndex = gpiWithPid(proc, procNum, rec);
            proc[pIndex].coolDown -= 1;
            if (proc[pIndex].coolDown == 0)
                fprintf(stdout, "pid=%ld; potato number %ld has cooled dow\n"
                ,(long)getpid(), (long)rec);
            
        } else if (stat == -1) {
            x_error(__func__, "Potato cannot be received");
        }
        x_sem_post(mySem);
    }
}

int checkPotatoFinish(Process* proc, int procNum) {
    for (int i=0; i<procNum; ++i)
        if (proc[i].coolDown != 0)
            return 0;
    return 1;
}

int gpiWithPid(Process* proc, int procNum, pid_t rec) {
    int i;
    for (i=0; i<procNum; ++i)
        if (proc[i].id == rec)
            break;
    return i;
}

int getProcessIndex(Process* proc, int procNum) {
    int i;
    for (i=0; i<procNum; ++i)
        if (proc[i].id == getpid())
            break;
    return i;
}

void openFifos(Process* proc, int procNum) {
    int index = getProcessIndex(proc, procNum);
    for (int i=0; i<procNum; ++i) {
        if (i%2 == 0) {
            if (proc[i].id !=  proc[index].id) {
                if ((proc[i].fd = open(proc[i].fifoname, O_WRONLY)) == -1)
                    x_error(__func__, "Fifo cannot be opened for writing");
            } else {
                if ((proc[i].fd = open(proc[i].fifoname, O_RDONLY)) == -1)
                    x_error(__func__, "Fifo cannot be opened for reading");
            }
        } else {
            if (proc[i].id !=  proc[index].id) {
                if ((proc[i].fd = open(proc[i].fifoname, O_RDONLY)) == -1)
                    x_error(__func__, "Fifo cannot be opened for reading");
            } else {
                if ((proc[i].fd = open(proc[i].fifoname, O_WRONLY)) == -1)
                    x_error(__func__, "Fifo cannot be opened for writing");
            }
        }
    }
}

Process* init_shared_mem(int procNum, int* fd) {

    int counter;
    sem_t* mySem, *memFilled;
    size_t len;
    struct stat stat;
    char buff[MAXNAME];
    Process* proc = NULL;

    mySem = x_sem_open(namedsemaphore, (unsigned int)1);
    memFilled = x_sem_open("memFilled", (unsigned int)0);

    x_sem_wait(mySem); // take main semaphore

    *fd = x_shm_open(nameofsharedmemory);

    x_fstat(*fd, &stat);
    len = sizeof(Process) + (unsigned long)stat.st_size;
    x_ftruncate(*fd, len);

    counter = (unsigned int)(stat.st_size / sizeof(Process));

    proc = (Process*)x_mmap(NULL, len,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, *fd, 0);
    fflush(stdin);

    proc[counter].id = getpid();
    proc[counter].coolDown = haspotatoornot;
    get_certain_fifo(filewithfifonames, counter, buff);
    strcpy(proc[counter].fifoname, buff);

    if(mkfifo(proc[counter].fifoname, 0666)) {
        x_sem_post(mySem);
        x_sem_close(mySem);
        x_sem_unlink(namedsemaphore);
        x_sem_close(memFilled);
        x_sem_unlink("memFilled");
        x_error(__func__, "Fifo cannot be created");
    }

    if (counter == procNum - 1)
        x_sem_post(memFilled);

    x_sem_post(mySem);   // release main semaphore

    x_sem_wait(memFilled);
    x_sem_post(memFilled);

    x_sem_close(mySem);
    x_sem_unlink(namedsemaphore);

    x_sem_close(memFilled);
    x_sem_unlink("memFilled");
    return proc;
}

void releaseMem(Process* proc, int procNum, int shrMemFd) {
    int counter, index;
    int fd, len;
    char* ret;
    struct stat stat;

    sem_t *memRelease = x_sem_open("memReleaseC", (unsigned int)1);
    sem_t *freeMem = x_sem_open("freeMemC", (unsigned int)0);

    x_sem_wait(memRelease);
    fd = x_shm_open("/counter3");
    x_fstat(fd, &stat);
    len = sizeof(char) + (unsigned long)stat.st_size;
    x_ftruncate(fd, len);

    counter = (unsigned int)(stat.st_size / sizeof(char));

    ret = (char*)x_mmap(NULL, len,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, 0);
    if (ret == NULL)
        x_error(__func__, "Release cannot be performed properly");

    fflush(stdin);

    if (counter == procNum - 1)
        x_sem_post(freeMem);
    x_sem_post(memRelease);

    x_sem_wait(freeMem);
    x_sem_post(freeMem);

    x_sem_close(memRelease);
    x_sem_unlink("memReleaseC");
    x_sem_close(freeMem);
    x_sem_unlink("freeMemC");

    index = getProcessIndex(proc, procNum);
    if (unlink((proc[index].fifoname)))
        x_error(__func__, "Fifos cannot be unlinked");

    x_munmap(ret, (procNum*sizeof(char)));
    x_shm_close(fd);
    x_shm_unlink("/counter3");
    
    x_munmap(proc, (procNum*sizeof(Process)));
    x_shm_close(shrMemFd);
    x_shm_unlink(nameofsharedmemory);
}

int get_fifos_number(char* filepath) {
    int count;
    char c, prev;
    FILE* fptr;

    if (!(fptr = fopen(filepath, "r")))
        x_error(__func__, "Fifo file cannot opened");

    count = 0;
    c = fgetc(fptr);
    while (c != EOF) {
        if (c == '\n') {
            if (prev == c)
                --count;
            count += 1;
        }

        prev = c;
        c = fgetc(fptr);
        if (prev != '\n' && c == EOF)
            ++count;
    }

    if(fclose(fptr))
        x_error(__func__, "Fifo file cannot be closed properly");

    fflush(fptr);
    return count;
}

void get_certain_fifo(char* filepath, int counter, char line[]) {

    int count = 0;
    FILE* fptr;

    strcpy(line, "\0");

    if (!(fptr = fopen(filepath, "r")))
        x_error(__func__, "Fifo file cannot opened");

    while (fgets(line, MAXNAME, fptr) != NULL) {
        if (count == counter) {
            if (line[strlen(line)-1] == '\n')
                line[strlen(line)-1] = '\0';
            break;
        }
        else
            ++count;
    }

    if(fclose(fptr))
        x_error(__func__, "Fifo file cannot be closed properly");

    fflush(fptr);
}