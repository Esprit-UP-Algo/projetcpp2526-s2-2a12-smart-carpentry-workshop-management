#include "modifierprojet.h"
#include "ui_modifierprojet.h"

modifierprojet::modifierprojet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::modifierprojet)
{
    ui->setupUi(this);
}

modifierprojet::~modifierprojet()
{
    delete ui;
}
