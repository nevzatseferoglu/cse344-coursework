#include "arg.h"

/* server arguments */
unsigned int PORT = 0;
char* pathToLogFile = NULL;
unsigned int poolSize = 0;
char* datasetPath = NULL;

/* existing checking */
int pFnd = 0;
int oFnd = 0;
int lFnd = 0;
int dFnd = 0;

void x_argerror(char* argv[]);
int isNotaNumber(const char* str);
int isValidPort(unsigned int PORT_NUM, unsigned int lower, unsigned int upper);

void set_opts_and_args(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "p:o:l:d:")) != -1) {
        switch (opt)
        {
            case 'p':
                if (isNotaNumber(optarg)) x_argerror(argv);
                else PORT = atoi(optarg);

                if (!isValidPort(PORT, VALID_LOWER_PORT_LIMIT,
                 VALID_UPPER_PORT_LIMIT)) x_argerror(argv);
                pFnd = 1;
                break;

            case 'o':
                pathToLogFile = optarg;
                oFnd = 1;
                break;

            case 'l':
                if (isNotaNumber(optarg)) x_argerror(argv);
                else poolSize = atoi(optarg);
                if (poolSize < 2) x_argerror(argv);
                lFnd = 1;
                break;

            case 'd':
                datasetPath = optarg;
                dFnd = 1;
                break;

            default:
                x_argerror(argv);
                break;
        }
    }

    if (!pFnd || !oFnd || !lFnd || !dFnd)
        x_argerror(argv);
}

int isValidPort(unsigned int PORT_NUM, unsigned int lower, unsigned int upper) {
    return (PORT_NUM >= lower && PORT_NUM <= upper);
}

void x_argerror(char* argv[]) {
    fprintf(stderr, "Usage: "
                    "%s -p PORT -o pathToLogFile -l poolSize -d datasetpath\n\n"
                    "PORT: Communication endpoint, [%d-%d]\n"
                    "pathToLogFile: Log file of server\n"
                    "poolSize: The number of threads in the pool (>=2)\n"
                    "datasetPath: CSV file containing single table\n", *argv,
                                                VALID_LOWER_PORT_LIMIT,
                                                VALID_UPPER_PORT_LIMIT);
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
