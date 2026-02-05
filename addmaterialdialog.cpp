#include "addmaterialdialog.h"
#include "ui_addmaterialdialog.h"

AddMaterialDialog::AddMaterialDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddMaterialDialog)
{
    ui->setupUi(this);
    // SUPPRIME le chargement de CSS ici
    // Le CSS est déjà chargé globalement dans main.cpp
}

AddMaterialDialog::~AddMaterialDialog()
{
    delete ui;
}
