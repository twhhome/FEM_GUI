#include "selectdialog.h"

SelectDialog::SelectDialog(QString str, QList<int> list, QWidget *parent) : QDialog(parent) {
    this->str = str;
    this->list = list;

    label = new QLabel(this->str + ":");

    comboBox = new QComboBox();
    for(int i = 0; i < this->list.size(); i++) {
        comboBox->addItem(this->str.split(" ", QString::SkipEmptyParts)[1] + " " + QString::number(this->list[i]), this->list[i]);
    }

    okButton = new QPushButton(tr("OK"));

    cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setDefault(true);

    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(label);
    topLayout->addWidget(comboBox);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle(this->str);
    setFixedHeight(sizeHint().height());
}
