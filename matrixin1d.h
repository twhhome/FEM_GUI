#ifndef MATRIXIN1D_H
#define MATRIXIN1D_H

#include <QVector>
//#include <assert.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

class MatrixIn1D
{
public:
    QVector<int> pDiag; // 主角元在一维储存中的编号
    QVector<double> data;

    MatrixIn1D() {}

    MatrixIn1D(QVector<int> &pDiag) {
        this->pDiag = pDiag;
        //this->data = Matrix<double>(1, this->pDiag(this->pDiag.m - 1) + 1);
        this->data = QVector<double>(this->pDiag.last() + 1);
    }

    double operator()(int i, int j);

    void set(int i, int j, double n);
};

QVector<double> cholesky(MatrixIn1D &A, QVector<double> &B);

#endif // MATRIXIN1D_H
