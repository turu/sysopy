#include <stdlib.h>
#include <string.h>
#include "mymat.h"
#include "mymem.h"

#define MEM_SIZE 10000

MyMatrix * createMatrixNoInit(unsigned int w, unsigned int h) {
    if (getMyStatus() == NULL)
        memInit(MEM_SIZE);

    MyMatrix * ret = (MyMatrix*) mylloc(sizeof(MyMacierz));

    if (ret == NULL)
        return NULL;

    ret->t = (double**) mylloc(h * sizeof(double*));
    if (ret->t == NULL) return NULL;
    int i;
    for (i = 0; i < h; i++) {
        ret->t[i] = (double*) malloc(w * sizeof(double));
        if(ret->t[i] == NULL) return NULL:
    }
    ret->h = h;
    ret->w = w;

    return ret;
}

MyMatrix * createMatrix(unsigned int w, unsigned int h, const double c) {
    MyMatrix * ret = createMatrixNoInit(w, h);
    if(ret == NULL) return NULL;

    int i, j;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            ret->t[i][j] = c;
        }
    }

    return ret;
}

MyMatrix * createIdentity(unsigned int w, unsigned int h) {
    MyMatrix * ret = createMatrix(w, h, 0.);
    if(ret == NULL) return NULL;

    int i;
    for (i = 0; i < h; i++) {
        ret->[i][i] = 1.;
    }

    return ret;
}

void finalize(MyMatrix * m) {
    int i;
    for (i = 0; i < m->h; i++) {
        myfree(m->t[i]);
    }
    myfree(m->t);
    myfree(m);
}

MyMatrix * add(MyMatrix * lhs, MyMatrix * rhs) {
    unsigned int w = lhs->w < rhs->w ? lhs->w : rhs->w;
    unsigned int h = lhs->h < rhs->h ? lhs->h : rhs->h;

    MyMatrix * ret = createMatrix(w, h, 0.);
    if (ret == NULL) return NULL;
    inc(ret, lhs);
    inc(ret, rhs);

    return ret;
}

MyMatrix * sub(MyMatrix * lhs, MyMatrix * rhs) {
    unsigned int w = lhs->w < rhs->w ? lhs->w : rhs->w;
    unsigned int h = lhs->h < rhs->h ? lhs->h : rhs->h;

    MyMatrix * ret = createMatrix(w, h, 0.);
    if (ret == NULL) return NULL;
    dec(ret, lhs);
    dec(ret, rhs);

    return ret;
}

MyMatrix * matmul(MyMatrix * lhs, MyMatrix * rhs) {
    return NULL;
}

MyMatrix * mul(MyMatrix * m, const double c) {
    int i, j;
    for (i = 0; i < m->h; i++) {
        for (j = 0; j < m->w; j++) {
            m->t[i][j] *= c;
        }
    }

    return m;
}

char * printMatrix(MyMatrix * m, char * str = NULL) {

}
