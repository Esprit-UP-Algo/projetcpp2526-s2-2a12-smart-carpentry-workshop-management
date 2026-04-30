#include "transactiondialog.h"
#include "src/database/connection.h"
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>

TransactionDialog::TransactionDialog(QWidget *parent) : QDialog(parent)
{
    setupUI();

    // Remplacer la connexion par défaut pour ajouter la validation
    disconnect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    // Personnaliser les boutons APRÈS avoir créé l'interface
    // Note: Il faut le faire après setupUI() car m_buttonBox y est créé

    connect(m_buttonBox, &QDialogButtonBox::accepted, [this]() {
        // Vérifier si le champ montant est vide
        if (m_montantEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Le montant est obligatoire.");
            m_montantEdit->setFocus();
            return;
        }

        // Vérifier si le montant est valide et supérieur à 0
        double montant = m_montantEdit->text().toDouble();
        if (montant <= 0) {
            QMessageBox::warning(this, "Erreur", "Le montant doit être supérieur à 0.");
            m_montantEdit->setFocus();
            m_montantEdit->selectAll();
            return;
        }

        // Toutes les validations sont passées
        accept();
    });

    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void TransactionDialog::setupUI()
{
    setMinimumWidth(480);

    QFormLayout *form = new QFormLayout(this);
    form->setSpacing(12);

    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({"Facture", "Devis", "Acompte"});

    m_modeCombo = new QComboBox();
    m_modeCombo->addItems({"Carte bancaire", "Especes", "Virement", "Cheque", "Prelevement"});

    m_statutCombo = new QComboBox();
    m_statutCombo->addItems({"Paye", "En attente", "Retard", "Annule"});

    m_categoryCombo = new QComboBox();
    m_categoryCombo->addItems({"Recette", "Depense"});

    // Combo projet
    m_projetCombo = new QComboBox();
    loadProjectsFromDatabase();  // Charger les projets

    m_montantEdit = new QLineEdit();
    m_montantEdit->setValidator(new QDoubleValidator(0.001, 9999999999.999, 3, m_montantEdit));
    m_montantEdit->setPlaceholderText("0.000");

    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd/MM/yyyy");

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Personnaliser les textes des boutons
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("Valider");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText("Annuler");

    // Ne pas connecter ici, la connexion sera faite dans le constructeur
    // connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject); // Déplacé dans le constructeur

    form->addRow("Type :", m_typeCombo);
    form->addRow("Mode de paiement :", m_modeCombo);
    form->addRow("Statut :", m_statutCombo);
    form->addRow("Catégorie :", m_categoryCombo);
    form->addRow("Projet :", m_projetCombo);
    form->addRow("Montant (DT) :", m_montantEdit);
    form->addRow("Date :", m_dateEdit);
    form->addRow(m_buttonBox);
}

void TransactionDialog::loadProjectsFromDatabase()
{
    // Ajouter l'option "Aucun projet"
    m_projetCombo->addItem("Aucun projet", QVariant(QVariant::Int));  // Valeur NULL

    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    if (!db.isOpen()) {
        qDebug() << "Database not open in TransactionDialog";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT ID_PROJET, NOM_PROJET FROM PROJET ORDER BY NOM_PROJET");

    if (!query.exec()) {
        qDebug() << "Error loading projects:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int idProjet = query.value(0).toInt();
        QString nomProjet = query.value(1).toString();
        m_projetCombo->addItem(nomProjet, idProjet);
        qDebug() << "Loaded project:" << idProjet << nomProjet;
    }
}

void TransactionDialog::setData(const QMap<QString, QString> &data)
{
    m_typeCombo->setCurrentText(data["TYPE_TRAN"]);
    m_modeCombo->setCurrentText(data["MODE_PAIEMENT"]);
    m_statutCombo->setCurrentText(data["STATUT_TRAN"]);
    m_categoryCombo->setCurrentText(data["CATEGORIE_TRAN"]);
    m_montantEdit->setText(data["MONTANT_TRAN"]);
    m_dateEdit->setDate(QDate::fromString(data["DATE_TRAN"], "dd/MM/yyyy"));

    // Sélectionner le projet par son ID
    if (data.contains("CONTRAT_PROJET") && !data["CONTRAT_PROJET"].isEmpty()) {
        int projetId = data["CONTRAT_PROJET"].toInt();
        int index = m_projetCombo->findData(projetId);
        if (index >= 0) {
            m_projetCombo->setCurrentIndex(index);
        }
    }
}

QMap<QString, QString> TransactionDialog::getData() const
{
    QMap<QString, QString> data;
    data["TYPE_TRAN"] = m_typeCombo->currentText();
    data["MODE_PAIEMENT"] = m_modeCombo->currentText();
    data["STATUT_TRAN"] = m_statutCombo->currentText();
    data["CATEGORIE_TRAN"] = m_categoryCombo->currentText();
    data["MONTANT_TRAN"] = QString::number(m_montantEdit->text().toDouble(), 'f', 3);
    data["DATE_TRAN"] = m_dateEdit->date().toString("dd/MM/yyyy");

    // Récupérer l'ID du projet
    int projetId = m_projetCombo->currentData().toInt();
    if (projetId > 0) {
        data["CONTRAT_PROJET"] = QString::number(projetId);
    } else {
        data["CONTRAT_PROJET"] = "";
    }

    return data;
}
