#include "deleteprojet.h"
#include "ui_deleteprojet.h"

deleteprojet::deleteprojet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::deleteprojet)
{
    ui->setupUi(this);
}

deleteprojet::~deleteprojet()
{
    delete ui;
}
