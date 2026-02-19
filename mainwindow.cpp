#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connection.h"

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_ajouter_clicked()
{
    bool ok;

    QString ui_id = QInputDialog::getText(this, "Id_tran",
        "Identifiant transaction:", QLineEdit::Normal, QString(), &ok);
    if (!ok || ui_id.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Annulé", "Id_tran requis.");
        return;
    }

    double ui_montant = QInputDialog::getDouble(this, "montant_tran",
        "Montant:", 0.0, -1e12, 1e12, 2, &ok);
    if (!ok) return;

    QString ui_date = QInputDialog::getText(this, "date_tran",
        "Date (YYYY-MM-DD):",
        QLineEdit::Normal,
        QDate::currentDate().toString("yyyy-MM-dd"),
        &ok);

    if (!ok || QDate::fromString(ui_date, "yyyy-MM-dd").isNull()) {
        QMessageBox::warning(this, "Erreur date", "Date invalide.");
        return;
    }

    QString ui_type = QInputDialog::getText(this, "type_tran",
        "Type:", QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QString ui_mode_paiement = QInputDialog::getText(this, "mode_paiement",
        "Mode de paiement:", QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QString ui_statut = QInputDialog::getText(this, "statut_tran",
        "Statut:", QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QString ui_categorie = QInputDialog::getText(this, "categorie_tran",
        "Categorie:", QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QString ui_description = QInputDialog::getText(this, "description",
        "Description:", QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QString ui_client = QInputDialog::getText(this, "client_tran",
        "Client:", QLineEdit::Normal, QString(), &ok);
    if (!ok) return;

    QSqlQuery query(QSqlDatabase::database(Connection::CONN_NAME));

    query.prepare(
        "INSERT INTO TRANSACTIONS "
        "(Id_tran, montant_tran, date_tran, type_tran, mode_paiement, statut_tran, categorie_tran, description, client_tran) "
        "VALUES "
        "(:id_tran, :montant_tran, TO_DATE(:date_tran, 'YYYY-MM-DD'), "
        ":type_tran, :mode_paiement, :statut_tran, :categorie_tran, :description, :client_tran)"
    );

    query.bindValue(":id_tran", ui_id.trimmed());
    query.bindValue(":montant_tran", ui_montant);
    query.bindValue(":date_tran", ui_date);
    query.bindValue(":type_tran", ui_type.trimmed().isEmpty() ? QVariant() : QVariant(ui_type.trimmed()));
    query.bindValue(":mode_paiement", ui_mode_paiement.trimmed().isEmpty() ? QVariant() : QVariant(ui_mode_paiement.trimmed()));
    query.bindValue(":statut_tran", ui_statut.trimmed().isEmpty() ? QVariant() : QVariant(ui_statut.trimmed()));
    query.bindValue(":categorie_tran", ui_categorie.trimmed().isEmpty() ? QVariant() : QVariant(ui_categorie.trimmed()));
    query.bindValue(":description", ui_description.trimmed().isEmpty() ? QVariant() : QVariant(ui_description.trimmed()));
    query.bindValue(":client_tran", ui_client.trimmed().isEmpty() ? QVariant() : QVariant(ui_client.trimmed()));

    if (query.exec())
    {
        QMessageBox::information(this, "Succès",
            QString("Transaction %1 ajoutée avec succès.").arg(ui_id.trimmed()));
    }
    else
    {
        qDebug() << "INSERT TRANSACTIONS error:" << query.lastError().text();
        QMessageBox::critical(this, "Erreur",
            "Échec de l'ajout de la transaction:\n" + query.lastError().text());
    }
}

void MainWindow::on_pushButton_reset_clicked()
{
    clearForm();
}

void MainWindow::clearForm()
{
    QMessageBox::information(this, "Reset",
        "Aucun champ à réinitialiser.\nLes données sont saisies via popup.");
}
