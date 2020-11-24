#include "matrixin1d.h"

double MatrixIn1D::operator()(int i, int j) {
    int x = MAX(i, j);
    int y = MIN(i, j);
    if (x == 0 && y == 0)
        return data[0];
    if (x - (pDiag[x] - pDiag[x - 1]) + 1 > y)
        return 0;
    else
        return data[y - x + pDiag[x]];
}

void MatrixIn1D::set(int i, int j, double n) {
    int x = MAX(i, j);
    int y = MIN(i, j);
    if (x == 0 && y == 0)
        data[0] = n;
    else if (x - (pDiag[x] - pDiag[x - 1]) + 1 > y)
        return;
    else
        data[y - x + pDiag[x]] = n;
}

QVector<double> cholesky(MatrixIn1D &A, QVector<double> &B) {
    //assert(A.pDiag.size() == B.size());

    MatrixIn1D l(A.pDiag);

    int n = A.pDiag.size();

    QVector<double> d(n, 0);

    d[0] = A(0, 0);
    l.set(0, 0, 1);

    for (int i = 1; i < n; i++) {
        for (int j = i - (l.pDiag[i] - l.pDiag[i - 1]) + 1; j <= i - 1; j++) {
            double temp = 0;
            if (j != 0) {
                for (int k = MAX(i - (l.pDiag[i] - l.pDiag[i - 1]) + 1, j - (l.pDiag[j] - l.pDiag[j - 1]) + 1); k <= j - 1; k++) {
                    temp += l(i, k) * d[k] * l(j, k);
                }
            }
            l.set(i, j, (A(i, j) - temp) / d[j]);
        }
        l.set(i, i, 1);
        double temp = 0;
        for (int k = i - (l.pDiag[i] - l.pDiag[i - 1]) + 1; k <= i - 1; k++) {
            temp += l(i, k) * l(i, k) * d[k];
        }
        d[i] = A(i, i) - temp;
    }

    QVector<double> x(n, 0), y(n, 0);
    y[0] = B[0];
    for (int i = 1; i < n; i++) {
        double temp = 0;
        for (int k = i - (l.pDiag[i] - l.pDiag[i - 1]) + 1; k <= i - 1; k++) {
            temp += l(i, k) * y[k];
        }
        y[i] = B[i] - temp;
    }

    x[n - 1] = y[n - 1] / d[n - 1];
    for (int i = n - 2; i >= 0; i--) {
        double temp = 0;
        for (int k = i + 1; k <= n - 1; k++) {
            temp += l(k, i) * x[k];
        }
        x[i] = y[i] / d[i] - temp;
    }

    return x;
}
