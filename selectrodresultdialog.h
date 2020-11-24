#ifndef SELECTRODRESULTDIALOG_H
#define SELECTRODRESULTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVector>
#include <QHBoxLayout>
#include <QVBoxLayout>

class SelectRodResultDialog : public QDialog
{
    Q_OBJECT

public:
    SelectRodResultDialog(QList<int> list, QWidget *parent = 0);

private:
    QList<int> list;

public:
    QComboBox *comboBox1;
    QComboBox *comboBox2;

private:
    QLabel *label1;
    QLabel *label2;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // SELECTRODRESULTDIALOG_H
