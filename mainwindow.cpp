#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addmaterialdialog.h"
#include "modifmaterieldialog.h"
#include "affichermaterieldialog.h"
#include "suppmaterieldialog.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Optionnel: Vérifier que le CSS est appliqué
    qDebug() << "MainWindow créée";
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Slot pour le bouton "Ajouter"
void MainWindow::on_btnAddMaterial_clicked()
{
    qDebug() << "Bouton Ajouter cliqué";
    AddMaterialDialog dialog(this);
    dialog.exec();
}

// Slot pour le bouton "Modifier"
void MainWindow::on_pushButton_2_clicked()
{
    qDebug() << "Bouton Modifier cliqué";
    ModifMaterielDialog dialog(this);
    dialog.exec();
}

// Slot pour le bouton "Afficher"
void MainWindow::on_pushButton_3_clicked()
{
    qDebug() << "Bouton Afficher cliqué";
    AfficherMaterielDialog dialog(this);
    dialog.exec();
}

// Slot pour le bouton "Supprimer"
void MainWindow::on_pushButton_4_clicked()
{
    qDebug() << "Bouton Supprimer cliqué";
    SuppMaterielDialog dialog(this);
    dialog.exec();
}
