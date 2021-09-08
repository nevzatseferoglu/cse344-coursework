#ifndef _GLOBAL_STRUCTURE
#define _GLOBAL_STRUCTURE

#include <pthread.h>
#include "list/arraylist.h"

#define TRUE 1
#define BUFFER 255

typedef ArrayList Queue;
Queue* hw_queue;

typedef struct StudentArgs {
    int i;
    char c;
    int moneyLeft;
} StudentArgs;

//students
typedef struct Student {
    pthread_t thread;
    int busy;
    sem_t readyBusy;
    sem_t startWork;
    char school[BUFFER];
    int quality;
    int speed;
    int cost;
    int balance;
    int solvedHw;
    StudentArgs args;
} Student;

Student* students;
int line_count;
sem_t available;

volatile sig_atomic_t finished;
volatile sig_atomic_t sigint;

//money (readers-writers)
sem_t wrtM;
sem_t saveM;
int readcnt;

int saveMoney;

sem_t saveEof;
int eof;
sem_t waitFile;

sem_t finito;

sem_t liveThread;
int liveThreadNum;

// threads related declerations
pthread_attr_t attr;
pthread_attr_t *attrp;

pthread_t thread_h;

#endif