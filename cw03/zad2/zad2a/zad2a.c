#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>

void printHelp() {
    printf("Program computes total size of regular files in a given directory \
and it's subdirectories.\nUsage:\n-d <path to a directory>\n");
}

int retrieveFileSize(char * filepath) {
    //TODO: retrieving file size
}

int isNotNavi(char * name) {
    if (!strcmp(name, ".") || !strcmp(name, "..")) {
        return 0;
    }
    //printf("name %s is not navi\n", name);
    return 1;
}

char * composePath(char * dirpath, char * name, char * retPath) {
    int dirpathSize = strlen(dirpath);
    strcpy(retPath, dirpath);
    if (dirpath[dirpathSize-1] != '/') {
        retPath[dirpathSize] = '/';
        retPath[dirpathSize+1] = '\0';
    }
    strcat(retPath, name);
    //printf("\tComposed path = %s\n", retPath);

    return retPath;
}

int dirSize(char * dirpath) {
    int size = 0;
    char composedPath[100];

    DIR * dir = opendir(dirpath);
    if (dir == NULL) {
        printf("Could not open dir: %s !!!\n", dirpath);
        return 0;
    }

    printf("Entering %s: \n", dirpath);

    struct dirent * entry;

    while ((entry = readdir(dir)) != NULL) {
        //printf("dirent name=%s, type=%d\n", entry->d_name, entry->d_type);
        if (entry->d_type == DT_DIR && isNotNavi(entry->d_name)) {
            size += dirSize(composePath(dirpath, entry->d_name, composedPath));
        } else if (entry->d_type == DT_REG) {
            size += retrieveFileSize(composePath(dirpath, entry->d_name, composedPath));
        }
    }

    printf("Dir %s processed.\n", dirpath);

    return size;
}

int main(int argc, char ** argv) {
    char c;
    char * optvalue;
    char dirpath[50];
    char pathgiven = 0;

    while ((c = getopt(argc, argv, "d:")) != -1) {
        switch(c) {
            case 'd':
                optvalue = optarg;
                if (optvalue != 0) {
                    strcpy(dirpath, optvalue);
                    pathgiven = 1;
                }
            break;
            default:
                printHelp();
                return 0;
            break;
        }
    }

    if (!pathgiven) {
        printHelp();
        return 0;
    }

    int totalSize = dirSize(dirpath);

    printf("Total size of regular files in %s directory and it's subdirectories is %dB\n",
           dirpath, totalSize);

    return 0;
}
