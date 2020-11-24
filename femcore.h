#ifndef FEMCORE_H
#define FEMCORE_H

#include <QVector>
#include <QPointF>
#include <QMap>
#include <limits>

#include "matrixin1d.h"
#include "matrix.h"

typedef struct {
    QPointF pos;
    QVector<double> displacement;
} Node;

typedef struct {
    int nodeNum1;
    int nodeNum2;
    double E;
    double A;
    double L;
    /*
    QVector<QVector<double>> TK; // 局部坐标系下的刚度矩阵
    QVector<QVector<double>> T; // 坐标转换矩阵
    QVector<QVector<double>> de; // 整体坐标系下的节点位移
    QVector<QVector<double>> dee; // 局部坐标系下的节点位移
    QVector<QVector<double>> fe; // 整体坐标系下的节点力
    QVector<QVector<double>> fee; // 局部坐标系下的节点力
    */
    Matrix<double> TK; // 局部坐标系下的刚度矩阵
    Matrix<double> T; // 坐标转换矩阵
    Matrix<double> de; // 整体坐标系下的节点位移
    Matrix<double> dee; // 局部坐标系下的节点位移
    Matrix<double> fe; // 整体坐标系下的节点力
    Matrix<double> fee; // 局部坐标系下的节点力
    double IF; // 单元内力(Internal Force)
    double stress; // 单元应力(Stress)
} Rod;

typedef struct {
    int node;
    int dir;
    double force;
} Constraint;

typedef struct {
    int node;
    int dir;
    double f;
} Load;

double calLength(QPointF &p1, QPointF &p2);
void initRod(Rod &rod, QMap<int, Node> &nodes, int nodeNum1, int nodeNum2, double E, double A);

void calStiffnessMatrix(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Rod> &rods, MatrixIn1D &K);
void processConstraints(QMap<int, Node> &nodes, QMap<int, Constraint> &constraints, MatrixIn1D &K);
void solveEqns(QMap<int, Node> &nodes, MatrixIn1D &K, QMap<int, Load> &loads);
void calRod(int nodeDOF, QMap<int, Node> &nodes, Rod &rod);
void calRods(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Rod> &rods);
//void calConstraintForce(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Rod> &rods, Constraint &constraint);
void calConstraintForce(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Rod> &rods, QMap<int, Constraint> &constraints);

#endif // FEMCORE_H
