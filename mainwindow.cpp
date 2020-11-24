#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "selectdialog.h"
#include "selectrodresultdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setLineEditValidator();

    connect(ui->NodeAddBtn, SIGNAL(clicked(bool)), this, SLOT(addNode()));
    connect(ui->NodeDelBtn, SIGNAL(clicked(bool)), this, SLOT(deleteNode()));
    connect(ui->RodAddBtn, SIGNAL(clicked(bool)), this, SLOT(addRod()));
    connect(ui->RodDelBtn, SIGNAL(clicked(bool)), this, SLOT(deleteRod()));
    connect(ui->ConAddBtn, SIGNAL(clicked(bool)), this, SLOT(addConstraint()));
    connect(ui->ConDelBtn, SIGNAL(clicked(bool)), this, SLOT(deleteConstraint()));
    connect(ui->LoadAddBtn, SIGNAL(clicked(bool)), this, SLOT(addLoad()));
    connect(ui->LoadDelBtn, SIGNAL(clicked(bool)), this, SLOT(deleteLoad()));
    connect(ui->SolveBtn, SIGNAL(clicked(bool)), this, SLOT(solve()));

    connect(ui->NewAction, SIGNAL(triggered(bool)), this, SLOT(newFile()));
    connect(ui->OpenAction, SIGNAL(triggered(bool)), this, SLOT(open()));
    connect(ui->SaveAction, SIGNAL(triggered(bool)), this, SLOT(save()));
    connect(ui->SaveAsAction, SIGNAL(triggered(bool)), this, SLOT(saveAs()));
    connect(ui->FitAction, SIGNAL(triggered(bool)), ui->Canvas, SLOT(fit()));
    connect(ui->ShowAxisAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showAxis(bool)));
    connect(ui->ShowLabelsAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showLabels(bool)));
    connect(ui->ShowNodesAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showNodes(bool)));
    connect(ui->ShowRodsAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showRods(bool)));
    connect(ui->ShowConstraintsAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showConstraints(bool)));
    connect(ui->ShowLoadsAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showLoads(bool)));
    connect(ui->ShowResetAction, SIGNAL(triggered(bool)), this, SLOT(showReset()));
    connect(ui->ShowConsoleAction, SIGNAL(toggled(bool)), ui->OutputEdit, SLOT(setVisible(bool)));
    connect(ui->PlotDefinedAction, SIGNAL(triggered(bool)), this, SLOT(plotDefinedShape()));
    connect(ui->DeformedShapeAction, SIGNAL(triggered(bool)), this, SLOT(drawDeformedShape()));
    connect(ui->RodInformationAction, SIGNAL(triggered(bool)), this, SLOT(plotRodInformation()));
    connect(ui->ConstraintForcesAction, SIGNAL(triggered(bool)), this, SLOT(plotConstraintForces()));
    connect(ui->AboutAction, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(ui->AboutQtAction, SIGNAL(triggered(bool)), qApp, SLOT(aboutQt()));

    setWindowIcon(QIcon(":/images/icon.png"));
    setCurrentFile("");

    solved = false;
    ui->Canvas->setSolved(false);

    layout()->setSizeConstraint(QLayout::SetFixedSize);
    ui->OutputEdit->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if(okToContinue()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::setLineEditValidator() {
    QDoubleValidator *doubleV = new QDoubleValidator(this);
    QDoubleValidator *positiveDoubleV = new QDoubleValidator(this);
    positiveDoubleV->setBottom(0);

    ui->NodeXEdit->setValidator(doubleV);
    ui->NodeYEdit->setValidator(doubleV);
    ui->RodEEdit->setValidator(positiveDoubleV);
    ui->RodAEdit->setValidator(positiveDoubleV);
    ui->LoadForceEdit->setValidator(doubleV);
}

void MainWindow::printNodes() {
    QString str = "Nodes:\n";
    QMap<int, Node>::iterator iter = nodes.begin();
    while(iter != nodes.end()) {
        str += QString("Node %1: (%2, %3)\n").arg(iter.key()).arg(iter.value().pos.x()).arg(iter.value().pos.y());
        iter++;
    }
    ui->OutputEdit->append(str);
}

void MainWindow::printRods() {
    QString str = "Rods:\n";
    QMap<int, Rod>::iterator iter = rods.begin();
    while(iter != rods.end()) {
        str += QString("Rod %1: Node %2 and Node %3, E = %4, A = %5, L = %6\n").arg(iter.key()).arg(iter.value().nodeNum1).arg(iter.value().nodeNum2).arg(iter.value().E).arg(iter.value().A).arg(iter.value().L);
        iter++;
    }
    ui->OutputEdit->append(str);
}

void MainWindow::printConstraints() {
    QString str = "Constraints:\n";
    QMap<int, Constraint>::iterator iter = constraints.begin();
    while(iter != constraints.end()) {
        str += QString("Constraint %1: %2 direction of Node %3\n").arg(iter.key()).arg(iter.value().dir == 0 ? "x" : "y").arg(iter.value().node);
        iter++;
    }
    ui->OutputEdit->append(str);
}

void MainWindow::printLoads() {
    QString str = "Loads:\n";
    QMap<int, Load>::iterator iter = loads.begin();
    while(iter != loads.end()) {
        str += QString("Load %1: %2 direction of Node %3, force = %4\n").arg(iter.key()).arg(iter.value().dir == 0 ? "x" : "y").arg(iter.value().node).arg(iter.value().f);
        iter++;
    }
    ui->OutputEdit->append(str);
}

void MainWindow::printMessage(QString message) {
    ui->OutputEdit->append(message);
}

void MainWindow::printSolution() {
    QString str = "\n";
    str += "---------------------------------------Solution---------------------------------------\n";

    str += "Node Displacement:\n";
    QMap<int, Node>::iterator nodeIter = nodes.begin();
    while(nodeIter != nodes.end()) {
        str += QString("Displacement of Node %1:(%2, %3)\n").arg(nodeIter.key()).arg(nodeIter.value().displacement[0]).arg(nodeIter.value().displacement[1]);
        nodeIter++;
    }
    str += "\n";

    str += "Rod Information:\n";
    QMap<int, Rod>::iterator rodIter = rods.begin();
    while(rodIter != rods.end()) {
        str += QString("Rod %1:\n").arg(rodIter.key());
        str += QString("Node displacement in global coordinate: (%1, %2), (%3, %4)\n").arg(rodIter.value().de(0)).arg(rodIter.value().de(1)).arg(rodIter.value().de(2)).arg(rodIter.value().de(3));
        str += QString("Node displacement in local coordinate: %1, %2\n").arg(rodIter.value().dee(0)).arg(rodIter.value().dee(1));
        str += QString("Node force in global coordinate: (%1, %2), (%3, %4)\n").arg(rodIter.value().fe(0)).arg(rodIter.value().fe(1)).arg(rodIter.value().fe(2)).arg(rodIter.value().fe(3));
        str += QString("Node force in local coordinate: %1, %2\n").arg(rodIter.value().fee(0)).arg(rodIter.value().fee(1));
        str += QString("Internal force: %1\n").arg(rodIter.value().IF);
        str += QString("Stress: %1\n").arg(rodIter.value().stress);
        rodIter++;
    }
    str += "\n";

    str += "Constraint force:\n";
    QMap<int, Constraint>::iterator constraintIter = constraints.begin();
    while(constraintIter != constraints.end()) {
        str += QString("Constraint force at %1 direction of %2: %3\n").arg(constraintIter.value().dir == 0 ? "x" : "y").arg(constraintIter.value().node).arg(constraintIter.value().force);
        constraintIter++;
    }

    ui->OutputEdit->append(str);
}

bool MainWindow::okToContinue() {
    if(isWindowModified()) {
        int r = QMessageBox::warning(this, tr("FEM"),
                                     tr("The file has been modified.\n"
                                        "Do you want to save your changes?"),
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(r == QMessageBox::Yes) {
            return save();
        } else if(r == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::clear() {
    ui->NodeXEdit->clear();
    ui->NodeYEdit->clear();
    ui->RodNode1CB->clear();
    ui->RodNode2CB->clear();
    ui->RodEEdit->clear();
    ui->RodAEdit->clear();
    ui->ConNodeCB->clear();
    ui->ConDirCB->setCurrentIndex(0);
    ui->LoadNodeCB->clear();
    ui->LoadDirCB->setCurrentIndex(0);
    ui->LoadForceEdit->clear();
    ui->Canvas->clear();
    ui->OutputEdit->clear();
    showReset();
    nodes.clear();
    rods.clear();
    constraints.clear();
    loads.clear();
    nodeInRods.clear();
    solved = false;
}

bool MainWindow::loadFile(const QString &filename) {
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("FEM"),
                             tr("Cannot read file %1:\n%2.").arg(file.fileName()).arg(file.errorString()));
        ui->statusBar->showMessage(tr("Loading failed"), 2000);
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_3);

    quint32 magic;
    in >> magic;
    if(magic != MagicNumber) {
        QMessageBox::warning(this, tr("FEM"), tr("The file is not a FEM file."));
        ui->statusBar->showMessage(tr("Loading failed"), 2000);
        return false;
    }

    clear();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    quint8 IfSolved;
    in >> IfSolved;
    solved = IfSolved;

    quint32 nodesSize;
    in >> nodesSize;
    for(quint32 i = 0; i < nodesSize; i++) {
        quint32 key;
        double x, y;
        in >> key >> x >> y;
        QPointF pos(x, y);
        Node node;
        node.pos = pos;
        if(solved) {
            double dx, dy;
            in >> dx >> dy;
            node.displacement = QVector<double>();
            node.displacement << dx << dy;
        }
        nodes[key] = node;
        nodeInRods[key] = 0;
        ui->RodNode1CB->addItem("Node " + QString::number(key), key);
        ui->RodNode2CB->addItem("Node " + QString::number(key), key);
        ui->ConNodeCB->addItem("Node " + QString::number(key), key);
        ui->LoadNodeCB->addItem("Node " + QString::number(key), key);
    }

    quint32 rodsSize;
    in >> rodsSize;
    for(quint32 i = 0; i < rodsSize; i++) {
        quint32 key, node1, node2;
        double E, A;
        in >> key >> node1 >> node2 >> E >> A;
        Rod rod;
        initRod(rod, nodes, node1, node2, E, A);
        //calRod(2, nodes, rod);
        rods[key] = rod;
        nodeInRods[node1]++;
        nodeInRods[node2]++;
    }
    if(solved)
        calRods(2, nodes, rods);

    quint32 constraintsSize;
    in >> constraintsSize;
    for(quint32 i = 0; i < constraintsSize; i++) {
        quint32 key, node;
        quint8 dir;
        in >> key >> node >> dir;
        Constraint constraint;
        constraint.node = node;
        constraint.dir = dir;
        constraints[key] = constraint;
    }
    if(solved)
        calConstraintForce(2, nodes, rods, constraints);

    quint32 loadsSize;
    in >> loadsSize;
    for(quint32 i = 0; i < loadsSize; i++) {
        quint32 key, node;
        quint8 dir;
        double f;
        in >> key >> node >> dir >> f;
        Load load;
        load.node = node;
        load.dir = dir;
        load.f = f;
        loads[key] = load;
    }

    QApplication::restoreOverrideCursor();

    printNodes();
    printRods();
    printConstraints();
    printLoads();

    ui->Canvas->setNodes(nodes);
    ui->Canvas->setRods(rods);
    ui->Canvas->setConstraints(constraints);
    ui->Canvas->setLoads(loads);
    ui->Canvas->setSolved(solved);
    ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    ui->Canvas->fit();

    setCurrentFile(filename);
    ui->statusBar->showMessage(tr("File loaded"), 2000);

    setResultActionsEnabled(solved);
    return true;
}

bool MainWindow::saveFile(const QString &filename) {
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("FEM"),
                             tr("Cannot write file %1:\n%2.").arg(file.fileName()).arg(file.errorString()));
        ui->statusBar->showMessage(tr("Saving canceled"), 2000);
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_3);

    out << quint32(MagicNumber);

    out << quint8(solved);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    out << quint32(nodes.size());
    QMap<int, Node>::iterator nodesIter = nodes.begin();
    while(nodesIter != nodes.end()) {
        out << quint32(nodesIter.key()) << nodesIter.value().pos.x() << nodesIter.value().pos.y();
        if(solved)
            out << nodesIter.value().displacement[0] << nodesIter.value().displacement[1];
        nodesIter++;
    }

    out << quint32(rods.size());
    QMap<int, Rod>::iterator rodsIter = rods.begin();
    while(rodsIter != rods.end()) {
        out << quint32(rodsIter.key()) << quint32(rodsIter.value().nodeNum1) << quint32(rodsIter.value().nodeNum2) << rodsIter.value().E << rodsIter.value().A;
        rodsIter++;
    }

    out << quint32(constraints.size());
    QMap<int, Constraint>::iterator constraintsIter = constraints.begin();
    while(constraintsIter != constraints.end()) {
        out << quint32(constraintsIter.key()) << quint32(constraintsIter.value().node) << quint8(constraintsIter.value().dir);
        constraintsIter++;
    }

    out << quint32(loads.size());
    QMap<int, Load>::iterator loadsIter = loads.begin();
    while(loadsIter != loads.end()) {
        out << quint32(loadsIter.key()) << quint32(loadsIter.value().node) << quint8(loadsIter.value().dir) << loadsIter.value().f;
        loadsIter++;
    }

    QApplication::restoreOverrideCursor();

    setCurrentFile(filename);
    ui->statusBar->showMessage(tr("File saved"), 2000);

    return true;
}

void MainWindow::setCurrentFile(const QString &filename) {
    curFile = filename;
    setWindowModified(false);
    QString shownName = tr("Untitled");
    if(!curFile.isEmpty()) {
        shownName = strippedName(curFile);
    }
    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("FEM")));
}

QString MainWindow::strippedName(const QString &fullFileName) {
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::setResultActionsEnabled(bool enabled) {
    ui->DeformedShapeAction->setEnabled(enabled);
    ui->RodInformationAction->setEnabled(enabled);
    ui->ConstraintForcesAction->setEnabled(enabled);
}

void MainWindow::addNode() {
    if(ui->NodeXEdit->text().isEmpty() || ui->NodeYEdit->text().isEmpty())
        return;
    double x = ui->NodeXEdit->text().toDouble();
    double y = ui->NodeYEdit->text().toDouble();

    QMap<int, Node>::iterator iter = nodes.begin();
    while(iter != nodes.end()) {
        if((iter.value().pos.x() == x && iter.value().pos.y() == y)) {
            printMessage(QString("Node at (%1, %2) already exists.").arg(x).arg(y));
            return;
        }
        iter++;
    }

    QPointF pos(x, y);
    Node node;
    node.pos = pos;
    if(nodes.isEmpty()) {
        nodes[1] = node;
    } else {
        nodes[nodes.lastKey() + 1] = node;
    }
    nodeInRods[nodes.lastKey()] = 0;

    ui->NodeXEdit->clear();
    ui->NodeYEdit->clear();
    ui->RodNode1CB->addItem("Node " + QString::number(nodes.lastKey()), nodes.lastKey());
    ui->RodNode2CB->addItem("Node " + QString::number(nodes.lastKey()), nodes.lastKey());
    ui->ConNodeCB->addItem("Node " + QString::number(nodes.lastKey()), nodes.lastKey());
    ui->LoadNodeCB->addItem("Node " + QString::number(nodes.lastKey()), nodes.lastKey());

    printNodes();
    setWindowModified(true);
    solved = false;
    ui->Canvas->setSolved(false);
    setResultActionsEnabled(false);
    ui->Canvas->setNodes(nodes);
    ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    if(nodes.size() == 1)
        ui->Canvas->fit();
}

void MainWindow::deleteNode() {
    if(nodes.size() == 0)
        return;
    SelectDialog *dialog = new SelectDialog("Delete Node", nodes.keys(), this);
    if(dialog->exec()) {
        int node = dialog->comboBox->currentData().toInt();
        bool rodsModed = false, constraintsModed = false, loadsModed = false;

        if(nodeInRods[node] != 0) {
            QMap<int, Rod>::iterator rodIter = rods.begin();
            while(rodIter != rods.end()) {
                if(rodIter.value().nodeNum1 == node) {
                    nodeInRods[rodIter.value().nodeNum2]--;
                    rodIter = rods.erase(rodIter);
                    rodsModed = true;
                    continue;
                } else if(rodIter.value().nodeNum2 == node) {
                    nodeInRods[rodIter.value().nodeNum1]--;
                    rodIter = rods.erase(rodIter);
                    rodsModed = true;
                    continue;
                } else {
                    rodIter++;
                }
            }
        }
        if(rodsModed) {
            printRods();
            ui->Canvas->setRods(rods);
        }

        QMap<int, Constraint>::iterator constraintIter = constraints.begin();
        while(constraintIter != constraints.end()) {
            if(constraintIter.value().node == node) {
                constraintIter = constraints.erase(constraintIter);
                constraintsModed = true;
                continue;
            } else {
                constraintIter++;
            }
        }
        if(constraintsModed) {
            printConstraints();
            ui->Canvas->setConstraints(constraints);
        }

        QMap<int, Load>::iterator loadIter = loads.begin();
        while(loadIter != loads.end()) {
            if(loadIter.value().node == node) {
                loadIter = loads.erase(loadIter);
                loadsModed = true;
                continue;
            } else {
                loadIter++;
            }
        }
        if(loadsModed) {
            printLoads();
            ui->Canvas->setLoads(loads);
        }

        ui->RodNode1CB->removeItem(dialog->comboBox->currentIndex());
        ui->RodNode2CB->removeItem(dialog->comboBox->currentIndex());
        ui->ConNodeCB->removeItem(dialog->comboBox->currentIndex());
        ui->LoadNodeCB->removeItem(dialog->comboBox->currentIndex());

        nodes.remove(node);
        nodeInRods.remove(node);
        printNodes();
        ui->Canvas->setNodes(nodes);
        ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);

        setWindowModified(true);
        solved = false;
        ui->Canvas->setSolved(false);
        setResultActionsEnabled(false);
    }
    delete dialog;
}

void MainWindow::addRod() {
    if(ui->RodNode1CB->count() == 0 || ui->RodNode2CB->count() == 0 || ui->RodEEdit->text().isEmpty() || ui->RodAEdit->text().isEmpty())
        return;
    if(ui->RodNode1CB->currentData() == ui->RodNode2CB->currentData()) {
        printMessage("Node1 can not be equal to Node2.");
        return;
    }

    int node1 = ui->RodNode1CB->currentData().toInt();
    int node2 = ui->RodNode2CB->currentData().toInt();

    QMap<int, Rod>::iterator iter = rods.begin();
    while(iter != rods.end()) {
        if((iter.value().nodeNum1 == node1 && iter.value().nodeNum2 == node2) || (iter.value().nodeNum1 == node2 && iter.value().nodeNum2 == node1)) {
            printMessage(QString("Node %1 and Node %2 have already been connected.").arg(node1).arg(node2));
            return;
        }
        iter++;
    }

    double E = ui->RodEEdit->text().toDouble();
    double A = ui->RodAEdit->text().toDouble();
    Rod rod;
    initRod(rod, nodes, node1, node2, E, A);
    if(rods.isEmpty()) {
        rods[1] = rod;
    } else {
        rods[rods.lastKey() + 1] = rod;
    }
    nodeInRods[node1]++;
    nodeInRods[node2]++;
    ui->RodEEdit->clear();
    ui->RodAEdit->clear();
    //ui->RodNode1CB->setCurrentIndex(0);
    //ui->RodNode2CB->setCurrentIndex(0);
    printRods();
    setWindowModified(true);
    solved = false;
    ui->Canvas->setSolved(false);
    setResultActionsEnabled(false);
    ui->Canvas->setRods(rods);
    ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
}

void MainWindow::deleteRod() {
    if(rods.size() == 0)
        return;
    SelectDialog *dialog = new SelectDialog("Delete Rod", rods.keys(), this);
    if(dialog->exec()) {
        nodeInRods[rods[dialog->comboBox->currentData().toInt()].nodeNum1]--;
        nodeInRods[rods[dialog->comboBox->currentData().toInt()].nodeNum2]--;
        rods.remove(dialog->comboBox->currentData().toInt());
        printRods();
        setWindowModified(true);
        solved = false;
        ui->Canvas->setSolved(false);
        setResultActionsEnabled(false);
        ui->Canvas->setRods(rods);
        ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    }
    delete dialog;
}

void MainWindow::addConstraint() {
    int node = ui->ConNodeCB->currentData().toInt();
    int dir = ui->ConDirCB->currentIndex();

    QMap<int, Constraint>::iterator iter = constraints.begin();
    while(iter != constraints.end()) {
        if(iter.value().node == node && iter.value().dir == dir) {
            printMessage(QString("Constraint at %1 direction of Node %2 already existes.").arg(dir == 0 ? "x" : "y").arg(node));
            return;
        }
        iter++;
    }
    QMap<int, Load>::iterator iter2 = loads.begin();
    while(iter2 != loads.end()) {
        if(iter2.value().node == node && iter2.value().dir == dir) {
            printMessage(QString("There is load at %1 direction of Node %2.").arg(dir == 0 ? "x" : "y").arg(node));
            return;
        }
        iter2++;
    }

    Constraint c;
    c.node = node;
    c.dir = dir;
    if(constraints.isEmpty()) {
        constraints[1] = c;
    } else {
        constraints[constraints.lastKey() + 1] = c;
    }
    //ui->ConNodeCB->setCurrentIndex(0);
    //ui->ConDirCB->setCurrentIndex(0);
    printConstraints();
    setWindowModified(true);
    solved = false;
    ui->Canvas->setSolved(false);
    setResultActionsEnabled(false);
    ui->Canvas->setConstraints(constraints);
    ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
}

void MainWindow::deleteConstraint() {
    if(constraints.size() == 0)
        return;
    SelectDialog *dialog = new SelectDialog("Delete Constraint", constraints.keys(), this);
    if(dialog->exec()) {
        constraints.remove(dialog->comboBox->currentData().toInt());
        printConstraints();
        setWindowModified(true);
        solved = false;
        ui->Canvas->setSolved(false);
        setResultActionsEnabled(false);
        ui->Canvas->setConstraints(constraints);
        ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    }
    delete dialog;
}

void MainWindow::addLoad() {
    if(ui->LoadForceEdit->text().isEmpty())
        return;

    int node = ui->LoadNodeCB->currentData().toInt();
    int dir = ui->LoadDirCB->currentIndex();
    double f = ui->LoadForceEdit->text().toDouble();

    QMap<int, Load>::iterator iter = loads.begin();
    while(iter != loads.end()) {
        if(iter.value().node == node && iter.value().dir == dir) {
            printMessage(QString("Load at %1 direction of Node %2 already existes.").arg(dir == 0 ? "x" : "y").arg(node));
            return;
        }
        iter++;
    }
    QMap<int, Constraint>::iterator iter2 = constraints.begin();
    while(iter2 != constraints.end()) {
        if(iter2.value().node == node && iter2.value().dir == dir) {
            printMessage(QString("There is constraint at %1 direction of Node %2.").arg(dir == 0 ? "x" : "y").arg(node));
            return;
        }
        iter2++;
    }

    Load load;
    load.node = node;
    load.dir = dir;
    load.f = f;
    if(loads.isEmpty()) {
        loads[1] = load;
    } else {
        loads[loads.lastKey() + 1] = load;
    }
    //ui->LoadNodeCB->setCurrentIndex(0);
    //ui->LoadDirCB->setCurrentIndex(0);
    ui->LoadForceEdit->clear();
    printLoads();
    setWindowModified(true);
    solved = false;
    ui->Canvas->setSolved(false);
    setResultActionsEnabled(false);
    ui->Canvas->setLoads(loads);
    ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
}

void MainWindow::deleteLoad() {
    if(loads.size() == 0)
        return;
    SelectDialog *dialog = new SelectDialog("Delete Load", loads.keys(), this);
    if(dialog->exec()) {
        loads.remove(dialog->comboBox->currentData().toInt());
        printLoads();
        setWindowModified(true);
        solved = false;
        ui->Canvas->setSolved(false);
        setResultActionsEnabled(false);
        ui->Canvas->setLoads(loads);
        ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    }
    delete dialog;
}

void MainWindow::solve() {
    if(nodes.size() == 0) {
        printMessage("There are no nodes.");
        return;
    }
    if(rods.size() == 0) {
        printMessage("There are no rods.");
        return;
    }
    if(constraints.size() == 0) {
        printMessage("There are no constraints.");
        return;
    }
    if(loads.size() == 0) {
        printMessage("There are no loads.");
        return;
    }
    int DOF = rods.size() * 3 - constraints.size();
    for(QMap<int, int>::iterator iter = nodeInRods.begin(); iter != nodeInRods.end(); iter++) {
        if(iter.value() == 0) {
            printMessage(QString("There is an unconnected node: Node %1.").arg(iter.key()));
            return;
        }
        DOF -= (iter.value() - 1) * 2;
    }
    if(DOF > 0) {
        printMessage(QString("DOF = %1\nDegree of Freedom(DOF) must be less than or equal to 0.").arg(DOF));
        return;
    }

    MatrixIn1D K;
    calStiffnessMatrix(2, nodes, rods, K);
    processConstraints(nodes, constraints, K);

    solveEqns(nodes, K, loads);
    calRods(2, nodes, rods);
    calConstraintForce(2, nodes, rods, constraints);

    printSolution();
    solved = true;
    ui->Canvas->setSolved(true);
    ui->Canvas->setNodes(nodes);
    ui->Canvas->setRods(rods);
    ui->Canvas->setConstraints(constraints);
    setResultActionsEnabled(true);
    setWindowModified(true);

    QMessageBox::information(this, tr("FEM"), tr("Successfully solved!"));
}

void MainWindow::newFile() {
    if(okToContinue()) {
        clear();
        setCurrentFile("");
    }
}

void MainWindow::open() {
    if(okToContinue()) {
        QString filename = QFileDialog::getOpenFileName(this,
                                                        tr("Open FEM file"), ".",
                                                        tr("FEM files (*.fem)"));
        if(!filename.isEmpty())
            loadFile(filename);
    }
}

bool MainWindow::save() {
    if(curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs() {
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save File"), ".",
                                                    tr("FEM files (*.fem)"));
    if(filename.isEmpty())
        return false;

    return saveFile(filename);
}

void MainWindow::about() {
    QMessageBox::about(this, tr("About FEM"),
                tr("<h2>FEM 1.1</h2>"
                   "<p>FEM is a small application that can solve problems of structural mechanics using Finite Element Method(FEM)."));
}

void MainWindow::showReset() {
    ui->ShowNodesAction->setChecked(true);
    ui->ShowRodsAction->setChecked(true);
    ui->ShowConstraintsAction->setChecked(true);
    ui->ShowLoadsAction->setChecked(true);
    ui->ShowAxisAction->setChecked(true);
    ui->ShowLabelsAction->setChecked(true);
}

void MainWindow::drawDeformedShape() {
    //ui->Canvas->setNodes(nodes);
    ui->Canvas->draw(CanvasWidget::DEFORM_SHAPE);
    ui->Canvas->fit();
}

void MainWindow::plotDefinedShape() {
    ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    ui->Canvas->fit();
}

void MainWindow::plotRodInformation() {
    SelectRodResultDialog *dialog = new SelectRodResultDialog(rods.keys(), this);
    if(dialog->exec()) {
        int rod = dialog->comboBox1->currentData().toInt();
        int coord = dialog->comboBox2->currentData().toInt();
        ui->Canvas->draw(CanvasWidget::ROD_INFORMATION, rod, coord);
        ui->Canvas->fit();
    }
    delete dialog;
}

void MainWindow::plotConstraintForces() {
    ui->Canvas->draw(CanvasWidget::CONSTRAINT_FORCES);
    ui->Canvas->fit();
}
