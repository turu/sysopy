#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int* locks;

void printHelp() {
}

void show_flock(struct flock* fl) {
    printf("FLOCK:\n\tfl->l_type: %d\n\tfl->l_whence: %d\n\tfl->l_start: %d\n\tfl->l_len: %d\n\tfl->l_pid: %d\n\n", fl->l_type, fl->l_whence, fl->l_start, fl->l_len, fl->l_pid);

}

int set_lock(int fd, short byte, int flag) {
    struct flock* fl = (struct flock*) malloc(sizeof(struct flock));

    if(flag == RDLK_FLAG) {
        fl->l_type = F_RDLCK;
    } else if(flag == WRLK_FLAG) {
        fl->l_type = F_WRLCK;
    } else if(flag == UNLOCK) {
        fl->l_type = F_UNLCK;
    } else {
        printf("Nieznany parametr typu rygla.\n");
        return -1;
    }

    fl->l_whence = SEEK_SET;
    fl->l_start = byte;
    fl->l_len = 1;

    if(fcntl(fd, F_SETLK, fl)==-1) {
        printf("NIEPOWODZENIE! Nie udalo sie utworzyc rygla.\n");
        return -1;
    }

    if(flag == UNLOCK) {
        locks[byte] = -1;
    } else {
        locks[byte] = flag;
    }

    flag==UNLOCK?printf("Rygiel zdjety z %d znaku pliku.\n", byte):printf("Rygiel do %s poprawnie utworzony na %d znaku pliku.\n", flag==RDLK_FLAG?"odczytu":"zapisu", byte);

    return 0;
}

int show_locked(int fd){
    int i = 0;
    int count = 0;
    int length = lseek(fd, 0, SEEK_END);
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_len = 1;
    for(i=0; i<length; i++){
        fl.l_type = F_WRLCK;
        fl.l_start = i;
        fcntl(fd, F_GETLK, &fl);
        if(fl.l_type!=2){
            printf("Znaleziono rygiel do odczytu na znaku %d\n", i);
            show_flock(&fl);
            count++;
        }
        fl.l_type = F_RDLCK;
        fcntl(fd, F_GETLK, &fl);
        if(fl.l_type!=2){
            printf("Znaleziono rygiel do zapisu na znaku %d\n", i);
            show_flock(&fl);
            count++;
        }
    }
    if(!count) printf("Nie znaleziono zadnych rygli innych procesow\n");
    count = 0;
    for(i=0; i<length; i++){
        if(locks[i]!=-1){
            if(!count) {
            printf("Znaleziono wlasne rygle:\n");
            count++;
            }
            printf("\tRygiel na pozycji %d - typ %d\n", i, locks[i]);
        }
    }
    if(!count) printf("Nie znaleziono wlasnych rygli.\n");

    return 0;
}

char read_char(int fd, int byte){
    struct flock fl;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = byte;
    fl.l_len = 1;

    if(fcntl(fd, F_SETLK, &fl)<0){
        printf("Znak zablokowany ryglem do zapisu.\n");
        return NULL;
    }

    lseek(fd, byte, SEEK_SET);
    char c;
    int count=read(fd, &c, sizeof(char));
    fl.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &fl);
    printf("%c\n", c);

    return 0;
}

int write_char(int fd, int byte, char c){
    int count;
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = byte;
    fl.l_len = 1;

    if(fcntl(fd, F_SETLK, &fl)<0){
        printf("Znak zablokowany ryglem przez inny proces.\n");
        return -1;
    }

    lseek(fd, byte, SEEK_SET);
    count=write(fd, &c, sizeof(char));
    fl.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &fl);

    return 0;
}

int main(int argc, char** argv){
    char *file, *cmd = (char*)malloc(sizeof(char)*256);
    char *type = malloc(sizeof(char)*256);

    int length;
    int fd, tmp, fd2;
    char c;

    if(argc<2){
        printHelp();
        return -1;
    }

    file = argv[1];

    if((fd=open(file, O_RDWR))==-1){
        printf("Nie udalo sie otworzyc pliku\n");
        return -2;
    }

    length = lseek(fd, 0, SEEK_END);
    locks = malloc(length*sizeof(int));

    for(tmp=0; tmp<length; tmp++)
        locks[tmp] = -1;

    printf("Wpisz help, aby uzyskac pomoc\n");
    while(strcmp(cmd, "koniec")) {
        scanf("%s", cmd);
        if(!strcmp(cmd, "help")) {
            printf("Dostepne polecenia:\n\tlock offset_w_pliku [READ | WRITE | UNLOCK] - blokuje/odblokowuje wybrany znak pliku\n\tlist - listuje dostepne rygle\n");
            printf("\tread numer_znaku - odczytuje znak z pliku spod pozycji okreslonej przez numer_znaku (jesli inny proces tego znaku nie blokuje).\n");
            printf("\twrite numer_znaku znak- zapisuje znak do pliku na pozycje okreslona przez numer_znaku (jesli inny proces tego znaku nie blokuje).\n");
            printf("\tkoniec - konczy program\n");
        } else if(!strcmp(cmd, "lock")) {
            scanf("%d", &tmp);
            scanf("%s", type);
            if(!strcmp(type, "READ")) set_lock(fd, tmp, RDLK_FLAG);
            else if(!strcmp(type, "WRITE")) set_lock(fd, tmp, WRLK_FLAG);
            else if(!strcmp(type, "UNLOCK")) set_lock(fd, tmp, UNLOCK);
            else printf("Zly typ rygla. Wpisz help, aby uzyskac pomoc.\n");
        } else if(!strcmp(cmd, "list")) {
            show_locked(fd);
        } else if(!strcmp(cmd, "read")) {
            scanf("%d", &tmp);
            read_char(fd, tmp);
        } else if(!strcmp(cmd, "write")) {
            scanf("%d", &tmp);
            scanf("%c", &c);
            scanf("%c", &c);
            write_char(fd, tmp, c);
        } else if (strcmp(cmd, "koniec")) {
            printf("Nieprawidlowe polecenie - wpisz help, aby uzyskac pomoc.\n");
        }
    }

    close(fd);
    return 0;
}
