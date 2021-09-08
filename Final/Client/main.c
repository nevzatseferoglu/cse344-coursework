#include <stdio.h>
#include "../lib/lib.h"
#include "arg.h"
#include "client.h"

void set_buffer() {
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0 ||
        setvbuf(stdout, NULL, _IONBF, 0) != 0) {
        x_error(stderr, __func__, "Buffer cannot be set properly");
    }
}

void sigint_handler(int signo) {
    if (signo == SIGINT)
        interrupt = 1;
}

void exit_client() {
    x_fclose(fptr, stderr);
    x_close(sockfd, stderr);
}

char* current_timestamp() {
    time_t time_t_v;
    struct tm *time_keep;
    time_t_v = time(NULL);
    time_keep = localtime(&time_t_v);
    
    sprintf(time_v, "%02d:%02d:%02d:", time_keep->tm_hour, time_keep->tm_min, time_keep->tm_sec);
    return time_v;
}

void init_client();
void client_request();
char *produce_query(char line[]);
char* substr(const char *src, int m, int n);

int main(int argc, char* argv[]) {

    struct sigaction sa_SIGINT;
    memset(&sa_SIGINT, 0, sizeof(sa_SIGINT));
    sa_SIGINT.sa_handler = sigint_handler;
    x_sigaction(SIGINT, &sa_SIGINT, NULL, stderr);

    set_opts_and_args(argc, argv);

    if (atexit(exit_client)) {
        x_error(stderr, __func__, "atexit cannot be set properly");
    }

    set_buffer();

    init_client();

    if (interrupt) {
        exit(EXIT_FAILURE);
    }

    client_request();

    exit(EXIT_SUCCESS);
}

void init_client() {

    memset(time_v, 0, 11);
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    x_inet_aton(IPv4, &client_addr.sin_addr, stderr);

    //x_inet_pton(AF_INET, IPv4, &client_addr.sin_addr);
    //client_addr.sin_addr.s_addr = inet_addr(IPv4);

    sockfd = x_socket(AF_INET, SOCK_STREAM, 0, stderr);
    x_connect(sockfd, (SA*)(&client_addr), sizeof(client_addr), stderr);
}

void client_request() {
    char line[BUFSIZ];
    char actualQuery[BUFSIZ];
    int queryID, numOfQuery = 0;
    int numOfRecords;
    
    if (interrupt) {
        exit(EXIT_FAILURE);
    }

    memset(line, 0, BUFSIZ);
    x_read(sockfd, line, BUFSIZ, stderr);
    if (strcmp (line, START_NOTIFICATION) == 0) {
        fprintf(stdout, "%s Client-%d connecting to %s:%d\n", current_timestamp(), id, IPv4, PORT);
    } else {
        fprintf(stdout, "%s Client-%d cannot connect to %s:%d\n", current_timestamp(), id, IPv4, PORT);
    }

    fptr = x_fopen(pathToQueryFile, stderr);

    memset(line, 0, BUFSIZ);
    while (fgets(line, BUFSIZ, fptr) != NULL) {

        if (interrupt) {
            exit(EXIT_FAILURE);
        }
        
        memset(actualQuery, 0, BUFSIZ);
        sscanf (line, "%d %[^\n]", &queryID, actualQuery);

        if (id != queryID)
            continue;
        
        calcTime.start = clock();
        fprintf(stdout, "%s Client-%d connected and sending query \'%s\'\n", current_timestamp(), id, actualQuery);
        if (write(sockfd, actualQuery, BUFSIZ) == -1) {
            fprintf(stderr, "%s Error, %s: %s with [errno:%s]\n", __func__, "Query cannot be sent to server from client!\n", current_timestamp(), strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        x_read(sockfd, &numOfRecords, sizeof(int), stderr);

        if (numOfRecords != 0) {
            calcTime.end = clock() - calcTime.start;
            calcTime.elapsedTime = ((double)calcTime.end)/CLOCKS_PER_SEC;
            fprintf(stdout, "%s Server's response to Client-%d is %d records, and arrived in %.5lf seconds.\n",
                current_timestamp(), id, (numOfRecords-1), (calcTime.elapsedTime));

            for (int i=0; i<numOfRecords; ++i) {
                memset(line, 0, BUFSIZ);
                x_read(sockfd, line, BUFSIZ, stderr);
                fprintf(stdout, "%s\n", line);
            }
        }
        
        memset(line, 0, BUFSIZ);
        ++numOfQuery;
    }

    if (write(sockfd, END_NOTIFICATION, strlen(END_NOTIFICATION)+1) == -1) {
        fprintf(stderr, "%s Error, %s: %s with [errno:%s]\n", __func__, "END_NOTIFICATION cannot be sent to server from client!\n", current_timestamp(), strerror(errno));
        exit(EXIT_FAILURE);
    }

    x_close(sockfd, stderr);

    fprintf(stdout, "%s A total of %d queries were executed, Client-%d is terminating.\n", current_timestamp(), numOfQuery, id);
}
