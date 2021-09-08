#include "lib.h"
void x_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen, FILE *stream) {
    if (setsockopt(sockfd, level, optname, optval, optlen) < 0) {
        x_error(stream, __func__, "Options cannot be added to socket");
    }
}

void x_inet_pton(int af, const char *src, void *dst, FILE *stream) {
    if (inet_pton(af, src, dst) == 0) {
        x_error(stream, __func__, "IPv4 cannot be converted from string into binary form");
    }
}

void x_inet_aton(const char *cp, struct in_addr *inp, FILE *stream) {
    if (inet_aton(cp, inp) == 0) {
        x_error(stream, __func__, "IPv4 cannot be converted from string into binary form");
    }
}

int x_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, FILE *stream) {
    return accept(sockfd, (SA*)(&addr), addrlen);
}

int x_socket(int domain, int type, int protocol, FILE *stream) {
    int ret = socket(AF_INET, SOCK_STREAM, 0);
    if (ret == -1) {
        x_error(stream, __func__, "Server socket cannot be initialized properly");
    }
    return ret;
}

void x_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen, FILE *stream) {
    if (connect(sockfd, addr, addrlen)) {
        x_error(stream, __func__, "Client cannot be connected to server");
    }
}

void x_listen(int sockfd, int backlog, FILE *stream) {
    if (listen(sockfd, backlog)) {
         x_error(stream, __func__, "Listening cannot be applied");
    }
}

void x_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen, FILE *stream) {
    if (bind(sockfd, addr, addrlen)) {
        x_error(stream, __func__, "Socket cannot be binded");
    }
}

/*
void x_pthread_cond_setpshared(pthread_condattr_t *attr, int pshared, FILE *stream) {
    int err;
    if (err = pthread_cond_setpshared(attr, pshared)) {
        x_error_en(stream, err, __func__, "Conditional variable scope cannot be set");
    }
}

void x_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr, FILE *stream) {
    int err;
    if (err = pthread_cond_init(cond, attr)) {
        x_error_en(stream, err, __func__, "Conditional variable cannot be initizalized");
    }
}

void x_pthread_cond_destroy(pthread_cond_t *cond, FILE *stream) {
    int err;
    if ((err = pthread_cond_destroy(cond)) && err != EINVAL) {
        x_error_en(stream, err, __func__, "Conditional variable cannot be destroyed");
    }
}

void x_pthread_condattr_init(pthread_cond_t *attr, FILE *stream) {
    int err;
    if (err = pthread_condattr_init(attr)) {
        x_error_en(stream, err, __func__, "Conditional variable attribute cannot be initialized");
    }
}

void x_pthread_condattr_destroy(pthread_cond_t *attr, FILE *stream) {
    int err;
    if ((err = pthread_condattr_destroy(attr) && err != EINVAL)) {
        x_error_en(stream, err, __func__, "Conditional variable attribute cannot be destroyed");
    }
}
*/

void x_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex, FILE *stream) {
    int err;
    if ((err = pthread_cond_wait(cond, mutex))) {
        x_error_en(stream, err, __func__, "Condinitinal variable cannot be waited");
    }
}

void x_pthread_cond_signal(pthread_cond_t *cond, FILE *stream) {
    int err;
    if ((err = pthread_cond_signal(cond))) {
        x_error_en(stream, err, __func__, "Condinitinal variable cannot be signaled");
    }
}

void x_pthread_cond_broadcast(pthread_cond_t *cond, FILE *stream) {
    int err;
    if ((err = pthread_cond_broadcast(cond))) {
        x_error_en(stream, err, __func__, "Condinitinal variable cannot be broadcasted");
    }
}

void x_pthread_mutex_lock(pthread_mutex_t *mutex, FILE *stream) {
    int err;
    if ((err = pthread_mutex_lock(mutex))) {
        x_error_en(stream, err, __func__, "Mutex cannot be locked");
    }
}

void x_pthread_mutex_unlock(pthread_mutex_t *mutex, FILE *stream) {
    int err;
    if ((err = pthread_mutex_unlock(mutex))) {
        x_error_en(stream, err, __func__, "Mutex cannot be unlocked");
    }
}

void x_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr, FILE *stream) {
    int err;
    if ((err = pthread_mutex_init(mutex, attr))) {
        x_error_en(stream, err, __func__, "Thread mutex cannot be initialized");
    }
}

void x_pthread_mutex_destroy(pthread_mutex_t *mutex, FILE *stream) {
    int err;
    if ((err = pthread_mutex_destroy(mutex)) && err != EINVAL) {
        x_error_en(stream, err, __func__, "Thread mutex cannot be destroyed");
    }
}

void x_pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type, FILE *stream) {
    int err;
    if ((err = pthread_mutexattr_settype(attr, type))) {
        x_error_en(stream, err, __func__, "Thread mutexattr cannot be initialized");
    } 
}

void x_pthread_mutexattr_init(pthread_mutexattr_t *attr, FILE *stream) {
    int err;
    if ((err = pthread_mutexattr_init(attr))) {
        x_error_en(stream, err, __func__, "Thread mutexattr cannot be initialized");
    }
}

void x_pthread_mutexattr_destroy(pthread_mutexattr_t *attr, FILE *stream) {
    int err;
    if ((err = pthread_mutexattr_destroy(attr)) && err != EINVAL) {
        x_error_en(stream, err, __func__, "Thread mutexattr cannot be initialized");
    }
}

void x_pthread_kill(pthread_t thread, int sig, FILE *stream) {
    int err;
    if ((err = pthread_kill(thread, sig))) {
        x_error_en(stream, err, __func__, "Signal cannot be sent with kill");
    }
}

void x_pthread_setdetachstate(pthread_attr_t *attr, int detachstate, FILE *stream) {
    int err;
    if ((err = pthread_attr_setdetachstate(attr, detachstate))) {
        x_error_en(stream, err, __func__, "Thread detach state cannot be set properly");
    }
}

void x_pthread_attr_init(pthread_attr_t *attr, FILE *stream) {
    int err;
    if ((err = pthread_attr_init(attr))) {
        x_error_en(stream, err, __func__, "Thread attribute cannot be set properly");
    }
}

void x_pthread_attr_destroy(pthread_attr_t *attr, FILE *stream) {
    int err;
    if ((err = pthread_attr_destroy(attr))) {
        x_error_en(stream, err, __func__, "Thread attribute cannot be destroyed properly");
    }
}

void x_pthread_exit(void *retval) {
    pthread_exit(retval);
}

void x_pthread_join(pthread_t thread, void **retval, FILE *stream) {
    int err;
    if ((err = pthread_join(thread, retval))) {
        x_error_en(stream, err, __func__, "Specified thread cannot be joined");
    }
}

void x_pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
    void *(*start_routine) (void*), void *arg, FILE *stream) {
    int err;
    if ((err = pthread_create(thread, attr, start_routine, arg))) {
        x_error_en(stream, err, __func__, "New thread cannot be created properly");
    }
}

void x_error_en(FILE *stream, int err, const char* func, char *msg) {

    char* ret = NULL;
    time_t time_t_v;
    struct tm *time_keep;
    time_t_v = time(NULL);
    time_keep = localtime(&time_t_v);

    if (stream == stdout) {
        stream = stderr;
    }
    
    ret = x_malloc(sizeof(char)* 11, stream);
    memset(ret, 0, 11);
    sprintf(ret, "%02d:%02d:%02d:", time_keep->tm_hour, time_keep->tm_min, time_keep->tm_sec);

    fprintf(stream,  "%s Error, %s: %s with [errno:%s]\n", ret, func, msg, strerror(err));
    free(ret);

    exit(EXIT_FAILURE);
}

void x_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact, FILE *stream) {
    if (sigaction(signum, act, oldact))
        x_error(stream, __func__, "Signal action cannot be assigned");
}

ssize_t x_read(int fd, void *buf, size_t count, FILE *stream) {
    ssize_t ret;
    if ((ret = read(fd, buf, count)) == -1)
        x_error(stream, __func__, "read cannot perform properly");
    return ret;
}

void x_close(int fd, FILE *stream) {
    if (close(fd) && errno != EBADF)
        x_error(stream, __func__, "Given fd cannot be closed");
}

off_t x_lseek(int fd, off_t offset, int whence, FILE *stream) {
    off_t ret;
    if ((ret = lseek(fd, offset, whence)) == -1) {
        x_error(stream, __func__, "Input csv file cannot been sought");
    }
    return ret;
}

int x_open(const char *pathname, int flags, mode_t mode, FILE *stream) {
    int fd;
    if ((fd = open(pathname, flags, mode)) == -1)
        x_error(stream, __func__, "open cannot perform properly");
    return fd;
}

void x_fseek(FILE* fptr, long offset, int whence, FILE *stream) {
    if (fseek(fptr, offset, whence))
        x_error(stream, __func__, "fseek does not work properly");
}

void x_sem_init(sem_t *sem, int pshared, unsigned int value, FILE *stream) {
    if (sem_init(sem, pshared, value))
        x_error(stream, __func__, "Semaphore cannot be inited");
}

void x_sem_destroy(sem_t *sem, FILE *stream) {
    if (sem_destroy(sem) && errno != EINVAL)
        x_error(stream, __func__, "Semaphore cannot be destroyed");
}

void x_fclose(FILE *fptr, FILE *stream) {

    if (stream == NULL) {
        stream = stderr;
    }

    if ((fptr != NULL) && (fclose(fptr) == EOF && errno != EBADF))
        x_error(stream, __func__, "Input file cannot be closed properly");
}

FILE* x_fopen(char *filename, FILE *stream) {
    FILE *fptr = NULL;
    fptr = fopen(filename, "r");
    if (fptr == NULL) {
        if (stream == NULL) {
            stream = stderr;
        }
        x_error(stream, __func__, "Input file cannot be opened properly");
    }
    fflush(fptr);
    return fptr;
}

pid_t x_fork(FILE *stream) {
    pid_t pid;
    if ((pid = fork()) == -1)
        x_error(stream, __func__, "Child process cannot be forked properly");
    return pid;
}

int x_shm_open(const char *name, FILE *stream) {
    int fd;
    if ((fd = shm_open(name, O_CREAT | O_RDWR, 0666)) == -1)
        x_error(stream, __func__, "shared memory cannot be opened");
    return fd;
}

void *x_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset, FILE *stream) {
    void *mem;
    if ((mem = mmap(addr, length, prot, flags, fd, offset)) == MAP_FAILED)
        x_error(stream, __func__, "mmap cannot work properly");
    return mem;
}

void x_munmap(void *addr, size_t lenght, FILE *stream) {
    if (munmap(addr, lenght))
        x_error(stream, __func__, "Memory cannot be unmapped properly");
}

void x_shm_close(int fd, FILE *stream) {
    if (close(fd) && errno != EBADF)
        x_error(stream, __func__, "Shared memory cannot be closed");
}

void x_shm_unlink(const char* name, FILE *stream) {
    if (shm_unlink(name) && errno != ENOENT)
        x_error(stream, __func__, "shm_unlink cannot work properly");
}

void x_ftruncate(int fd, off_t length, FILE *stream) {
    if (ftruncate(fd, length))
        x_error(stream, __func__, "Shared memory cannot be resized");
}

void x_fstat(int fd, struct stat *buf, FILE *stream) {
    if (fstat(fd, buf))
        x_error(stream, __func__, "fstat cannot work properly");
}

sem_t* x_sem_open(const char *name, unsigned int value, FILE *stream) {
    sem_t *sem = NULL;
    
    if((sem = sem_open(name, O_CREAT | O_RDWR, 0666, value)) == SEM_FAILED)
        x_error(stream, __func__, "semaphore cannot be opened");
    return sem;
}

void x_sem_close(sem_t *sem, FILE *stream) {
    if (sem_close(sem) && errno != EINVAL)
        x_error(stream, __func__, "semaphore cannot be closed");
}

void x_sem_unlink(const char *name, FILE *stream) {
    if (sem_unlink(name) && errno != ENOENT)
        x_error(stream, __func__, "semaphore cannot be unlinked");
}

void x_sem_wait(sem_t *sem, FILE *stream) {
    if (sem_wait(sem) && errno != EINTR)
        x_error(stream, __func__, "sem_wait cannot work properly");
}

void x_sem_post(sem_t *sem, FILE *stream) {
    if (sem_post(sem))
        x_error(stream, __func__, "sem_post cannot work properly");
}

int x_sem_getvalue(sem_t *sem, FILE *stream) {
    int sval;
    if (sem_getvalue(sem, &sval))
        x_error(stream, __func__, "sem_getvalue cannot work properly");
    return sval;
}

void* x_malloc(size_t size, FILE *stream) {
    void* my = malloc(size);
    if (!my)
        x_error(stream, __func__, "Calloc could not allocate space properly");
    return my;
}

void x_error(FILE *stream, const char* func, char *msg) {

    char* ret = NULL;
    time_t time_t_v;
    struct tm *time_keep;
    time_t_v = time(NULL);
    time_keep = localtime(&time_t_v);

    if (stream == stdout) {
        stream = stderr;
    }
    
    ret = x_malloc(sizeof(char)* 11, stream);
    memset(ret, 0, 11);
    sprintf(ret, "%02d:%02d:%02d:", time_keep->tm_hour, time_keep->tm_min, time_keep->tm_sec);

    fprintf(stream, "%s Error, %s: %s with [errno:%s]\n", ret, func, msg, strerror(errno));
    free(ret);

    exit(EXIT_FAILURE);
}