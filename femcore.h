#ifndef FEMCORE_H
#define FEMCORE_H

#include <QVector>
#include <QPointF>
#include <QMap>
#include <limits>

#include "matrixin1d.h"
#include "matrix.h"

typedef struct {
    double E;
    double poisson;
} Material;

typedef struct {
    double A;
    double Iz;
    double Ay, K; // 考虑横向效应引入的系数
} Section;

typedef struct {
    double a1, b1;
    double a2, b2;
} Offset; // 梁的截面的偏心参数

typedef struct {
    QPointF pos;
    QVector<double> displacement;
} Node;

typedef struct {
    int nodeNum1;
    int nodeNum2;
    int materialNum;
    int sectionNum;
    int offsetNum;
    double E;
    double A;
    double L;
    Matrix<double> Kee; // 局部坐标系（节点坐标系）下的刚度矩阵
    Matrix<double> TT; // 位移转换矩阵（考虑偏心时用到）
    Matrix<double> T; // 坐标转换矩阵
    Matrix<double> de; // 整体坐标系下的节点位移
    Matrix<double> dee; // 局部坐标系下的节点位移
    Matrix<double> fe; // 整体坐标系下的节点力
    Matrix<double> fee; // 局部坐标系下的节点力
    double IF; // 单元内力(Internal Force)
    double stress; // 单元应力(Stress)
} Element;

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
//void initRod(Element &rod, QMap<int, Node> &nodes, int nodeNum1, int nodeNum2, double E, double A);
void initElement(Element &element, int elementType, QMap<int, Node> &nodes, QMap<int, Material> &materials, QMap<int, Section> &sections, QMap<int, Offset> &offsets);

void calStiffnessMatrix(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Element> &elements, MatrixIn1D &K);
void processConstraints(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Constraint> &constraints, MatrixIn1D &K);
void solveEqns(int nodeDOF, QMap<int, Node> &nodes, MatrixIn1D &K, QMap<int, Load> &loads);
//void calRod(int nodeDOF, QMap<int, Node> &nodes, Element &element);
void calElements(int nodeDOF, int elementType, QMap<int, Section> &sections, QMap<int, Node> &nodes, QMap<int, Element> &elements);
void calConstraintForce(int nodeDOF, QMap<int, Node> &nodes, QMap<int, Element> &elements, QMap<int, Constraint> &constraints);

#endif // FEMCORE_H
