#include "canvaswidget.h"

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent)
{
    IfShowAxis = true;
    IfShowLabels = true;
    IfShowNodes = true;
    IfShowRods = true;
    IfShowConstraints = true;
    IfShowLoads = true;
    mousePressed = false;
    scaleRatio = 1.0f;
    viewTranslate = QPointF(0, 0);
    solved = false;
    maxRodLength = -std::numeric_limits<double>::max();

    this->setStyleSheet("background-color:white;");
    setMouseTracking(true);
}

void CanvasWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    drawTopLeftLabel(painter);
    drawTopRightLabel(painter);

    painter.translate(originPos);

    if(draw_type != ROD_INFORMATION)
        drawAxis(painter);

    if(draw_type == DEFINE_SHAPE) {
        drawNodes(painter);
        drawRods(painter);
        drawConstraints(painter);
        drawLoads(painter);
    } else if(draw_type == DEFORM_SHAPE) {
        drawDeformedShape(painter);
    } else if(draw_type == ROD_INFORMATION) {
        drawRodInformation(painter);
    } else if(draw_type == CONSTRAINT_FORCES) {
        drawConstraintForces(painter);
    }
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        mousePressed = true;
        mouseLastPos = event->pos();
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    if(mousePressed == true) {
        QPointF pos = event->pos();
        viewTranslate = pos - mouseLastPos;
        originPos += viewTranslate;
        originPosBeforeScale = originPos;
        mouseLastPos = pos;
        update();
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *) {
    if(mousePressed == true)
        mousePressed = false;
}

void CanvasWidget::showEvent(QShowEvent *) {
    originPos = QPointF(this->size().width() / 2, this->size().height() / 2);
    originPosBeforeScale = originPos;
}

void CanvasWidget::wheelEvent(QWheelEvent *event) {
    if(event->angleDelta().y() > 0) {
        scaleRatio *= 1.1;
        originPos = event->pos() + (originPos - event->pos()) * 1.1;
    } else if(event->angleDelta().y() < 0) {
        if (scaleRatio > 0.001) {
            scaleRatio /= 1.1;
            originPos = event->pos() + (originPos - event->pos()) / 1.1;
        }
    }
    update();
}

void CanvasWidget::resizeEvent(QResizeEvent *event) {
    fit();
}

void CanvasWidget::drawAxis(QPainter &p) {
    if(!IfShowAxis)
        return;

    QSize s = this->size();
    double xLength = s.width() / 20;
    double yLength = s.height() / 20;
    double axisLength = (xLength < yLength) ? xLength : yLength;
    drawArrow(p, QPointF(0, 0), axisLength, 0, "x");
    drawArrow(p, QPointF(0, 0), axisLength, 90, "y");
}

void CanvasWidget::drawNodes(QPainter &p) {
    if(!IfShowNodes && draw_type == DEFINE_SHAPE)
        return;

    if(nodes.size() == 0)
        return;

    p.save();

    QPen pen;
    pen.setWidth(4);
    p.setPen(pen);

    QMap<int, Node>::iterator iter = nodes.begin();
    while(iter != nodes.end()) {
        QPointF point = QPointF(iter.value().pos.x(), -iter.value().pos.y()) * scaleRatio;
        p.drawPoint(point);
        if(IfShowLabels) {
            int width = 200, height = 20;
            QRectF rect = QRectF(point.x(), point.y(), width, height);
            p.drawText(rect, Qt::AlignLeft | Qt::AlignTop, QString("Node %1(%2,%3)").arg(iter.key()).arg(iter.value().pos.x()).arg(iter.value().pos.y()));
            //p.drawRect(rect);
        }
        QList<int> availableDir;
        availableDir << 1 << 1 << 1 << 1;
        nodesAvailableDir[iter.key()] = availableDir;
        iter++;
    }

    p.restore();
}

void CanvasWidget::drawRods(QPainter &p) {
    if(!IfShowRods && draw_type == DEFINE_SHAPE)
        return;

    if(rods.size() == 0)
        return;

    p.save();

    QPen pen;
    pen.setWidth(1);
    p.setPen(pen);

    QMap<int, Rod>::iterator iter = rods.begin();
    while(iter != rods.end()) {
        QPointF point1 = QPointF(nodes[iter.value().nodeNum1].pos.x(), -nodes[iter.value().nodeNum1].pos.y()) * scaleRatio;
        QPointF point2 = QPointF(nodes[iter.value().nodeNum2].pos.x(), -nodes[iter.value().nodeNum2].pos.y()) * scaleRatio;
        p.drawLine(point1, point2);
        if(IfShowLabels)
            p.drawText((point1 + point2) / 2, QString("Rod %1").arg(iter.key()));

        if(nodes[iter.value().nodeNum1].pos.x() == nodes[iter.value().nodeNum2].pos.x()) {
            if(nodes[iter.value().nodeNum1].pos.y() < nodes[iter.value().nodeNum2].pos.y()) {
                nodesAvailableDir[iter.value().nodeNum1][1] = 0;
                nodesAvailableDir[iter.value().nodeNum2][3] = 0;
            } else {
                nodesAvailableDir[iter.value().nodeNum1][3] = 0;
                nodesAvailableDir[iter.value().nodeNum2][1] = 0;
            }
        } else {
            double k = (nodes[iter.value().nodeNum2].pos.y() - nodes[iter.value().nodeNum1].pos.y()) / (nodes[iter.value().nodeNum2].pos.x() - nodes[iter.value().nodeNum1].pos.x());
            if(fabs(k) > 2) {
                if(nodes[iter.value().nodeNum1].pos.y() < nodes[iter.value().nodeNum2].pos.y()) {
                    nodesAvailableDir[iter.value().nodeNum1][1] = 0;
                    nodesAvailableDir[iter.value().nodeNum2][3] = 0;
                } else {
                    nodesAvailableDir[iter.value().nodeNum1][3] = 0;
                    nodesAvailableDir[iter.value().nodeNum2][1] = 0;
                }
            } else if(fabs(k) < 0.5) {
                if(nodes[iter.value().nodeNum1].pos.x() < nodes[iter.value().nodeNum2].pos.x()) {
                    nodesAvailableDir[iter.value().nodeNum1][0] = 0;
                    nodesAvailableDir[iter.value().nodeNum2][2] = 0;
                } else {
                    nodesAvailableDir[iter.value().nodeNum1][2] = 0;
                    nodesAvailableDir[iter.value().nodeNum2][0] = 0;
                }
            }
        }
        //if(iter.value().L > maxRodLength)
        //    maxRodLength = iter.value().L;

        iter++;
    }

    p.restore();
}

void CanvasWidget::drawConstraints(QPainter &p) {
    if(!IfShowConstraints && draw_type == DEFINE_SHAPE)
        return;

    if(constraints.size() == 0)
        return;

    p.save();

    QPen pen;
    pen.setWidth(1);
    p.setPen(pen);

    QMap<int, Constraint>::iterator iter = constraints.begin();
    while(iter != constraints.end()) {
        int dir = 0;
        double l = (size().width() < size().height()) ? size().width() / 40 : size().height() / 40;
        if(iter.value().dir == 0) { // x direction
            if(nodesAvailableDir[iter.value().node][2])
                dir = 2;
            else if(nodesAvailableDir[iter.value().node][0])
                dir = 0;
            else
                dir = 2;
        } else if(iter.value().dir == 1) { // y direction
            if(nodesAvailableDir[iter.value().node][3])
                dir = 3;
            else if(nodesAvailableDir[iter.value().node][1])
                dir = 1;
            else
                dir = 3;
        }
        QPointF point1, point2, point3, point4, point5, point6, point7;
        switch (dir) {
        case 0:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            point2 = point1 + QPointF(l, 0);// * scaleRatio;
            point3 = point2 + QPointF(0, -l / 2);
            point4 = point2 + QPointF(0, l / 2);
            point5 = point3 + QPointF(l / 4, -l / 4);
            point6 = point2 + QPointF(l / 4, -l / 4);
            point7 = point4 + QPointF(l / 4, -l / 4);
            p.drawLine(point1, point2);
            p.drawLine(point3, point4);
            p.drawLine(point3, point5);
            p.drawLine(point2, point6);
            p.drawLine(point4, point7);
            if(IfShowLabels)
                p.drawText(point5 + QPointF(0, -5), QString("Constraint %1").arg(iter.key()));
            break;
        case 1:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            point2 = point1 + QPointF(0, -l);// * scaleRatio;
            point3 = point2 + QPointF(-l / 2, 0);
            point4 = point2 + QPointF(l / 2, 0);
            point5 = point3 + QPointF(-l / 4, -l / 4);
            point6 = point2 + QPointF(-l / 4, -l / 4);
            point7 = point4 + QPointF(-l / 4, -l / 4);
            p.drawLine(point1, point2);
            p.drawLine(point3, point4);
            p.drawLine(point3, point5);
            p.drawLine(point2, point6);
            p.drawLine(point4, point7);
            if(IfShowLabels)
                p.drawText(point5 + QPointF(0, -5), QString("Constraint %1").arg(iter.key()));
            break;
        case 2:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            point2 = point1 + QPointF(-l, 0);// * scaleRatio;
            point3 = point2 + QPointF(0, -l / 2);
            point4 = point2 + QPointF(0, l / 2);
            point5 = point3 + QPointF(-l / 4, l / 4);
            point6 = point2 + QPointF(-l / 4, l / 4);
            point7 = point4 + QPointF(-l / 4, l / 4);
            p.drawLine(point1, point2);
            p.drawLine(point3, point4);
            p.drawLine(point3, point5);
            p.drawLine(point2, point6);
            p.drawLine(point4, point7);
            if(IfShowLabels)
                p.drawText(point3 + QPointF(-80, 0), QString("Constraint %1").arg(iter.key()));
            break;
        case 3:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            point2 = point1 + QPointF(0, l);// * scaleRatio;
            point3 = point2 + QPointF(-l / 2, 0);
            point4 = point2 + QPointF(l / 2, 0);
            point5 = point3 + QPointF(-l / 4, l / 4);
            point6 = point2 + QPointF(-l / 4, l / 4);
            point7 = point4 + QPointF(-l / 4, l / 4);
            p.drawLine(point1, point2);
            p.drawLine(point3, point4);
            p.drawLine(point3, point5);
            p.drawLine(point2, point6);
            p.drawLine(point4, point7);
            if(IfShowLabels)
                p.drawText(point5 + QPointF(0, 15), QString("Constraint %1").arg(iter.key()));
            break;
        }

        p.save();
        pen.setWidth(4);
        p.setPen(pen);
        p.drawPoint(point2);
        p.restore();

        iter++;
    }

    p.restore();
}

void CanvasWidget::drawLoads(QPainter &p) {
    if(!IfShowLoads && draw_type == DEFINE_SHAPE)
        return;

    if(loads.size() == 0)
        return;

    p.save();

    QPen pen;
    pen.setWidth(1);
    p.setPen(pen);

    QMap<int, Load>::iterator iter = loads.begin();
    while(iter != loads.end()) {
        int dir = 0;
        double l = (size().width() < size().height()) ? size().width() / 20 : size().height() / 20;
        //double arrowLength = l / 3;
        if(iter.value().dir == 0) { // x direction
            if(nodesAvailableDir[iter.value().node][2])
                dir = 2;
            else if(nodesAvailableDir[iter.value().node][0])
                dir = 0;
            else
                dir = 2;
        } else if(iter.value().dir == 1) { // y direction
            if(nodesAvailableDir[iter.value().node][3])
                dir = 3;
            else if(nodesAvailableDir[iter.value().node][1])
                dir = 1;
            else
                dir = 3;
        }
        QPointF point1;
        switch (dir) {
        case 0:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 0, QString("Load %1 f=%2").arg(iter.key()).arg(fabs(iter.value().f)), !(iter.value().f > 0));
            break;
        case 1:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 90, QString("Load %1 f=%2").arg(iter.key()).arg(fabs(iter.value().f)), !(iter.value().f > 0));
            break;
        case 2:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 180, QString("Load %1 f=%2").arg(iter.key()).arg(fabs(iter.value().f)), !(iter.value().f < 0));
            break;
        case 3:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 270, QString("Load %1 f=%2").arg(iter.key()).arg(fabs(iter.value().f)), !(iter.value().f < 0));
            break;
        }
        iter++;
    }

    p.restore();
}

void CanvasWidget::drawTopLeftLabel(QPainter &p) {
    p.save();

    QFont font;
    font.setPointSize(11);
    font.setBold(true);
    p.setFont(font);

    QRect rect = QRect(10, 5, 120, 15);
    p.drawText(rect, QString("FEM(%1)").arg(solved ? "solved" : "unsolved"));


    if(draw_type == DEFORM_SHAPE || draw_type == ROD_INFORMATION || draw_type == CONSTRAINT_FORCES) {
        if(!topLeftMessage.isEmpty()) {
            p.drawLine(QPointF(rect.x(), rect.y() + rect.height() + 5), QPointF(rect.x() + rect.width(), rect.y() + rect.height() + 5));

            font.setPointSize(9);
            font.setBold(false);
            p.setFont(font);
            rect = QRect(rect.x(), rect.y() + rect.height() + 10, 500, 500);
            p.drawText(rect, topLeftMessage);
        }
    }

    p.restore();
}

void CanvasWidget::drawTopRightLabel(QPainter &p) {
    p.save();

    QFont font;
    font.setPointSize(11);
    font.setBold(false);
    p.setFont(font);

    QSize s = this->size();
    int width = 200, height = 15, margin = 5;
    QRect rect = QRect(s.width() - margin - width, margin, width, height);

    QString str;
    if(draw_type == DEFINE_SHAPE) {
        str = "Defined Shape";
    } else if(draw_type == DEFORM_SHAPE) {
        str = "Deform Shape";
    } else if(draw_type == ROD_INFORMATION) {
        str = "Rod Information";
    } else if(draw_type == CONSTRAINT_FORCES) {
        str = "Constraint Forces";
    }

    p.drawText(rect, Qt::AlignRight | Qt::AlignTop, str);

    p.restore();
}

void CanvasWidget::drawDeformedShape(QPainter &p) {
    p.save();

    QPen pen;
    pen.setWidth(4);
    p.setPen(pen);

    QMap<int, QPointF>::iterator pointIter = deformedPoints.begin();
    while(pointIter != deformedPoints.end()) {
        QPointF point = QPointF(pointIter.value().x(), -pointIter.value().y()) * scaleRatio; // deformedPoints 有问题
        p.drawPoint(point);
        point = QPointF(nodes[pointIter.key()].pos.x(), -nodes[pointIter.key()].pos.y()) * scaleRatio;
        p.drawPoint(point);
        if(IfShowLabels) {
            int width = 200, height = 20;
            QRectF rect = QRectF(point.x(), point.y(), width, height);
            p.drawText(rect, Qt::AlignLeft | Qt::AlignTop, QString("Node %1").arg(pointIter.key()));
        }
        pointIter++;
    }

    pen.setWidth(1);
    p.setPen(pen);

    QMap<int, Rod>::iterator rodIter = rods.begin();
    while(rodIter != rods.end()) {
        QPointF point1 = QPointF(nodes[rodIter.value().nodeNum1].pos.x(), -nodes[rodIter.value().nodeNum1].pos.y()) * scaleRatio;
        QPointF point2 = QPointF(nodes[rodIter.value().nodeNum2].pos.x(), -nodes[rodIter.value().nodeNum2].pos.y()) * scaleRatio;
        p.drawLine(point1, point2);
        if(IfShowLabels)
            p.drawText((point1 + point2) / 2, QString("Rod %1").arg(rodIter.key()));
        p.save();
        pen.setStyle(Qt::DashDotDotLine);
        p.setPen(pen);
        point1 = QPointF(deformedPoints[rodIter.value().nodeNum1].x(), -deformedPoints[rodIter.value().nodeNum1].y()) * scaleRatio;
        point2 = QPointF(deformedPoints[rodIter.value().nodeNum2].x(), -deformedPoints[rodIter.value().nodeNum2].y()) * scaleRatio;
        p.drawLine(point1, point2);
        p.restore();

        rodIter++;
    }

    p.restore();
}

void CanvasWidget::drawRodInformation(QPainter &p) {
    p.save();

    QPen pen;
    pen.setWidth(4);
    p.setPen(pen);

    QPointF point1 = QPointF(nodes[rods[rodToDraw].nodeNum1].pos.x(), -nodes[rods[rodToDraw].nodeNum1].pos.y()) * scaleRatio;
    QPointF point2 = QPointF(nodes[rods[rodToDraw].nodeNum2].pos.x(), -nodes[rods[rodToDraw].nodeNum2].pos.y()) * scaleRatio;
    p.drawPoint(point1);
    p.drawPoint(point2);
    if(IfShowLabels) {
        int width = 200, height = 20;
        QRectF rect = QRectF(point1.x(), point1.y(), width, height);
        p.drawText(rect, Qt::AlignLeft | Qt::AlignTop, QString("Node %1").arg(rods[rodToDraw].nodeNum1));
        rect = QRectF(point2.x(), point2.y(), width, height);
        p.drawText(rect, Qt::AlignLeft | Qt::AlignTop, QString("Node %1").arg(rods[rodToDraw].nodeNum2));
    }

    pen.setWidth(1);
    p.setPen(pen);
    p.drawLine(point1, point2);
    if(IfShowLabels)
        p.drawText((point1 + point2) / 2, QString("Rod %1").arg(rodToDraw));

    double l = (size().width() < size().height()) ? size().width() / 20 : size().height() / 20;

    if(coord == 0) { // global
        drawArrow(p, point1, l, rods[rodToDraw].fe(0) > 0 ? 0 : 180, QString("f=%1").arg(fabs(rods[rodToDraw].fe(0))));
        drawArrow(p, point1, l, rods[rodToDraw].fe(1) > 0 ? 90 : 270, QString("f=%1").arg(fabs(rods[rodToDraw].fe(1))));
        drawArrow(p, point2, l, rods[rodToDraw].fe(2) > 0 ? 0 : 180, QString("f=%1").arg(fabs(rods[rodToDraw].fe(2))));
        drawArrow(p, point2, l, rods[rodToDraw].fe(3) > 0 ? 90 : 270, QString("f=%1").arg(fabs(rods[rodToDraw].fe(3))));
    } else if(coord == 1) { // local
        QPointF delta = nodes[rods[rodToDraw].nodeNum2].pos - nodes[rods[rodToDraw].nodeNum1].pos;
        double angle = atan2(delta.y(), delta.x()) * 180 / PI;
        drawArrow(p, point1, l, angle + 180, QString("f=%1").arg(fabs(rods[rodToDraw].fee(0))), rods[rodToDraw].fee(0) > 0);
        drawArrow(p, point2, l, angle, QString("f=%1").arg(fabs(rods[rodToDraw].fee(1))), rods[rodToDraw].fee(1) < 0);
    }

    p.restore();
}

void CanvasWidget::drawConstraintForces(QPainter &p) {
    p.save();

    drawNodes(p);
    drawRods(p);
    drawLoads(p);

    QPen pen;
    pen.setWidth(1);
    p.setPen(pen);

    QMap<int, Constraint>::iterator iter = constraints.begin();
    while(iter != constraints.end()) {
        int dir = 0;
        double l = (size().width() < size().height()) ? size().width() / 20 : size().height() / 20;
        //double arrowLength = l / 3;
        if(iter.value().dir == 0) { // x direction
            if(nodesAvailableDir[iter.value().node][2])
                dir = 2;
            else if(nodesAvailableDir[iter.value().node][0])
                dir = 0;
            else
                dir = 2;
        } else if(iter.value().dir == 1) { // y direction
            if(nodesAvailableDir[iter.value().node][3])
                dir = 3;
            else if(nodesAvailableDir[iter.value().node][1])
                dir = 1;
            else
                dir = 3;
        }
        QPointF point1;
        switch (dir) {
        case 0:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 0, QString("Constraint %1 f=%2").arg(iter.key()).arg(fabs(iter.value().force)), !(iter.value().force > 0));
            break;
        case 1:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 90, QString("Constraint %1 f=%2").arg(iter.key()).arg(fabs(iter.value().force)), !(iter.value().force > 0));
            break;
        case 2:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 180, QString("Constraint %1 f=%2").arg(iter.key()).arg(fabs(iter.value().force)), !(iter.value().force < 0));
            break;
        case 3:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 270, QString("Constraint %1 f=%2").arg(iter.key()).arg(fabs(iter.value().force)), !(iter.value().force < 0));
            break;
        }
        iter++;
    }

    p.restore();
}

void CanvasWidget::drawArrow(QPainter &p, QPointF point, double l, double theta, QString str, bool invert) {
    p.save();

    QPen pen;
    pen.setWidth(2);
    p.setPen(pen);

    double arrowLength = l / 3;
    double arrowAngle = 30;
    if(invert)
        arrowAngle = 180 - arrowAngle;
    QPointF p1, p2, p3, p4;
    p1 = point;
    p2 = p1 + l * QPointF(cos(theta * 2 * PI / 360), -sin(theta * 2 * PI / 360));
    p3 = (invert ? p1 : p2) + arrowLength * QPointF(cos((theta + 180 - arrowAngle) * 2 * PI / 360), -sin((theta + 180 - arrowAngle) * 2 * PI / 360));
    p4 = (invert ? p1 : p2) + arrowLength * QPointF(cos((theta + arrowAngle - 180) * 2 * PI / 360), -sin((theta + arrowAngle - 180) * 2 * PI / 360));
    p.drawLine(p1, p2);
    p.drawLine((invert ? p1 : p2), p3);
    p.drawLine((invert ? p1 : p2), p4);
    if(IfShowLabels) {
        QRectF rect;
        int flag, width = 200, height = 50;
        theta = (int)theta % 360;
        if(theta >= 0 && theta <= 90) {
            rect = QRectF(p2.x(), p2.y() - height, width, height);
            flag = Qt::AlignLeft | Qt::AlignBottom;
        } else if(theta > 90 && theta <= 180) {
            rect = QRectF(p2.x() - width, p2.y() - height, width, height);
            flag = Qt::AlignRight | Qt::AlignBottom;
        } else if(theta > 180 && theta <= 270) {
            rect = QRectF(p2.x() - width, p2.y(), width, height);
            flag = Qt::AlignRight | Qt::AlignTop;
        } else {
            rect = QRectF(p2.x(), p2.y(), width, height);
            flag = Qt::AlignLeft | Qt::AlignTop;
        }
        p.drawText(rect, flag, str);
        //p.drawRect(rect);
    }

    p.restore();
}

void CanvasWidget::clear() {
    this->nodes.clear();
    this->rods.clear();
    this->constraints.clear();
    this->loads.clear();
    this->deformedPoints.clear();
    originPos = QPointF(this->size().width() / 2, this->size().height() / 2);
    scaleRatio = 1.0f;
    maxRodLength = 0;
    this->solved = false;
    draw_type = DEFINE_SHAPE;
    topLeftMessage = "";
    this->update();
}

void CanvasWidget::setNodes(QMap<int, Node> &nodes) {
    this->nodes = nodes;
}

void CanvasWidget::setRods(QMap<int, Rod> &rods) {
    this->rods = rods;
    QMap<int, Rod>::iterator iter = this->rods.begin();
    maxRodLength = 0;
    while(iter != this->rods.end()) {
        if(iter.value().L > maxRodLength)
            maxRodLength = iter.value().L;
        iter++;
    }
}

void CanvasWidget::setConstraints(QMap<int, Constraint> &constraints) {
    this->constraints = constraints;
}

void CanvasWidget::setLoads(QMap<int, Load> &loads) {
    this->loads = loads;
}

void CanvasWidget::draw(DRAW_TYPE type, int rod, int coord) {
    draw_type = type;
    if(draw_type == DEFORM_SHAPE) {
        topLeftMessage = "Node Displacement:\n";
        QMap<int, Node>::iterator iter = nodes.begin();
        while(iter != nodes.end()) {
            topLeftMessage += QString("Node %1:(%2,%3)\n").arg(iter.key()).arg(iter.value().displacement.x()).arg(iter.value().displacement.y());
            iter++;
        }
    } else if(draw_type == ROD_INFORMATION) {
        rodToDraw = rod;
        this->coord = coord;
        topLeftMessage = QString("Information of Rod %1(%2):\n").arg(rodToDraw).arg(coord == 0 ? "Global Coordinate" : "Local Coordinate");
        if(coord == 0) {
            topLeftMessage += QString("Node %1 Displacement:(%2,%3)\n").arg(rods[rodToDraw].nodeNum1).arg(rods[rodToDraw].de(0)).arg(rods[rodToDraw].de(1));
            topLeftMessage += QString("Node %1 Displacement:(%2,%3)\n").arg(rods[rodToDraw].nodeNum2).arg(rods[rodToDraw].de(2)).arg(rods[rodToDraw].de(3));
            topLeftMessage += QString("Node %1 Force:(%2,%3)\n").arg(rods[rodToDraw].nodeNum1).arg(rods[rodToDraw].fe(0)).arg(rods[rodToDraw].fe(1));
            topLeftMessage += QString("Node %1 Force:(%2,%3)\n").arg(rods[rodToDraw].nodeNum2).arg(rods[rodToDraw].fe(2)).arg(rods[rodToDraw].fe(3));
        } else if(coord == 1) {
            topLeftMessage += QString("Node %1 Displacement:%2\n").arg(rods[rodToDraw].nodeNum1).arg(rods[rodToDraw].dee(0));
            topLeftMessage += QString("Node %1 Displacement:%2\n").arg(rods[rodToDraw].nodeNum2).arg(rods[rodToDraw].dee(1));
            topLeftMessage += QString("Node %1 Force:%2\n").arg(rods[rodToDraw].nodeNum1).arg(rods[rodToDraw].fee(0));
            topLeftMessage += QString("Node %1 Force:%2\n").arg(rods[rodToDraw].nodeNum2).arg(rods[rodToDraw].fee(1));
        }
        topLeftMessage += QString("Internal Force:%1\n").arg(rods[rodToDraw].IF);
        topLeftMessage += QString("Stress:%1\n").arg(rods[rodToDraw].stress);
    } else if(draw_type == CONSTRAINT_FORCES) {
        topLeftMessage = "Constraint Forces:\n";
        QMap<int, Constraint>::iterator iter = constraints.begin();
        while(iter != constraints.end()) {
            topLeftMessage += QString("Constraint %1:f=%2\n").arg(iter.key()).arg(iter.value().force);
            iter++;
        }
    }
    this->update();
}

void CanvasWidget::setSolved(bool solved) {
    //qDebug() << solved;
    if(solved) {
        QMap<int, Node>::iterator iter = nodes.begin();
        double max_displacement = -std::numeric_limits<double>::max();
        while(iter != nodes.end()) {
            if(iter.value().displacement.x() > max_displacement)
                max_displacement = iter.value().displacement.x();
            if(iter.value().displacement.y() > max_displacement)
                max_displacement = iter.value().displacement.y();
            iter++;
        }
        double ratio = 0.01 * maxRodLength / max_displacement;
        //qDebug() << ratio;
        iter = nodes.begin();
        while(iter != nodes.end()) {
            QPointF point;
            point = iter.value().pos + ratio * iter.value().displacement;
            deformedPoints[iter.key()] = point;
            iter++;
        }
    }
    if(solved != this->solved) {
        this->solved = solved;
        this->update();
    }
}

void CanvasWidget::fit() {
    originPos = QPointF(this->size().width() / 2, this->size().height() / 2);
    if(nodes.size() == 0)
        scaleRatio = 1;
    else {
        QSize s = this->size();

        double maxX = -std::numeric_limits<double>::max(), maxY = -std::numeric_limits<double>::max();
        double minX = std::numeric_limits<double>::max(), minY = std::numeric_limits<double>::max();
        if(draw_type == ROD_INFORMATION) {
            maxX = (nodes[rods[rodToDraw].nodeNum1].pos.x() > nodes[rods[rodToDraw].nodeNum2].pos.x()) ? nodes[rods[rodToDraw].nodeNum1].pos.x() : nodes[rods[rodToDraw].nodeNum2].pos.x();
            minX = (nodes[rods[rodToDraw].nodeNum1].pos.x() < nodes[rods[rodToDraw].nodeNum2].pos.x()) ? nodes[rods[rodToDraw].nodeNum1].pos.x() : nodes[rods[rodToDraw].nodeNum2].pos.x();
            maxY = (nodes[rods[rodToDraw].nodeNum1].pos.y() > nodes[rods[rodToDraw].nodeNum2].pos.y()) ? nodes[rods[rodToDraw].nodeNum1].pos.y() : nodes[rods[rodToDraw].nodeNum2].pos.y();
            minY = (nodes[rods[rodToDraw].nodeNum1].pos.y() < nodes[rods[rodToDraw].nodeNum2].pos.y()) ? nodes[rods[rodToDraw].nodeNum1].pos.y() : nodes[rods[rodToDraw].nodeNum2].pos.y();
        } else {
            QMap<int, Node>::iterator iter = nodes.begin();
            while(iter != nodes.end()) {
                if(iter.value().pos.x() > maxX)
                    maxX = iter.value().pos.x();
                if(iter.value().pos.x() < minX)
                    minX = iter.value().pos.x();
                if(iter.value().pos.y() > maxY)
                    maxY = iter.value().pos.y();
                if(iter.value().pos.y() < minY)
                    minY = iter.value().pos.y();
                iter++;
            }
            if(maxX == minX && maxY == minY) {
                maxX += 1;
                minX -= 1;
                maxY += 1;
                minY -= 1;
            }
        }
        QPointF center = QPointF(0.5 * (maxX + minX), 0.5 * (maxY + minY));
        double l = ((maxX - minX) > (maxY - minY) ? (maxX - minX) : (maxY - minY));
        l *= 1.3;
        scaleRatio = ((s.width() > s.height()) ? s.height() : s.width()) / l;
        originPos = originPos + QPointF(-center.x(), center.y()) * scaleRatio;
    }
    this->update();
}

void CanvasWidget::showAxis(bool checked) {
    IfShowAxis = checked;
    this->update();
}

void CanvasWidget::showLabels(bool checked) {
    IfShowLabels = checked;
    this->update();
}

void CanvasWidget::showNodes(bool checked) {
    IfShowNodes = checked;
    if(draw_type == DEFINE_SHAPE)
        this->update();
}

void CanvasWidget::showRods(bool checked) {
    IfShowRods = checked;
    if(draw_type == DEFINE_SHAPE)
        this->update();
}

void CanvasWidget::showConstraints(bool checked) {
    IfShowConstraints = checked;
    if(draw_type == DEFINE_SHAPE)
        this->update();
}

void CanvasWidget::showLoads(bool checked) {
    IfShowLoads = checked;
    if(draw_type == DEFINE_SHAPE)
        this->update();
}
