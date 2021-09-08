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
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct sockaddr SA;

/* server-client */
void x_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen, FILE *stream);
void x_inet_pton(int af, const char *src, void *dst, FILE *stream);
void x_inet_aton(const char *cp, struct in_addr *inp, FILE *stream);
int x_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, FILE *stream);
int x_socket(int domain, int type, int protocol, FILE *stream);
void x_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen, FILE *stream);
void x_listen(int sockfd, int backlog, FILE *stream);
void x_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen, FILE *stream);


/*
void x_pthread_cond_setpshared(pthread_condattr_t *attr, int pshared, FILE *stream);
void x_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr, FILE *stream);
void x_pthread_cond_destroy(pthread_cond_t *cond, FILE *stream);
void x_pthread_condattr_init(pthread_cond_t *attr, FILE *stream);
void x_pthread_condattr_destroy(pthread_cond_t *attr, FILE *stream);
*/

/* pthread */
void x_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex, FILE *stream);
void x_pthread_cond_signal(pthread_cond_t *cond, FILE *stream);
void x_pthread_cond_broadcast(pthread_cond_t *cond, FILE *stream);

void x_pthread_mutex_lock(pthread_mutex_t *mutex, FILE *stream);
void x_pthread_mutex_unlock(pthread_mutex_t *mutex, FILE *stream);
void x_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr, FILE *stream);
void x_pthread_mutex_destroy(pthread_mutex_t *mutex, FILE *stream);
void x_pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type, FILE *stream);
void x_pthread_mutexattr_init(pthread_mutexattr_t *attr, FILE *stream);
void x_pthread_mutexattr_destroy(pthread_mutexattr_t *attr, FILE *stream);

void x_pthread_kill(pthread_t thread, int sig, FILE *stream);
void x_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine) (void*), void *arg, FILE *stream);
void x_pthread_exit(void *retval);
void x_pthread_setdetachstate(pthread_attr_t *attr, int detachstate, FILE *stream);
void x_pthread_attr_init(pthread_attr_t *attr, FILE *stream);
void x_pthread_attr_destroy(pthread_attr_t *attr, FILE *stream);
void x_pthread_join(pthread_t thread, void **retval, FILE *stream);
void x_error_en(FILE *stream, int err, const char* func, char *msg);

/* signals */
void x_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact, FILE *stream);

/* fork */
pid_t x_fork(FILE *stream);

/* FILE IO with system call */
int x_open(const char *pathname, int flags, mode_t mode, FILE *stream);
void x_close(int fd, FILE *stream);
off_t x_lseek(int fd, off_t offset, int whence, FILE *stream);
ssize_t x_read(int fd, void *buf, size_t count, FILE *stream);

/* FILE IO without system call */
void x_fclose(FILE *fptr, FILE *stream);
FILE* x_fopen(char *filename, FILE *stream);
void x_fseek(FILE* fptr, long offset, int whence, FILE *stream);

/* named semaphore */
void x_sem_unlink(const char *name, FILE *stream);

/* unnamed semaphore */
void x_sem_init(sem_t *sem, int pshared, unsigned int value, FILE *stream);
void x_sem_destroy(sem_t *sem, FILE *stream);

/* common semaphore functionality*/
void x_sem_wait(sem_t *sem, FILE *stream);
void x_sem_post(sem_t *sem, FILE *stream);
int x_sem_getvalue(sem_t *sem, FILE *stream);
sem_t* x_sem_open(const char *name, unsigned int value, FILE *stream);
void x_sem_close(sem_t *sem, FILE *stream);

/* shared memory */
int x_shm_open(const char *name, FILE *stream);
void x_shm_close(int fd, FILE *stream);
void x_ftruncate(int fd, off_t length, FILE *stream);
void x_fstat(int fd, struct stat *buf, FILE *stream);
void *x_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset, FILE *stream);
void x_shm_unlink(const char* name, FILE *stream);
void x_munmap(void *addr, size_t lenght, FILE *stream);

/* common functionality */
void* x_malloc(size_t size, FILE *stream);
void x_error(FILE *stream, const char *func, char *msg);

#endif /* _LIB */