#include "suppmaterieldialog.h"
#include "ui_suppmaterieldialog.h"

SuppMaterielDialog::SuppMaterielDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SuppMaterielDialog)
{
    ui->setupUi(this);
}

SuppMaterielDialog::~SuppMaterielDialog()
{
    delete ui;
}
