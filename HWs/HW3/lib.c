#include "lib.h"

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
    if (sem_wait(sem))
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

void x_error(const char* func, char *msg) {
    fprintf(stderr, "Error, %s: %s with [errno:%s]\n", func, msg, strerror(errno));
    exit(EXIT_FAILURE);
}