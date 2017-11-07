#include "q7zip.h"
#include "ui_q7zip.h"

Q7Zip::Q7Zip(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Q7Zip)
{
    ui->setupUi(this);
}

Q7Zip::~Q7Zip()
{
    delete ui;
}
