#include "outputdialog.h"

OutputDialog::OutputDialog(QString &str, QString title, QWidget *parent) : QDialog(parent)
{
    edit = new QTextEdit;

    QSize s(500, 300);
    edit->setMinimumSize(s);

    edit->setReadOnly(true);
    edit->setText(str);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(edit);

    setLayout(mainLayout);
    setWindowTitle(title);
}
