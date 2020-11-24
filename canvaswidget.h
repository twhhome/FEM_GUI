#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QShowEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <limits>

#include "femcore.h"

#define PI 3.14159

class CanvasWidget : public QWidget
{
    Q_OBJECT
public:
    enum DRAW_TYPE { DEFINE_SHAPE, DEFORM_SHAPE, ROD_INFORMATION, CONSTRAINT_FORCES };

    explicit CanvasWidget(QWidget *parent = nullptr);
    void clear();
    void setNodes(QMap<int, Node> &nodes);
    void setRods(QMap<int, Rod> &rods);
    void setConstraints(QMap<int, Constraint> &constraints);
    void setLoads(QMap<int, Load> &loads);
    void draw(DRAW_TYPE type, int rod = -1, int coord = -1);
    void setSolved(bool solved);

private:
    void drawAxis(QPainter &p);
    void drawNodes(QPainter &p);
    void drawRods(QPainter &p);
    void drawConstraints(QPainter &p);
    void drawLoads(QPainter &p);
    void drawTopLeftLabel(QPainter &p);
    void drawTopRightLabel(QPainter &p);
    void drawDeformedShape(QPainter &p);
    void drawRodInformation(QPainter &p);
    void drawConstraintForces(QPainter &p);
    void drawArrow(QPainter &p, QPointF point, double l, double theta, QString str, bool invert = false);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void showEvent(QShowEvent *);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    bool IfShowAxis;
    bool IfShowLabels;
    bool IfShowNodes;
    bool IfShowRods;
    bool IfShowConstraints;
    bool IfShowLoads;
    bool mousePressed;
    QPointF mouseLastPos;
    double scaleRatio;
    QPointF originPos;
    QPointF originPosBeforeScale;
    QPointF viewTranslate;

    QMap<int, Node> nodes;
    QMap<int, QList<int>> nodesAvailableDir;
    QMap<int, Rod> rods;
    QMap<int, Constraint> constraints;
    QMap<int, Load> loads;
    bool solved;
    DRAW_TYPE draw_type;
    QMap<int, QPointF> deformedPoints;
    double maxRodLength;
    QString topLeftMessage;
    int rodToDraw, coord;

signals:

public slots:
    void fit();
    void showAxis(bool checked);
    void showLabels(bool checked);
    void showNodes(bool checked);
    void showRods(bool checked);
    void showConstraints(bool checked);
    void showLoads(bool checked);
};

#endif // CANVASWIDGET_H
