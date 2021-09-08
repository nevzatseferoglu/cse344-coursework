#ifndef _CLIENT_API
#define _CLIENT_API

const char *START_NOTIFICATION = "READY\0";
const char *END_NOTIFICATION = "DONE\0";

typedef struct TIME {
    clock_t start;
    clock_t end;
    double elapsedTime;
} TIME;

int sockfd;
FILE *fptr = NULL;
struct sockaddr_in client_addr;
TIME calcTime;

char time_v[11];
sig_atomic_t interrupt = 0;


#endif /* _CLIENT_API */