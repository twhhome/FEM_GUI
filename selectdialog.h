#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVector>
#include <QHBoxLayout>
#include <QVBoxLayout>

class SelectDialog : public QDialog
{
    Q_OBJECT

public:
    SelectDialog(QString str, QList<int> list, QWidget *parent = 0);

private:
    QString str;
    QList<int> list;

public:
    QComboBox *comboBox;

private:
    QLabel *label;
    QPushButton *okButton;
    QPushButton *cancelButton;

};

#endif // DELETEDIALOG_H
