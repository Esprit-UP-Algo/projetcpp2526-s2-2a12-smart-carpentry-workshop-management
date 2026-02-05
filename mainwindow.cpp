#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Ajoutprojet= nullptr;
    deleteProjet= nullptr;
    seeProjet= nullptr;
    modifierProjet= nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_delete_2_clicked()
{
    if (!deleteProjet) {
        deleteProjet = new deleteprojet(this);
    }

    deleteProjet->show();
}

void MainWindow::on_ajout_clicked()
{
    if (!Ajoutprojet) {
        Ajoutprojet = new AjoutProjet(this);
    }

    Ajoutprojet->show();
}

void MainWindow::on_modifier_clicked()
{
    if (!modifierProjet) {
        modifierProjet = new modifierprojet(this);
    }

    modifierProjet->show();
}

void MainWindow::on_see_clicked()
{
    if (!seeProjet) {
        seeProjet = new seeprojet(this);
    }

    seeProjet->show();
}
