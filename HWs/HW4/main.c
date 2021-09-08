#include <stdio.h>
#include "arg.h"
#include "lib.h"
#include "global.h"


void sigint_handler() {
    sigint = 1;
}

void exit_main() {

    //student's sync destroying
    for (int i=0; i<line_count; ++i) {
        x_sem_destroy(&(students[i].readyBusy));
        x_sem_destroy(&(students[i].startWork));
        x_sem_destroy(&available);
    }

    //money sync destroying
    x_sem_destroy(&wrtM);
    x_sem_destroy(&saveM);

    //thread_h's sync destroying
    x_sem_destroy(&waitFile);
    x_sem_destroy(&saveEof);
    x_sem_destroy(&liveThread);
    x_sem_destroy(&finito);

    if (attrp != NULL) {
        x_pthread_attr_destroy(&attr);
    }

    free (students);
    ArrayList_destroy(hw_queue);
}

void set_buffer() {
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0 ||
        setvbuf(stdout, NULL, _IONBF, 0) != 0) {
        x_error(__func__, "Buffer cannot be set properly");
    }
}

void write_on_money(int new_money);
void read_cur_money(int *ret);
void init_money_sync();
void init_students();
void init_detached_state();
void calculate_line_number();
void fill_global_students();

void print_students();
void init_student_threads();
void join_student_threads();
void main_thread();
void* thread_student_handler(void *arg);
void* thread_h_handler(void *arg);
void init_components();

void maximumQuality(int *index);
void maximumSpeed(int *index);
void minimumCost(int *index);
void finishedOutput();

int main(int argc, char* argv[]) {

    struct sigaction sa_sigint;
    set_opts_and_args(argc, argv);
    
    if (atexit(exit_main)) {
        x_error(__func__, "atexit cannot be set properly");
    }

    memset(&sa_sigint, 0, sizeof(sa_sigint));
    sa_sigint.sa_handler = sigint_handler;
    x_sigaction(SIGINT, &sa_sigint, NULL);

    set_buffer();
    init_components();

    if (sigint) {
        fprintf(stdout, "Termination signal received, closing.\n");
        exit(EXIT_FAILURE);
    }

    main_thread();

    if (sigint) {
        fprintf(stdout, "Termination signal received, closing.\n");
        join_student_threads();
        exit(EXIT_FAILURE);
    }
    
    join_student_threads();
    finishedOutput();

    exit(EXIT_SUCCESS);
}

void maximumQuality(int *index) {
    int quality = INT_MIN;
    for (int i=0; i<line_count; ++i) {
        x_sem_wait(&(students[i].readyBusy));
        if (!(students[i].busy) && (students[i].quality > quality)) {
            quality = students[i].quality;
            *index = i;
        }
        x_sem_post(&(students[i].readyBusy));  
    }
}

void maximumSpeed(int *index) {
    int speed = INT_MIN;
    for (int i=0; i<line_count; ++i) {
        x_sem_wait(&(students[i].readyBusy));
        if (!(students[i].busy) && (students[i].speed > speed)) {
            speed = students[i].speed;
            *index = i;
        }
        x_sem_post(&(students[i].readyBusy));   
    }
}

void minimumCost(int *index) {
    int cost = INT_MAX;
    for (int i=0; i<line_count; ++i) {
        x_sem_wait(&(students[i].readyBusy));
        if (!(students[i].busy) && (students[i].cost < cost)) {
            cost = students[i].cost;
            *index = i;
        }
        x_sem_post(&(students[i].readyBusy));   
    }
}

void join_student_threads() {
    for (int i=0; i<line_count; ++i) {
        x_pthread_join(students[i].thread, NULL);
    }
}

void init_student_threads() {
    for (int i=0; i<line_count; ++i) {
        x_pthread_create(&(students[i].thread), NULL, &thread_student_handler,
         (void*)(&(students[i].args)));
    }
}

void main_thread() {

    char c;
    int curMoney, index;

    x_pthread_create(&thread_h, &attr, &thread_h_handler, NULL);

    init_student_threads();
    print_students();

    while (TRUE) {

        if (sigint) {
            for (int i=0; i<line_count; ++i) {
                x_pthread_kill(students[i].thread, SIGINT);
            }
            break;
        }

        x_sem_wait(&waitFile);
        x_sem_wait(&saveEof);

        if (eof && ArrayList_isEmpty(hw_queue)) {
            x_sem_post(&saveEof);
            fprintf(stdout, "No more homeworks left or coming in, closing\n");
            x_sem_wait(&finito);
            finished = 1;
            for (int i=0; i<line_count; ++i) {
                x_sem_post(&(students[i].startWork));
            }
            break;
        }
        x_sem_post(&saveEof);


        ArrayList_get(hw_queue, 0, &c);    
        x_sem_wait(&available);
        switch (c)
        {
            case 'Q':
                maximumQuality(&index);
                break;
            case 'S':
                maximumSpeed(&index);
                break;
            case 'C':
                minimumCost(&index);
                break;
            
            default:
                break;
        }
        read_cur_money(&curMoney);
        if (curMoney < (students[index].cost)) {
            fprintf(stdout, "Money is over, closing\n");
            finished = 1;
            for (int i=0; i<line_count; ++i) {
                x_sem_post(&(students[i].startWork));
            }
            break;
        }

        if (ArrayList_remove(hw_queue, 0)) {
            x_error(__func__, "Element cannot ben removed from queue");
        }

        students[index].args.c = c;
        students[index].args.moneyLeft = (curMoney - students[index].cost);
        x_sem_post(&(students[index].startWork));
        write_on_money(students[index].args.moneyLeft);
    }
}

void finishedOutput() {
    int totalCost = 0;
    int totalHw = 0, curMoney;
    fprintf(stdout, "Homeworks solved and money made by the students:\n");
    for (int i=0; i<line_count; ++i) {
        fprintf(stdout, "%s %d %d\n", 
            students[i].school, students[i].solvedHw, students[i].balance);
        totalCost += students[i].balance;
        totalHw +=students[i].solvedHw;
    }
    fprintf(stdout, "Total cost for %d homeworks %dTL\n", totalHw, totalCost);
    read_cur_money(&curMoney);
    //fprintf(stdout, "Money left at G’s account: %dTL\n", saveMoney-totalCost);
    fprintf(stdout, "Money left at G’s account: %dTL\n", saveMoney - totalCost);
}

void print_students() {
    fprintf(stdout, "%d students-for-hire threads have been created.\n", line_count);
    fprintf(stdout, "Name Q S C\n");
    for (int i=0; i<line_count; ++i) {
        fprintf(stdout, "%s %d %d %d\n", 
            students[i].school,
            students[i].quality,
            students[i].speed,
            students[i].cost);
    }
}

void* thread_student_handler(void *arg) {
    StudentArgs* info = ((StudentArgs*)arg);
    int i, moneyLeft, curEof;
    char c;
    
    i = info->i;
    while (TRUE) {
        
        if (sigint) {
            break;
        }

        fprintf(stdout, "%s is waiting for a homework\n", students[i].school);
        x_sem_wait(&(students[i].startWork));

        if (finished) {
            break;
        }

        c = info->c;
        moneyLeft = info->moneyLeft;

        x_sem_wait(&(students[i].readyBusy));
        students[i].busy = 1;
        x_sem_wait(&liveThread);
        ++liveThreadNum;
        x_sem_post(&liveThread);
        x_sem_post(&(students[i].readyBusy));

        
        fprintf(stdout, "%s is solving homework %c for %d, H has %dTL left.\n",
            students[i].school,
            c,
            students[i].cost,
            moneyLeft);
        
        sleep(6 - students[i].speed);
        students[i].balance += students[i].cost;
        students[i].solvedHw += 1;

        x_sem_wait(&(students[i].readyBusy));
        students[i].busy = 0;
        x_sem_post(&available);
        x_sem_wait(&liveThread);
        --liveThreadNum;
        x_sem_wait(&saveEof);
        curEof = eof;
        x_sem_post(&saveEof);
        if (curEof && ArrayList_isEmpty(hw_queue) && (liveThreadNum == 0)) {
            x_sem_post(&finito);
        }
        x_sem_post(&liveThread);
        
        x_sem_post(&(students[i].readyBusy));
    }

    return NULL;
}

void init_components() {
    hw_queue = ArrayList_init(sizeof(char));
    if (!hw_queue) {
        x_error(__func__, "Queue cannot be inited");
    }

    finished = 0;
    eof = 0;
    saveMoney = money;
    init_detached_state();
    
    init_money_sync();
    init_students();
    x_sem_init(&available, 0, line_count);
    x_sem_init(&waitFile, 0, 0);
    x_sem_init(&saveEof, 0, 1);
    x_sem_init(&liveThread, 0, 1);
    liveThreadNum = 0;
    x_sem_init(&finito, 0, 0);
    sigint = 0;
}

void init_detached_state() {
    attrp = NULL;
    x_pthread_attr_init(&attr);
    attrp = &attr;
    x_pthread_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
}

void init_students() {
    calculate_line_number();
    students = x_malloc(sizeof(Student) * line_count);
    fill_global_students(line_count);
}

void fill_global_students() {
    
    FILE *fptr;
    char line[BUFFER];
    int quality, speed, cost;

    fptr = x_fopen(studentsFilePath);

    strcpy(line, "\0");

    for (int i=0; i<line_count; ++i) {
        fgets(line, BUFFER, fptr);
        strcpy(students[i].school, "\0");
        sscanf(line, "%s%d%d%d", students[i].school, &quality, &speed, &cost);

        students[i].busy = 0;
        x_sem_init(&(students[i].readyBusy), 0, 1);
        x_sem_init(&(students[i].startWork), 0, 0);
        students[i].quality = quality;
        students[i].speed = speed;
        students[i].cost = cost;
        students[i].balance = 0;
        students[i].solvedHw = 0;
        students[i].args.i = i;
    }

    x_fclose(fptr);
}

void calculate_line_number() {
    char c, prev;
    FILE* fptr;

    fptr = x_fopen(studentsFilePath);

    line_count = 0;
    c = fgetc(fptr);
    while (c != EOF) {
        if (c == '\n') {
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
    x_fclose(fptr);
}

void write_on_money(int new_money) {
    x_sem_wait(&wrtM);
    money = new_money;
    x_sem_post(&wrtM);
}

void read_cur_money(int *ret) {
    x_sem_wait(&saveM);
    readcnt += 1;
    if (readcnt == 1)
        x_sem_wait(&wrtM);
    x_sem_post(&saveM);
    
    *ret = money;

    x_sem_wait(&saveM);
    readcnt -= 1;
    if (readcnt == 0)
        x_sem_post(&wrtM);
    x_sem_post(&saveM);
}

void init_money_sync() {
    x_sem_init(&wrtM, 0, 1);
    x_sem_init(&saveM, 0, 1);
    readcnt = 0;
}

void* thread_h_handler(void *arg) {

    int fd, moneyRet, retByte;
    char c;
    fd = x_open(homeworkFilePath, O_RDONLY, 0666);
    
    while (TRUE) {
        if (sigint) {
            break;
        }

        retByte = x_read(fd, &c, 1);
        if (retByte == 0) {
            x_sem_wait(&saveEof);
            eof = 1;
            x_sem_post(&saveEof);
            break;
        }

        read_cur_money(&moneyRet);

        if (!moneyRet)
            break;

        fprintf(stdout, "H has a new homework %c; "
        "remaining money is %dTL\n", c, moneyRet);
        x_sem_wait(&saveEof);
        ArrayList_append(hw_queue, &c);
        x_sem_post(&saveEof);
        x_sem_post(&waitFile);
    }

    x_close(fd);
    x_sem_post(&waitFile);
    return NULL;
}