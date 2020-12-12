#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QMap>
#include <QMessageBox>

#include "femcore.h"

class InputDialog : public QDialog
{
    Q_OBJECT

public:
    InputDialog(int *elementType, QWidget *parent = 0);
    InputDialog(Material *material, bool flag, QWidget *parent = 0);
    InputDialog(Section *section, int *elementType, bool flag, QWidget *parent = 0);
    InputDialog(Offset *offset, bool flag, QWidget *parent = 0);
    InputDialog(Node *node, QMap<int, Node> *nodes, bool flag, QWidget *parent = 0);
    InputDialog(Element *element, int *elementType, QMap<int, Material> *materials, QMap<int, Section> *sections,
                QMap<int, Offset> *offsets, QMap<int, Node> *nodes, QMap<int, Element> *elements,
                bool flag, QWidget *parent = 0);
    InputDialog(Constraint *constraint, int *elementType, QMap<int, Node> *nodes, QMap<int, Constraint> *constraints, QMap<int, Load> *loads, bool flag, QWidget *parent = 0);
    InputDialog(Load *load, int *elementType, QMap<int, Node> *nodes, QMap<int, Constraint> *constraints, QMap<int, Load> *loads, bool flag, QWidget *parent = 0);
    ~InputDialog();

private:
    enum INPUT_TYPE { ELEMENT_TYPE, MATERIAL, SECTION, OFFSET, NODE, ELEMENT, CONSTRAINT, LOAD };
    enum MODE { CREATE, EDIT };
    INPUT_TYPE type;
    MODE mode;
    int *elementType;
    Material *material;
    Section *section;
    Offset *offset;
    Node *node;
    Element *element;
    Constraint *constraint;
    Load *load;

private:
    QMap<int, Material> *materials;
    QMap<int, Section> *sections;
    QMap<int, Offset> *offsets;
    QMap<int, Node> *nodes;
    QMap<int, Element> *elements;
    QMap<int, Constraint> *constraints;
    QMap<int, Load> *loads;

private:
    QLabel *label1, *label2, *label3, *label4, *label5;
    QComboBox *comboBox1, *comboBox2, *comboBox3, *comboBox4, *comboBox5;
    QLineEdit *edit1, *edit2, *edit3, *edit4;
    QPushButton *okButton, *cancelButton;
    QHBoxLayout *topLayout, *midLayout, *bottomLayout;
    QVBoxLayout *mainLayout;

private slots:
    void okPressed();
};

#endif // INPUTDIALOG_H
