#ifndef _SERVER_API
#define _SERVER_API

#include <stdio.h>
#include "queue/queue.h"

#define D_MAX 8192

typedef char* string;

const char *START_NOTIFICATION = "READY\0";
const char *END_NOTIFICATION = "DONE\0";
FILE* logFile = NULL;

// CSV components
int csvfd;
int release_check = 0;
FILE *fptr = NULL;
unsigned int rSize;
unsigned int cSize;
string **table = NULL;

// socket components
int sockfd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
socklen_t socklen;
int clientfd;

typedef struct ThreadArgument {
    int i;
} ThreadArgument;

typedef struct Thread {
    pthread_t threadID;
    ThreadArgument args;
} Thread;

Thread *threads = NULL;
Queue *queue = NULL;

// general attribute
pthread_mutexattr_t mattr;

// synchronization barrier
pthread_mutex_t mBarrier;
int arrived;
pthread_cond_t cBarrier = PTHREAD_COND_INITIALIZER;

// unbound producer-consumer
pthread_mutex_t mQueue;
pthread_cond_t cQueue = PTHREAD_COND_INITIALIZER;

// reader-write
int AR = 0;
int AW = 0;
int WR = 0;
int WW = 0;
pthread_mutex_t mRW;
pthread_cond_t okToRead = PTHREAD_COND_INITIALIZER;
pthread_cond_t okToWrite = PTHREAD_COND_INITIALIZER;

// available threads
int available = 0;
pthread_mutex_t mAvailable;
pthread_cond_t cAvailable = PTHREAD_COND_INITIALIZER;

// interrupt signal handling
volatile sig_atomic_t interrupt = 0;

// daemon handler
volatile sig_atomic_t sigterm = 0;
volatile sig_atomic_t sighub = 0;

// preventing double instantiation
sem_t* sem;

#endif /* _SERVER_API */