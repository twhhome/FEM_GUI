#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "selectdialog.h"
#include "selectelementresultdialog.h"
#include "outputdialog.h"
#include "inputdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    elementType = 0;

    setupUI();

    connect(ui->NewAction, SIGNAL(triggered(bool)), this, SLOT(newFile()));
    connect(ui->OpenAction, SIGNAL(triggered(bool)), this, SLOT(open()));
    connect(ui->SaveAction, SIGNAL(triggered(bool)), this, SLOT(save()));
    connect(ui->SaveAsAction, SIGNAL(triggered(bool)), this, SLOT(saveAs()));

    connect(ui->MaterialsListAction, SIGNAL(triggered(bool)), this, SLOT(listMaterials()));
    connect(ui->SectionsListAction, SIGNAL(triggered(bool)), this, SLOT(listSections()));
    connect(ui->OffsetsListAction, SIGNAL(triggered(bool)), this, SLOT(listOffsets()));
    connect(ui->NodesListAction, SIGNAL(triggered(bool)), this, SLOT(listNodes()));
    connect(ui->ElementsListAction, SIGNAL(triggered(bool)), this, SLOT(listElements()));
    connect(ui->ConstraintsListAction, SIGNAL(triggered(bool)), this, SLOT(listConstraints()));
    connect(ui->LoadsListAction, SIGNAL(triggered(bool)), this, SLOT(listLoads()));
    connect(ui->ListNodeResultAction, SIGNAL(triggered(bool)), this, SLOT(listNodesDisplacement()));
    connect(ui->ListElementResultAction, SIGNAL(triggered(bool)), this, SLOT(listElementsInformation()));
    connect(ui->ListConstraintResultAction, SIGNAL(triggered(bool)), this, SLOT(listConstraintForces()));

    connect(ui->FitAction, SIGNAL(triggered(bool)), ui->Canvas, SLOT(fit()));
    connect(ui->ShowAxisAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showAxis(bool)));
    connect(ui->ShowLabelsAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showLabels(bool)));
    connect(ui->ShowNodesAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showNodes(bool)));
    connect(ui->ShowElementsAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showElements(bool)));
    connect(ui->ShowConstraintsAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showConstraints(bool)));
    connect(ui->ShowLoadsAction, SIGNAL(toggled(bool)), ui->Canvas, SLOT(showLoads(bool)));
    connect(ui->ShowResetAction, SIGNAL(triggered(bool)), this, SLOT(showReset()));
    connect(ui->PlotDefinedAction, SIGNAL(triggered(bool)), this, SLOT(plotDefinedShape()));
    connect(ui->DeformedShapeAction, SIGNAL(triggered(bool)), this, SLOT(drawDeformedShape()));
    connect(ui->ElementInformationAction, SIGNAL(triggered(bool)), this, SLOT(plotElementInformation()));
    connect(ui->ConstraintForcesAction, SIGNAL(triggered(bool)), this, SLOT(plotConstraintForces()));

    connect(ui->UserGuideAction, SIGNAL(triggered(bool)), this, SLOT(userguide()));
    connect(ui->AboutAction, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(ui->AboutQtAction, SIGNAL(triggered(bool)), qApp, SLOT(aboutQt()));

    setWindowIcon(QIcon(":/images/icon.png"));
    setCurrentFile("");

    solved = false;
    ui->Canvas->setSolved(false);

    //layout()->setSizeConstraint(QLayout::SetFixedSize);
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

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
    case Qt::Key_Delete:
        //qDebug() << "Delete";
        //qDebug() << ui->treeWidget->hasFocus();
        break;
    }
}

void MainWindow::setSolved(bool solved) {
    this->solved = solved;
    ui->Canvas->setSolved(solved);
    ExportItem->setDisabled(!solved);
    setResultActionsEnabled(solved);
}

void MainWindow::setupUI() {
    ui->treeWidget->setHeaderHidden(true);

    PreprocessorItem = new QTreeWidgetItem(ui->treeWidget, QStringList() << tr("Preprocessor"));

    ElementTypeItem = new QTreeWidgetItem(PreprocessorItem, QStringList() << tr("Element Type"));
    PreprocessorItem->addChild(ElementTypeItem);

    MaterialsItem = new QTreeWidgetItem(PreprocessorItem, QStringList() << tr("Materials"));
    PreprocessorItem->addChild(MaterialsItem);
    AddMaterialItem = new QTreeWidgetItem(MaterialsItem, QStringList() << tr("Add"));
    MaterialsItem->addChild(AddMaterialItem);

    SectionsItem = new QTreeWidgetItem(PreprocessorItem, QStringList() << tr("Sections"));
    PreprocessorItem->addChild(SectionsItem);
    AddSectionItem = new QTreeWidgetItem(SectionsItem, QStringList() << tr("Add"));
    SectionsItem->addChild(AddSectionItem);

    OffsetsItem = new QTreeWidgetItem(PreprocessorItem, QStringList() << tr("Offsets"));
    PreprocessorItem->addChild(OffsetsItem);
    AddOffsetItem = new QTreeWidgetItem(OffsetsItem, QStringList() << tr("Add"));
    OffsetsItem->addChild(AddOffsetItem);

    NodesItem = new QTreeWidgetItem(PreprocessorItem, QStringList() << tr("Nodes"));
    PreprocessorItem->addChild(NodesItem);
    AddNodeItem = new QTreeWidgetItem(NodesItem, QStringList() << tr("Add"));
    NodesItem->addChild(AddNodeItem);

    ElementsItem = new QTreeWidgetItem(PreprocessorItem, QStringList() << tr("Elements"));
    PreprocessorItem->addChild(ElementsItem);
    AddElementItem = new QTreeWidgetItem(ElementsItem, QStringList() << tr("Add"));
    ElementsItem->addChild(AddElementItem);

    ConstraintsItem = new QTreeWidgetItem(PreprocessorItem, QStringList() << tr("Constraints"));
    PreprocessorItem->addChild(ConstraintsItem);
    AddConstraintItem = new QTreeWidgetItem(ConstraintsItem, QStringList() << tr("Add"));
    ConstraintsItem->addChild(AddConstraintItem);

    LoadsItem = new QTreeWidgetItem(PreprocessorItem, QStringList() << tr("Loads"));
    PreprocessorItem->addChild(LoadsItem);
    AddLoadItem = new QTreeWidgetItem(LoadsItem, QStringList() << tr("Add"));
    LoadsItem->addChild(AddLoadItem);

    PreprocessorItem->setExpanded(true);

    SolveItem = new QTreeWidgetItem(ui->treeWidget, QStringList() << "Solve");

    ExportItem = new QTreeWidgetItem(ui->treeWidget, QStringList() << "Export Report");
    ExportItem->setDisabled(true);

    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(treeWidgetItemDoubleClick(QTreeWidgetItem*,int)));
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(treeWidgetItemPopMenu(const QPoint&)));
}

void MainWindow::treeWidgetItemDoubleClick(QTreeWidgetItem *item, int column) {
    if(item == ElementTypeItem) {
        if(elementType != 0)
            return;
        if(selectElementType()) {
            QString typeStr;
            if(elementType == 1) {
                typeStr = "Rod";
                OffsetsItem->setDisabled(true);
            } else if(elementType == 2) {
                typeStr = "Beam";
                OffsetsItem->setDisabled(false);
            }
            item->setText(column, QString("Element Type (%1)").arg(typeStr));
            setWindowModified(true);
            setSolved(false);
            ui->Canvas->setElementType(elementType);
        }
    } else if(item == AddMaterialItem) {
        if(elementType == 0) {
            QMessageBox::warning(this, tr("FEM"), tr("Please select element type first."));
            return;
        }
        Material material;
        if(inputMaterial(material, false)) {
            if(materials.isEmpty())
                materials[1] = material;
            else
                materials[materials.lastKey() + 1] = material;
            QTreeWidgetItem *materialItem = new QTreeWidgetItem(MaterialsItem, QStringList() << QString("Material %1").arg(materials.lastKey()));
            MaterialsItem->addChild(materialItem);
            setWindowModified(true);
            setSolved(false);
        }
    } else if(item == AddSectionItem) {
        if(elementType == 0) {
            QMessageBox::warning(this, tr("FEM"), tr("Please select element type first."));
            return;
        }
        Section section;
        if(inputSection(section, false)) {
            if(sections.isEmpty())
                sections[1] = section;
            else
                sections[sections.lastKey() + 1] = section;
            QTreeWidgetItem *sectionItem = new QTreeWidgetItem(SectionsItem, QStringList() << QString("Section %1").arg(sections.lastKey()));
            SectionsItem->addChild(sectionItem);
            setWindowModified(true);
            setSolved(false);
        }
    } else if(item == AddOffsetItem) {
        if(item->isDisabled())
            return;
        if(elementType == 0) {
            QMessageBox::warning(this, tr("FEM"), tr("Please select element type first."));
            return;
        }
        Offset offset;
        if(inputOffset(offset, false)) {
            if(offsets.isEmpty())
                offsets[1] = offset;
            else
                offsets[offsets.lastKey() + 1] = offset;
            QTreeWidgetItem *offsetItem = new QTreeWidgetItem(OffsetsItem, QStringList() << QString("Offset %1").arg(offsets.lastKey()));
            OffsetsItem->addChild(offsetItem);
            setWindowModified(true);
            setSolved(false);
        }
    } else if(item == AddNodeItem) {
        if(elementType == 0) {
            QMessageBox::warning(this, tr("FEM"), tr("Please select element type first."));
            return;
        }
        Node node;
        if(inputNode(node, false)) {
            if(nodes.isEmpty())
                nodes[1] = node;
            else
                nodes[nodes.lastKey() + 1] = node;
            nodeInElements[nodes.lastKey()] = 0;
            QTreeWidgetItem *nodeItem = new QTreeWidgetItem(NodesItem, QStringList() << QString("Node %1").arg(nodes.lastKey()));
            NodesItem->addChild(nodeItem);
            setWindowModified(true);
            setSolved(false);
            ui->Canvas->setNodes(nodes);
            ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
            if(nodes.size() == 1)
                ui->Canvas->fit();
        }
    } else if(item == AddElementItem) {
        if(elementType == 0) {
            QMessageBox::warning(this, tr("FEM"), tr("Please select element type first."));
            return;
        }
        if(nodes.isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), tr("Please input nodes first."));
            return;
        }
        if(materials.isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), tr("Please input materials first."));
            return;
        }
        if(sections.isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), tr("Please input sections first."));
            return;
        }
        /*
        if(elementType == 2 && offsets.isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), tr("Please input offsets first."));
            return;
        }
        */
        Element element;
        if(inputElement(element, false)) {
            initElement(element, elementType, nodes, materials, sections, offsets);
            if(elements.isEmpty())
                elements[1] = element;
            else
                elements[elements.lastKey() + 1] = element;
            nodeInElements[elements.last().nodeNum1]++;
            nodeInElements[elements.last().nodeNum2]++;
            QTreeWidgetItem *elementItem = new QTreeWidgetItem(ElementsItem, QStringList() << QString("Element %1").arg(elements.lastKey()));
            ElementsItem->addChild(elementItem);
            setWindowModified(true);
            setSolved(false);
            ui->Canvas->setElements(elements);
            ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
        }
    } else if(item == AddConstraintItem) {
        if(elementType == 0) {
            QMessageBox::warning(this, tr("FEM"), tr("Please select element type first."));
            return;
        }
        if(nodes.isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), tr("Please input nodes first."));
            return;
        }
        Constraint constraint;
        if(inputConstraint(constraint, false)) {
            if(constraints.isEmpty())
                constraints[1] = constraint;
            else
                constraints[constraints.lastKey() + 1] = constraint;
            QTreeWidgetItem *constraintItem = new QTreeWidgetItem(ConstraintsItem, QStringList() << QString("Constraint %1").arg(constraints.lastKey()));
            ConstraintsItem->addChild(constraintItem);
            setWindowModified(true);
            setSolved(false);
            ui->Canvas->setConstraints(constraints);
            ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
        }
    } else if(item == AddLoadItem) {
        if(elementType == 0) {
            QMessageBox::warning(this, tr("FEM"), tr("Please select element type first."));
            return;
        }
        if(nodes.isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), tr("Please input nodes first."));
            return;
        }
        Load load;
        if(inputLoad(load, false)) {
            if(loads.isEmpty())
                loads[1] = load;
            else
                loads[loads.lastKey() + 1] = load;
            QTreeWidgetItem *loadItem = new QTreeWidgetItem(LoadsItem, QStringList() << QString("Load %1").arg(loads.lastKey()));
            LoadsItem->addChild(loadItem);
            setWindowModified(true);
            setSolved(false);
            ui->Canvas->setLoads(loads);
            ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
        }
    } else if(item == SolveItem) {
        solve();
    } else if(item == ExportItem) {
        if(item->isDisabled())
            return;
        exportReport();
    } else if(item->parent() == MaterialsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        if(inputMaterial(materials[key], true)) {
            QMap<int, Element>::iterator iter = elements.begin();
            while(iter != elements.end()) {
                if(iter.value().materialNum == key)
                    initElement(iter.value(), elementType, nodes, materials, sections, offsets);
                iter++;
            }
            setWindowModified(true);
            setSolved(false);
        }
    } else if(item->parent() == SectionsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        if(inputSection(sections[key], true)) {
            QMap<int, Element>::iterator iter = elements.begin();
            while(iter != elements.end()) {
                if(iter.value().sectionNum == key)
                    initElement(iter.value(), elementType, nodes, materials, sections, offsets);
                iter++;
            }
            setWindowModified(true);
            setSolved(false);
        }
    } else if(item->parent() == OffsetsItem) {
        if(item->isDisabled())
            return;
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        if(inputOffset(offsets[key], true)) {
            QMap<int, Element>::iterator iter = elements.begin();
            while(iter != elements.end()) {
                if(iter.value().offsetNum == key)
                    initElement(iter.value(), elementType, nodes, materials, sections, offsets);
                iter++;
            }
            setWindowModified(true);
            setSolved(false);
        }
    } else if(item->parent() == NodesItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        if(inputNode(nodes[key], true)) {
            QMap<int, Element>::iterator iter = elements.begin();
            while(iter != elements.end()) {
                if(iter.value().nodeNum1 == key || iter.value().nodeNum2 == key)
                    initElement(iter.value(), elementType, nodes, materials, sections, offsets);
                iter++;
            }
            setWindowModified(true);
            setSolved(false);
            ui->Canvas->setNodes(nodes);
            ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
            if(nodes.size() == 1)
                ui->Canvas->fit();
        }
    } else if(item->parent() == ElementsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        if(inputElement(elements[key], true)) {
            initElement(elements[key], elementType, nodes, materials, sections, offsets);
            setWindowModified(true);
            setSolved(false);
            ui->Canvas->setElements(elements);
            ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
        }
    } else if(item->parent() == ConstraintsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        if(inputConstraint(constraints[key], true)) {
            setWindowModified(true);
            setSolved(false);
            ui->Canvas->setConstraints(constraints);
            ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
        }
    } else if(item->parent() == LoadsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        if(inputLoad(loads[key], true)) {
            setWindowModified(true);
            setSolved(false);
            ui->Canvas->setLoads(loads);
            ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
        }
    }
}

void MainWindow::treeWidgetItemPopMenu(const QPoint &) {
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if(item == NULL)
        return;
    if(item == PreprocessorItem || item == ElementTypeItem || item == MaterialsItem || item == AddMaterialItem
            || item == SectionsItem || item == AddSectionItem || item == OffsetsItem || item == AddOffsetItem
            || item == NodesItem || item == AddNodeItem || item == ElementsItem || item == AddElementItem
            || item == ConstraintsItem || item == AddConstraintItem || item == LoadsItem || item == AddLoadItem
            || item == SolveItem || item == ExportItem)
        return;
    if(item->isDisabled())
        return;

    QAction deleteAction(tr("Delete"), this);
    connect(&deleteAction, SIGNAL(triggered(bool)), this, SLOT(deleteItem()));

    QMenu menu(ui->treeWidget);
    menu.addAction(&deleteAction);
    menu.exec(QCursor::pos());
}

bool MainWindow::selectElementType() {
    InputDialog *dialog = new InputDialog(&elementType, this);
    if(dialog->exec()) {
        delete dialog;
        return true;
    } else {
        delete dialog;
        return false;
    }
}

bool MainWindow::inputMaterial(Material &material, bool flag) {
    InputDialog *dialog = new InputDialog(&material, flag, this);
    if(dialog->exec()) {
        delete dialog;
        return true;
    } else {
        delete dialog;
        return false;
    }
}

bool MainWindow::inputSection(Section &section, bool flag) {
    InputDialog *dialog = new InputDialog(&section, &elementType, flag, this);
    if(dialog->exec()) {
        delete dialog;
        return true;
    } else {
        delete dialog;
        return false;
    }
}

bool MainWindow::inputOffset(Offset &offset, bool flag) {
    InputDialog *dialog = new InputDialog(&offset, flag, this);
    if(dialog->exec()) {
        delete dialog;
        return true;
    } else {
        delete dialog;
        return false;
    }
}

bool MainWindow::inputNode(Node &node, bool flag) {
    InputDialog *dialog = new InputDialog(&node, &nodes, flag, this);
    if(dialog->exec()) {
        delete dialog;
        return true;
    } else {
        delete dialog;
        return false;
    }
}

bool MainWindow::inputElement(Element &element, bool flag) {
    InputDialog *dialog = new InputDialog(&element, &elementType, &materials, &sections, &offsets, &nodes, &elements, flag, this);
    if(dialog->exec()) {
        delete dialog;
        return true;
    } else {
        delete dialog;
        return false;
    }
}

bool MainWindow::inputConstraint(Constraint &constraint, bool flag) {
    InputDialog *dialog = new InputDialog(&constraint, &elementType, &nodes, &constraints, &loads, flag, this);
    if(dialog->exec()) {
        delete dialog;
        return true;
    } else {
        delete dialog;
        return false;
    }
}

bool MainWindow::inputLoad(Load &load, bool flag) {
    InputDialog *dialog = new InputDialog(&load, &elementType, &nodes, &constraints, &loads, flag, this);
    if(dialog->exec()) {
        delete dialog;
        return true;
    } else {
        delete dialog;
        return false;
    }
}

void MainWindow::deleteItem() {
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if(item->parent() == MaterialsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        materials.remove(key);
        MaterialsItem->removeChild(item);
        delete item;
        setWindowModified(true);
        setSolved(false);
    } else if(item->parent() == SectionsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        sections.remove(key);
        SectionsItem->removeChild(item);
        delete item;
        setWindowModified(true);
        setSolved(false);
    } else if(item->parent() == OffsetsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        offsets.remove(key);
        OffsetsItem->removeChild(item);
        delete item;
        setWindowModified(true);
        setSolved(false);
    } else if(item->parent() == NodesItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();

        bool elementsModed = false, constraintsModed = false, loadsModed = false;

        if(nodeInElements[key] != 0) {
            QTreeWidgetItemIterator iter(AddElementItem);
            iter += 1;
            while((*iter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Element") {
                qDebug() << (*iter)->text(0);
                int elementKey = (*iter)->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
                if(elements[elementKey].nodeNum1 == key) {
                    nodeInElements[elements[elementKey].nodeNum2]--;
                    elements.remove(elementKey);
                    QTreeWidgetItem *tmp = *iter;
                    ElementsItem->removeChild(*iter);
                    delete tmp;
                    elementsModed = true;
                    continue;
                } else if(elements[elementKey].nodeNum2 == key) {
                    nodeInElements[elements[elementKey].nodeNum1]--;
                    elements.remove(elementKey);
                    QTreeWidgetItem *tmp = *iter;
                    ElementsItem->removeChild(*iter);
                    delete tmp;
                    elementsModed = true;
                    continue;
                }
                ++iter;
            }
        }
        if(elementsModed) {
            ui->Canvas->setElements(elements);
        }

        QTreeWidgetItemIterator constraintIter(AddConstraintItem);
        constraintIter += 1;
        while((*constraintIter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Constraint") {
            qDebug() << (*constraintIter)->text(0);
            int constraintKey = (*constraintIter)->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
            if(constraints[constraintKey].node == key) {
                constraints.remove(constraintKey);
                QTreeWidgetItem *tmp = *constraintIter;
                ConstraintsItem->removeChild(*constraintIter);
                delete tmp;
                constraintsModed = true;
                continue;
            }
            ++constraintIter;
        }
        if(constraintsModed) {
            ui->Canvas->setConstraints(constraints);
        }

        QTreeWidgetItemIterator loadIter(AddLoadItem);
        loadIter += 1;
        while((*loadIter) && (*loadIter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Load") {
            qDebug() << (*loadIter)->text(0);
            int loadKey = (*loadIter)->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
            if(loads[loadKey].node == key) {
                loads.remove(loadKey);
                QTreeWidgetItem *tmp = *loadIter;
                LoadsItem->removeChild(*loadIter);
                delete tmp;
                loadsModed = true;
                continue;
            }
            ++loadIter;
        }
        if(loadsModed) {
            ui->Canvas->setLoads(loads);
        }

        nodes.remove(key);
        nodeInElements.remove(key);
        NodesItem->removeChild(item);
        delete item;

        setWindowModified(true);
        setSolved(false);
        ui->Canvas->setNodes(nodes);
        ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    } else if(item->parent() == ElementsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();

        nodeInElements[elements[key].nodeNum1]--;
        nodeInElements[elements[key].nodeNum2]--;

        elements.remove(key);
        ElementsItem->removeChild(item);
        delete item;

        setWindowModified(true);
        setSolved(false);
        ui->Canvas->setElements(elements);
        ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    } else if(item->parent() == ConstraintsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        constraints.remove(key);
        ConstraintsItem->removeChild(item);
        delete item;

        setWindowModified(true);
        setSolved(false);
        ui->Canvas->setConstraints(constraints);
        ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    } else if(item->parent() == LoadsItem) {
        int key = item->text(0).split(" ", QString::SkipEmptyParts)[1].toInt();
        loads.remove(key);
        LoadsItem->removeChild(item);
        delete item;

        setWindowModified(true);
        setSolved(false);
        ui->Canvas->setLoads(loads);
        ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    }
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
    elementType = 0;
    ElementTypeItem->setText(0, "Element Type");
    QTreeWidgetItemIterator iter(MaterialsItem);
    while((*iter)) {
        if((*iter)->text(0).contains(" ")) {
            QTreeWidgetItem *tmp = *iter;
            if((*iter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Material")
                MaterialsItem->removeChild(*iter);
            else if((*iter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Section")
                SectionsItem->removeChild(*iter);
            else if((*iter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Offset")
                OffsetsItem->removeChild(*iter);
            else if((*iter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Node")
                NodesItem->removeChild(*iter);
            else if((*iter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Element")
                ElementsItem->removeChild(*iter);
            else if((*iter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Constraint")
                ConstraintsItem->removeChild(*iter);
            else if((*iter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Load")
                LoadsItem->removeChild(*iter);
            else if((*iter)->text(0).split(" ", QString::SkipEmptyParts)[0] == "Export")
                break;
            delete tmp;
            continue;
        }
        ++iter;
    }
    OffsetsItem->setDisabled(false);
    ui->Canvas->clear();
    showReset();
    materials.clear();
    sections.clear();
    offsets.clear();
    nodes.clear();
    elements.clear();
    constraints.clear();
    loads.clear();
    nodeInElements.clear();
    solved = false;
    setResultActionsEnabled(false);
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

    quint8 type;
    in >> type;
    elementType = type;
    QString typeStr;
    if(elementType == 1) {
        typeStr = "Rod";
        OffsetsItem->setDisabled(true);
    } else if(elementType == 2) {
        typeStr = "Beam";
        OffsetsItem->setDisabled(false);
    }
    ElementTypeItem->setText(0, QString("Element Type (%1)").arg(typeStr));

    quint32 materialsSize;
    in >> materialsSize;
    for(quint32 i = 0; i < materialsSize; i++) {
        quint32 key;
        double E, poisson;
        in >> key >> E >> poisson;
        Material material;
        material.E = E;
        material.poisson = poisson;
        materials[key] = material;
        QTreeWidgetItem *materialItem = new QTreeWidgetItem(MaterialsItem, QStringList() << QString("Material %1").arg(key));
        MaterialsItem->addChild(materialItem);
    }

    quint32 sectionsSize;
    in >> sectionsSize;
    for(quint32 i = 0; i < sectionsSize; i++) {
        quint32 key;
        double A, Ay = 0, Iz = 0, K = 0;
        in >> key >> A;
        if(elementType == 2)
            in >> Ay >> Iz >> K;
        Section section;
        section.A = A;
        section.Ay = Ay;
        section.Iz = Iz;
        section.K = K;
        sections[key] = section;
        QTreeWidgetItem *sectionItem = new QTreeWidgetItem(SectionsItem, QStringList() << QString("Section %1").arg(key));
        SectionsItem->addChild(sectionItem);
    }

    if(elementType == 2) {
        quint32 offsetsSize;
        in >> offsetsSize;
        for(quint32 i = 0; i < offsetsSize; i++) {
            quint32 key;
            double a1, b1, a2, b2;
            in >> key >> a1 >> b1 >> a2 >> b2;
            Offset offset;
            offset.a1 = a1;
            offset.b1 = b1;
            offset.a2 = a2;
            offset.b2 = b2;
            offsets[key] = offset;
            QTreeWidgetItem *offsetItem = new QTreeWidgetItem(OffsetsItem, QStringList() << QString("Offset %1").arg(key));
            OffsetsItem->addChild(offsetItem);
        }
    }

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
            double dx, dy, theta;
            if(elementType == 1) {
                in >> dx >> dy;
                node.displacement << dx << dy;
            } else if(elementType == 2) {
                in >> dx >> dy >> theta;
                node.displacement << dx << dy << theta;
            }
        }
        nodes[key] = node;
        nodeInElements[key] = 0;
        QTreeWidgetItem *nodeItem = new QTreeWidgetItem(NodesItem, QStringList() << QString("Node %1").arg(key));
        NodesItem->addChild(nodeItem);
    }

    quint32 elementsSize;
    in >> elementsSize;
    for(quint32 i = 0; i < elementsSize; i++) {
        quint32 key, node1, node2, materialNum, sectionNum, offsetNum;
        in >> key >> node1 >> node2 >> materialNum >> sectionNum;
        if(elementType == 2) {
            in >> offsetNum;
        }
        Element element;
        element.nodeNum1 = node1;
        element.nodeNum2 = node2;
        element.materialNum = materialNum;
        element.sectionNum = sectionNum;
        if(elementType == 2)
            element.offsetNum = offsetNum;
        initElement(element, elementType, nodes, materials, sections, offsets);
        elements[key] = element;
        nodeInElements[node1]++;
        nodeInElements[node2]++;
        QTreeWidgetItem *elementItem = new QTreeWidgetItem(ElementsItem, QStringList() << QString("Element %1").arg(key));
        ElementsItem->addChild(elementItem);
    }
    if(solved)
        calElements(2, elementType, sections, nodes, elements);

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
        QTreeWidgetItem *constraintItem = new QTreeWidgetItem(ConstraintsItem, QStringList() << QString("Constraint %1").arg(key));
        ConstraintsItem->addChild(constraintItem);
    }
    if(solved)
        calConstraintForce(2, nodes, elements, constraints);

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
        QTreeWidgetItem *loadItem = new QTreeWidgetItem(LoadsItem, QStringList() << QString("Load %1").arg(key));
        LoadsItem->addChild(loadItem);
    }

    QApplication::restoreOverrideCursor();

    ui->Canvas->setElementType(elementType);
    ui->Canvas->setNodes(nodes);
    ui->Canvas->setElements(elements);
    ui->Canvas->setConstraints(constraints);
    ui->Canvas->setLoads(loads);
    ui->Canvas->setSolved(solved);
    ExportItem->setDisabled(!solved);
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

    out << quint8(elementType);

    out << quint32(materials.size());
    QMap<int, Material>::iterator materialsIter = materials.begin();
    while(materialsIter != materials.end()) {
        out << quint32(materialsIter.key()) << materialsIter.value().E << materialsIter.value().poisson;
        materialsIter++;
    }

    out << quint32(sections.size());
    QMap<int, Section>::iterator sectionsIter = sections.begin();
    while(sectionsIter != sections.end()) {
        out << quint32(sectionsIter.key()) << sectionsIter.value().A;
        if(elementType == 2)
            out << sectionsIter.value().Ay << sectionsIter.value().Iz << sectionsIter.value().K;
        sectionsIter++;
    }

    if(elementType == 2) {
        out << quint32(offsets.size());
        QMap<int, Offset>::iterator offsetsIter = offsets.begin();
        while(offsetsIter != offsets.end()) {
            out << quint32(offsetsIter.key()) << offsetsIter.value().a1 << offsetsIter.value().b1 << offsetsIter.value().a2 << offsetsIter.value().b2;
            offsetsIter++;
        }
    }

    out << quint32(nodes.size());
    QMap<int, Node>::iterator nodesIter = nodes.begin();
    while(nodesIter != nodes.end()) {
        out << quint32(nodesIter.key()) << nodesIter.value().pos.x() << nodesIter.value().pos.y();
        if(solved) {
            for(int i = 0; i < nodesIter.value().displacement.size(); i++)
                out << nodesIter.value().displacement[i];
        }
        nodesIter++;
    }

    out << quint32(elements.size());
    QMap<int, Element>::iterator elementsIter = elements.begin();
    while(elementsIter != elements.end()) {
        out << quint32(elementsIter.key())
            << quint32(elementsIter.value().nodeNum1) << quint32(elementsIter.value().nodeNum2)
            << quint32(elementsIter.value().materialNum) << quint32(elementsIter.value().sectionNum);
        if(elementType == 2)
            out << quint32(elementsIter.value().offsetNum);
        elementsIter++;
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
    ui->ListNodeResultAction->setEnabled(enabled);
    ui->ListElementResultAction->setEnabled(enabled);
    ui->ListConstraintResultAction->setEnabled(enabled);
    ui->DeformedShapeAction->setEnabled(enabled);
    ui->ElementInformationAction->setEnabled(enabled);
    ui->ConstraintForcesAction->setEnabled(enabled);
}

void MainWindow::exportReport() {
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Export Report"), ".",
                                                    tr("txt files (*.txt)"));
    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("FEM"),
                             tr("Cannot write file %1:\n%2.").arg(file.fileName()).arg(file.errorString()));
        ui->statusBar->showMessage(tr("Export canceled"), 2000);
        return;
    }

    QTextStream out(&file);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QDateTime time;
    out << QString("FEM Report (%1)\n").arg(time.currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    out << "-------------------------------------------------------------------------------------\n";

    out << "1. Input Parameters\n";
    out << QString("Element Type: %1\n").arg(elementType == 1 ? "Rod" : "Beam");
    out << QString("Number of materials: %1\n").arg(materials.size());
    QMap<int, Material>::iterator materialIter = materials.begin();
    while(materialIter != materials.end()) {
        out << QString("Material %1: E = %2, poisson's ratio = %3\n").arg(materialIter.key()).arg(materialIter.value().E).arg(materialIter.value().poisson);
        materialIter++;
    }
    out << "\n";
    out << QString("Number of sections: %1\n").arg(sections.size());
    QMap<int, Section>::iterator sectionIter = sections.begin();
    while(sectionIter != sections.end()) {
        out << QString("Section %1: A = %2").arg(sectionIter.key()).arg(sectionIter.value().A);
        if(elementType == 2)
            out << QString(", Ay = %1, Iz = %2, K = %3").arg(sectionIter.value().Ay).arg(sectionIter.value().Iz).arg(sectionIter.value().K);
        out << "\n";
        sectionIter++;
    }
    out << "\n";
    out << QString("Number of offsets: %1\n").arg(offsets.size());
    QMap<int, Offset>::iterator offsetIter = offsets.begin();
    while(offsetIter != offsets.end()) {
        out << QString("Offset %1: a1 = %2, b1 = %3, a2 = %4, b2 = %5\n").arg(offsetIter.key()).arg(offsetIter.value().a1).arg(offsetIter.value().b1).arg(offsetIter.value().a2).arg(offsetIter.value().b2);
        offsetIter++;
    }
    out << "\n";
    out << QString("Number of nodes: %1\n").arg(nodes.size());
    QMap<int, Node>::iterator nodeIter = nodes.begin();
    while(nodeIter != nodes.end()) {
        out << QString("Node %1: (%2, %3)\n").arg(nodeIter.key()).arg(nodeIter.value().pos.x()).arg(nodeIter.value().pos.y());
        nodeIter++;
    }
    out << "\n";
    out << QString("Number of elements: %1\n").arg(elements.size());
    QMap<int, Element>::iterator elementIter = elements.begin();
    while(elementIter != elements.end()) {
        out << QString("Element %1: Node %2 and Node %3, Material %4, Section %5").arg(elementIter.key()).arg(elementIter.value().nodeNum1).arg(elementIter.value().nodeNum2).arg(elementIter.value().materialNum).arg(elementIter.value().sectionNum);
        if(elementType == 2)
            out << QString(", Offset %1").arg(elementIter.value().offsetNum);
        out << "\n";
        elementIter++;
    }
    out << "\n";
    out << QString("Number of constraints: %1\n").arg(constraints.size());
    QMap<int, Constraint>::iterator constraintIter = constraints.begin();
    while(constraintIter != constraints.end()) {
        out << QString("Constraint %1: %2 direction of Node %3\n").arg(constraintIter.key()).arg(constraintIter.value().dir == 0 ? "x" : (constraintIter.value().dir == 1 ? "y" : "rz")).arg(constraintIter.value().node);
        constraintIter++;
    }
    out << "\n";
    out << QString("Number of loads: %1\n").arg(loads.size());
    QMap<int, Load>::iterator loadIter = loads.begin();
    while(loadIter != loads.end()) {
        out << QString("Load %1: %2 direction of Node %3, force = %4\n").arg(loadIter.key()).arg(loadIter.value().dir == 0 ? "x" : (loadIter.value().dir == 1 ? "y" : "rz")).arg(loadIter.value().node).arg(loadIter.value().f);
        loadIter++;
    }

    out << "-------------------------------------------------------------------------------------\n";

    out << "2. Solution\n";
    out << "Node Displacement:\n";
    nodeIter = nodes.begin();
    while(nodeIter != nodes.end()) {
        out << QString("Node %1:(").arg(nodeIter.key());
        for(int i = 0; i < nodeIter.value().displacement.size(); i++) {
            if(i != 0)
                out << ",";
            out << QString("%1").arg(nodeIter.value().displacement[i]);
        }
        out << ")\n";
        nodeIter++;
    }
    out << "\n";
    out << "Elements:\n";
    elementIter = elements.begin();
    while(elementIter != elements.end()) {
        out << QString("Element %1:\n").arg(elementIter.key());
        out << QString("Node displacement in global coordinate: (");
        for(int i = 0; i < elementIter.value().de.n / 2; i++) {
            if(i != 0)
                out << ",";
            out << QString("%1").arg(elementIter.value().de(i));
        }
        out << "), (";
        for(int i = elementIter.value().de.n / 2; i < elementIter.value().de.n; i++) {
            if(i != elementIter.value().de.n / 2)
                out << ",";
            out << QString("%1").arg(elementIter.value().de(i));
        }
        out << ")\n";

        out << QString("Node displacement in local coordinate: (");
        for(int i = 0; i < elementIter.value().dee.n / 2; i++) {
            if(i != 0)
                out << ",";
            out << QString("%1").arg(elementIter.value().dee(i));
        }
        out << "), (";
        for(int i = elementIter.value().dee.n / 2; i < elementIter.value().dee.n; i++) {
            if(i != elementIter.value().dee.n / 2)
                out << ",";
            out << QString("%1").arg(elementIter.value().dee(i));
        }
        out << ")\n";

        out << QString("Node force in global coordinate: (");
        for(int i = 0; i < elementIter.value().fe.n / 2; i++) {
            if(i != 0)
                out << ",";
            out << QString("%1").arg(elementIter.value().fe(i));
        }
        out << "), (";
        for(int i = elementIter.value().fe.n / 2; i < elementIter.value().fe.n; i++) {
            if(i != elementIter.value().fe.n / 2)
                out << ",";
            out << QString("%1").arg(elementIter.value().fe(i));
        }
        out << ")\n";

        out << QString("Node force in local coordinate: (");
        for(int i = 0; i < elementIter.value().fee.n / 2; i++) {
            if(i != 0)
                out << ",";
            out << QString("%1").arg(elementIter.value().fee(i));
        }
        out << "), (";
        for(int i = elementIter.value().fee.n / 2; i < elementIter.value().fee.n; i++) {
            if(i != elementIter.value().fee.n / 2)
                out << ",";
            out << QString("%1").arg(elementIter.value().fee(i));
        }
        out << ")\n";

        if(elementType == 1) {
            out << QString("Internal force: %1\n").arg(elementIter.value().IF);
            out << QString("Stress: %1\n").arg(elementIter.value().stress);
        }
        elementIter++;
    }
    out << "\n";
    out << "Constraint Forces:\n";
    constraintIter = constraints.begin();
    while(constraintIter != constraints.end()) {
        out << QString("Constraint force at %1 direction of node %2: %3\n").arg(constraintIter.value().dir == 0 ? "x" : (constraintIter.value().dir == 1 ? "y" : "rz")).arg(constraintIter.value().node).arg(constraintIter.value().force);
        constraintIter++;
    }

    QApplication::restoreOverrideCursor();

    file.close();

    ui->statusBar->showMessage(tr("Export success"), 2000);
}

void MainWindow::solve() {
    if(elementType == 0) {
        QMessageBox::warning(this, tr("FEM"), "Please select element type.");
        return;
    }
    if(materials.size() == 0) {
        QMessageBox::warning(this, tr("FEM"), "There are no materials.");
        return;
    }
    if(sections.size() == 0) {
        QMessageBox::warning(this, tr("FEM"), "There are no sections.");
        return;
    }
    if(nodes.size() == 0) {
        QMessageBox::warning(this, tr("FEM"), "There are no nodes.");
        return;
    }
    if(elements.size() == 0) {
        QMessageBox::warning(this, tr("FEM"), "There are no elements.");
        return;
    }
    if(constraints.size() == 0) {
        QMessageBox::warning(this, tr("FEM"), "There are no constraints.");
        return;
    }
    if(loads.size() == 0) {
        QMessageBox::warning(this, tr("FEM"), "There are no loads.");
        return;
    }
    int DOF = elements.size() * 3 - constraints.size();
    for(QMap<int, int>::iterator iter = nodeInElements.begin(); iter != nodeInElements.end(); iter++) {
        if(iter.value() == 0) {
            QMessageBox::warning(this, tr("FEM"),
                                 QString("There is an unconnected node: Node %1.").arg(iter.key()));
            return;
        }
        DOF -= (iter.value() - 1) * (elementType == 1 ? 2 : 3);
    }
    if(DOF > 0) {
        QMessageBox::warning(this, tr("FEM"),
                             QString("DOF = %1\nDegree of Freedom(DOF) must be less than or equal to 0.").arg(DOF));
        return;
    }

    MatrixIn1D K;
    int nodeDOF;
    if(elementType == 1)
        nodeDOF = 2;
    else if(elementType == 2)
        nodeDOF = 3;
    calStiffnessMatrix(nodeDOF, nodes, elements, K);
    processConstraints(nodeDOF, nodes, constraints, K);

    solveEqns(nodeDOF, nodes, K, loads);
    calElements(nodeDOF, elementType, sections, nodes, elements);
    calConstraintForce(nodeDOF, nodes, elements, constraints);

    ui->Canvas->setNodes(nodes);
    ui->Canvas->setElements(elements);
    ui->Canvas->setConstraints(constraints);
    setSolved(true);
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
                tr("<h2>FEM v1.2 (by TWH)</h2>"
                   "<p>FEM is a small application that can solve problems of structural mechanics using Finite Element Method(FEM)."));
}

void MainWindow::userguide() {
    QString filePath = QCoreApplication::applicationDirPath() + "/user-guide.pdf";
    QFileInfo fileInfo(filePath);
    if(!fileInfo.exists()) {
        QMessageBox::warning(this, tr("FEM"), QString("File does not exist!"));
        return;
    }
    QDesktopServices::openUrl(QUrl(filePath));
}

void MainWindow::listMaterials() {
    QString str = "Materials:\n";
    QMap<int, Material>::iterator iter = materials.begin();
    while(iter != materials.end()) {
        str += QString("Material %1: E = %2, poisson's ratio = %3\n").arg(iter.key()).arg(iter.value().E).arg(iter.value().poisson);
        iter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Materials", this);
    dialog->show();
}

void MainWindow::listSections() {
    QString str = "Sections:\n";
    QMap<int, Section>::iterator iter = sections.begin();
    while(iter != sections.end()) {
        str += QString("Section %1: A = %2").arg(iter.key()).arg(iter.value().A);
        if(elementType == 2)
            str += QString(", Ay = %1, Iz = %2, K = %3").arg(iter.value().Ay).arg(iter.value().Iz).arg(iter.value().K);
        str += "\n";
        iter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Sections", this);
    dialog->show();
}

void MainWindow::listOffsets() {
    QString str = "Offsets:\n";
    QMap<int, Offset>::iterator iter = offsets.begin();
    while(iter != offsets.end()) {
        str += QString("Offset %1: a1 = %2, b1 = %3, a2 = %4, b2 = %5\n").arg(iter.key()).arg(iter.value().a1).arg(iter.value().b1).arg(iter.value().a2).arg(iter.value().b2);
        iter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Offsets", this);
    dialog->show();
}

void MainWindow::listNodes() {
    QString str = "Nodes:\n";
    QMap<int, Node>::iterator iter = nodes.begin();
    while(iter != nodes.end()) {
        str += QString("Node %1: (%2, %3)\n").arg(iter.key()).arg(iter.value().pos.x()).arg(iter.value().pos.y());
        iter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Nodes", this);
    dialog->show();
}

void MainWindow::listElements() {
    QString str = QString("Element Type: %1\n").arg(elementType == 1 ? "Rod" : "Beam");
    str += "Elements:\n";
    QMap<int, Element>::iterator iter = elements.begin();
    while(iter != elements.end()) {
        str += QString("Element %1: Node %2 and Node %3, Material %4, Section %5").arg(iter.key()).arg(iter.value().nodeNum1).arg(iter.value().nodeNum2).arg(iter.value().materialNum).arg(iter.value().sectionNum);
        if(elementType == 2)
            str += QString(", Offset %1").arg(iter.value().offsetNum);
        str += "\n";
        iter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Elements", this);
    dialog->show();
}

void MainWindow::listConstraints() {
    QString str = "Constraints:\n";
    QMap<int, Constraint>::iterator iter = constraints.begin();
    while(iter != constraints.end()) {
        str += QString("Constraint %1: %2 direction of Node %3\n").arg(iter.key()).arg(iter.value().dir == 0 ? "x" : (iter.value().dir == 1 ? "y" : "rz")).arg(iter.value().node);
        iter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Constraints", this);
    dialog->show();
}

void MainWindow::listLoads() {
    QString str = "Loads:\n";
    QMap<int, Load>::iterator iter = loads.begin();
    while(iter != loads.end()) {
        str += QString("Load %1: %2 direction of Node %3, force = %4\n").arg(iter.key()).arg(iter.value().dir == 0 ? "x" : (iter.value().dir == 1 ? "y" : "rz")).arg(iter.value().node).arg(iter.value().f);
        iter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Loads", this);
    dialog->show();
}

void MainWindow::listNodesDisplacement() {
    QString str = "Nodes Displacement:\n";
    QMap<int, Node>::iterator nodeIter = nodes.begin();
    while(nodeIter != nodes.end()) {
        str += QString("Displacement of Node %1:(").arg(nodeIter.key());
        for(int i = 0; i < nodeIter.value().displacement.size(); i++) {
            if(i != 0)
                str += ",";
            str += QString("%1").arg(nodeIter.value().displacement[i]);
        }
        str += ")\n";
        nodeIter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Nodes Displacement", this);
    dialog->show();
}

void MainWindow::listElementsInformation() {
    QString str = "Elements Information:\n";
    QMap<int, Element>::iterator elementIter = elements.begin();
    while(elementIter != elements.end()) {
        str += QString("Element %1:\n").arg(elementIter.key());
        str += QString("Node displacement in global coordinate: (");
        for(int i = 0; i < elementIter.value().de.n / 2; i++) {
            if(i != 0)
                str += ",";
            str += QString("%1").arg(elementIter.value().de(i));
        }
        str += "), (";
        for(int i = elementIter.value().de.n / 2; i < elementIter.value().de.n; i++) {
            if(i != elementIter.value().de.n / 2)
                str += ",";
            str += QString("%1").arg(elementIter.value().de(i));
        }
        str += ")\n";

        str += QString("Node displacement in local coordinate: (");
        for(int i = 0; i < elementIter.value().dee.n / 2; i++) {
            if(i != 0)
                str += ",";
            str += QString("%1").arg(elementIter.value().dee(i));
        }
        str += "), (";
        for(int i = elementIter.value().dee.n / 2; i < elementIter.value().dee.n; i++) {
            if(i != elementIter.value().dee.n / 2)
                str += ",";
            str += QString("%1").arg(elementIter.value().dee(i));
        }
        str += ")\n";

        str += QString("Node force in global coordinate: (");
        for(int i = 0; i < elementIter.value().fe.n / 2; i++) {
            if(i != 0)
                str += ",";
            str += QString("%1").arg(elementIter.value().fe(i));
        }
        str += "), (";
        for(int i = elementIter.value().fe.n / 2; i < elementIter.value().fe.n; i++) {
            if(i != elementIter.value().fe.n / 2)
                str += ",";
            str += QString("%1").arg(elementIter.value().fe(i));
        }
        str += ")\n";

        str += QString("Node force in local coordinate: (");
        for(int i = 0; i < elementIter.value().fee.n / 2; i++) {
            if(i != 0)
                str += ",";
            str += QString("%1").arg(elementIter.value().fee(i));
        }
        str += "), (";
        for(int i = elementIter.value().fee.n / 2; i < elementIter.value().fee.n; i++) {
            if(i != elementIter.value().fee.n / 2)
                str += ",";
            str += QString("%1").arg(elementIter.value().fee(i));
        }
        str += ")\n";

        if(elementType == 1) {
            str += QString("Internal force: %1\n").arg(elementIter.value().IF);
            str += QString("Stress: %1\n").arg(elementIter.value().stress);
        }
        str += "\n";
        elementIter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Elements Information", this);
    dialog->show();
}

void MainWindow::listConstraintForces() {
    QString str = "Constraint forces:\n";
    QMap<int, Constraint>::iterator constraintIter = constraints.begin();
    while(constraintIter != constraints.end()) {
        str += QString("Constraint force at %1 direction of node %2: %3\n").arg(constraintIter.value().dir == 0 ? "x" : (constraintIter.value().dir == 1 ? "y" : "rz")).arg(constraintIter.value().node).arg(constraintIter.value().force);
        constraintIter++;
    }
    OutputDialog *dialog = new OutputDialog(str, "List of Constraint Forces", this);
    dialog->show();
}

void MainWindow::showReset() {
    ui->ShowNodesAction->setChecked(true);
    ui->ShowElementsAction->setChecked(true);
    ui->ShowConstraintsAction->setChecked(true);
    ui->ShowLoadsAction->setChecked(true);
    ui->ShowAxisAction->setChecked(true);
    ui->ShowLabelsAction->setChecked(true);
}

void MainWindow::drawDeformedShape() {
    ui->Canvas->draw(CanvasWidget::DEFORM_SHAPE);
    ui->Canvas->fit();
}

void MainWindow::plotDefinedShape() {
    ui->Canvas->draw(CanvasWidget::DEFINE_SHAPE);
    ui->Canvas->fit();
}

void MainWindow::plotElementInformation() {
    SelectElementResultDialog *dialog = new SelectElementResultDialog(elements.keys(), this);
    if(dialog->exec()) {
        int element = dialog->comboBox1->currentData().toInt();
        int coord = dialog->comboBox2->currentData().toInt();
        ui->Canvas->draw(CanvasWidget::ELEMENT_INFORMATION, element, coord);
        ui->Canvas->fit();
    }
    delete dialog;
}

void MainWindow::plotConstraintForces() {
    ui->Canvas->draw(CanvasWidget::CONSTRAINT_FORCES);
    ui->Canvas->fit();
}
