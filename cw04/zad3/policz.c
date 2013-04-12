#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <getopt.h>

int sleepy = 0, verbose = 1;
char ** exts = 0;

void printHelp() {
    printf("Arguments:\n-v force verbose\n-w enable sleepy mode in which processes go to sleep for 15 seconds before\
collecting children's output\n");
}

char ** str_split(char * a_str) {
    char ** result = 0;
    size_t count = 0;
    char * tmp = a_str;
    char * last_comma = 0;

    while (*tmp) {
        if (*tmp == ':') {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    count += last_comma < (a_str + strlen(a_str) - 1);
    count++;

    result = malloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char * token = strtok(a_str, ":");

        while (token)
        {
            //printf("%s\n", token);
            *(result + idx++) = strdup(token);
            token = strtok(0, ":");
        }
        *(result + idx) = 0;
    }

    return result;
}

void processExtensions() {
    char * extVar = getenv("EXT_TO_BROWSE");
    //printf("%s\n", extVar);

    if (extVar && *extVar) {
        exts = str_split(extVar);
    }
}

int doesApply(char * filename) {
    if (exts) {
        int i;
        for (i = 0; *(exts + i); i++) {
            if (strstr(filename, *(exts + i)) != NULL)
                return 1;
        }
    } else return 1;

    return 0;
}

void processDirectory(char ** argv) {
    struct dirent * entry;
    char newPath[100];
    int ret = 0, children = 0, i, self = 0;

    char * path = getenv("PATH_TO_BROWSE");
    if (!path || !(*path)) {
        path = ".";
    }

    DIR * dir = opendir(path);

    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_DIR && strcmp(".", entry->d_name) && strcmp("..", entry->d_name)) {
            sprintf(newPath, "%s/%s", path, entry->d_name);
            children++;

            int childPID = fork();
            if (childPID == 0) {
                setenv("PATH_TO_BROWSE", newPath, 1);
                execv(argv[0], argv);
            }
        } else if (entry->d_type == DT_REG) {
            if (doesApply(entry->d_name)) ret++;
        }
    }
    if (sleepy)
        sleep(15);
    closedir(dir);
    self = ret;

    for (i = 0; i < children; i++) {
        int status = -1;
        wait(&status);
        ret += WEXITSTATUS(status);
    }

    if (verbose) {
        printf("Path: %s;  Files: %d; Files in total: %d\n", path, self, ret);
    }

    exit(ret);
}

int main(int argc, char** argv) {
    int c;
    while ((c = getopt(argc, argv, "vw")) != -1) {
        switch (c) {
            case 'v':
                verbose = 1;
            break;
            case 'w':
                sleepy = 1;
            break;
            default:
                printf("Bad arguments\n");
                printHelp();
                return 1;
        }
    }

    processExtensions();
    /*if (exts) {
        int i;
        for (i = 0; *(exts + i); i++) {
            printf("%s\n", *(exts + i));
        }
    }*/
    processDirectory(argv);

    if (exts) {
        int i;
        for (i = 0; *(exts + i); i++) {
            free(*(exts + i));
        }
    }
    free(exts);

    return 0;
}
