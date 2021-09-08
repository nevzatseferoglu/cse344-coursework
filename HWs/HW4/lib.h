#ifndef _LIB
#define _LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

/* pthread */
void x_pthread_kill(pthread_t thread, int sig);
void x_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine) (void*), void *arg);
void x_pthread_exit(void *retval);
void x_pthread_setdetachstate(pthread_attr_t *attr, int detachstate);
void x_pthread_attr_init(pthread_attr_t *attr);
void x_pthread_attr_destroy(pthread_attr_t *attr);
void x_pthread_join(pthread_t thread, void **retval);
void x_error_en(int err, const char* func, char *msg);

/* signals */
void x_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

/* fork */
pid_t x_fork(void);

/* FILE IO with system call */
int x_open(const char *pathname, int flags, mode_t mode);
void x_close(int fd);
ssize_t x_read(int fd, void *buf, size_t count);

/* FILE IO without system call */
void x_fclose(FILE *fptr);
FILE* x_fopen(char *filename);
void x_fseek(FILE* fptr, long offset, int whence);

/* named semaphore */
void x_sem_unlink(const char *name);

/* unnamed semaphore */
void x_sem_init(sem_t *sem, int pshared, unsigned int value);
void x_sem_destroy(sem_t *sem);

/* common semaphore functionality*/
void x_sem_wait(sem_t *sem);
void x_sem_post(sem_t *sem);
int x_sem_getvalue(sem_t *sem);
sem_t* x_sem_open(const char *name, unsigned int value);
void x_sem_close(sem_t *sem);

/* shared memory */
int x_shm_open(const char *name);
void x_shm_close(int fd);
void x_ftruncate(int fd, off_t length);
void x_fstat(int fd, struct stat *buf);
void *x_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
void x_shm_unlink(const char* name);
void x_munmap(void *addr, size_t lenght);

/* common functionality */
void* x_malloc(size_t size);
void x_error(const char *func, char *msg);

#endif /* _LIB */