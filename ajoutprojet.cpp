#include "ajoutprojet.h"
#include "ui_ajoutprojet.h"

AjoutProjet::AjoutProjet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AjoutProjet)
{
    ui->setupUi(this);
}

AjoutProjet::~AjoutProjet()
{
    delete ui;
}
