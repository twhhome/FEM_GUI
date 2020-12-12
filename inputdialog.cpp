#include "inputdialog.h"
#include <QDebug>

InputDialog::~InputDialog() {
    elementType = NULL;
    material = NULL;
    section = NULL;
    offset = NULL;
    node = NULL;
    element = NULL;
    constraint = NULL;
    load = NULL;

    materials = NULL;
    sections = NULL;
    offsets = NULL;
    nodes = NULL;
    elements = NULL;
    constraints = NULL;
    loads = NULL;
}

InputDialog::InputDialog(int *elementType, QWidget *parent) : QDialog(parent)
{
    type = ELEMENT_TYPE;
    this->elementType = elementType;

    label1 = new QLabel(tr("Select Element Type:"));

    comboBox1 = new QComboBox();
    comboBox1->addItem("Rod", 1);
    comboBox1->addItem("Beam", 2);

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    topLayout = new QHBoxLayout;
    topLayout->addWidget(label1);
    topLayout->addWidget(comboBox1);

    bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Select Element Type"));
}

InputDialog::InputDialog(Material *material, bool flag, QWidget *parent) : QDialog(parent)
{
    type = MATERIAL;
    mode = (!flag) ? CREATE : EDIT;
    this->material = material;

    QDoubleValidator *validator = new QDoubleValidator(this);
    validator->setBottom(0);

    label1= new QLabel(tr("E = "));
    edit1 = new QLineEdit();
    edit1->setValidator(validator);

    label2 = new QLabel(tr("Poisson's Ratio = "));
    edit2 = new QLineEdit();
    edit2->setValidator(validator);

    if(flag == true) {
        edit1->setText(QString("%1").arg(material->E));
        edit2->setText(QString("%1").arg(material->poisson));
    }

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    topLayout = new QHBoxLayout;
    topLayout->addWidget(label1);
    topLayout->addWidget(edit1);

    midLayout = new QHBoxLayout;
    midLayout->addWidget(label2);
    midLayout->addWidget(edit2);

    bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(midLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Input Material"));
}

InputDialog::InputDialog(Section *section, int *elementType, bool flag, QWidget *parent) : QDialog(parent)
{
    type = SECTION;
    mode = (!flag) ? CREATE : EDIT;
    this->section = section;
    this->elementType = elementType;

    QDoubleValidator *validator = new QDoubleValidator(this);
    validator->setBottom(0);

    label1= new QLabel(tr("A = "));
    edit1 = new QLineEdit();
    edit1->setValidator(validator);

    if(*elementType == 2) {
        label2 = new QLabel(tr("Iz = "));
        edit2 = new QLineEdit();
        edit2->setValidator(validator);

        label3 = new QLabel(tr("Ay = "));
        edit3 = new QLineEdit();
        edit3->setValidator(validator);

        label4 = new QLabel(tr("K = "));
        edit4 = new QLineEdit();
        edit4->setValidator(validator);
    }

    if(flag == true) {
        edit1->setText(QString("%1").arg(section->A));
        if(*elementType == 2) {
            edit2->setText(QString("%1").arg(section->Iz));
            edit3->setText(QString("%1").arg(section->Ay));
            edit4->setText(QString("%1").arg(section->K));
        }
    }

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    topLayout = new QHBoxLayout;
    topLayout->addWidget(label1);
    topLayout->addWidget(edit1);

    if(*elementType == 2) {
        topLayout->addWidget(label2);
        topLayout->addWidget(edit2);

        midLayout = new QHBoxLayout;
        midLayout->addWidget(label3);
        midLayout->addWidget(edit3);
        midLayout->addWidget(label4);
        midLayout->addWidget(edit4);
    }

    bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    if(*elementType == 2)
        mainLayout->addLayout(midLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Input Section"));
}

InputDialog::InputDialog(Offset *offset, bool flag, QWidget *parent) : QDialog(parent)
{
    type = OFFSET;
    mode = (!flag) ? CREATE : EDIT;
    this->offset = offset;

    QDoubleValidator *validator = new QDoubleValidator(this);

    label1= new QLabel(tr("a1 = "));
    edit1 = new QLineEdit();
    edit1->setValidator(validator);

    label2 = new QLabel(tr("b1 = "));
    edit2 = new QLineEdit();
    edit2->setValidator(validator);

    label3 = new QLabel(tr("a2 = "));
    edit3 = new QLineEdit();
    edit3->setValidator(validator);

    label4 = new QLabel(tr("b2 = "));
    edit4 = new QLineEdit();
    edit4->setValidator(validator);

    if(flag == true) {
        edit1->setText(QString("%1").arg(offset->a1));
        edit2->setText(QString("%1").arg(offset->b1));
        edit3->setText(QString("%1").arg(offset->a2));
        edit4->setText(QString("%1").arg(offset->b2));
    }

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    topLayout = new QHBoxLayout;
    topLayout->addWidget(label1);
    topLayout->addWidget(edit1);
    topLayout->addWidget(label2);
    topLayout->addWidget(edit2);

    midLayout = new QHBoxLayout;
    midLayout->addWidget(label3);
    midLayout->addWidget(edit3);
    midLayout->addWidget(label4);
    midLayout->addWidget(edit4);

    bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(midLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Input Offset"));
}

InputDialog::InputDialog(Node *node, QMap<int, Node> *nodes, bool flag, QWidget *parent) : QDialog(parent)
{
    type = NODE;
    mode = (!flag) ? CREATE : EDIT;
    this->node = node;
    this->nodes = nodes;

    QDoubleValidator *validator = new QDoubleValidator(this);

    label1= new QLabel(tr("x = "));
    edit1 = new QLineEdit();
    edit1->setValidator(validator);

    label2 = new QLabel(tr("y = "));
    edit2 = new QLineEdit();
    edit2->setValidator(validator);

    if(flag == true) {
        edit1->setText(QString("%1").arg(node->pos.x()));
        edit2->setText(QString("%1").arg(node->pos.y()));
    }

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    topLayout = new QHBoxLayout;
    topLayout->addWidget(label1);
    topLayout->addWidget(edit1);
    topLayout->addWidget(label2);
    topLayout->addWidget(edit2);

    bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Input Node"));
}

InputDialog::InputDialog(Element *element, int *elementType, QMap<int, Material> *materials, QMap<int, Section> *sections,
                         QMap<int, Offset> *offsets, QMap<int, Node> *nodes, QMap<int, Element> *elements,
                         bool flag, QWidget *parent) : QDialog(parent)
{
    type = ELEMENT;
    mode = (!flag) ? CREATE : EDIT;
    this->element = element;
    this->elementType = elementType;
    this->materials = materials;
    this->sections = sections;
    this->offsets = offsets;
    this->nodes = nodes;
    this->elements = elements;

    label1 = new QLabel(tr("Node1: "));
    label2 = new QLabel(tr("Node2: "));
    comboBox1 = new QComboBox();
    comboBox2 = new QComboBox();
    for(QMap<int, Node>::iterator iter = nodes->begin(); iter != nodes->end(); iter++) {
        comboBox1->addItem(QString("Node %1").arg(iter.key()), iter.key());
        comboBox2->addItem(QString("Node %1").arg(iter.key()), iter.key());
    }

    label3 = new QLabel(tr("Material: "));
    comboBox3 = new QComboBox();
    for(QMap<int, Material>::iterator iter = materials->begin(); iter != materials->end(); iter++) {
        comboBox3->addItem(QString("Material %1").arg(iter.key()), iter.key());
    }

    label4 = new QLabel(tr("Section: "));
    comboBox4 = new QComboBox();
    for(QMap<int, Section>::iterator iter = sections->begin(); iter != sections->end(); iter++) {
        comboBox4->addItem(QString("Section %1").arg(iter.key()), iter.key());
    }

    if(*elementType == 2) {
        label5 = new QLabel(tr("Offset: "));
        comboBox5 = new QComboBox();
        comboBox5->addItem("None", 0);
        for(QMap<int, Offset>::iterator iter = offsets->begin(); iter != offsets->end(); iter++) {
            comboBox5->addItem(QString("Offset %1").arg(iter.key()), iter.key());
        }
    }

    if(flag == true) {
        comboBox1->setCurrentIndex(comboBox1->findData(element->nodeNum1));
        comboBox2->setCurrentIndex(comboBox2->findData(element->nodeNum2));
        comboBox3->setCurrentIndex(comboBox3->findData(element->materialNum));
        comboBox4->setCurrentIndex(comboBox4->findData(element->sectionNum));
        if(*elementType == 2) {
            //if(element->offsetNum > 0)
            //qDebug() << "Offset Number:" << element->offsetNum;
                comboBox5->setCurrentIndex(comboBox5->findData(element->offsetNum));
            //else
                //comboBox5->setCurrentIndex(1);
        }
    }

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    topLayout = new QHBoxLayout;
    topLayout->addWidget(label1);
    topLayout->addWidget(comboBox1);
    topLayout->addWidget(label2);
    topLayout->addWidget(comboBox2);

    midLayout = new QHBoxLayout;
    midLayout->addWidget(label3);
    midLayout->addWidget(comboBox3);
    midLayout->addWidget(label4);
    midLayout->addWidget(comboBox4);
    if(*elementType == 2) {
        midLayout->addWidget(label5);
        midLayout->addWidget(comboBox5);
    }

    bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(midLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Input Element"));
}

InputDialog::InputDialog(Constraint *constraint, int *elementType, QMap<int, Node> *nodes, QMap<int, Constraint> *constraints, QMap<int, Load> *loads,
                         bool flag, QWidget *parent) : QDialog(parent)
{
    type = CONSTRAINT;
    mode = (!flag) ? CREATE : EDIT;
    this->constraint = constraint;
    this->nodes = nodes;
    this->constraints = constraints;
    this->loads = loads;

    label1 = new QLabel(tr("Node: "));
    comboBox1 = new QComboBox();
    for(QMap<int, Node>::iterator iter = nodes->begin(); iter != nodes->end(); iter++) {
        comboBox1->addItem(QString("Node %1").arg(iter.key()), iter.key());
    }

    label2 = new QLabel(tr("Direction: "));
    comboBox2 = new QComboBox();
    comboBox2->addItem("u(x)", 0);
    comboBox2->addItem("v(y)", 1);
    if(*elementType == 2)
        comboBox2->addItem("Rz", 2);

    if(flag == true) {
        comboBox1->setCurrentIndex(comboBox1->findData(constraint->node));
        if(constraint->dir == -1)
            comboBox2->setCurrentIndex(0);
        else
            comboBox2->setCurrentIndex(constraint->dir);
    }

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    topLayout = new QHBoxLayout;
    topLayout->addWidget(label1);
    topLayout->addWidget(comboBox1);
    topLayout->addWidget(label2);
    topLayout->addWidget(comboBox2);

    bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Input Constraint"));
}

InputDialog::InputDialog(Load *load, int *elementType, QMap<int, Node> *nodes, QMap<int, Constraint> *constraints, QMap<int, Load> *loads,
                         bool flag, QWidget *parent) : QDialog(parent)
{
    type = LOAD;
    mode = (!flag) ? CREATE : EDIT;
    this->load = load;
    this->elementType = elementType;
    this->nodes = nodes;
    this->constraints = constraints;
    this->loads = loads;

    QDoubleValidator *validator = new QDoubleValidator(this);

    label1 = new QLabel(tr("Node: "));
    comboBox1 = new QComboBox();
    for(QMap<int, Node>::iterator iter = nodes->begin(); iter != nodes->end(); iter++) {
        comboBox1->addItem(QString("Node %1").arg(iter.key()), iter.key());
    }

    label2 = new QLabel(tr("Direction: "));
    comboBox2 = new QComboBox();
    comboBox2->addItem("u(x)", 0);
    comboBox2->addItem("v(y)", 1);
    if(*elementType == 2)
        comboBox2->addItem("Rz", 2);

    label3 = new QLabel(tr("Force: "));
    edit1 = new QLineEdit();
    edit1->setValidator(validator);

    if(flag == true) {
        comboBox1->setCurrentIndex(comboBox1->findData(load->node));
        if(load->dir == -1)
            comboBox2->setCurrentIndex(0);
        else
            comboBox2->setCurrentIndex(load->dir);
        edit1->setText(QString::number(load->f));
    }

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    topLayout = new QHBoxLayout;
    topLayout->addWidget(label1);
    topLayout->addWidget(comboBox1);
    topLayout->addWidget(label2);
    topLayout->addWidget(comboBox2);

    midLayout = new QHBoxLayout;
    midLayout->addWidget(label3);
    midLayout->addWidget(edit1);

    bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(midLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Input Load"));
}

void InputDialog::okPressed() {
    if(type == ELEMENT_TYPE)
        *elementType = comboBox1->currentData().toInt();
    else if(type == MATERIAL) {
        if(edit1->text().isEmpty() || edit2->text().isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), QString("Please input material."));
            return;
        }
        material->E = edit1->text().toDouble();
        material->poisson = edit2->text().toDouble();
    } else if(type == SECTION) {
        if(edit1->text().isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), QString("Please input section."));
            return;
        }
        section->A = edit1->text().toDouble();
        if(*elementType == 1) {
            section->Iz = 0;
            section->Ay = 0;
            section->K = 0;
        } else if(*elementType == 2) {
            if(edit2->text().isEmpty() || edit3->text().isEmpty() || edit4->text().isEmpty()) {
                QMessageBox::warning(this, tr("FEM"), QString("Please input section."));
                return;
            }
            section->Iz = edit2->text().toDouble();
            section->Ay = edit3->text().toDouble();
            section->K = edit4->text().toDouble();
        }
    } else if(type == OFFSET) {
        if(edit1->text().isEmpty() || edit2->text().isEmpty() || edit3->text().isEmpty() || edit4->text().isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), QString("Please input offset."));
            return;
        }
        offset->a1 = edit1->text().toDouble();
        offset->b1 = edit2->text().toDouble();
        offset->a2 = edit3->text().toDouble();
        offset->b2 = edit4->text().toDouble();
    } else if(type == NODE) {
        if(edit1->text().isEmpty() || edit2->text().isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), QString("Please input node."));
            return;
        }
        double x, y;
        x = edit1->text().toDouble();
        y = edit2->text().toDouble();
        if(mode == CREATE || node->pos.x() != x || node->pos.y() != y) {
            QMap<int, Node>::iterator iter = nodes->begin();
            while(iter != nodes->end()) {
                if(iter.value().pos.x() == x && iter.value().pos.y() == y) {
                    QMessageBox::warning(this, tr("FEM"), QString("Node at (%1, %2) already exists.").arg(x).arg(y));
                    return;
                }
                iter++;
            }
        }
        node->pos.setX(x);
        node->pos.setY(y);
    } else if(type == ELEMENT) {
        int node1, node2;
        node1 = comboBox1->currentData().toInt();
        node2 = comboBox2->currentData().toInt();
        if(node1 == node2) {
            QMessageBox::warning(this, tr("FEM"), "Node1 can not be equal to Node2.");
            return;
        }
        if(mode == CREATE || (node1 != element->nodeNum1 && node1 != element->nodeNum2)
                || (node2 != element->nodeNum1 && node2 != element->nodeNum2)) {
            QMap<int, Element>::iterator iter = elements->begin();
            while(iter != elements->end()) {
                if((iter.value().nodeNum1 == node1 && iter.value().nodeNum2 == node2) || (iter.value().nodeNum1 == node2 && iter.value().nodeNum2 == node1)) {
                    QMessageBox::warning(this, tr("FEM"), QString("Node %1 and Node %2 have already been connected.").arg(node1).arg(node2));
                    return;
                }
                iter++;
            }
        }
        element->nodeNum1 = node1;
        element->nodeNum2 = node2;
        element->materialNum = comboBox3->currentData().toInt();
        element->sectionNum = comboBox4->currentData().toInt();
        if(*elementType == 1)
            element->offsetNum = 0;
        else if(*elementType == 2)
            element->offsetNum = comboBox5->currentData().toInt();
    } else if(type == CONSTRAINT) {
        int node, dir;
        node = comboBox1->currentData().toInt();
        dir = comboBox2->currentData().toInt();
        if(mode == CREATE || node != constraint->node || dir != constraint->dir) {
            QMap<int, Constraint>::iterator iter = constraints->begin();
            while(iter != constraints->end()) {
                if(iter.value().node == node && iter.value().dir == dir) {
                    QMessageBox::warning(this, tr("FEM"), QString("Constraint at %1 direction of Node %2 already existes.").arg(comboBox2->currentText()).arg(node));
                    return;
                }
                iter++;
            }
            QMap<int, Load>::iterator iter2 = loads->begin();
            while(iter2 != loads->end()) {
                if(iter2.value().node == node && iter2.value().dir == dir) {
                    QMessageBox::warning(this, tr("FEM"), QString("There is load at %1 direction of Node %2.").arg(comboBox2->currentText()).arg(node));
                    return;
                }
                iter2++;
            }
        }
        constraint->node = node;
        constraint->dir = dir;
    } else if(type == LOAD) {
        if(edit1->text().isEmpty()) {
            QMessageBox::warning(this, tr("FEM"), QString("Please input load."));
            return;
        }
        int node, dir;
        node = comboBox1->currentData().toInt();
        dir = comboBox2->currentData().toInt();
        if(mode == CREATE || node != load->node || dir != load->dir) {
            QMap<int, Load>::iterator iter = loads->begin();
            while(iter != loads->end()) {
                if(iter.value().node == node && iter.value().dir == dir) {
                    QMessageBox::warning(this, tr("FEM"), QString("Load at %1 direction of Node %2 already existes.").arg(comboBox2->currentText()).arg(node));
                    return;
                }
                iter++;
            }
            QMap<int, Constraint>::iterator iter2 = constraints->begin();
            while(iter2 != constraints->end()) {
                if(iter2.value().node == node && iter2.value().dir == dir) {
                    QMessageBox::warning(this, tr("FEM"), QString("There is constraint at %1 direction of Node %2.").arg(comboBox2->currentText()).arg(node));
                    return;
                }
                iter2++;
            }
        }
        load->node = node;
        load->dir = dir;
        load->f = edit1->text().toDouble();
    }
    accept();
}
