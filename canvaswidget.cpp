#include "canvaswidget.h"

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent)
{
    IfShowAxis = true;
    IfShowLabels = true;
    IfShowNodes = true;
    IfShowElements = true;
    IfShowConstraints = true;
    IfShowLoads = true;
    mousePressed = false;
    scaleRatio = 1.0f;
    viewTranslate = QPointF(0, 0);
    solved = false;
    maxElementLength = -std::numeric_limits<double>::max();

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
    //painter.scale(scaleRatio2, scaleRatio2);

    if(draw_type != ELEMENT_INFORMATION)
        drawAxis(painter);

    if(draw_type == DEFINE_SHAPE) {
        drawNodes(painter);
        drawElements(painter);
        drawConstraints(painter);
        drawLoads(painter);
    } else if(draw_type == DEFORM_SHAPE) {
        drawDeformedShape(painter);
    } else if(draw_type == ELEMENT_INFORMATION) {
        drawElementInformation(painter);
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

void CanvasWidget::drawElements(QPainter &p) {
    if(!IfShowElements && draw_type == DEFINE_SHAPE)
        return;

    if(elements.size() == 0)
        return;

    p.save();

    QPen pen;
    pen.setWidth(1);
    p.setPen(pen);

    QMap<int, Element>::iterator iter = elements.begin();
    while(iter != elements.end()) {
        QPointF point1 = QPointF(nodes[iter.value().nodeNum1].pos.x(), -nodes[iter.value().nodeNum1].pos.y()) * scaleRatio;
        QPointF point2 = QPointF(nodes[iter.value().nodeNum2].pos.x(), -nodes[iter.value().nodeNum2].pos.y()) * scaleRatio;
        p.drawLine(point1, point2);
        if(IfShowLabels)
            p.drawText((point1 + point2) / 2, QString("Element %1").arg(iter.key()));

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

    QMap<int, QString> constraintsLabels;

    QMap<int, Constraint>::iterator iter = constraints.begin();
    while(iter != constraints.end()) {
        QString str = QString("Constraint %1(%2)").arg(iter.key()).arg((iter.value().dir == 0) ? "x" : ((iter.value().dir == 1) ? "y" : "rz"));
        if(constraintsLabels.contains(iter.value().node))
            str = "\n" + str;
        constraintsLabels[iter.value().node] += str;

/*
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
        } else if(iter.value().dir == 2) { // Rz direction

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
        */
        iter++;
    }

    QMap<int, QString>::iterator labelIter = constraintsLabels.begin();
    while(labelIter != constraintsLabels.end()) {
        if(IfShowLabels) {
            int width = 200, height = 60;
            QPointF point = QPointF(nodes[labelIter.key()].pos.x(), -nodes[labelIter.key()].pos.y()) * scaleRatio;
            QRectF rect = QRectF(point.x() - width, point.y(), width, height);
            p.drawText(rect, Qt::AlignRight | Qt::AlignTop, labelIter.value());
        }
        labelIter++;
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
        } else if(iter.value().dir == 2) {
            QPointF point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawCircularArrow(p, point1, 15, 45, iter.value().f > 0 ? 180 : -180, QString("Load %1 M=%2").arg(iter.key()).arg(fabs(iter.value().f)));
            iter++;
            continue;
        }
        QPointF point1;
        switch (dir) {
        case 0:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 0, QString("Load %1 F=%2").arg(iter.key()).arg(fabs(iter.value().f)), !(iter.value().f > 0));
            break;
        case 1:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 90, QString("Load %1 F=%2").arg(iter.key()).arg(fabs(iter.value().f)), !(iter.value().f > 0));
            break;
        case 2:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 180, QString("Load %1 F=%2").arg(iter.key()).arg(fabs(iter.value().f)), !(iter.value().f < 0));
            break;
        case 3:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 270, QString("Load %1 F=%2").arg(iter.key()).arg(fabs(iter.value().f)), !(iter.value().f < 0));
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


    if(draw_type == DEFORM_SHAPE || draw_type == ELEMENT_INFORMATION || draw_type == CONSTRAINT_FORCES) {
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
    } else if(draw_type == ELEMENT_INFORMATION) {
        str = "Element Information";
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

    QMap<int, Node>::iterator iter = nodes.begin();
    while(iter != nodes.end()) {
        QPointF point = QPointF(iter.value().pos.x(), -iter.value().pos.y()) * scaleRatio;
        p.drawPoint(point);
        if(IfShowLabels) {
            int width = 200, height = 20;
            QRectF rect = QRectF(point.x(), point.y(), width, height);
            p.drawText(rect, Qt::AlignLeft | Qt::AlignTop, QString("Node %1").arg(iter.key()));
        }
        point += QPointF(iter.value().displacement[0], -iter.value().displacement[1]) * deformRatio * scaleRatio;
        p.drawPoint(point);
        iter++;
    }

    pen.setWidth(1);
    p.setPen(pen);

    QMap<int, QList<QPointF>>::iterator pointIter = originalPoints.begin();
    while(pointIter != originalPoints.end()) {
        p.drawLine(pointIter.value().first() * scaleRatio, pointIter.value().last() * scaleRatio);
        if(IfShowLabels)
            p.drawText((pointIter.value().first() + pointIter.value().last()) * scaleRatio / 2, QString("Element %1").arg(pointIter.key()));
        p.save();
        pen.setStyle(Qt::DashDotDotLine);
        p.setPen(pen);
        QPainterPath path;
        for(int i = 0; i < pointIter.value().size(); i++) {
            QPointF point = pointIter.value().at(i) + deformRatio * deformedDisplacements[pointIter.key()].at(i);
            point *= scaleRatio;
            if(i == 0)
                path.moveTo(point);
            else
                path.lineTo(point);
        }
        p.drawPath(path);
        p.restore();
        pointIter++;
    }

    p.restore();
}

void CanvasWidget::drawElementInformation(QPainter &p) {
    p.save();

    QPen pen;
    pen.setWidth(4);
    p.setPen(pen);

    QPointF point1 = QPointF(nodes[elements[elementToDraw].nodeNum1].pos.x(), -nodes[elements[elementToDraw].nodeNum1].pos.y()) * scaleRatio;
    QPointF point2 = QPointF(nodes[elements[elementToDraw].nodeNum2].pos.x(), -nodes[elements[elementToDraw].nodeNum2].pos.y()) * scaleRatio;
    p.drawPoint(point1);
    p.drawPoint(point2);
    if(IfShowLabels) {
        int width = 200, height = 20;
        QRectF rect = QRectF(point1.x(), point1.y(), width, height);
        p.drawText(rect, Qt::AlignLeft | Qt::AlignTop, QString("Node %1").arg(elements[elementToDraw].nodeNum1));
        rect = QRectF(point2.x(), point2.y(), width, height);
        p.drawText(rect, Qt::AlignLeft | Qt::AlignTop, QString("Node %1").arg(elements[elementToDraw].nodeNum2));
    }

    pen.setWidth(1);
    p.setPen(pen);
    p.drawLine(point1, point2);
    if(IfShowLabels)
        p.drawText((point1 + point2) / 2, QString("Element %1").arg(elementToDraw));

    double l = (size().width() < size().height()) ? size().width() / 20 : size().height() / 20;

    if(coord == 0) { // global
        drawArrow(p, point1, l, elements[elementToDraw].fe(0) > 0 ? 0 : 180, QString("F=%1").arg(fabs(elements[elementToDraw].fe(0))));
        drawArrow(p, point1, l, elements[elementToDraw].fe(1) > 0 ? 90 : 270, QString("F=%1").arg(fabs(elements[elementToDraw].fe(1))));
        drawArrow(p, point2, l, elements[elementToDraw].fe(elementType == 1 ? 2 : 3) > 0 ? 0 : 180, QString("F=%1").arg(fabs(elements[elementToDraw].fe(elementType == 1 ? 2 : 3))));
        drawArrow(p, point2, l, elements[elementToDraw].fe(elementType == 1 ? 3 : 4) > 0 ? 90 : 270, QString("F=%1").arg(fabs(elements[elementToDraw].fe(elementType == 1 ? 3 : 4))));
        if(elementType == 2) {
            drawCircularArrow(p, point1, 15, 225, elements[elementToDraw].fe(2) > 0 ? 180 : -180, QString("M=%1").arg(fabs(elements[elementToDraw].fe(2))));
            drawCircularArrow(p, point2, 15, 225, elements[elementToDraw].fe(5) > 0 ? 180 : -180, QString("M=%1").arg(fabs(elements[elementToDraw].fe(5))));
        }
    } else if(coord == 1) { // local
        QPointF delta = nodes[elements[elementToDraw].nodeNum2].pos - nodes[elements[elementToDraw].nodeNum1].pos;
        double angle = atan2(delta.y(), delta.x()) * 180 / PI;
        if(elementType == 1) {
            drawArrow(p, point1, l, angle + 180, QString("F=%1").arg(fabs(elements[elementToDraw].fee(0))), elements[elementToDraw].fee(0) > 0);
            drawArrow(p, point2, l, angle, QString("F=%1").arg(fabs(elements[elementToDraw].fee(1))), elements[elementToDraw].fee(1) < 0);
        } else if(elementType == 2) {
            drawArrow(p, point1, l, angle + 180, QString("F=%1").arg(fabs(elements[elementToDraw].fee(0))), elements[elementToDraw].fee(0) > 0);
            drawArrow(p, point1, l, angle + 90, QString("F=%1").arg(fabs(elements[elementToDraw].fee(1))), elements[elementToDraw].fee(1) < 0);
            drawCircularArrow(p, point1, 15, 225, elements[elementToDraw].fee(2) > 0 ? 180 : -180, QString("M=%1").arg(fabs(elements[elementToDraw].fee(2))));
            drawArrow(p, point2, l, angle, QString("F=%1").arg(fabs(elements[elementToDraw].fee(3))), elements[elementToDraw].fee(3) < 0);
            drawArrow(p, point2, l, angle + 90, QString("F=%1").arg(fabs(elements[elementToDraw].fee(4))), elements[elementToDraw].fee(4) < 0);
            drawCircularArrow(p, point2, 15, 225, elements[elementToDraw].fee(5) > 0 ? 180 : -180, QString("M=%1").arg(fabs(elements[elementToDraw].fee(5))));
        }
    }

    p.restore();
}

void CanvasWidget::drawConstraintForces(QPainter &p) {
    p.save();

    drawNodes(p);
    drawElements(p);
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
        } else if(iter.value().dir == 2) {
            QPointF point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawCircularArrow(p, point1, 15, 225, iter.value().force > 0 ? 180 : -180, QString("Constraint %1 M=%2").arg(iter.key()).arg(fabs(iter.value().force)));
            iter++;
            continue;
        }
        QPointF point1;
        switch (dir) {
        case 0:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 0, QString("Constraint %1 F=%2").arg(iter.key()).arg(fabs(iter.value().force)), !(iter.value().force > 0));
            break;
        case 1:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 90, QString("Constraint %1 F=%2").arg(iter.key()).arg(fabs(iter.value().force)), !(iter.value().force > 0));
            break;
        case 2:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 180, QString("Constraint %1 F=%2").arg(iter.key()).arg(fabs(iter.value().force)), !(iter.value().force < 0));
            break;
        case 3:
            point1 = QPointF(nodes[iter.value().node].pos.x(), -nodes[iter.value().node].pos.y()) * scaleRatio;
            drawArrow(p, point1, l, 270, QString("Constraint %1 F=%2").arg(iter.key()).arg(fabs(iter.value().force)), !(iter.value().force < 0));
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

void CanvasWidget::drawCircularArrow(QPainter &p, QPointF point, double r, double alpha, double theta, QString str) {
    p.save();

    QPen pen;
    pen.setWidth(2);
    p.setPen(pen);

    QRectF rect(point.x() - r, point.y() - r, 2 * r, 2 * r);
    int startAngle = alpha * 16;
    int spanAngle = theta * 16;
    p.drawArc(rect, startAngle, spanAngle);

    QPointF point2(point.x() + r * cos((alpha + theta) * PI / 180), point.y() - r * sin((alpha + theta) * PI / 180));
    double arrowLength = r / 2;
    double arrowAngle = 30;
    QPointF point3, point4;
    if(theta > 0) {
        point3 = point2 + arrowLength * QPointF(cos((alpha + theta - 10 + 270 - arrowAngle) * PI / 180), -sin((alpha + theta - 10 + 270 - arrowAngle) * PI / 180));
        point4 = point2 + arrowLength * QPointF(cos((alpha + theta - 10 - 90 + arrowAngle) * PI / 180), -sin((alpha + theta - 10 - 90 + arrowAngle) * PI / 180));
    } else if(theta < 0) {
        point3 = point2 + arrowLength * QPointF(cos((alpha + theta + 10 + 90 - arrowAngle) * PI / 180), -sin((alpha + theta + 10 + 90 - arrowAngle) * PI / 180));
        point4 = point2 + arrowLength * QPointF(cos((alpha + theta + 10 - 270 + arrowAngle) * PI / 180), -sin((alpha + theta + 10 - 270 + arrowAngle) * PI / 180));
    }
    p.drawLine(point2, point3);
    p.drawLine(point2, point4);

    if(IfShowLabels) {
        QRectF rect;
        int flag, width = 200, height = 50;
        int angle = (int)(alpha + theta) % 360;
        if(angle >= 0 && angle <= 90) {
            rect = QRectF(point2.x(), point2.y() - height, width, height);
            flag = Qt::AlignLeft | Qt::AlignBottom;
        } else if(angle > 90 && angle <= 180) {
            rect = QRectF(point2.x() - width, point2.y() - height, width, height);
            flag = Qt::AlignRight | Qt::AlignBottom;
        } else if(angle > 180 && angle <= 270) {
            rect = QRectF(point2.x() - width, point2.y(), width, height);
            flag = Qt::AlignRight | Qt::AlignTop;
        } else {
            rect = QRectF(point2.x(), point2.y(), width, height);
            flag = Qt::AlignLeft | Qt::AlignTop;
        }
        p.drawText(rect, flag, str);
        //p.drawRect(rect);
    }

    p.restore();
}

void CanvasWidget::clear() {
    elementType = 0;
    this->nodes.clear();
    this->elements.clear();
    this->constraints.clear();
    this->loads.clear();
    this->deformedDisplacements.clear();
    originalPoints.clear();
    originPos = QPointF(this->size().width() / 2, this->size().height() / 2);
    scaleRatio = 1.0f;
    maxElementLength = 0;
    this->solved = false;
    draw_type = DEFINE_SHAPE;
    topLeftMessage = "";
    this->update();
}

void CanvasWidget::setElementType(int type) {
    elementType = type;
}

void CanvasWidget::setNodes(QMap<int, Node> &nodes) {
    this->nodes = nodes;
}

void CanvasWidget::setElements(QMap<int, Element> &elements) {
    this->elements = elements;
    QMap<int, Element>::iterator iter = this->elements.begin();
    maxElementLength = 0;
    while(iter != this->elements.end()) {
        if(iter.value().L > maxElementLength)
            maxElementLength = iter.value().L;
        iter++;
    }
}

void CanvasWidget::setConstraints(QMap<int, Constraint> &constraints) {
    this->constraints = constraints;
}

void CanvasWidget::setLoads(QMap<int, Load> &loads) {
    this->loads = loads;
}

void CanvasWidget::draw(DRAW_TYPE type, int element, int coord) {
    draw_type = type;
    if(draw_type == DEFORM_SHAPE) {
        topLeftMessage = "Node Displacement:\n";
        QMap<int, Node>::iterator iter = nodes.begin();
        while(iter != nodes.end()) {
            topLeftMessage += QString("Node %1:(").arg(iter.key());
            for(int i = 0; i < iter.value().displacement.size(); i++) {
                if(i != 0)
                    topLeftMessage += ",";
                topLeftMessage += QString("%1").arg(iter.value().displacement[i]);
            }
            topLeftMessage += ")\n";
            iter++;
        }
        topLeftMessage += QString("Display Ratio=%1").arg(deformRatio);
    } else if(draw_type == ELEMENT_INFORMATION) {
        elementToDraw = element;
        this->coord = coord;
        topLeftMessage = QString("Information of Element %1(%2):\n").arg(elementToDraw).arg(coord == 0 ? "Global Coordinate" : "Local Coordinate");
        if(coord == 0) {
            topLeftMessage += QString("Node %1 Displacement:(").arg(elements[elementToDraw].nodeNum1);
            for(int i = 0; i < elements[elementToDraw].de.n / 2; i++) {
                if(i != 0)
                    topLeftMessage += ",";
                topLeftMessage += QString("%1").arg(elements[elementToDraw].de(i));
            }
            topLeftMessage += ")\n";
            topLeftMessage += QString("Node %1 Displacement:(").arg(elements[elementToDraw].nodeNum2);
            for(int i = elements[elementToDraw].de.n / 2; i < elements[elementToDraw].de.n; i++) {
                if(i != elements[elementToDraw].de.n / 2)
                    topLeftMessage += ",";
                topLeftMessage += QString("%1").arg(elements[elementToDraw].de(i));
            }
            topLeftMessage += ")\n";
            topLeftMessage += QString("Node %1 Force:(").arg(elements[elementToDraw].nodeNum1);
            for(int i = 0; i < elements[elementToDraw].fe.n / 2; i++) {
                if(i != 0)
                    topLeftMessage += ",";
                topLeftMessage += QString("%1").arg(elements[elementToDraw].fe(i));
            }
            topLeftMessage += ")\n";
            topLeftMessage += QString("Node %1 Force:(").arg(elements[elementToDraw].nodeNum2);
            for(int i = elements[elementToDraw].fe.n / 2; i < elements[elementToDraw].fe.n; i++) {
                if(i != elements[elementToDraw].fe.n / 2)
                    topLeftMessage += ",";
                topLeftMessage += QString("%1").arg(elements[elementToDraw].fe(i));
            }
            topLeftMessage += ")\n";
        } else if(coord == 1) {
            topLeftMessage += QString("Node %1 Displacement:(").arg(elements[elementToDraw].nodeNum1);
            for(int i = 0; i < elements[elementToDraw].dee.n / 2; i++) {
                if(i != 0)
                    topLeftMessage += ",";
                topLeftMessage += QString("%1").arg(elements[elementToDraw].dee(i));
            }
            topLeftMessage += ")\n";
            topLeftMessage += QString("Node %1 Displacement:(").arg(elements[elementToDraw].nodeNum2);
            for(int i = elements[elementToDraw].dee.n / 2; i < elements[elementToDraw].dee.n; i++) {
                if(i != elements[elementToDraw].dee.n / 2)
                    topLeftMessage += ",";
                topLeftMessage += QString("%1").arg(elements[elementToDraw].dee(i));
            }
            topLeftMessage += ")\n";
            topLeftMessage += QString("Node %1 Force:(").arg(elements[elementToDraw].nodeNum1);
            for(int i = 0; i < elements[elementToDraw].fee.n / 2; i++) {
                if(i != 0)
                    topLeftMessage += ",";
                topLeftMessage += QString("%1").arg(elements[elementToDraw].fee(i));
            }
            topLeftMessage += ")\n";
            topLeftMessage += QString("Node %1 Force:(").arg(elements[elementToDraw].nodeNum2);
            for(int i = elements[elementToDraw].fee.n / 2; i < elements[elementToDraw].fee.n; i++) {
                if(i != elements[elementToDraw].fee.n / 2)
                    topLeftMessage += ",";
                topLeftMessage += QString("%1").arg(elements[elementToDraw].fee(i));
            }
            topLeftMessage += ")\n";
        }
        if(elementType == 1) {
            topLeftMessage += QString("Internal Force:%1\n").arg(elements[elementToDraw].IF);
            topLeftMessage += QString("Stress:%1\n").arg(elements[elementToDraw].stress);
        }
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
    if(solved) {
        originalPoints.clear();
        deformedDisplacements.clear();
        double max_displacement = -std::numeric_limits<double>::max();
        QMap<int, Element>::iterator elementIter = elements.begin();
        while(elementIter != elements.end()) {
            QPointF point1 = QPointF(nodes[elementIter.value().nodeNum1].pos.x(), -nodes[elementIter.value().nodeNum1].pos.y());// * scaleRatio;
            QPointF point2 = QPointF(nodes[elementIter.value().nodeNum2].pos.x(), -nodes[elementIter.value().nodeNum2].pos.y());// * scaleRatio;
            if(elementType == 1) {
                QPointF d0, d1;
                d0 = QPointF(nodes[elementIter.value().nodeNum1].displacement[0], -nodes[elementIter.value().nodeNum1].displacement[1]);// * deformRatio * scaleRatio;
                d1 = QPointF(nodes[elementIter.value().nodeNum2].displacement[0], -nodes[elementIter.value().nodeNum2].displacement[1]);// * deformRatio * scaleRatio;
                if(fabs(d0.x()) > max_displacement)
                    max_displacement = fabs(d0.x());
                if(fabs(d0.y()) > max_displacement)
                    max_displacement = fabs(d0.y());
                if(fabs(d1.x()) > max_displacement)
                    max_displacement = fabs(d1.x());
                if(fabs(d1.y()) > max_displacement)
                    max_displacement = fabs(d1.y());
                deformedDisplacements[elementIter.key()] << d0 << d1;
                originalPoints[elementIter.key()] << point1 << point2;
            } else if(elementType == 2) {
                QPainterPath path;
                int num = 100;
                double L = elementIter.value().L;
                Matrix<double> A = Matrix<double>(4, 4);
                A(0, 0) = A(1, 1) = 1;
                A(2, 0) = -3 / L / L; A(2, 1) = -2 / L; A(2, 2) = 3 / L / L; A(2, 3) = -1 / L;
                A(3, 0) = 2 / L / L / L; A(3, 1) = 1 / L / L; A(3, 2) = -2 / L / L / L; A(3, 3) = 1 / L / L;
                Matrix<double> d3 = Matrix<double>(4, 1);
                d3(0) = elementIter.value().dee(1);
                d3(1) = elementIter.value().dee(2);
                d3(2) = elementIter.value().dee(4);
                d3(3) = elementIter.value().dee(5);
                for(int i = 0; i < num; i++) {
                    QPointF point = point1 + (point2 - point1) * i / (num - 1);
                    double x = i * L / (num - 1);
                    Matrix<double> temp = Matrix<double>(1, 4);
                    temp(0) = 1; temp(1) = x; temp(2) = x * x; temp(3) = x * x * x;
                    temp = temp * A * d3;
                    double ve = temp(0);
                    double ue = (1 - x / L) * elementIter.value().dee(0) + x / L * elementIter.value().dee(3);
                    Matrix<double> dee = Matrix<double>(6, 1);
                    dee(3) = ue;
                    dee(4) = ve;
                    Matrix<double> de = elementIter.value().T.trans() * dee;
                    double u, v;
                    u = de(3);
                    v = de(4);
                    if(fabs(u) > max_displacement)
                        max_displacement = fabs(u);
                    if(fabs(v) > max_displacement)
                        max_displacement = fabs(v);
                    originalPoints[elementIter.key()] << point;
                    deformedDisplacements[elementIter.key()] << QPointF(u, -v);
                }
            }
            elementIter++;
        }
        deformRatio = 0.03 * maxElementLength / max_displacement;
    }
    if(solved != this->solved) {
        this->solved = solved;
        this->update();
    }
}

void CanvasWidget::fit() {
    originPos = QPointF(this->size().width() / 2, this->size().height() / 2);
    if(nodes.size() == 0) {
        scaleRatio = 1.0f;
    } else {
        QSize s = this->size();

        double maxX = -std::numeric_limits<double>::max(), maxY = -std::numeric_limits<double>::max();
        double minX = std::numeric_limits<double>::max(), minY = std::numeric_limits<double>::max();
        if(draw_type == ELEMENT_INFORMATION) {
            maxX = (nodes[elements[elementToDraw].nodeNum1].pos.x() > nodes[elements[elementToDraw].nodeNum2].pos.x()) ? nodes[elements[elementToDraw].nodeNum1].pos.x() : nodes[elements[elementToDraw].nodeNum2].pos.x();
            minX = (nodes[elements[elementToDraw].nodeNum1].pos.x() < nodes[elements[elementToDraw].nodeNum2].pos.x()) ? nodes[elements[elementToDraw].nodeNum1].pos.x() : nodes[elements[elementToDraw].nodeNum2].pos.x();
            maxY = (nodes[elements[elementToDraw].nodeNum1].pos.y() > nodes[elements[elementToDraw].nodeNum2].pos.y()) ? nodes[elements[elementToDraw].nodeNum1].pos.y() : nodes[elements[elementToDraw].nodeNum2].pos.y();
            minY = (nodes[elements[elementToDraw].nodeNum1].pos.y() < nodes[elements[elementToDraw].nodeNum2].pos.y()) ? nodes[elements[elementToDraw].nodeNum1].pos.y() : nodes[elements[elementToDraw].nodeNum2].pos.y();
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

void CanvasWidget::showElements(bool checked) {
    IfShowElements = checked;
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
