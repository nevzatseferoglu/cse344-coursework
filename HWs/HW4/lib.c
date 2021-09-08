#include "lib.h"

/* pthread */
void x_pthread_kill(pthread_t thread, int sig) {
    int err;
    if ((err = pthread_kill(thread, sig))) {
        x_error_en(err, __func__, "Signal cannot be sent with kill");
    }
}

void x_pthread_setdetachstate(pthread_attr_t *attr, int detachstate) {
    int err;
    if ((err = pthread_attr_setdetachstate(attr, detachstate))) {
        x_error_en(err, __func__, "Thread detach state cannot be set properly");
    }
}

void x_pthread_attr_init(pthread_attr_t *attr) {
    int err;
    if ((err = pthread_attr_init(attr))) {
        x_error_en(err, __func__, "Thread attribute cannot be set properly");
    }
}

void x_pthread_attr_destroy(pthread_attr_t *attr) {
    int err;
    if ((err = pthread_attr_destroy(attr))) {
        x_error_en(err, __func__, "Thread attribute cannot be destroyed properly");
    }
}

void x_pthread_exit(void *retval) {
    pthread_exit(retval);
}

void x_pthread_join(pthread_t thread, void **retval) {
    int err;
    if ((err = pthread_join(thread, retval))) {
        x_error_en(err, __func__, "Specified thread cannot be joined");
    }
}

void x_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine) (void*), void *arg) {
    int err;
    if ((err = pthread_create(thread, attr, start_routine, arg))) {
        x_error_en(err, __func__, "New thread cannot be created properly");
    }
}

void x_error_en(int err, const char* func, char *msg) {
    fprintf(stderr, "Error, %s: %s with [errno:%s]\n", func, msg, strerror(err));
    exit(EXIT_FAILURE);
}

void x_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    if (sigaction(signum, act, oldact))
        x_error(__func__, "Signal action cannot be assigned");
}

ssize_t x_read(int fd, void *buf, size_t count) {
    ssize_t ret;
    if ((ret = read(fd, buf, count)) == -1)
        x_error(__func__, "read cannot perform properly");
    return ret;
}

void x_close(int fd) {
    if (close(fd) && errno != EBADF)
        x_error(__func__, "Given fd cannot be closed");
}

int x_open(const char *pathname, int flags, mode_t mode) {
    int fd;
    if ((fd = open(pathname, flags, mode)) == -1)
        x_error(__func__, "open cannot perform properly");
    return fd;
}

void x_fseek(FILE* fptr, long offset, int whence) {
    if (fseek(fptr, offset, whence))
        x_error(__func__, "fseek does not work properly");
}

void x_sem_init(sem_t *sem, int pshared, unsigned int value) {
    if (sem_init(sem, pshared, value))
        x_error(__func__, "Semaphore cannot be inited");
}

void x_sem_destroy(sem_t *sem) {
    if (sem_destroy(sem) && errno != EINVAL)
        x_error(__func__, "Semaphore cannot be destroyed");
}

void x_fclose(FILE *fptr) {
    if (fclose(fptr) == EOF && errno != EBADF)
        x_error(__func__, "Input file cannot be closed properly");
}

FILE* x_fopen(char *filename) {
    FILE *fptr;
    fptr = fopen(filename, "r");
    if (!fptr)
        x_error(__func__, "Input file cannot be opened properly");
    fflush(fptr);
    return fptr;
}

pid_t x_fork(void) {
    pid_t pid;
    if ((pid = fork()) == -1)
        x_error(__func__, "Child process cannot be forked properly");
    return pid;
}

int x_shm_open(const char *name) {
    int fd;
    if ((fd = shm_open(name, O_CREAT | O_RDWR, 0666)) == -1)
        x_error(__func__, "shared memory cannot be opened");
    return fd;
}

void *x_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void *mem;
    if ((mem = mmap(addr, length, prot, flags, fd, offset)) == MAP_FAILED)
        x_error(__func__, "mmap cannot work properly");
    return mem;
}

void x_munmap(void *addr, size_t lenght) {
    if (munmap(addr, lenght))
        x_error(__func__, "Memory cannot be unmapped properly");
}

void x_shm_close(int fd) {
    if (close(fd) && errno != EBADF)
        x_error(__func__, "Shared memory cannot be closed");
}

void x_shm_unlink(const char* name) {
    if (shm_unlink(name) && errno != ENOENT)
        x_error(__func__, "shm_unlink cannot work properly");
}

void x_ftruncate(int fd, off_t length) {
    if (ftruncate(fd, length))
        x_error(__func__, "Shared memory cannot be resized");
}

void x_fstat(int fd, struct stat *buf) {
    if (fstat(fd, buf))
        x_error(__func__, "fstat cannot work properly");
}

sem_t* x_sem_open(const char *name, unsigned int value) {
    sem_t *sem = NULL;
    
    if((sem = sem_open(name, O_CREAT | O_RDWR, 0666, value)) == SEM_FAILED)
        x_error(__func__, "semaphore cannot be opened");
    return sem;
}

void x_sem_close(sem_t *sem) {
    if (sem_close(sem) && errno != EINVAL)
        x_error(__func__, "semaphore cannot be closed");
}

void x_sem_unlink(const char *name) {
    if (sem_unlink(name) && errno != ENOENT)
        x_error(__func__, "semaphore cannot be unlinked");
}

void x_sem_wait(sem_t *sem) {
    if (sem_wait(sem) && errno != EINTR)
        x_error(__func__, "sem_wait cannot work properly");
}

void x_sem_post(sem_t *sem) {
    if (sem_post(sem))
        x_error(__func__, "sem_post cannot work properly");
}

int x_sem_getvalue(sem_t *sem) {
    int sval;
    if (sem_getvalue(sem, &sval))
        x_error(__func__, "sem_getvalue cannot work properly");
    return sval;
}

void* x_malloc(size_t size) {
    void* my = malloc(size);
    if (!my)
        x_error(__func__, "Calloc could not allocate space properly");
    return my;
}

void x_error(const char* func, char *msg) {
    fprintf(stderr, "Error, %s: %s with [errno:%s]\n", func, msg, strerror(errno));
    exit(EXIT_FAILURE);
}