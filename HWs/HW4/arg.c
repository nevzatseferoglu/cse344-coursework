#include "arg.h"

/* arguments */
char* homeworkFilePath = NULL;
char* studentsFilePath = NULL;
int money = 0;

void x_argerror(char* argv[]);
int isNotaNumber(const char* str);

void set_opts_and_args(int argc, char *argv[]) {
    if (argc != 4 || isNotaNumber(argv[3]) || atoi(argv[3]) == 0) {
        x_argerror(argv);
        exit(EXIT_FAILURE);
    }

    homeworkFilePath = argv[1];
    studentsFilePath = argv[2];
    money = atoi(argv[3]);
}

void x_argerror(char* argv[]) {
    fprintf(stderr, "Usage:"
                    "%s homeworkFilePath studentsFilePath money\n\n"
                    "homeworkFilePath: path of file which contanins homeworks\n"
                    "studentsFilePath: path of file which contanins students\n"
                    "money: total balance\n", *argv);
    exit(EXIT_FAILURE);
}

int isNotaNumber(const char* str) {
    if (str == NULL)
        return -1;
        
    while(*str) {
        if(!(*str >= '0' && *str <= '9'))
            return -1;
        ++str;
    }
    return 0;
}
