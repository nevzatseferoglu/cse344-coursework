#ifndef _LIB
#define _LIB

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

void x_error(const char*, char*);


/* semaphore */
sem_t* x_sem_open(const char *name, unsigned int value);
void x_sem_close(sem_t *sem);
void x_sem_unlink(const char *name);
void x_sem_wait(sem_t *sem);
void x_sem_post(sem_t *sem);
int x_sem_getvalue(sem_t *sem);

/* shared memory */
int x_shm_open(const char *name);
void x_shm_unlink(const char* name);
void x_ftruncate(int fd, off_t length);
void x_fstat(int fd, struct stat *buf);
void *x_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
void x_shm_close(int fd);
void x_munmap(void *addr, size_t lenght);
#endif /* _LIB */