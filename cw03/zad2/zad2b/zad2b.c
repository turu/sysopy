#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

void printHelp() {
    printf("Program displays absolute paths to files from a given directory, which \
last modified date matches provided cryteria.\nUsage:\n-p <path to a directory>\n\
-c >|=|<\n-d YYYY/MM/DD_hh:mm:ss\n");
}

int main(int argc, char ** argv) {
    char c;
    char * optvalue;
    char dirpath[50];
    char date[50];
    char pathgiven = 0, dategiven = 0;
    char compare = 0;

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

    return 0;
}
