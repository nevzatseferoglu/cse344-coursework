#include <stdio.h>
#include "../lib/lib.h"
#include "arg.h"
#include "server.h"

// make process daemon
void become_daemon();

// csv handling functions
void read_csv();

// server functions
void init_server();
void server_loop();
void release_table();
int calculate_row_number();
int calculate_column_number();
char** get_current_row(char* data, int *curIndex, int *size);
void set_table();

char** split(char *str, char c, char out, int *size);
void freepath(char **path, int size);
char* substr(const char *src, int m, int n);

void init_mutex_attribute();
void init_cond_attribute();
void init_component();
void* thread_pool_handler(void *arg);
void thread_pool_join();

char** split2(char *str, char c, char opt, int *size);
char* parse_query(char* query, size_t *size);
int which_header(char* token);
int update_req_token(char* req, int lrowi);

void sigterm_handler(int signo){
	if (signo == SIGTERM)
        sigterm = 1;
}

void sighub_handler(int signo){
	if (signo == SIGHUP)
        sighub = 1;
}

void sigint_handler(int signo) {
    if (signo == SIGINT)
        interrupt = 1;
}

char* current_timestamp() {
    char* ret = NULL;
    time_t time_t_v;
    struct tm *time_keep;
    time_t_v = time(NULL);
    time_keep = localtime(&time_t_v);
    
    ret = x_malloc(sizeof(char)* 11, logFile);
    memset(ret, 0, 11);
    sprintf(ret, "%02d:%02d:%02d:", time_keep->tm_hour, time_keep->tm_min, time_keep->tm_sec);
    return ret;
}

void exit_server() {

    sem_close(sem);
    sem_unlink("PDA");

    if (interrupt) {
        char *time = current_timestamp();
        fprintf(logFile, "%s All threads have terminated, server shutting down.\n", time);
        free(time);
    }

    free(threads);
    destroy_queue(queue);

    x_close(csvfd, logFile);
    x_fclose(fptr, logFile);
    release_table();
    x_close(clientfd, logFile);
    x_close(sockfd, logFile);
    x_pthread_mutex_destroy(&mAvailable, logFile);
    x_pthread_mutex_destroy(&mRW, logFile);
    x_pthread_mutex_destroy(&mBarrier, logFile);
    x_pthread_mutexattr_destroy(&mattr, logFile);
    x_fclose(logFile, logFile);
}

void info() {
    char *time = current_timestamp();
    fprintf(logFile, "%s All threads have terminated, server shutting down.\n", time);
    fprintf(logFile, "%s Executing with parameters:\n", time);
    fprintf(logFile, "%s -p %d\n", time, PORT);
    fprintf(logFile, "%s -o %s\n", time, pathToLogFile);
    fprintf(logFile, "%s -l %d\n", time, poolSize);
    fprintf(logFile, "%s -d %s\n", time, datasetPath);
    free(time);
}

int main(int argc, char* argv[]) {

    set_opts_and_args(argc, argv);

    // prevent double initialization
    sem = sem_open("PDA", (O_CREAT | O_RDWR | O_EXCL), 0666, 0);
    if (sem == SEM_FAILED) {
        fprintf(stdout, "Error: Server cannot be instantiated more than ones!\n");
        exit(EXIT_FAILURE);
    }

    clock_t start, end;
    struct sigaction sa_SIGINT;
    char* time = NULL;
    double datasetLoadingTime;

    become_daemon();

    logFile = fopen(pathToLogFile, "w");
    if (logFile == NULL) {
        x_error(stderr, __func__, "Input file cannot be opened properly\n");
    }

    if (setvbuf(logFile, NULL, _IONBF, 0) != 0) {
        x_error(logFile, __func__, "Log file buffer cannot be set properly");
    }

    memset(&sa_SIGINT, 0, sizeof(sa_SIGINT));
    sa_SIGINT.sa_handler = sigint_handler;
    x_sigaction(SIGINT, &sa_SIGINT, NULL, logFile);
    //logFile = stdout;

    if (atexit(exit_server)) {
        time = current_timestamp();
        fprintf(logFile, "%s atexit cannot be set properly\n", time);
        free(time);
        exit(EXIT_FAILURE);
    }
    
    info();
    
    time = current_timestamp();
    fprintf(logFile, "%s Loading dataset...\n", time);
    free(time);

    start = clock();
    read_csv();
    end = clock() - start;

    datasetLoadingTime = ((double)end)/CLOCKS_PER_SEC;
    time = current_timestamp();
    fprintf(logFile, "%s Dataset loaded in %.5lf seconds with %d records.\n", time, datasetLoadingTime, rSize-1);
    free(time);

    if (interrupt) {
        exit(EXIT_FAILURE);
    }

    init_component();
    init_server();
    server_loop();
    
    thread_pool_join();

    exit(EXIT_SUCCESS);
}

void init_component() {

    char *time = NULL;
    // set mutex attribute
    x_pthread_mutexattr_init(&mattr, logFile);
    x_pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK, logFile);

    // init queue
    queue = init_queue();
    if (queue == NULL) {
        time = current_timestamp();
        fprintf(logFile, "%s Queue cannot be initialized\n", time);
        free(time);
        exit(EXIT_FAILURE);
    }
    
    // init threads array
    threads =  (Thread*) x_malloc((poolSize*sizeof(Thread)), logFile);
    
    // start thread pool
    arrived = 0;

    time = current_timestamp();
    fprintf(logFile, "%s A pool of %d threads has been created\n", time, poolSize);
    free(time);
    
    for (int i=0; i<poolSize; ++i) {
        threads[i].args.i = i;
        x_pthread_create(&(threads[i].threadID), NULL, thread_pool_handler,
         (void*)(&(threads[i].args)), stdout);
    }
    
    // synchronization barrier
    x_pthread_mutex_init(&mBarrier, &mattr, logFile);
    x_pthread_mutex_lock(&mBarrier, logFile);
    while (arrived < poolSize) {
        x_pthread_cond_wait(&cBarrier, &mBarrier, logFile);
        if (interrupt) {
            time = current_timestamp();
            fprintf(logFile, "%s Termination signal received, waiting for ongoing threads to complete.\n", time);
            free(time);
            x_pthread_mutex_unlock(&mBarrier, logFile);
            exit(EXIT_FAILURE);
        }
    }

    x_pthread_mutex_unlock(&mBarrier, logFile);
    x_pthread_mutex_destroy(&mBarrier, logFile);

    // unbounded producer-consumer
    x_pthread_mutex_init(&mQueue, &mattr, logFile);
    
    // writer prioritize reader-writer
    x_pthread_mutex_init(&mRW, &mattr, logFile);

    // available thread
    x_pthread_mutex_init(&mAvailable, &mattr, logFile);
    available = poolSize;

    x_pthread_mutexattr_destroy(&mattr, logFile);
}

void thread_pool_join() {
    for (int i=0; i<poolSize; ++i) {
        x_pthread_join(threads[i].threadID, NULL, logFile);
    }
}

void* thread_pool_handler(void *arg) {

    ThreadArgument *curArg = ((ThreadArgument*)arg);
    int i, fd, ret;
    char query[BUFSIZ];
    char *answer = NULL, *time = NULL;
    char **records = NULL;
    int numOfRecords;
    size_t size;
    
    i = curArg->i;

    x_pthread_mutex_lock(&mBarrier, logFile);
    ++arrived;
    x_pthread_cond_signal(&cBarrier, logFile);
    x_pthread_mutex_unlock(&mBarrier, logFile);

    for (;;) {

        if (interrupt) {
            break;
        }

        time = current_timestamp();
        fprintf(logFile, "%s Thread #%d: waiting for connection\n", time, i);
        free(time);

        x_pthread_mutex_lock(&mQueue, logFile);
        while (queue->size == 0) {
            x_pthread_cond_wait(&cQueue, &mQueue, logFile);
            if (interrupt) {
                x_pthread_mutex_unlock(&mQueue, logFile);
                x_pthread_cond_broadcast(&cQueue, logFile);
                break;
            }
        }

        if (interrupt) {
            break;
        }

        fd = dequeue(queue);
        if (fd == -1) {
            x_pthread_mutex_unlock(&mQueue, logFile);
            break;
        }

        x_pthread_mutex_unlock(&mQueue, logFile);

        if (interrupt) {
            break;
        }

        if ((ret = write(fd, START_NOTIFICATION, strlen(START_NOTIFICATION))) == -1) {
            time = current_timestamp();
            fprintf(logFile, "%s Returned byte: %d , Thread pool first write is wrong! \n", time, ret);
            free(time);
            break;
        }

        time = current_timestamp();
        fprintf(logFile, "%s A connection has been delegated to thread id #%d\n", time, i);
        free(time);

        memset(query, 0, BUFSIZ);
        x_read(fd, query, BUFSIZ, logFile);        
        while (strcmp(query, END_NOTIFICATION) != 0) {

            time = current_timestamp();
            fprintf(logFile, "%s Thread #%d: received query \'%s\'\n", time, i, query);
            free(time);

            answer = parse_query(query, &size);
            if (answer != NULL) {
                records = split2(answer, '\n', '\n', &numOfRecords);
                free(answer);
            }

            if (records == NULL)
                numOfRecords = 0;

            if (write(fd, &numOfRecords, sizeof(int)) == -1) {
                time = current_timestamp();
                fprintf(logFile, "%s Error, %s: %s with [errno:%s]\n", __func__, "Answer size info cannot be sent from server!\n", time, strerror(errno));
                free(time);
                break;
            }

            if (interrupt)
                break;

            if (records != NULL) {
                for (int j=0; j<numOfRecords; ++j) {
                    memset(query, 0, BUFSIZ);
                    strcpy(query, records[j]);

                    if (write(fd, query, BUFSIZ) == -1) {
                        time = current_timestamp();
                        fprintf(logFile, "%s Error, %s: %s with [errno:%s]\n", __func__, "Answer cannot be sent to from server to client!\n", time, strerror(errno));
                        free(time);
                        break;
                    }  
                }
            }

            freepath(records, numOfRecords);

            if (interrupt)
                break;

            memset(query, 0, BUFSIZ);
            x_read(fd, query, BUFSIZ, logFile);
        }

        // simulation for intensive database execution
        usleep(500000);

        x_pthread_mutex_lock(&mAvailable, logFile);
        ++available;
        x_pthread_cond_signal(&cAvailable, logFile);
        x_pthread_mutex_unlock(&mAvailable, logFile);

        x_close(fd, logFile);
    }

    return NULL;
}

int which_header(char* token) {

    int ret = -1;

    if (token == NULL)
        return -1;

    for (int j=0; j<cSize; ++j) {
        if (!strcmp(table[0][j], token)) {
            ret = j;
            break;
        }
    }

    return ret;
}

char* parse_query(char* query, size_t *size) {
    char **tokens = NULL, *result = NULL, *prev = NULL, **cond = NULL;
    int length, i, j, *header = NULL, subTokenSize, lcoli, lrowi;
    char p = 0;
    
    *size = 0;
    tokens = split2(query, ' ', ',', &length);
    if (tokens == NULL)
        return NULL;
    

    if (!strcmp(tokens[0], "SELECT")) {

        x_pthread_mutex_lock(&mRW, logFile);
        while ((AW+WW) > 0) {
            WR++;
            x_pthread_cond_wait(&okToRead, &mRW, logFile);
            WR--;
        }
        AR++;
        x_pthread_mutex_unlock(&mRW, logFile);

        if (!strcmp(tokens[1], "DISTINCT")) {
            // intentionaly left blank 
        } else {
            header = x_malloc(sizeof(int)*cSize, logFile);
            for (int k=0; k<cSize; ++k)
                header[k] = -1;

            for (i=1; strcmp(tokens[i], "FROM"); ++i) {
                j = which_header(tokens[i]);
                if (j == -1)
                    continue;
                header[j] = 1;
                p = 1;
            }
        }

        for (i=0; i<rSize; ++i) {
            for (j=0; j<cSize; ++j) {
                if (header[j] != -1 || (!strcmp(tokens[1], "*"))) {
                    prev = result;
                    if (prev == NULL) {
                        *size += strlen(table[i][j])+2;
                        result = (char*)realloc(result, (*size) * sizeof(char));
                        memset(result, 0, *size*sizeof(char));
                        strcat(result, table[i][j]);
                        strcat(result, " ");
                    } else {
                        *size += strlen(table[i][j])+1;
                        result = (char*)realloc(result, (*size) * sizeof(char));
                        strcat(result, table[i][j]);
                        strcat(result, " ");
                    }
                }
            }
            if (p || (!strcmp(tokens[1], "*"))) {
                *size += 1;
                result = realloc(result, (*size) * sizeof(char));
                strcat(result, "\n");
            }
        }
        //printf("hey\n");
        *size += 1;
        result = realloc(result, (*size) * sizeof(char));
        strcat(result, "\0");
        
        free(header);
        
        x_pthread_mutex_lock(&mRW, logFile);
        AR--;
        if (AR == 0 && WW > 0)
            x_pthread_cond_signal(&okToWrite, logFile);
        x_pthread_mutex_unlock(&mRW, logFile);

    } else if (!strcmp(tokens[0], "UPDATE")) {
        
        x_pthread_mutex_lock(&mRW, logFile);
        while ((AW+AR)>0) {
            WW++;
            x_pthread_cond_wait(&okToWrite, &mRW, logFile);
            WW--;
        }

        AW++;
        x_pthread_mutex_unlock(&mRW, logFile);


        lcoli = -1, lrowi = -1;
        cond = split2(tokens[length-1], '=', '\'', &subTokenSize);

        for (i=0; i<cSize; ++i) {
            if (!strcmp(table[0][i], cond[0])) {
                lcoli = i;
                break;
            }
        }

        if (lcoli != -1) {
            for (i=1; i<rSize; ++i) {
                if (!strcmp(table[i][lcoli], cond[1])) {
                    lrowi = i;
                    break;
                }
            }
        }

        freepath(cond, subTokenSize);

        *size += 18;
        result = realloc(result, (*size) * sizeof(char));
        memset(result, 0, (*size) * sizeof(char));
        strcpy(result, "UPDATED COLUMNS: ");

        if (lrowi != -1) {
            for (i=3; i<length; ++i) {
                if (!strcmp(tokens[i], "WHERE")) {
                    break;
                } else {
                    j = update_req_token(tokens[i], lrowi);
                    if (j != -1) {
                        *size += strlen(table[0][j])+1;
                        result = (char*)realloc(result, (*size) * sizeof(char));
                        strcat(result, table[0][j]);
                        strcat(result, " ");
                    }             
                }
            }
        }
        
        *size += 2;
        result = realloc(result, (*size) * sizeof(char));
        strcat(result, "\n");

        x_pthread_mutex_lock(&mRW, logFile);
        AW--;
        if (WW > 0)
            x_pthread_cond_signal(&okToWrite, logFile);
        else if (WR > 0)
            x_pthread_cond_broadcast(&okToRead, logFile);
        x_pthread_mutex_unlock(&mRW, logFile);
    }

    freepath(tokens, length);
    return result;
}

int update_req_token(char* req, int lrowi) {
    char **subtokens = NULL;
    int length, j;

    subtokens = split2(req, '=', '\'', &length);
    
    j = -1;
    for (j=0; j<cSize; ++j) {
        if (!strcmp(subtokens[0], table[0][j])) {
            free(table[lrowi][j]);
            table[lrowi][j] = NULL;
            table[lrowi][j] = x_malloc((strlen(subtokens[1])+1), logFile);
            strcpy(table[lrowi][j], subtokens[1]);
            break;
        }
    }

    freepath(subtokens, length);

    return j;
}

void server_loop() {
    char *time = NULL;

    for (;;) {

        clientfd = x_accept(sockfd, (SA*)(&client_addr), &socklen, logFile);
        if (interrupt && clientfd == -1) {
            time = current_timestamp();
            fprintf(logFile, "%s Termination signal received, waiting for ongoing threads to complete.\n", time);
            free(time);
            x_pthread_cond_broadcast(&cQueue, logFile);
            break;
        }
        
        x_pthread_mutex_lock(&mAvailable, logFile);
        while (available <=0) {
            time = current_timestamp();
            fprintf(logFile, "%s No thread is available! Waiting…\n", time);
            free(time);

            x_pthread_cond_wait(&cAvailable, &mAvailable, logFile);
            if (interrupt) {

                time = current_timestamp();
                fprintf(logFile, "%s Termination signal received, waiting for ongoing threads to complete.\n", time);
                free(time);
                x_pthread_cond_broadcast(&cAvailable, logFile);
                break;
            }
        }

        --available;
        x_pthread_mutex_unlock(&mAvailable, logFile);
        
        x_pthread_mutex_lock(&mQueue, logFile);
        enqueue(queue, clientfd);

        if (queue == NULL) {
            x_pthread_mutex_unlock(&mQueue, logFile);
            x_pthread_cond_broadcast(&cQueue, logFile);
            break;  
        }
        
        x_pthread_cond_broadcast(&cQueue, logFile);
        x_pthread_mutex_unlock(&mQueue, logFile);
    }
}

void init_server() {

    int opt = 1;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    //x_inet_aton(_IPv4_ADDRESS, &server_addr.sin_addr);
    //server_addr.sin_addr.s_addr = inet_addr(_IPv4_ADDRESS);

    sockfd = x_socket(AF_INET, SOCK_STREAM, 0, logFile);
    x_setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int), logFile);

    x_bind(sockfd, (SA*)(&server_addr), sizeof(server_addr), logFile);
    x_listen(sockfd, 1000, logFile);
    socklen = sizeof(client_addr);
}

void read_csv() {

    rSize = calculate_row_number();
    cSize = calculate_column_number();

    table = NULL;
    table = x_malloc(sizeof(string*)*rSize, logFile);

    release_check = 1;

    for (int i=0; i<rSize; ++i) {
        table[i] = NULL;
        table[i] = x_malloc(sizeof(string)*cSize, logFile);
    }
    
    set_table();
}

void set_table() {
    size_t rowSize;
    int length, curIndex, size;
    char *data, **rowCells;

    csvfd = x_open(datasetPath, O_RDONLY, 0666, logFile);
    length = x_lseek(csvfd, 0, SEEK_END, logFile);
    data = (char*)x_mmap(0, length, PROT_READ, MAP_PRIVATE, csvfd, 0, logFile);
    x_close(csvfd, logFile);
    
    curIndex = 0;
    for (int i=0; curIndex<length; ++i) {

        rowCells = get_current_row(data, &curIndex, &size);
        if (rowCells == NULL)
            continue;

        for (int j=0; j<cSize; ++j) {
            table[i][j] = NULL;
            rowSize = strlen(rowCells[j]);
            table[i][j] = x_malloc(sizeof(char)*(rowSize + 1), logFile);
            memcpy(table[i][j], rowCells[j], sizeof(char)*(rowSize+1));
            table[i][j][rowSize] = '\0';
        }
        freepath(rowCells, size);
    }
    x_munmap(data, length, logFile);
}

char** get_current_row(char* data, int *curIndex, int *size) {
    int from, to;
    char c, dQuate, **rowCells, *cell;
    
    *size = 0;
    dQuate = 0;
    from = *curIndex;
    to = *curIndex;

    while (1) {
        c = data[*curIndex];

        if (c == '\"' && dQuate == 0)  dQuate = 1;
        else if (c == '\"') dQuate = 0;

        if (dQuate == 0 && (c == '\n' || c == '\0')) {
            ++(*curIndex);
            break;
        }

        ++(*curIndex);
        ++to;
    }

    if (from == to)
        return NULL;

    cell = substr(data, from, to);
    if (cell == NULL)
        return NULL;

    rowCells = split(cell, ',', '\"', size);
    free(cell);
    if (rowCells == NULL) {
        return NULL;
    }

    return rowCells;
}

int calculate_column_number() {
    fptr = x_fopen(datasetPath, logFile);
    int size;
    char c, dQuate, prev;

    dQuate = 0;
    size = 0;

    while (1) {
        c = fgetc(fptr);

        if (c == '\"' && dQuate == 0)   dQuate = 1;
        else if (c == '\"')      dQuate = 0;

        if (dQuate != 1 && (c == '\n' || c == EOF)) {
            if (prev != ',')
                ++size;
            break;
        } else if (dQuate != 1 && c == ',') {
            ++size;
        }
        prev = c;
    }

    fflush(fptr);
    x_fclose(fptr, logFile);
    fptr = NULL;

    return size;
}

int calculate_row_number() {
    int line_count;
    char c, prev, dQuate = 0;

    fptr = x_fopen(datasetPath, logFile);

    line_count = 0;
    c = fgetc(fptr);
    while (c != EOF) {

        if (c == '\"' && dQuate == 0)   dQuate = 1;
        else if (c == '\"')      dQuate = 0;

        if (dQuate != 1 && c == '\n') {
            if (prev == c)
                --line_count;
            line_count += 1;
        }

        prev = c;
        c = fgetc(fptr);
        if (prev != '\n' && c == EOF)
            ++line_count;
    }

    fflush(fptr);
    x_fclose(fptr, logFile);
    fptr = NULL;

    return line_count;
}

char** split(char *str, char c, char out, int *size) {

    char **path = NULL , **temp = NULL, outDetect, common, dQuate, prev, *time = NULL;
    const char *save;
    int from, i;

    if(str == NULL)
        return NULL;

    dQuate = 0;
    common = 0;
    outDetect = 0;
    *size = 0;
    save=str;
    from = -1;
    
    for(i=0 ; 1; ++i) {
        if (*str == out && outDetect == 0)   outDetect = 1;
        else if (*str == out)      outDetect = 0;

        if (*str == '\0') {
            common = 1;
        }
        else if (outDetect || *str == out) {
            if (*str != out) {
                if (from == -1)
                    from = i;
            } else
                common = 1;
        } else {
            if(*str != c) {
                if(from == -1)
                    from = i;
            } else
                common = 1;
        }

        if (common) {
            if(from != -1 || (*str == c && dQuate == 0) || (*str == '\0' && prev == c) || (*str == out && prev == out)) {
                ++(*size);
                temp = path;

                path = realloc(path, (sizeof(char*)*(*size)));
                if(path == NULL) {
                    time = current_timestamp();
                    fprintf(logFile, "%s split: allocation error\n", time);
                    free(time);
                    freepath(temp, (*size)-1);
                    return NULL;
                }

                if ((from == -1 && (*str == c || *str == '\0' || prev == out))) {
                    *(path+(*size)-1) = x_malloc(sizeof(char)*5, logFile);
                    memcpy(*(path+(*size)-1), "null", sizeof(char)*5);
                    (*(path+(*size)-1))[4] = '\0';
                }
                else
                    *(path+(*size)-1) = substr(save, from, i);

                if(*(path+(*size)-1) == NULL) {
                    time = current_timestamp();
                    fprintf(logFile, "%s split: allocation error\n", time);
                    free(time);
                    free(path);
                    return NULL;
                }
            }

            if (*str == out)    dQuate = 1;
            else    dQuate = 0;

            if (*str == '\0')
                break;
                
            from = -1;
        }
        common = 0;
        prev = *str;
        ++str;
    }
    return path;
}

void freepath(char **path, int size) {
    int i = 0;
    if (path != NULL) {
        for(i = 0; i<size; ++i) {
            free(*(path+i));
            *(path+i) = NULL;
        }
        free(path);
        path = NULL;
    }
}

char* substr(const char *src, int m, int n)
{
    int len = n - m;
    char *dest = x_malloc(sizeof(char) * (len + 1), logFile);
    char *time = NULL;

    if(dest == NULL) {
        time = current_timestamp();
        fprintf(logFile, "%s substr: allocation error\n", time);
        free(time);
        return NULL;
    }

    for (int i = m; i < n && (*(src + i) != '\0'); i++) {
        *dest = *(src + i);
        ++dest;
    }

    *dest = '\0';
    return dest - len;
}

char** split2(char *str, char c, char opt, int *size) {

    char **path = NULL , **temp = NULL, *time=NULL;
    const char *save=str;
    int from=-1, i = 0;

    *size = 0;
    if(str == NULL)
        return NULL;

    for(i=0 ; 1; ++i) {
        if(*str == '\0') {
            if(from != -1) {
                ++(*size);
                temp = path;
                path = realloc(path, (sizeof(char*)*(*size)));

                if(path == NULL) {
                    time = current_timestamp();
                    fprintf(logFile, "%s split: allocation error\n", time);
                    free(time);
                    freepath(temp, (*size)-1);
                    return NULL;
                }

                *(path+(*size)-1) = substr(save, from, i);
                if(*(path+(*size)-1) == NULL) {
                    time = current_timestamp();
                    fprintf(logFile, "%s split: allocation error\n", time);
                    free(time);
                    free(path);
                    return NULL;
                }
            }
            break;
        }
        if((*str != c)&&(*str != opt)) {
            if(from == -1)
                from = i;
        }
        else {
            if(from != -1) {
                ++(*size);
                temp = path;
                path = realloc(path, (sizeof(char*)*(*size)));

                if(path == NULL) {
                    fprintf(stderr, "split: allocation error\n");
                    freepath(temp, (*size)-1);
                    return NULL;
                }

                *(path+(*size)-1) = substr(save, from, i);
                if(*(path+(*size)-1) == NULL) {
                    fprintf(stderr, "split: allocation error\n");
                    free(path);
                    return NULL;
                }
            }
            from = -1;
        }
        ++str;
    }
    return path;
}

void release_table() {
    if (table != NULL && release_check) {
        for (int i=0; i<rSize; ++i) {
            if (table[i] != NULL) {
                for (int j=0; j<cSize; ++j) {
                    free(table[i][j]);
                }
            }
        }
        for (int i=0; i<rSize; ++i) {
           free(table[i]);
        }

        free(table);
    }
}

void become_daemon() {
	int maxfd, fd;
	struct sigaction sa;

	switch(fork()){
		case -1	: exit(EXIT_FAILURE);
		case  0	: break;
		default	: exit(EXIT_SUCCESS);
	}

	if(setsid() == -1){
        fprintf(stderr, "Error detected %s, errno:%s", __func__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigterm_handler;
	sigaction(SIGHUP, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sighub_handler;
	sigaction(SIGTERM, &sa, NULL);

	switch(fork()){
		case -1	: exit(EXIT_FAILURE);
		case  0	: break;
		default	: exit(EXIT_SUCCESS);
	}

	umask(0);

	fd = ((maxfd = sysconf(_SC_OPEN_MAX)) > 0) ? maxfd-1 : D_MAX-1;
	for( ; fd>=0 ; --fd) close(fd);

	close(STDIN_FILENO);

	fd = open("/dev/null", O_RDWR);	

	if(fd != STDIN_FILENO)
        exit(EXIT_FAILURE);

	if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
        exit(EXIT_FAILURE);

	if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
        exit(EXIT_FAILURE);
}