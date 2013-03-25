#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include "mymat.h"
#include "mymem.h"

MyMatrix * createMatrixNoInit(unsigned int w, unsigned int h) {
    if (getMyStatus() == NULL)
        memInit(MEM_SIZE);

    MyMatrix * ret = (MyMatrix*) mylloc(sizeof(MyMatrix));

    if (ret == NULL)
        return NULL;

    ret->t = (double**) mylloc(h * sizeof(double*));
    if (ret->t == NULL) return NULL;
    int i;
    for (i = 0; i < h; i++) {
        ret->t[i] = (double*) malloc(w * sizeof(double));
        if(ret->t[i] == NULL) return NULL;
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
        ret->t[i][i] = 1.;
    }

    return ret;
}

void finalizeMatrix(MyMatrix * m) {
    int i;
    for (i = 0; i < m->h; i++) {
        myfree(m->t[i]);
    }
    myfree(m->t);
    myfree(m);
}

void inc(MyMatrix * lhs, MyMatrix * rhs) {
    if (lhs->h != rhs->h || lhs->w != rhs->w) return;

    int i, j;
    for (i = 0; i < lhs->h; i++) {
        for (j = 0; j < lhs->w; j++) {
            lhs->t[i][j] += rhs->t[i][j];
        }
    }
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

void dec(MyMatrix * lhs, MyMatrix * rhs) {
    if (lhs->h != rhs->h || lhs->w != rhs->w) return;

    int i, j;
    for (i = 0; i < lhs->h; i++) {
        for (j = 0; j < lhs->w; j++) {
            lhs->t[i][j] -= rhs->t[i][j];
        }
    }
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
    if (lhs->w != rhs->h) return NULL;
    MyMatrix * ret = createMatrix(lhs->h, rhs->w, 0.);
    if (ret == NULL) return NULL;

    int i,j,k;
    for (i = 0; i < ret->h; i++) {
        for (j = 0; j < ret->w; j++) {
            for (k = 0; k < lhs->w; k++) {
                ret->t[i][j] += lhs->t[i][k] * rhs->t[k][j];
            }
        }
    }

    return ret;
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

void printMatrix(MyMatrix * m) {
    printf("ADDR = %d; H = %d; W = %d\n*************************************************\n", m, m->h, m->w);

    int i, j;
    for (i = 0; i < m->h; i++) {
        printf("[ ");
        for (j = 0; j < m->w - 1; j++) {
            printf("%lf, ", m->t[i][j]);
        }
        printf("%lf ", m->t[i][j]);
        printf("]\n");
    }
}
