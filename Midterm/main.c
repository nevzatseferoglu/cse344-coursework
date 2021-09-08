
#include <stdio.h>
#include "lib.h"
#include "getopt.h"

Buffer* gbuffer = NULL;
File* gfile = NULL;

void safe_exit_parent();
void safe_exit_vaccinators();
void safe_exit_nurses();
void safe_exit_citizens();

pid_t* allocActors(int);
Buffer* allocBuffSharedMem();
File* allocFptrSharedMem();
void createProcess(void (*)(), int);
void nurseChild();
void vaccinatorChild();
void citizenChild();
void bringVaccine();
int isThereAnyDose();

void handler_SIGINT() {
    gbuffer->sigint = 1;
}

int main(int argc, char *argv[]) {
    
    struct sigaction sa_SIGINT;
    set_opts_and_args(argc, argv);

    fprintf(stdout, "Welcome to the GTU344 clinic. "
    "Number of citizens to vaccinate c=%d with t=%d doses.\n",
    numOfCitizens, numOfDose);

    gbuffer = allocBuffSharedMem();
    gfile = allocFptrSharedMem();

    memset(&sa_SIGINT, 0, sizeof(sa_SIGINT));
    sa_SIGINT.sa_handler = handler_SIGINT;
    x_sigaction(SIGINT, &sa_SIGINT, NULL);

    gfile->fd = x_open(inputFile, O_RDONLY, 0666);
    
    createProcess(nurseChild, numOfNurses);
    createProcess(vaccinatorChild, numOfVaccinators);
    createProcess(citizenChild, numOfCitizens);
    
    if (atexit(safe_exit_parent)) {
        fprintf(stderr, "Cannot set exit function (Parent)\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    while(wait(NULL) != -1);
    if (errno != ECHILD) {
        x_error(__func__, "Child process cannot be released to system properly");
    }

    if (gbuffer->sigint == 1) {
        fprintf(stdout, "\nCTRL+C is pressed.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "The clinic is now closed. Stay healthy.\n");
    exit (EXIT_SUCCESS);
}

void nurseChild() {

    if (atexit(safe_exit_nurses)) {
        fprintf(stderr, "Cannot set exit function (Nurses)\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    while (TRUE) {

        if (gbuffer->sigint == 1) {
            exit(EXIT_FAILURE);
        }

        x_sem_wait(&(gbuffer->empty));

        if (gbuffer->sigint == 1) {
            x_sem_post(&(gbuffer->empty));
            exit(EXIT_FAILURE);
        }

        x_sem_wait(&(gbuffer->m));

        if (gbuffer->sigint == 1) {
            x_sem_post(&(gbuffer->m));
            x_sem_post(&(gbuffer->empty));
            exit(EXIT_FAILURE);
        }
        
        if (gbuffer->fNurse == -1)
            gbuffer->fNurse = (long) getpid();
            
        bringVaccine();

        if (gbuffer->sigint == 1) {
            x_sem_post(&(gbuffer->m));
            x_sem_post(&(gbuffer->empty));
            exit(EXIT_FAILURE);
        }

        if (gfile->isEof) {
            ++gbuffer->nurseTermination;
            if (gbuffer->nurseTermination == numOfNurses)
                fprintf(stdout, "Nurses have carried all vaccines to the buffer, terminating.\n");
            x_sem_post(&(gbuffer->m));
            x_sem_post(&(gbuffer->full));
            break;
        }

        if (gfile->c != '1' && gfile->c != '2') {
            x_sem_post(&(gbuffer->m));
            x_sem_post(&(gbuffer->empty));
            continue;
        }

        if (gfile->c == '1') {
            ++(gbuffer->firstDose);
            fprintf(stdout, "Nurse %2d (pid=%ld) has brought "
            "vacinne 1: the clinic has %2d vaccine1 and %2d vacinne 2.\n",
            ((int)(((long)getpid())-(gbuffer->fNurse)+1)), (long)getpid(),
            gbuffer->firstDose, gbuffer->secDose);
        }
        else if (gfile->c == '2') {
            ++(gbuffer->secDose);
            fprintf(stdout, "Nurse %2d (pid=%ld) has brought "
            "vacinne 2: the clinic has %2d vaccine1 and %2d vacinne 2.\n",
            ((int)(((long)getpid())-(gbuffer->fNurse)+1)), (long)getpid(),
            gbuffer->firstDose, gbuffer->secDose);
        }
        
        x_sem_post(&(gbuffer->m));
        x_sem_post(&(gbuffer->full));
    }
    
    exit(EXIT_SUCCESS);
}

void vaccinatorChild() {

    int appliedDoses = 0;

    if (atexit(safe_exit_vaccinators)) {
        fprintf(stderr, "Cannot set exit function (Vaccinator)\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    while (TRUE) {

        if (gbuffer->sigint == 1) {
            exit(EXIT_FAILURE);
        }

        x_sem_wait(&(gbuffer->mConsumer));

        if (gbuffer->sigint == 1) {
            x_sem_post(&(gbuffer->mConsumer));
            exit(EXIT_FAILURE);
        }

        x_sem_wait(&(gbuffer->full));

        if (gbuffer->sigint == 1) {
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->mConsumer));
            exit(EXIT_FAILURE);
        }

        x_sem_wait(&(gbuffer->full));

        if (gbuffer->sigint == 1) {
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->mConsumer));
            exit(EXIT_FAILURE);
        }

        x_sem_wait(&(gbuffer->m));

        if (gbuffer->sigint == 1) {
            x_sem_wait(&(gbuffer->m));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->mConsumer));
            exit(EXIT_FAILURE);
        }

        if (gbuffer->fVaccinator == -1)
            gbuffer->fVaccinator = (long) getpid();

        if (gbuffer->totalAppliedDose == numOfCitizens*numOfDose) {
            fprintf(stdout, "Vaccinator %d (pid=%ld) vaccinated %d doses. ",
            ((int)(((long)getpid())-(gbuffer->fVaccinator)+1)),
            ((long)getpid()),
            appliedDoses);
            x_sem_post(&(gbuffer->m));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->mConsumer));
            break;
        }

        if (!isThereAnyDose()) {
            x_sem_post(&(gbuffer->m));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->mConsumer));
            continue;
        }

        if (gbuffer->sigint == 1) {
            x_sem_post(&(gbuffer->m));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->mConsumer));
            exit(EXIT_FAILURE);
        }

        x_sem_wait(&(gbuffer->mCitizen));

        if (gbuffer->sigint == 1) {
            x_sem_post(&(gbuffer->mCitizen));
            x_sem_post(&(gbuffer->m));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->full));
            x_sem_post(&(gbuffer->mConsumer));
            exit(EXIT_FAILURE);
        }

        x_sem_wait(&(gbuffer->mSafe));

        --(gbuffer->firstDose);
        --(gbuffer->secDose);
        ++appliedDoses;
        ++(gbuffer->totalAppliedDose);
        fprintf(stdout, "Vaccinator %d (pid=%ld) is inviting citizen pid=%ld to the clinic\n",
            ((int)(((long)getpid())-(gbuffer->fVaccinator)+1)),
            ((long)getpid()),
            (gbuffer->currCitizen));
        x_sem_post(&(gbuffer->mSafe));

        x_sem_post(&(gbuffer->m));
        x_sem_post(&(gbuffer->empty));
        x_sem_post(&(gbuffer->empty));
        x_sem_post(&(gbuffer->mConsumer));
    }

    exit(EXIT_SUCCESS);
}

void citizenChild() {
    if (atexit(safe_exit_citizens)) {
        fprintf(stderr, "Cannot set exit function (Cizitens)\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    int i=numOfDose;

    while(i) {
        
        if (gbuffer->sigint == 1) {
            exit(EXIT_FAILURE);
        }

        if (gbuffer->fCitizen == -1)
            gbuffer->fCitizen = (long) getpid();

        x_sem_wait(&(gbuffer->mSafe));
        x_sem_post(&(gbuffer->mCitizen));

        if (gbuffer->sigint == 1) {
            x_sem_post(&(gbuffer->mSafe));
            exit(EXIT_FAILURE);
        }

        gbuffer->currCitizen = (long)getpid();
        fprintf(stdout, "Citizen %d (pid=%ld) is vaccinated for the %dst time:"
        " the clinic has %d vaccine1 and %d vaccine2\n",
        ((int)(((long)getpid())-(gbuffer->fCitizen)+1)),
        (long)(getpid()),
        (numOfDose-i+1),
        gbuffer->firstDose,
        gbuffer->secDose);
        x_sem_post(&(gbuffer->mSafe));
        
        if (gbuffer->sigint == 1) {
            exit(EXIT_FAILURE);
        }
        --i;
    }
    
    if (gbuffer->sigint == 1) {
        exit(EXIT_FAILURE);
    }

    x_sem_wait(&(gbuffer->mSafe));
    ++(gbuffer->deadCitizen);

    if (gbuffer->sigint == 1) {
        x_sem_post(&(gbuffer->mSafe));
        exit(EXIT_FAILURE);
    }

    if (gbuffer->deadCitizen == numOfCitizens) {
        fprintf(stdout, "All citizens have been vaccinated.\n");
    }
    else {
        fprintf(stdout, "citizen is leaving."
        " Remaining citizens to vaccinate: %d\n",(numOfCitizens-(gbuffer->deadCitizen)));
    }
    x_sem_post(&(gbuffer->mSafe));

    exit(EXIT_SUCCESS);
}

int isThereAnyDose() {
    return ((gbuffer->firstDose)>0) && ((gbuffer->secDose)>0);
}

void bringVaccine() {   
    int retByte;
    if (!gfile->isEof) {
        retByte = x_read(gfile->fd, &(gfile->c), 1);
        if (retByte == 0)
            gfile->isEof = 1;
    }
}

void createProcess(void (*myChild)(), int count) {
    for (int i=0; i<count; ++i)
        if (x_fork() == 0)
            myChild();
}

File* allocFptrSharedMem() {
    int fd_file;
    struct stat stat;
    File* file;

    fd_file = x_shm_open("File");
    x_fstat(fd_file, &stat);
    x_ftruncate(fd_file, sizeof(File));
    
    file = (File*)x_mmap(NULL, sizeof(File),
            PROT_READ | PROT_WRITE,
            MAP_SHARED, fd_file, 0);
    
    memset(file, 0, sizeof(File));
    return file;
}

Buffer* allocBuffSharedMem() {

    int fd_buff;
    struct stat stat;
    Buffer* buffer;

    fd_buff = x_shm_open("Buffer");
    x_fstat(fd_buff, &stat);
    x_ftruncate(fd_buff, sizeof(Buffer));
    
    buffer = (Buffer*)x_mmap(NULL, (sizeof(Buffer)),
            PROT_READ | PROT_WRITE,
            MAP_SHARED, fd_buff, 0);
    
    buffer->sigint = 0;
    buffer->totalAppliedDose = 0;
    buffer->deadCitizen = 0;
    buffer->currCitizen = -1;
    buffer->nurseTermination = 0;
    buffer->fNurse = -1;
    buffer->fVaccinator = -1;
    buffer->fCitizen = -1;

    buffer->firstDose = 0;
    buffer->secDose = 0;

    x_sem_init(&(buffer->empty), 1, sizeOftheBuffer);
    x_sem_init(&(buffer->full), 1, 0);
    x_sem_init(&(buffer->m), 1, 1);
    x_sem_init(&(buffer->mConsumer), 1, 1);
    x_sem_init(&(buffer->mCitizen), 1, 0);
    x_sem_init(&(buffer->mSafe), 1, 1);
    
    return buffer;
}

void safe_exit_nurses() { /* Intentionally left blank */ }
void safe_exit_vaccinators() { /* Intentionally left blank */ }
void safe_exit_citizens() { /* Intentionally left blank */ }

void safe_exit_parent() {

    if (gfile != NULL) {
        x_close(gfile->fd);
        x_shm_unlink("File");
    }

    if (gbuffer != NULL) {
        x_sem_destroy(&(gbuffer->mSafe));
        x_sem_destroy(&(gbuffer->mConsumer));
        x_sem_destroy(&(gbuffer->mCitizen));
        x_sem_destroy(&(gbuffer->empty));
        x_sem_destroy(&(gbuffer->full));
        x_sem_destroy(&(gbuffer->m));
        x_shm_unlink("Buffer");
    }
}