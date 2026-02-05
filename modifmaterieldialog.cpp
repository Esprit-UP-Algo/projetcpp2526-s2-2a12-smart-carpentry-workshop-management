#include "modifmaterieldialog.h"
#include "ui_modifmaterieldialog.h"

ModifMaterielDialog::ModifMaterielDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ModifMaterielDialog)
{
    ui->setupUi(this);
}

ModifMaterielDialog::~ModifMaterielDialog()
{
    delete ui;
}
