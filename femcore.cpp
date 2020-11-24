#include "femcore.h"

double calLength(QPointF &p1, QPointF &p2) {
    QPointF delta = p1 - p2;
    return sqrt(QPointF::dotProduct(delta, delta));
}

void initRod(Rod &rod, QMap<int, Node> &nodes, int nodeNum1, int nodeNum2, double E, double A) {
    rod.nodeNum1 = nodeNum1;
    rod.nodeNum2 = nodeNum2;
    rod.E = E;
    rod.A = A;
    rod.L = calLength(nodes[nodeNum1].pos, nodes[nodeNum2].pos);
/*
    rod.TK.resize(2);
    rod.TK[0] = QVector<double>(2, 0);
    rod.TK[1] = QVector<double>(2, 0);
    rod.TK[0][0] = rod.TK[1][1] = E * A / rod.L;
    rod.TK[0][1] = rod.TK[1][0] = -E * A / rod.L;

    rod.T.resize(2);
    rod.T[0] = QVector<double>(4, 0);
    rod.T[1] = QVector<double>(4, 0);
    QPointF delta = (nodes[nodeNum2].pos - nodes[nodeNum1].pos) / rod.L;
    rod.T[0][0] = rod.T[1][2] = delta.x();
    rod.T[0][1] = rod.T[1][3] = delta.y();
*/
    Matrix<double> TK(2, 2), T(2, 4);
    TK(0, 0) = TK(1, 1) = E * A / rod.L;
    TK(0, 1) = TK(1, 0) = -E * A / rod.L;
    rod.TK = TK;

    QPointF delta = (nodes[nodeNum2].pos - nodes[nodeNum1].pos) / rod.L;
    T(0, 0) = T(1, 2) = delta.x();
    T(0, 1) = T(1, 3) = delta.y();
    rod.T = T;

}

void calStiffnessMatrix(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Rod> &rods, MatrixIn1D &K) {
    // calculate pDiag
    K.pDiag = QVector<int>(nodeDOF * nodes.size());
    K.pDiag[0] = 0;
    for (int i = 0; i < nodes.size(); i++) {
        int minIndex = std::numeric_limits<int>::max();
        QMap<int, Rod>::iterator rodIter = rods.begin();
        while(rodIter != rods.end()) {
            if(nodes.keys()[i] == rodIter.value().nodeNum1 || nodes.keys()[i] == rodIter.value().nodeNum2) {
                if(MIN(rodIter.value().nodeNum1, rodIter.value().nodeNum2) < minIndex)
                    minIndex = MIN(rodIter.value().nodeNum1, rodIter.value().nodeNum2);
            }
            rodIter++;
        }
        for (int j = 0; j < nodeDOF; j++) {
            if (i == 0 && j == 0)
                continue;
            int width = (i - nodes.keys().indexOf(minIndex)) * nodeDOF + j + 1;
            K.pDiag[i * nodeDOF + j] = K.pDiag[i * nodeDOF + j - 1] + width;
        }
    }
    K.data = QVector<double>(K.pDiag.last() + 1);

    QMap<int, Rod>::iterator iter = rods.begin();
    while(iter != rods.end()) {
        Matrix<double> temp = iter.value().T.trans() * iter.value().TK;
        Matrix<double> Ke = temp * iter.value().T;

        for (int j = 0; j < Ke.n; j++) {
            for (int k = 0; k < Ke.m; k++) {
                int Kj, Kk;
                Kj = nodeDOF * (((j < nodeDOF) ? iter.value().nodeNum1 : iter.value().nodeNum2) - 1) + j % nodeDOF;
                Kk = nodeDOF * (((k < nodeDOF) ? iter.value().nodeNum1 : iter.value().nodeNum2) - 1) + k % nodeDOF;
                if (Kj < Kk)
                    continue;
                int index = K.pDiag[Kj] - (Kj - Kk);
                K.data[index] += Ke(j, k);
            }
        }
        iter++;
    }
}

void processConstraints(QMap<int, Node> &nodes, QMap<int, Constraint> &constraints, MatrixIn1D &K) {
    QMap<int, Constraint>::iterator iter = constraints.begin();
    while(iter != constraints.end()) {
        K.set(nodes.keys().indexOf(iter.value().node) * 2 + iter.value().dir, nodes.keys().indexOf(iter.value().node) * 2 + iter.value().dir, std::numeric_limits<double>::max());
        iter++;
    }
}

void solveEqns(QMap<int, Node> &nodes, MatrixIn1D &K, QMap<int, Load> &loads) {
    QVector<double> p(K.pDiag.size(), 0);
    QMap<int, Load>::iterator loadIter = loads.begin();
    while(loadIter != loads.end()) {
        p[nodes.keys().indexOf(loadIter.value().node) * 2 + loadIter.value().dir] = loadIter.value().f;
        loadIter++;
    }

    QVector<double> d = cholesky(K, p);

    for(int i = 0; i < nodes.size(); i++) {
        nodes[nodes.keys()[i]].displacement = QVector<double>();
        nodes[nodes.keys()[i]].displacement << d[i * 2] << d[i * 2 + 1];
    }
}

void calRod(int nodeDOF, QMap<int, Node> &nodes, Rod &rod) {
    Matrix<double> de(2 * nodeDOF, 1);
    for (int j = 0; j < nodeDOF; j++) {
        de(j) = nodes[rod.nodeNum1].displacement[j];
        de(j + nodeDOF) = nodes[rod.nodeNum2].displacement[j];
    }
    rod.de = de;
    rod.dee = rod.T * de;
    rod.fee = rod.TK * rod.dee;
    rod.fe = rod.T.trans() * rod.fee;
    rod.IF = rod.fee(1);
    rod.stress = rod.IF / rod.A;
}

void calRods(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Rod> &rods) {
    QMap<int, Rod>::iterator iter = rods.begin();
    while(iter != rods.end()) {
        Matrix<double> de(2 * nodeDOF, 1);
        for (int j = 0; j < nodeDOF; j++) {
            de(j) = nodes[iter.value().nodeNum1].displacement[j];
            de(j + nodeDOF) = nodes[iter.value().nodeNum2].displacement[j];
        }
        iter.value().de = de;
        iter.value().dee = iter.value().T * de;
        iter.value().fee = iter.value().TK * iter.value().dee;
        iter.value().fe = iter.value().T.trans() * iter.value().fee;
        iter.value().IF = iter.value().fee(1);
        iter.value().stress = iter.value().IF / iter.value().A;
        iter++;
    }
}
/*
void calConstraintForce(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Rod> &rods, Constraint &constraint) {

}
*/
void calConstraintForce(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Rod> &rods, QMap<int, Constraint> &constraints) {
    QVector<double> F(nodes.size() * nodeDOF, 0);
    QMap<int, Rod>::iterator rodIter = rods.begin();
    while(rodIter != rods.end()) {
        for (int j = 0; j < nodeDOF; j++) {
            F[nodes.keys().indexOf(rodIter.value().nodeNum1) * nodeDOF + j] += rodIter.value().fe(j);
            F[nodes.keys().indexOf(rodIter.value().nodeNum2) * nodeDOF + j] += rodIter.value().fe(j + nodeDOF);
        }
        rodIter++;
    }

    QMap<int, Constraint>::iterator constraintIter = constraints.begin();
    while(constraintIter != constraints.end()) {
        constraintIter.value().force = F[nodes.keys().indexOf(constraintIter.value().node) * nodeDOF + constraintIter.value().dir];
        constraintIter++;
    }
}
