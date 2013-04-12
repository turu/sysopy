#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

/*******************
PATH_TO_BROWSE=/home/ cos dopisac
export PATH_TO_BROWSE
EXTENSION_TO_BROWSE=.c
*/
/***** FUNKCJA *****/
int forkuj(int wpath, int wwait, char* path, char* ext, char* new_path) {
	DIR* dir = opendir(path);
	if(dir == NULL) {
		printf("%s\n", strerror(errno));
		return 0;
	}

	/***** ZMIENNE *****/
	char pth[MAXPATHLEN];
	struct dirent* dt;
	struct stat sbuf;
	int suma = 0;
	int sumaext = 0;
	int ile = 0;

	/* OBSLUGA WARUNKOW */
	while((dt = readdir(dir))) {
		if(strcmp(dt->d_name, ".") && strcmp(dt->d_name, "..")) {
			strcpy(pth, path);
			strcat(pth, "/");
			strcat(pth, dt->d_name);
			if(stat(pth, &sbuf) == -1) {
				printf("%s\n", strerror(errno));
				return suma;
			}

			if(!S_ISDIR(sbuf.st_mode)) {
				if(ext) {
					int l1 = strlen(ext);
					int l2 = strlen(dt->d_name);

					int flag = 0;
					int i;
					for(i = 0; i<l1; ++i) {
						if(ext[l1-i-1] != dt->d_name[l2-i-1]) {
							flag = 1;
							break;
						}
					}

					if(!flag)
						++suma;
				}
				else
					++suma;
			}
			else
			{
				++ile;
				int pid = fork();
				if(pid == -1)
				{
					printf("%s\n", strerror(errno));
					return suma;
				}

				if(!pid)
				{
					setenv("PATH_TO_BROWSE", pth, 1);
					setenv("PATH", new_path, 1);

					if(wpath && wwait)
						exit(execlp("policz", "policz", "-v", "-w", NULL));
					else if(wpath)
						exit(execlp("policz", "policz", "-v", NULL));
					else if(wwait)
						exit(execlp("policz", "policz", "-w", NULL));
					else
						exit(execlp("policz", "policz", NULL));
				}
			}
		}
	}

	if(wwait) // czekamy chwile
		sleep(15);

	int i;
	for(i=0; i<ile; ++i) { // zliczanie
		int st;
		wait(&st);
		sumaext += WEXITSTATUS(st);
	}

	if(wpath) { // wypis
		printf("%s %i %i\n", path, suma, suma+sumaext);
	}

	if(closedir(dir) == -1) { // blad
		printf("Zamkniecie katalogu nie powiodlo sie\n");
	}

	return suma+sumaext;
}

int main(int argc, char* argv[]) {
	/* Ustawienie zmiennych */
	const char* const short_opt = "vw";
	/* OPCJE */
	const struct option long_opt[] =
		{{"path", 0, NULL, 'v'},
		{"write", 0, NULL, 'w'},
		{NULL, 0, NULL, 0}};

	int next_opt;
	int wpath = 0, write = 0;
	while((next_opt = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
		switch(next_opt){
			case 'v': { // opcja v
				wpath = 1;
				break;
			}
			case 'w': { // opcja w
				write = 1;
				break;
			}
		}
	}

	int kat = 0;
	char* path = getenv("PATH_TO_BROWSE"); // PATH TO BROWSE
	if(path == NULL) {
		path = (char*)malloc(MAXPATHLEN*sizeof(char));

		if(path == NULL) { // blad
			printf("%s", strerror(errno));
			return 0;
		}

    		if(getcwd(path, MAXPATHLEN) == NULL) { // blad
			printf("%s\n", strerror(errno));
			return 0;
		}

		kat = 1;
	}

	char* ext = getenv("EXTENSION_TO_BROWSE"); // EXT TO BROWSE

	char* p;
	char* tmp = argv[0];
	if(tmp[0] == '.') { // katalog aktualny .
		char* dow = (char*)malloc(MAXPATHLEN*sizeof(char));
		if(getcwd(dow, MAXPATHLEN) == NULL) {
			printf("%s\n", strerror(errno));
			return 0;
		}

		p = getenv("PATH");
		strcat(p, ":");
		strcat(p, dow);

		int l = strlen(argv[0]);
		int j = 0;
		char d[2] = "";

		for(j = 1; j<l-6; ++j) {
			d[0] = tmp[j];
			strcat(p, d);
		}

		free(dow); // zwolnienie
	}
	else {
		p = getenv("PATH");
		strcat(p, ":");

		int l = strlen(argv[0]);
		int j = 0;
		char d[2] = "";

		for(j = 0; j<l-6; ++j)
		{
			d[0] = tmp[j];
			strcat(p, d);
		}
	}

	int suma = forkuj(wpath, write, path, ext, p); // wywolanie funkcji
						       // po zebraniu argumentow

	if(kat == 1)
		free(path); // zwolnienie

	exit(suma);
}
