#ifndef OUTPUTDIALOG_H
#define OUTPUTDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QHBoxLayout>

class OutputDialog : public QDialog
{
    Q_OBJECT

public:
    OutputDialog(QString &str, QString title, QWidget *parent = 0);

private:
    QTextEdit *edit;
};

#endif // OUTPUTDIALOG_H
