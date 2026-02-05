#include "affichermaterieldialog.h"
#include "ui_affichermaterieldialog.h"

AfficherMaterielDialog::AfficherMaterielDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AfficherMaterielDialog)
{
    ui->setupUi(this);
}

AfficherMaterielDialog::~AfficherMaterielDialog()
{
    delete ui;
}
