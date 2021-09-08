#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <signal.h>
#include "getopt.h"

#define MAX_LINE 256

typedef enum file_type{socket, symlink, regfile, blck_dev, dir_t, char_dev, pipe, none} file_type;

typedef struct FD {
    file_type type;
    char perm[10];
} FD;

typedef struct Tree {
    char filename[MAX_LINE];
    struct Tree* next;
    struct Tree* child;
} Tree;

char** split(char*, char, int*);
char* substr(const char*, int, int);
void freepath(char**, int);
void traverse(Tree*, int);
int add(Tree**, char**, int, int);
void freeTree(Tree*);
bool is_char_equal(const char, const char);
bool is_valid_regex(const char*, const char*);
int traverse_filesystem(char[], FD, Tree**);
void set_type(FD*, const struct stat*);
void set_perm(FD*, const struct stat*);
bool validate(const FD*, const struct stat*, const char*);
void handle_signal();

sig_atomic_t curSignal = 0;

const char* ftable[] = {"s", "l", "f", "b", "d", "c", "p", "-"};

int main(int argc, char *argv[])
{
    struct sigaction sa;
    Tree* root = NULL, *t = NULL;
    char** initPath = NULL;
    int initSize = 0, i =0;
    FD cur;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_signal;
    sigaction(SIGINT, &sa, NULL);

    if (set_opts_and_args(argc, argv))
        return -1;

    initPath = split(spath, '/', &initSize);
    freepath(initPath, initSize);

    if(traverse_filesystem(spath, cur, &root)) {
        freeTree(root);
        if (curSignal)
           fprintf(stderr, "Exiting program with CTRL+C\n");
        return -1;
    }

    t = root;
    if(t != NULL) {
        for(i=0; i<initSize; ++i)
            t = t->child;
        
        fprintf(stdout, "%s\n",spath);
        if(t != NULL)
            traverse(t, 1);
    }
    else 
        fprintf(stdout, "No file found.\n");
    
    freeTree(root);
    return 0;
}

void handle_signal() {
    ++curSignal;
}

int traverse_filesystem(char direntName[], FD cur ,Tree** root) {
    
    /* directory stream pointer */
    DIR *dir = NULL;

    /* directory structure pointer */
    struct dirent *dirent = NULL;

    /* inode content of file */
    struct stat sb;

    /* current path */
    char *path = NULL;

    /* current path array constructed according to given delimeter */
    char** path_arr = NULL;

    /* keep the file size in current path */
    int size;

    /* readdir error detection */
    int save_errno;

    save_errno = errno;

    dir = opendir(direntName);
    if (!dir) {
        perror("opendir");
        return -1;
    }

    while ((dirent = readdir(dir))) {
        
        if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
            continue;

        path = (char*)malloc(sizeof(char) * (strlen(direntName) + strlen(dirent->d_name) + 2));
        if (!path) {
            fprintf(stderr, "traverse_filesystem: allocation error\n");
            if(closedir(dir))
                perror("closedir");
            return -1;
        }

        strcpy(path, direntName);
        strcat(path, "/");
        strcat(path, dirent->d_name);
        strcat(path, "\0");

        if(lstat(path, &sb)) {
            perror("lstat");
            free(path);
            if(closedir(dir))
                perror("closedir");
            return -1;
        }
        
        set_type(&cur, &sb);
        if (cur.type == none) {
            fprintf(stderr, "traverse_filesystem: internal error has been detected\n");
            free(path);
            if(closedir(dir))
                perror("closedir");
            return -1;
        }

        set_perm(&cur, &sb);

        if(validate(&cur, &sb, dirent->d_name)) {
            size = 0;
            path_arr = split(path, '/', &size);

            if(path_arr == NULL) {
                fprintf(stderr, "traverse_filesystem: path cannot be splitted\n");
                free(path);
                if(closedir(dir))
                    perror("closedir");
                return -1;
            }

            if(add(root, path_arr, 0, size)) {
                fprintf(stderr, "traverse_filesystem: path cannot be added to tree\n");
                free(path);
                freepath(path_arr, size);
                if(closedir(dir))
                    perror("closedir");
                return -1;
            }
            freepath(path_arr, size);
        }


        if (curSignal) {
            free(path);
            if(closedir(dir))
                perror("closedir");
            return -1;
        }
        

        if(cur.type == dir_t)
            traverse_filesystem(path, cur, root);

        free(path);
    }

    if(save_errno != errno && errno != EACCES) {
        perror("traverse_filesystem");
        if(closedir(dir))
           perror("closedir");
        return -1;
    }

    if(closedir(dir)) {
        perror("closedir");
        return -1;
    }

    return 0;
}

bool validate(const FD* cur, const struct stat* sb, const char* pname) {
    bool name = false, size = false, type = false;
    bool perm = false , nlink = false;

    if (!ffnd) name = true;
    else name = (is_valid_regex(pname, fname) ? true : false);

    if (!bfnd) size = true;
    else size = (((int)sb->st_size) == fsize ? true : false);

    if (!tfnd) type = true;
    else type = (strcmp(ftype, ftable[cur->type]) ? false : true);

    if (!pfnd) perm = true;
    else perm = (strcmp(fprmns, cur->perm) ? true : false);

    if (!lfnd) nlink = true;
    else nlink = (((int)sb->st_nlink) == fnlinks ? true : false);
    
    return name && size && type && perm && nlink;
}

void set_perm(FD* cur, const struct stat* sb) {

    memset(cur->perm, 0, sizeof(cur->perm));

    (cur->perm)[0] = ((sb->st_mode) & S_IRUSR) ? 'r' : '-';
    (cur->perm)[1] = ((sb->st_mode) & S_IWUSR) ? 'w' : '-';
    (cur->perm)[2] = ((sb->st_mode) & S_IXUSR) ? 'x' : '-';

    (cur->perm)[3] = ((sb->st_mode) & S_IRGRP) ? 'r' : '-';
    (cur->perm)[4] = ((sb->st_mode) & S_IWGRP) ? 'w' : '-';
    (cur->perm)[5] = ((sb->st_mode) & S_IXGRP) ? 'x' : '-';

    (cur->perm)[6] = ((sb->st_mode) & S_IROTH) ? 'r' : '-';
    (cur->perm)[7] = ((sb->st_mode) & S_IWOTH) ? 'w' : '-';
    (cur->perm)[8] = ((sb->st_mode) & S_IXOTH) ? 'x' : '-';
}

void set_type(FD* cur, const struct stat* sb) {

    switch ((sb->st_mode) & S_IFMT)
    {
        case S_IFSOCK:
            cur->type = socket;
            break;
        
        case S_IFLNK:
            cur->type = symlink;
            break;
        
        case S_IFREG:
            cur->type = regfile;
            break;

        case S_IFBLK:
            cur->type = blck_dev;
            break;
        
        case S_IFDIR:
            cur->type = dir_t;
            break;
        
        case S_IFCHR:
            cur->type = char_dev;
            break;

        case S_IFIFO:
            cur->type = pipe;
            break;
        
        default:
            cur->type = none;
            break;
    }
}

bool is_char_equal(const char c1, const char c2) {
    if ((c1 - c2 == ('a' - 'A')) && ('a' <= c1 && 'z' >= c1))
        return true;
    else if((c2 - c1 == ('a' - 'A')) && ('a' <= c2 && 'z' >= c2))
        return true;
    return  c1 == c2;
}

bool is_valid_regex(const char* filen, const char* fregex) {

    char prev;
    int r = 0;

    if (filen == NULL || fregex == NULL)
        return -1;

    for ( ; *fregex; ) {
        if(is_char_equal(*filen, *fregex)) {
            r = 0;
            prev = *filen;
            ++fregex;
            ++filen; 
        }
        else if(*fregex == '+' && (is_char_equal(*filen, prev))) {
                ++r;
                ++filen;
        }
        else if(*fregex == '+') {
            ++fregex;
            while(is_char_equal(*fregex, prev)) {
                --r;
                ++fregex;
            }
            if (r < 0)
                return false;
        }
        else if(!is_char_equal(*filen, *fregex))
            return false;
    }

    return *filen ? false : true;
}

void freeTree(Tree* root) {

    Tree* temp = NULL;

    while(root) {
        freeTree(root->child);
        temp = root;
        root = root->next;
        free(temp);
    }

    temp = NULL;
}

void traverse(Tree* root, int depth) {
    int i = 0;
    while(root) {
        for(i=0; i<depth*2 ;++i) {
            if (i == 0)
                fprintf(stdout, "|-");
            else
                fprintf(stdout, "-");
        }
        fprintf(stdout, "%s\n", root->filename);
        traverse(root->child, depth+1);
        root = root->next;
    }
}

int add(Tree** root, char** path, int cur, int path_size) {

    Tree* temp = NULL, *temp1=NULL ,*temp2=NULL;

    if(cur < path_size) {
        if ((*root) == NULL) {

            temp1 = *root;
            (*root) = realloc((*root), sizeof(Tree)*1);

            if(*root == NULL) {
                fprintf(stderr, "add: allocation error\n");
                freeTree(temp1);
                return -1;
            }

            strcpy((*root)->filename ,path[cur]);
            (*root)->next = NULL;
            (*root)->child = NULL;
            add(&((*root)->child), path, cur+1, path_size);
            return 0;
        }

        temp = *root;
        if(!strcmp((*root)->filename, path[cur])) {
            add(&((*root)->child), path, cur+1, path_size);
            return 0;
        }
        else {
            while(temp->next != NULL) {
                if(!strcmp(temp->next->filename, path[cur])) {
                    add(&(temp->next->child), path, cur+1, path_size);
                    return 0;
                }
                temp = temp->next;
            }
        }

        temp2 = temp->next;
        (temp->next) = realloc((temp->next), sizeof(Tree)*1);

        if((temp->next) == NULL) {
            fprintf(stderr, "add: allocation error\n");
            freeTree(temp2);
            return -1;
        }
        
        strcpy((temp->next)->filename ,path[cur]);
        (temp->next)->next = NULL;
        (temp->next)->child = NULL;
        add(&(temp->next->child), path, cur+1, path_size);
    }

    temp = NULL;
    temp1 = NULL;
    temp2 = NULL;
    return 0;
}

void freepath(char **path, int size) {

    int i = 0;
    for(i = 0; i<size; ++i) {
        free(*(path+i));
        *(path+i) = NULL;
    }
    free(path);
    path = NULL;
}

char** split(char *str, char c, int *size) {

    char **path = NULL , **temp = NULL;
    const char *save=str;
    int from=-1, i = 0;

    if(str == NULL)
        return NULL;

    for(i=0 ; 1; ++i) {
        if(*str == '\0') {
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
            break;
        }
        if(*str != c) {
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

char* substr(const char *src, int m, int n)
{
    int len = n - m;
    char *dest = (char*)malloc(sizeof(char) * (len + 1));

    if(dest == NULL) {
        fprintf(stderr, "substr: allocation error\n");
        return NULL;
    }

    for (int i = m; i < n && (*(src + i) != '\0'); i++) {
        *dest = *(src + i);
        ++dest;
    }

    *dest = '\0';
    return dest - len;
}