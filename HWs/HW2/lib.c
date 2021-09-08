#include "lib.h"

char* isValidArgs (int argc, char* argv[]) {
    
    if (argc != 2) {
        return NULL;
    }
    return argv[1];
}

void setCoefs(double coefs[], double x[], double y[]){
    
    double newCoef[CHILD_PAIR_NUMBER-1];
    int startIndex;

    for(int i=0; i<(CHILD_PAIR_NUMBER-1); i++)
        coefs[i]=0;
    for (int i=0; i<(CHILD_PAIR_NUMBER-1); i++) {
        for (int nc=0; nc<(CHILD_PAIR_NUMBER-1); nc++) 
            newCoef[nc]=0;
        if (i>0) {
            newCoef[0]=-x[0]/(x[i]-x[0]);
            newCoef[1]=1/(x[i]-x[0]);
        } else {
            newCoef[0]=-x[1]/(x[i]-x[1]);
            newCoef[1]=1/(x[i]-x[1]);
        }
        startIndex=1;
        if (i == 0)
            startIndex=2;
        for(int n=startIndex; n<(CHILD_PAIR_NUMBER-1); n++){
            if(i==n)
                continue;
            for(int nc=6; nc>=1; nc--){
                newCoef[nc]= newCoef[nc]*(-x[n]/(x[i]-x[n]))+newCoef[nc-1]/(x[i]-x[n]);
            }
            newCoef[0]=newCoef[0]*(-x[n]/(x[i]-x[n])); 
        }
        for (int nc=0; nc<(CHILD_PAIR_NUMBER-1); nc++) coefs[nc]+=y[i]*newCoef[nc];
    }
}

int setRowPairs(FILE* fptr, int rowId, char line[]) {

    int count = 0, sline = 0;
    char myLine[MAXLINE];

    
    if (fptr == NULL || rowId < 0 || rowId > (CHILD_PAIR_NUMBER-1))
        return -1;

    strcpy(line, "\0");
    strcpy(myLine, "\0");

    while (sline != 8 && (fgets(myLine, MAXLINE, fptr) != NULL)) {
        if (count == rowId) {
            if (myLine[strlen(myLine)-1] == '\n')
                myLine[strlen(myLine)-1] = '\0';

            strcpy(line, myLine);
            break;
        }
        else
            ++count;
        ++sline;
    }
    return 1;
}

double Li(int i, int n, double x[], double X){
    int j;
    double prod=1;
    for(j=0;j<=n;j++) {
        if(j!=i)
            prod=prod*(X-x[j])/(x[i]-x[j]);
    }
        return prod;
}
 
double Pn(int n, double x[], double y[], double X){
    double sum=0;
    int i;
    for(i=0;i<=n;i++) {
        sum=sum+Li(i,n,x,X)*y[i];
    }
    return sum;
}

int childRound(FILE* fptr, int cindex, int round) {

    char line[POLY_NUMBER][MAXLINE];
    double x[CHILD_PAIR_NUMBER];
    double y[CHILD_PAIR_NUMBER];
    double coefs[CHILD_PAIR_NUMBER-1];
    struct flock flck;
    double lValue;
    int ret = 0;

    memset(&flck, 0, sizeof(flck));
    flck.l_type = F_WRLCK;
    fcntl(fileno(fptr), F_SETLKW, &flck);

    rewind(fptr);
    for (int t=0; t<POLY_NUMBER && !ret; ++t) {
        rewind(fptr);
        if(setRowPairs(fptr, t, line[t]) == -1)
            ret = -1;
    }

    sscanf(line[cindex],
            "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
            &x[0],&y[0],&x[1],&y[1],&x[2],&y[2],&x[3],&y[3],&x[4],&y[4],&x[5],
            &y[5],&x[6],&y[6],&x[7],&y[7]);


    if (round == 1) lValue = Pn(5, x, y, x[7]);
    else lValue = Pn(6, x, y, x[7]);

    if (round == 2) {
        setCoefs(coefs, x, y);
        fprintf(stdout, "Polynomial %d: ", cindex);
        for (int i=0; i<(CHILD_PAIR_NUMBER-1); ++i)
            if (i == (CHILD_PAIR_NUMBER-2))
                fprintf(stdout, "%.1lf", coefs[i]);
            else
                fprintf(stdout, "%.1lf,", coefs[i]);
        fprintf(stdout, "\n");
    }
    
    if (ret != -1) {
        rewind(fptr);
        for (int t=0; t<POLY_NUMBER; ++t) {
            if(cindex==t)
                fprintf(fptr, "%s,%.1lf", line[t], lValue);
            else
                fprintf(fptr, "%s", line[t]);
            fprintf(fptr, "\n");
        }
    }

    fflush(fptr);
    fflush(stdout);
    fflush(stderr);

    flck.l_type = F_UNLCK;
    fcntl(fileno(fptr), F_SETLKW, &flck);
    
    return ret;
}

void calcErrPrint(FILE* fptr, int round) {
    struct flock flck;
    char myLine[MAXLINE];
    double averageErr;
    double ry;
    double lValue;
    double tp;
    int sline;

    memset(&flck, 0, sizeof(flck));
    flck.l_type = F_WRLCK;
    fcntl(fileno(fptr), F_SETLKW, &flck);

    strcpy(myLine, "\0");

    sline = 0;
    averageErr = 0;
    rewind(fptr);
    while (sline != 8 && (fgets(myLine, MAXLINE, fptr) != NULL)) {
        if (myLine[strlen(myLine)-1] == '\n')
            myLine[strlen(myLine)-1] = '\0';
        
        if (round == 1) {
            sscanf(myLine,
            "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
            &tp,&tp,&tp,&tp,&tp,&tp,&tp,&tp,&tp,&tp,&tp,
            &tp,&tp,&tp,&tp,&ry,&lValue);
        } else {
            sscanf(myLine,
            "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
            &tp,&tp,&tp,&tp,&tp,&tp,&tp,&tp,&tp,&tp,&tp,
            &tp,&tp,&tp,&tp,&ry,&tp,&lValue);
        }
        
        averageErr += fabs((ry-lValue));
        ++sline;
    }
    averageErr /= (POLY_NUMBER);
    if (round == 1)
        fprintf(stdout, "Error of polynomial of degree 5: %.1lf\n", averageErr);
    else
        fprintf(stdout, "Error of polynomial of degree 6: %.1lf\n", averageErr);

    fflush(fptr);
    fflush(stdout);
    fflush(stderr);

    flck.l_type = F_UNLCK;
    fcntl(fileno(fptr), F_SETLKW, &flck);
}