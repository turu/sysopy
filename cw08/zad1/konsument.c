#include <math.h>
#include "commons.h"

int sem;
int shm;
void * mem;

void clean(int a) {
    shmdt(mem);
}

double computeDet(int n, double ** a) {
    int i, j, j1, j2;
    double det = 0;
    double ** m = NULL;

    if (n == 1) {
        det = a[0][0];
    } else if (n == 2) {
        det = a[0][0] * a[1][1] - a[1][0] * a[0][1];
    } else {
        det = 0;
        for (j1=0;j1<n;j1++) {
            m = malloc((n-1)*sizeof(double *));
            for (i=0;i<n-1;i++)
                m[i] = malloc((n-1)*sizeof(double));
            for (i=1;i<n;i++) {
                j2 = 0;
                for (j=0;j<n;j++) {
                    if (j == j1)
                        continue;
                    m[i-1][j2] = a[i][j];
                    j2++;
                }
            }
            det += pow(-1.0, 1.0+j1+1.0) * a[0][j1] * computeDet(n-1, m);
            for (i=0;i<n-1;i++)
                free(m[i]);
            free(m);
        }
    }
    return det;
}

void echoMat(double M[MAXMTX][MAXMTX]) {
    int i, j;
    for (i = 0; i < MAXMTX; i++) {
        for (j = 0; j < MAXMTX; j++) {
            printf("%lf ", M[i][j]);
        }
        printf("\n");
    }
}

void process(command e) {
    printf("-----------------------------------------------\n");
    echoMat(e.mat);
    int i;
    double ** temp = (double**) malloc(MAXMTX * sizeof(double*));
    for (i = 0; i < MAXMTX; i++) {
        temp[i] = (double*) malloc(MAXMTX * sizeof(double));
        memcpy(temp[i], e.mat[i], MAXMTX * sizeof(double));
    }
	double det = computeDet(MAXMTX, temp);

	printf("Zadanie otrzymalem, zadanie wykonalem. Wynik to %lf.\n", det);
	printf("-----------------------------------------------\n");
    for (i = 0; i < MAXMTX; i++) {
        free(temp[i]);
    }
    free(temp);

}

void konsumuj(int id) {
    command e;

	struct sembuf sb;
	sb.sem_flg = 0;

	int* begin_k = mem + sizeof(int);
	command * tab = mem + 2 * sizeof(int);

	if (id) {
		*begin_k = 0;
	}
	int licznik;

	while (1) {
		sb.sem_num = CONSUMER;
		sb.sem_op = DEC;

		semop(sem, &sb, 1);
		sb.sem_num = CTR;
		semop(sem, &sb, 1);

		licznik = *begin_k;
		(*begin_k) = (licznik + 1) % MAXTAB;

		e = tab[licznik];
		process(e);
		sb.sem_op = INC;
		semop(sem, &sb, 1);
		sb.sem_num = PRODUCENT;
		semop(sem, &sb, 1);
	}
}


int main(int argc, char ** argv) {
	key_t klucz;
	int id;

	signal(SIGINT, clean);

	klucz = ftok(KPATH, KVAL);

	if ((sem = semget(klucz, 3, IPC_CREAT | IPC_EXCL | S_IRWXU)) == -1) {
        id = 0;
        sem = semget(klucz, 3, 0);
	} else {
        semctl(sem, PRODUCENT, SETVAL, MAXTAB);
		semctl(sem, CONSUMER, SETVAL, 0);
		semctl(sem, CTR, SETVAL, 1);
	}

	if ((shm = shmget(klucz, MAXTAB * sizeof(command) + 2 * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRWXU)) == -1) {
        id = 0;
        if ((shm = shmget(klucz, MAXTAB * sizeof(command) + 2 * sizeof(int), 0)) == -1) {
            printf("%s\n", strerror(errno));
            return -1;
        }
	}

	mem = shmat(shm, NULL, 0);
	konsumuj(id);
	clean(0);

	return 0;
}
