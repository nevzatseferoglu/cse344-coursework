#include "arg.h"

/* arguments */
int id = -1;
char* IPv4 = NULL;
unsigned int PORT = -1;
char* pathToQueryFile = NULL;

int iFnd = 0;
int aFnd = 0;
int pFnd = 0;
int oFnd = 0;

void x_argerror(char* argv[]);
int isNotaNumber(const char* str);
int isValidPort(unsigned int PORT_NUM, unsigned int lower, unsigned int upper);

void set_opts_and_args(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "i:a:p:o:")) != -1) {
        switch (opt)
        {
            case 'i':
                if (isNotaNumber(optarg)) x_argerror(argv);
                else id = atoi(optarg);
                iFnd = 1;
                break;

            case 'a':
                IPv4 = optarg;
                aFnd = 1;
                break;

            case 'p':
                if (isNotaNumber(optarg)) x_argerror(argv);
                else PORT = atoi(optarg);

                if (!isValidPort(PORT, VALID_LOWER_PORT_LIMIT,
                 VALID_UPPER_PORT_LIMIT)) x_argerror(argv);
                pFnd = 1;
                break;

            case 'o':
                pathToQueryFile = optarg;
                oFnd = 1;
                break;
            
            default:
                x_argerror(argv);
                break;
        }
    }

    if (!iFnd || !aFnd || !pFnd || !oFnd)
        x_argerror(argv);
}

int isValidPort(unsigned int PORT_NUM, unsigned int lower, unsigned int upper) {
    return (PORT_NUM >= lower && PORT_NUM <= upper);
}

void x_argerror(char* argv[]) {
    fprintf(stderr, "Usage: "
                    "%s -i id -a 127.0.0.1 -p PORT -o pathToQueryFile\n\n"
                    "id: Integer id of the client\n"
                    "127.0.0.1: IPv4 address of the machine running the server\n"
                    "PORT: Communication endpoint, [%d-%d]\n"
                    "pathToQueryFile: File containing an arbitrary number of queries\n", *argv,
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
