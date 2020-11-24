#include "selectrodresultdialog.h"

SelectRodResultDialog::SelectRodResultDialog(QList<int> list, QWidget *parent)
{
    this->list = list;

    label1 = new QLabel("Select Rod:");

    comboBox1 = new QComboBox();
    for(int i = 0; i < this->list.size(); i++) {
        comboBox1->addItem("Rod " + QString::number(this->list[i]), this->list[i]);
    }

    label2 = new QLabel("Select coordinate system:");

    comboBox2 = new QComboBox();
    comboBox2->addItem("Global", 0);
    comboBox2->addItem("Local", 1);

    okButton = new QPushButton(tr("OK"));

    cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setDefault(true);

    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(label1);
    topLayout->addWidget(comboBox1);

    QHBoxLayout *midLayout = new QHBoxLayout;
    midLayout->addWidget(label2);
    midLayout->addWidget(comboBox2);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(midLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
    setWindowTitle("Select Rod");
    setFixedHeight(sizeHint().height());
}
