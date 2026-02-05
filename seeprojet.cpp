#include "seeprojet.h"
#include "ui_seeprojet.h"

seeprojet::seeprojet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::seeprojet)
{
    ui->setupUi(this);
    deleteProjet= nullptr;
    modifierProjet= nullptr;
}

seeprojet::~seeprojet()
{
    delete ui;
}


void seeprojet::on_sup_clicked()
{
    if (!deleteProjet) {
        deleteProjet = new deleteprojet(this);
    }

    deleteProjet->show();
}

void seeprojet::on_mod_clicked()
{
    if (!modifierProjet) {
        modifierProjet = new modifierprojet(this);
    }

    modifierProjet->show();
}

