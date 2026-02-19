#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connection.h"

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->dateEdit_embauche->setDate(QDate::currentDate());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_ajouter_clicked()
{
    if (ui->lineEdit_cin->text().trimmed().isEmpty() ||
        ui->lineEdit_nom->text().trimmed().isEmpty() ||
        ui->lineEdit_prenom->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Champs manquants",
                             "CIN, Nom et Prénom sont obligatoires.");
        return;
    }

    QSqlQuery query(QSqlDatabase::database(Connection::CONN_NAME));
    query.prepare(
        "INSERT INTO EMPLOYE "
        "(ID_EMP, CIN, NOM_EMP, PRENOM_EMP, POST_EMP, EMAIL_EMP, NUM_TEL, "
        " DATE_EMBAUCHE, SALAIRE, COMPETENCES, DISPO_EMP, PERFORMANCE, NJC, NJA, HDT) "
        "VALUES "
        "((SELECT NVL(MAX(ID_EMP),0)+1 FROM EMPLOYE), "
        " :cin, :nom, :prenom, :poste, :email, :tel, "
        " TO_DATE(:date_emb, 'YYYY-MM-DD'), :salaire, :competences, :dispo, :perf, :njc, :nja, :hdt)"
    );

    query.bindValue(":cin",         ui->lineEdit_cin->text().trimmed());
    query.bindValue(":nom",         ui->lineEdit_nom->text().trimmed());
    query.bindValue(":prenom",      ui->lineEdit_prenom->text().trimmed());
    query.bindValue(":poste",       ui->lineEdit_poste->text().trimmed().isEmpty()
                                        ? QVariant() : QVariant(ui->lineEdit_poste->text().trimmed()));
    query.bindValue(":email",       ui->lineEdit_email->text().trimmed().isEmpty()
                                        ? QVariant() : QVariant(ui->lineEdit_email->text().trimmed()));
    query.bindValue(":tel",         ui->lineEdit_tel->text().trimmed().isEmpty()
                                        ? QVariant() : QVariant(ui->lineEdit_tel->text().trimmed()));
    query.bindValue(":date_emb",    ui->dateEdit_embauche->date().toString("yyyy-MM-dd"));
    query.bindValue(":salaire",     ui->spinBox_salaire->value());
    query.bindValue(":competences", ui->lineEdit_competences->text().trimmed().isEmpty()
                                        ? QVariant() : QVariant(ui->lineEdit_competences->text().trimmed()));
    query.bindValue(":dispo",       ui->comboBox_dispo->currentText());
    query.bindValue(":perf",        ui->spinBox_perf->value());
    query.bindValue(":njc",         ui->spinBox_njc->value());
    query.bindValue(":nja",         ui->spinBox_nja->value());
    query.bindValue(":hdt",         ui->spinBox_hdt->value());

    if (query.exec())
    {
        QMessageBox::information(this, "Succès",
            QString("Employé %1 %2 ajouté avec succès.")
                .arg(ui->lineEdit_prenom->text().trimmed())
                .arg(ui->lineEdit_nom->text().trimmed()));
        clearForm();
    }
    else
    {
        qDebug() << "INSERT error:" << query.lastError().text();
        QMessageBox::critical(this, "Erreur",
            "Échec de l'ajout:\n" + query.lastError().text());
    }
}

void MainWindow::on_pushButton_reset_clicked()
{
    clearForm();
}

void MainWindow::clearForm()
{
    ui->lineEdit_cin->clear();
    ui->lineEdit_nom->clear();
    ui->lineEdit_prenom->clear();
    ui->lineEdit_poste->clear();
    ui->lineEdit_email->clear();
    ui->lineEdit_tel->clear();
    ui->lineEdit_competences->clear();
    ui->dateEdit_embauche->setDate(QDate::currentDate());
    ui->spinBox_salaire->setValue(0);
    ui->comboBox_dispo->setCurrentIndex(0);
    ui->spinBox_perf->setValue(0);
    ui->spinBox_njc->setValue(0);
    ui->spinBox_nja->setValue(0);
    ui->spinBox_hdt->setValue(0);
}
