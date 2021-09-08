#ifndef _LIB
#define _LIB

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>

typedef struct Buffer {
    sig_atomic_t sigint;
    int totalAppliedDose;
    int deadCitizen;
    long currCitizen;
    int nurseTermination;
    long fNurse, fVaccinator, fCitizen;
    int firstDose;
    int secDose;
    int fd;
    sem_t empty;
    sem_t full;
    sem_t m;
    sem_t mConsumer;
    sem_t mCitizen;
    sem_t mSafe;
} Buffer;

typedef struct File {
    int fd;
    int isEof;
    int c;
} File;

#define TRUE 1
#define FALSE 0

/* Signals */
void x_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

/* file operations */
ssize_t x_read(int fd, void *buf, size_t count);
void x_close(int fd);
int x_open(const char *pathname, int flags, mode_t mode);
void x_fclose(FILE *fptr);
FILE* x_fopen(char *filename);
void x_fseek(FILE* fptr, long offset, int whence);

pid_t x_fork(void);

/* semaphore */
sem_t* x_sem_open(const char *name, unsigned int value);
void x_sem_close(sem_t *sem);
void x_sem_unlink(const char *name);
void x_sem_wait(sem_t *sem);
void x_sem_post(sem_t *sem);
int x_sem_getvalue(sem_t *sem);
void x_sem_init(sem_t *sem, int pshared, unsigned int value);
void x_sem_destroy(sem_t *sem);

/* shared memory */
int x_shm_open(const char *name);
void x_shm_unlink(const char* name);
void x_ftruncate(int fd, off_t length);
void x_fstat(int fd, struct stat *buf);
void *x_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
void x_shm_close(int fd);
void x_munmap(void *addr, size_t lenght);

/* common */
void* x_malloc(size_t size);
void x_error(const char*, char*);


#endif /* _LIB */