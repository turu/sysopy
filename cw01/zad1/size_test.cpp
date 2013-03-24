/**
    Poniższy program pozwala w bardzo zoptymalizowany sposób rozkładać zbiór liczb na czynniki pierwsze.
    Jako wejście przyjmuje ilość liczb do wczytania (n), a następnie w n liniach liczby do rozłożenia.
    Algorytm jest zoptymalizowany pod przetwarzanie dużej ilości liczb. Zoptymalizowano również wejście
    i wyjście. Powyższe w połączeniu z dużym zapotrzebowaniem algorytmu na pamięć sprawia, że jest on idealny
    do sprawdzenia działania flagi -Os.

    Kod jest moim autorskim kodem, rozwiązaniem zadania z platformy contestowej SPOJ.
*/

#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <cmath>

#define SQRTMAX 2828
#define SSQRTMAX 53
#define PMAX 410
#define MAX 8000000
#define QMAX 1000000
#define BPOINT 35000

using namespace std;

int lpp = 0, lqp = 0, lsp = 0;
int primes[PMAX];
bool S[SQRTMAX + 1];
char primess[PMAX][20];
int primele[PMAX];

short int * Fs[MAX + 1];
short int * Ts[QMAX];
int iQs[MAX + 1];
int Qs[QMAX];
int Ss[QMAX];
short int lcps[MAX + 1];
short int ltps[QMAX];
bool occ[MAX + 1];

struct INTERVAL{
	int beg;
	int end;
	INTERVAL(){beg=end=0;}
};

vector<INTERVAL> intervals;

int k;

void sGen(){ //generuje tablice licz p. w postaci stringow oraz zapisuje ich dlugosci
	int i=0;
	for(; i < lpp; i++){
		primele[i] = sprintf(primess[i], "%d", primes[i]);
	}
}

void Init(){
	int i=0, j=0, i2=0;
	for(i = 3; i <= SSQRTMAX; i += 2){
		if(!S[i]){
			i2 = i<<1;
			for(j = i * i; j <= SQRTMAX; j += i2){
				S[j] = true;
			}
		}
	}
	primes[lpp++] = 2;
	for(i = 3; i <= SQRTMAX; i += 2){
		if(!S[i]){
			primes[lpp++]=i;
		}
	}
	Fs[1] = (short int *) malloc(2);
	Fs[1][0] = 0;
	lcps[1] = 1;
	sGen();
}

int count(int p, int & n){
	int res = 0;
	div_t q;
	q = div(n, p);
	while(!q.rem){
		n = q.quot;
		q = div(n, p);
		res++;
	}
	return res;
}

void attach(int p, int c, int n){
	while(c--){
		Fs[n][lcps[n]++] = p;
	}
}

void attach2(int c, int i){
	/*ltps[i] = 0;
	Ts[i] = (short int *) malloc(c * 2);
	while(c--){
		Ts[i][ltps[i]++] = 0;
	}*/
	ltps[i] = 0;
	Ts[i] = (short int *) calloc(c, 2);
	ltps[i] += c;
}

void Preprocess(int n, int i){
	int res = 0, cn = n;
	while(!(cn&1)){ //liczymy ile 2 w rozkladzie n
		cn >>= 1;
		res++;
	}
	if(cn == 1 && n != 1){ //n jest potega 2
		Qs[lqp++] = n;
		if(!occ[n]){
			Fs[n] = (short int *) malloc(60);
			attach(0, res, n);
		}
		occ[n] = true;
	}else if(n == 1){
		Qs[lqp++] = 1;
		occ[n] = true;
	}else{ //nie rozlozylismy calej wiec przekazujemy ja dalej
		if(!occ[cn]){
			Fs[cn] = (short int *) malloc(60);
			lcps[cn] = 0;
		}
		if(res)attach2(res, i);
		Qs[lqp++] = Ss[lsp++] = cn;
		iQs[cn] = cn;
		occ[cn] = true;
	}
}

void Partition(){
	intervals.clear();
	intervals.push_back(INTERVAL());
	int i=1, c=0;
	intervals[0].beg = Ss[0];
	for(; i<lsp; i++){
		if(Ss[i] - Ss[i - 1] >= BPOINT){
			intervals[c].end = Ss[i - 1];
			c++;
			intervals.push_back(INTERVAL());
			intervals[c].beg = Ss[i];
		}
	}
	intervals[c].end = Ss[lsp - 1];
}

void sieve(int m, int n){
	int r=0, sqrtn=0, d=0, p2, s=0, i=0;
	sqrtn = (int)sqrt((double)n);
	div_t q;
	while(i < lpp && primes[i] <= sqrtn){
		q = div(m, primes[i]);
		d = q.quot;
		r = q.rem;
		if(r != 0){
			d++;
			s = m + (primes[i] - r);
		}else s = m;
		if(!(d&1)){
			d++;
			s += primes[i];
		}
		if(s < m){i++; continue;}
		p2 = primes[i]<<1;
		for(; s <= n; s += p2){
			if(iQs[s]){
				attach(i, count(primes[i], iQs[s]), s);
			}
		}
		i++;
	}
}

void Solve(){
	int n = intervals.size(), i=0;
	for(; i < n; i++){
		//printf("%d %d\n", intervals[i].beg, intervals[i].end);
		sieve(intervals[i].beg, intervals[i].end);
	}
}

void Postprocess(){
	int i = 0, l, bpt, ttt=0, j;
	char * last, * buf;
	last = (char *) malloc(30);
	buf = (char *) malloc(15000000);
	for(; i < lqp; i++){
		if(Qs[i] == 1){
			memcpy(buf + ttt, "1\n", 2);
			ttt += 2;
			continue;
		}
		bpt = 0; j = 0;
		if(ltps[i] > 0){
			memcpy(buf + ttt, primess[Ts[i][j]], primele[Ts[i][j]]);
			bpt += primele[Ts[i][j]];
			j++;
			for(; j < ltps[i]; j++){
				buf[ttt + (bpt++)] = '*';
				memcpy(buf + ttt + bpt, primess[Ts[i][j]], primele[Ts[i][j]]);
				bpt += primele[Ts[i][j]];
			}
		}
		j = 0;
		if(bpt == 0 && lcps[Qs[i]] > 0){
			memcpy(buf + ttt, primess[Fs[Qs[i]][j]], primele[Fs[Qs[i]][j]]);
			bpt += primele[Fs[Qs[i]][j]];
			j++;
		}
		for(; j < lcps[Qs[i]]; j++){
			buf[ttt + (bpt++)] = '*';
			memcpy(buf + ttt + bpt, primess[Fs[Qs[i]][j]], primele[Fs[Qs[i]][j]]);
			bpt += primele[Fs[Qs[i]][j]];
		}
		if(iQs[Qs[i]] > 1){
			l = sprintf(last, "%d", iQs[Qs[i]]);
			if(bpt != 0){
				buf[ttt + (bpt++)] = '*';
			}
			memcpy(buf + ttt + bpt, last, l);
			bpt += l;
		}
		buf[ttt + (bpt++)] = '\n';
		ttt += bpt;
		//printf("%s\n", buf);
	}
	fwrite_unlocked(buf, 1, ttt, stdout);
	free(buf);
}

int main(){
	char * buf = (char *) malloc(10000000), * pt;
	int n, i = 0;
	Init();
	fread_unlocked(buf, 1, 10000000, stdin);
	k = atoi(buf);
	pt = (char *) memchr(buf, '\n', 50);
	i=0;
	do{
		n = atoi(pt + 1);
		Preprocess(n, i);
		i++;
		if(i == k)break;
	}while((pt = (char *) memchr(pt + 1, '\n', 50)) != NULL);
	free(buf);
	sort(Ss, Ss+lsp);
	Partition();
	Solve();
	Postprocess();
	return 0;
}
