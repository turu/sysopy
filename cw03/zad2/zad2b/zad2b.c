#define _XOPEN_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <ftw.h>
#include <time.h>

time_t argTime;
char compare = 0;

void printHelp() {
    printf("Program displays absolute paths to files from a given directory, which \
last modified date matches provided cryteria.\nUsage:\n-p <path to a directory>\n\
-c >|=|<\n-d YYYY-MM-DD_hh:mm:ss\n");
}

int visitor(const char *file, const struct stat *sb, int flag) {
    if (flag == FTW_F) {
        printf("Processing file %s %d\n", file, sb->st_mtime);
        if ((compare == '<' && sb->st_mtime < argTime) ||
            (compare == '=' && sb->st_mtime == argTime) ||
            (compare == '>' && sb->st_mtime > argTime)) {
            printf("%s\n", file);
        }
    }

    return 0;
}

int main(int argc, char ** argv) {
    char c;
    char * optvalue;
    char dirpath[50];
    char date[50];
    char abspath[50];
    char pathgiven = 0, dategiven = 0;

    while ((c = getopt(argc, argv, "p:c:d:")) != -1) {
        switch(c) {
            case 'p':
                optvalue = optarg;
                if (optvalue != 0) {
                    strcpy(dirpath, optvalue);
                    pathgiven = 1;
                }
            break;
            case 'c':
                optvalue = optarg;
                if (optvalue != 0) {
                    compare = optvalue[0];
                }
            break;
            case 'd':
                optvalue = optarg;
                if (optvalue != 0) {
                    strcpy(date, optvalue);
                    dategiven = 1;
                }
            break;
            default:
                printHelp();
                return 0;
            break;
        }
    }

    if (!pathgiven || !dategiven || (compare != '<' && compare != '>' && compare != '=')) {
        printHelp();
        return 0;
    }

    struct tm * tm = (struct tm*) malloc(sizeof(struct tm));

    if (strptime(date, "%Y-%m-%d_%T", tm) == NULL) {
        printf("Could not process date. Check if it conforms to the required format.\n");
        free(tm);
        return 0;
    }

    argTime = mktime(tm);

    printf("%c %d\n", compare, argTime);

    realpath(dirpath, abspath);

    if (ftw(abspath, visitor, 10) != 0) {
        printf("Could not scan entire file tree!\n");
    }

    return 0;
}
