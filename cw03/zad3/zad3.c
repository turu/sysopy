#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int* locks;

void printHelp() {
    printf("Argumenty: nazwa_pliku\n");
    printf("Polecenia w programie:\n\tlock offset_w_pliku READ|WRITE|UNLOCK - zaklada/zdejmuje rygiel z zadanego znaku w pliku\n\tlist - listuje aktywne rygle\n");
    printf("\tread offset_w_pliku - o ile jest to mozliwe, odczytuje zadany znak z pliku.\n");
    printf("\twrite offset_w_pliku - o ile jest to możliwe, zapisuje zadan.\n");
    printf("\texit - wyjscie\n");
}

void printFlock(struct flock * fl) {
    printf("FLOCK:\n\tfl->l_type: %d\n\tfl->l_whence: %d\n\tfl->l_start: %d\n\tfl->l_len: %d\n\tfl->l_pid: %d\n\n", (int)fl->l_type, (int)fl->l_whence, (int)fl->l_start, (int)fl->l_len, (int)fl->l_pid);
}

int setLock(int fd, int id, int flag) {
    struct flock* fl = (struct flock*) malloc(sizeof(struct flock));

    fl->l_type = flag;
    fl->l_whence = SEEK_SET;
    fl->l_start = id;
    fl->l_len = 1;
    locks[id] = flag;

    if (fcntl(fd, F_SETLK, fl) == -1) {
        printf("Nie udalo sie utworzyc rygla na zadanym znaku!\n");
        free(fl);
        return -1;
    }

    if (flag == F_UNLCK) {
        printf("Rygiel zdjety z %d znaku\n", id);
    } else {
        printf("Rygiel zalozony na %d znak. ", id);
        if (flag == F_WRLCK) printf("Rygiel do zapisu\n");
        else printf("Rygiel do odczytu\n");
    }

    free(fl);
    return 0;
}

int showLocked(int fd){
    int i = 0;
    int count = 0;
    int length = lseek(fd, 0, SEEK_END);
    struct flock fl;

    fl.l_whence = SEEK_SET;
    fl.l_len = 1;

    for (i = 0; i < length; i++) {
        fl.l_type = F_WRLCK;
        fl.l_start = i;
        fcntl(fd, F_GETLK, &fl);

        if (fl.l_type != F_UNLCK) {
            printf("Znaleziono rygiel do odczytu na znaku %d\n", i);
            printFlock(&fl);
            count++;
        }

        fl.l_type = F_RDLCK;
        fcntl(fd, F_GETLK, &fl);

        if (fl.l_type != F_UNLCK) {
            printf("Znaleziono rygiel do zapisu na znaku %d\n", i);
            printFlock(&fl);
            count++;
        }
    }

    count = 0;
    for (i = 0; i < length; i++) {
        if (locks[i] != F_UNLCK) {
            if (!count) {
                printf("Znaleziono wlasne rygle:\n");
                count++;
            }
            printf("\tRygiel na pozycji %d - typ %d\n", i, locks[i]);
        }
    }

    return 0;
}

char readChar(int fd, int id){
    struct flock fl;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = id;
    fl.l_len = 1;

    if(fcntl(fd, F_GETLK, &fl) < 0 || fl.l_type != F_UNLCK) {
        printf("Znak zablokowany przez inny proces.\n");
        return 0;
    }

    lseek(fd, id, SEEK_SET);
    char c;
    read(fd, &c, sizeof(char));
    printf("%c\n", c);

    return 0;
}

int writeChar(int fd, int id, char c){
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = id;
    fl.l_len = 1;

    if(fcntl(fd, F_GETLK, &fl) < 0 || fl.l_type != F_UNLCK) {
        printf("Znak zablokowany przez inny proces.\n");
        return -1;
    }

    lseek(fd, id, SEEK_SET);
    write(fd, &c, sizeof(char));

    return 0;
}

int main(int argc, char** argv){
    char * file;
    char cmd[100];
    char type[100];
    int length;
    int fd;
    int id;
    char c;

    if(argc < 2) {
        printHelp();
        return -1;
    }

    file = argv[1];

    if ((fd = open(file, O_RDWR)) == -1) {
        printf("Nie udalo sie otworzyc pliku\n");
        return -2;
    }

    length = lseek(fd, 0, SEEK_END);
    locks = malloc(length * sizeof(int));

    for(id = 0; id < length; id++)
        locks[id] = F_UNLCK;

    printf("Wpisz 'help', aby wyswietlic pomoc:\n");
    while (strcmp(cmd, "exit")) {
        scanf("%s", cmd);
        if (!strcmp(cmd, "help")) {
            printHelp();
        } else if(!strcmp(cmd, "lock")) {
            scanf("%d %s", &id, type);
            if (!strcmp(type, "READ")) setLock(fd, id, F_RDLCK);
            else if (!strcmp(type, "WRITE")) setLock(fd, id, F_WRLCK);
            else if (!strcmp(type, "UNLOCK")) setLock(fd, id, F_UNLCK);
            else printf("Zly typ rygla. Wpisz help, aby uzyskac pomoc.\n");
        } else if (!strcmp(cmd, "list")) {
            showLocked(fd);
        } else if (!strcmp(cmd, "read")) {
            scanf("%d", &id);
            readChar(fd, id);
        } else if (!strcmp(cmd, "write")) {
            scanf("%d %c", &id, &c);
            writeChar(fd, id, c);
        } else if (strcmp(cmd, "exit")) {
            printf("Nieprawidlowe polecenie. Wpisz help, aby uzyskac pomoc.\n");
        }
    }

    close(fd);
    return 0;
}
