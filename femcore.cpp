#include "femcore.h"

double calLength(QPointF &p1, QPointF &p2) {
    QPointF delta = p1 - p2;
    return sqrt(QPointF::dotProduct(delta, delta));
}
/*
void initRod(Element &rod, QMap<int, Node> &nodes, int nodeNum1, int nodeNum2, double E, double A) {
    rod.nodeNum1 = nodeNum1;
    rod.nodeNum2 = nodeNum2;
    rod.E = E;
    rod.A = A;
    rod.L = calLength(nodes[nodeNum1].pos, nodes[nodeNum2].pos);

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

    Matrix<double> TK(2, 2), T(2, 4);
    TK(0, 0) = TK(1, 1) = E * A / rod.L;
    TK(0, 1) = TK(1, 0) = -E * A / rod.L;
    rod.Kee = TK;

    QPointF delta = (nodes[nodeNum2].pos - nodes[nodeNum1].pos) / rod.L;
    T(0, 0) = T(1, 2) = delta.x();
    T(0, 1) = T(1, 3) = delta.y();
    rod.T = T;

}
*/
void initElement(Element &element, int elementType, QMap<int, Node> &nodes, QMap<int, Material> &materials, QMap<int, Section> &sections, QMap<int, Offset> &offsets) {
    if(elementType == 1) {
        double E, A, L;
        E = materials[element.materialNum].E;
        A = sections[element.sectionNum].A;

        QPointF node1 = nodes[element.nodeNum1].pos;
        QPointF node2 = nodes[element.nodeNum2].pos;
        L = calLength(node1, node2);

        Matrix<double> TK(2, 2), T(2, 4);
        TK(0, 0) = TK(1, 1) = E * A / L;
        TK(0, 1) = TK(1, 0) = -E * A / L;

        QPointF delta = (node2 - node1) / L;
        T(0, 0) = T(1, 2) = delta.x();
        T(0, 1) = T(1, 3) = delta.y();

        element.L = L;
        element.Kee = TK;
        element.T = T;
    } else if(elementType == 2) {
        double E, poisson, G, A, Iz, Ay, K, a1, b1, a2, b2, L;
        E = materials[element.materialNum].E;
        poisson = materials[element.materialNum].poisson;
        G = E / (2 * (1 + poisson));

        A = sections[element.sectionNum].A;
        Iz = sections[element.sectionNum].Iz;
        Ay = sections[element.sectionNum].Ay;
        K = sections[element.sectionNum].K;

        if(element.offsetNum == 0) {
            a1 = b1 = a2 = b2 = 0;
        } else {
            a1 = offsets[element.offsetNum].a1;
            b1 = offsets[element.offsetNum].b1;
            a2 = offsets[element.offsetNum].a2;
            b2 = offsets[element.offsetNum].b2;
        }

        QPointF node1 = nodes[element.nodeNum1].pos;
        QPointF node2 = nodes[element.nodeNum2].pos;
        L = calLength(node1, node2) + a1 - a2;

        double by = 12 * K * E * Iz / G / Ay / L / L;

        Matrix<double> TK(6, 6);
        TK(0, 0) = TK(3, 3) = E * A / L;
        TK(1, 1) = TK(4, 4) = 12 * E * Iz / (1 + by) / L / L / L;
        TK(2, 2) = TK(5, 5) = (4 + by) * E * Iz / (1 + by) / L;
        TK(2, 1) = TK(1, 2) = TK(5, 1) = TK(1, 5) = 6 * E * Iz / (1 + by) / L / L;
        TK(3, 0) = TK(0, 3) = -E * A / L;
        TK(4, 1) = TK(1, 4) = -12 * E * Iz / (1 + by) / L / L / L;
        TK(4, 2) = TK(2, 4) = TK(5, 4) = TK(4, 5) = -6 * E * Iz / (1 + by) / L / L;
        TK(5, 2) = TK(2, 5) = (2 - by) * E * Iz / (1 + by) / L;

        Matrix<double> TT(6, 6);
        for (int j = 0; j < 6; j++)
            TT(j, j) = 1;
        TT(1, 2) = -a1;
        TT(0, 2) = b1;
        TT(4, 5) = -a2;
        TT(3, 5) = b2;

        Matrix<double> T(6, 6);
        QPointF delta = (node2 - node1) / L;
        for (int j = 0; j < 2; j++) {
            T(0 + 3 * j, 0 + 3 * j) = delta.x();
            T(0 + 3 * j, 1 + 3 * j) = delta.y();
            T(1 + 3 * j, 0 + 3 * j) = -delta.y();
            T(1 + 3 * j, 1 + 3 * j) = delta.x();
            T(2 + 3 * j, 2 + 3 * j) = 1;
        }

        Matrix<double> Kee = TT.trans() * TK * TT;

        element.L = L;
        element.Kee = Kee;
        element.T = T;
        element.TT = TT;
    }
}

void calStiffnessMatrix(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Element> &elements, MatrixIn1D &K) {
    // calculate pDiag
    K.pDiag = QVector<int>(nodeDOF * nodes.size());
    K.pDiag[0] = 0;
    for (int i = 0; i < nodes.size(); i++) {
        int minIndex = std::numeric_limits<int>::max();
        QMap<int, Element>::iterator elementIter = elements.begin();
        while(elementIter != elements.end()) {
            if(nodes.keys()[i] == elementIter.value().nodeNum1 || nodes.keys()[i] == elementIter.value().nodeNum2) {
                if(MIN(elementIter.value().nodeNum1, elementIter.value().nodeNum2) < minIndex)
                    minIndex = MIN(elementIter.value().nodeNum1, elementIter.value().nodeNum2);
            }
            elementIter++;
        }
        for (int j = 0; j < nodeDOF; j++) {
            if (i == 0 && j == 0)
                continue;
            int width = (i - nodes.keys().indexOf(minIndex)) * nodeDOF + j + 1;
            K.pDiag[i * nodeDOF + j] = K.pDiag[i * nodeDOF + j - 1] + width;
        }
    }
    K.data = QVector<double>(K.pDiag.last() + 1);

    QMap<int, Element>::iterator iter = elements.begin();
    while(iter != elements.end()) {
        Matrix<double> temp = iter.value().T.trans() * iter.value().Kee;
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

void processConstraints(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Constraint> &constraints, MatrixIn1D &K) {
    QMap<int, Constraint>::iterator iter = constraints.begin();
    while(iter != constraints.end()) {
        K.set(nodes.keys().indexOf(iter.value().node) * nodeDOF + iter.value().dir, nodes.keys().indexOf(iter.value().node) * nodeDOF + iter.value().dir, std::numeric_limits<double>::max());
        iter++;
    }
}

void solveEqns(int nodeDOF, QMap<int, Node> &nodes, MatrixIn1D &K, QMap<int, Load> &loads) {
    QVector<double> p(K.pDiag.size(), 0);
    QMap<int, Load>::iterator loadIter = loads.begin();
    while(loadIter != loads.end()) {
        p[nodes.keys().indexOf(loadIter.value().node) * nodeDOF + loadIter.value().dir] = loadIter.value().f;
        loadIter++;
    }

    QVector<double> d = cholesky(K, p);

    for(int i = 0; i < nodes.size(); i++) {
        nodes[nodes.keys()[i]].displacement.clear();
        for(int j = 0; j < nodeDOF; j++) {
            if(fabs(d[i * nodeDOF + j]) < 1e-300)
                d[i * nodeDOF + j] = 0;
            nodes[nodes.keys()[i]].displacement << d[i * nodeDOF + j];
        }
    }
}
/*
void calRod(int nodeDOF, QMap<int, Node> &nodes, Element &element) {
    Matrix<double> de(2 * nodeDOF, 1);
    de(0) = nodes[element.nodeNum1].displacement.x();
    de(1) = nodes[element.nodeNum1].displacement.y();
    de(2) = nodes[element.nodeNum2].displacement.x();
    de(3) = nodes[element.nodeNum2].displacement.y();
    element.de = de;
    element.dee = element.T * de;
    element.fee = element.Kee * element.dee;
    element.fe = element.T.trans() * element.fee;
    element.IF = element.fee(1);
    element.stress = element.IF / element.A;
}
*/
void calElements(int nodeDOF, int elementType, QMap<int, Section> &sections, QMap<int, Node> &nodes, QMap<int, Element> &elements) {
    QMap<int, Element>::iterator iter = elements.begin();
    while(iter != elements.end()) {
        Matrix<double> de(2 * nodeDOF, 1);
        for(int i = 0; i < nodeDOF; i++) {
            de(i) = nodes[iter.value().nodeNum1].displacement[i];
            de(i + nodeDOF) = nodes[iter.value().nodeNum2].displacement[i];
        }
        iter.value().de = de;
        iter.value().dee = iter.value().T * de;
        iter.value().fee = iter.value().Kee * iter.value().dee;
        iter.value().fe = iter.value().T.trans() * iter.value().fee;
        if(elementType == 1) {
            iter.value().IF = iter.value().fee(1);
            iter.value().stress = iter.value().IF / sections[iter.value().sectionNum].A;
        }
        iter++;
    }
}

void calConstraintForce(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Element> &elements, QMap<int, Constraint> &constraints) {
    QVector<double> F(nodes.size() * nodeDOF, 0);
    QMap<int, Element>::iterator elementIter = elements.begin();
    while(elementIter != elements.end()) {
        for (int j = 0; j < nodeDOF; j++) {
            F[nodes.keys().indexOf(elementIter.value().nodeNum1) * nodeDOF + j] += elementIter.value().fe(j);
            F[nodes.keys().indexOf(elementIter.value().nodeNum2) * nodeDOF + j] += elementIter.value().fe(j + nodeDOF);
        }
        elementIter++;
    }

    QMap<int, Constraint>::iterator constraintIter = constraints.begin();
    while(constraintIter != constraints.end()) {
        constraintIter.value().force = F[nodes.keys().indexOf(constraintIter.value().node) * nodeDOF + constraintIter.value().dir];
        constraintIter++;
    }
}
