#ifndef _mymat_h_
#define _mymat_h_

typedef struct {
    double ** t;
    unsigned int h;
    unsigned int w;
} MyMatrix;


MyMatrix * createMatrixNoInit(unsigned int w, unsigned int h);

MyMatrix * createMatrix(unsigned int w, unsigned int h, const double c);

MyMatrix * createIdentity(unsigned int w, unsigned int h);

void finalizeMatrix(MyMatrix * m);

void inc(MyMatrix * lhs, MyMatrix * rhs);

MyMatrix * add(MyMatrix * lhs, MyMatrix * rhs);

void dec(MyMatrix * lhs, MyMatrix * rhs);

MyMatrix * sub(MyMatrix * lhs, MyMatrix * rhs);

MyMatrix * matmul(MyMatrix * lhs, MyMatrix * rhs);

MyMatrix * mul(MyMatrix * m, const double c);

void printMatrix(MyMatrix * m);

#endif
