#include "transactiondialog.h"
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QDate>

TransactionDialog::TransactionDialog(QWidget *parent) : QDialog(parent)
{
    setupUI();
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

    m_montantEdit = new QLineEdit();
    m_montantEdit->setValidator(new QDoubleValidator(0, 9999999999.999, 3, m_montantEdit));

    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd/MM/yyyy");

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    form->addRow("Type :", m_typeCombo);
    form->addRow("Mode de paiement :", m_modeCombo);
    form->addRow("Statut :", m_statutCombo);
    form->addRow("Catégorie :", m_categoryCombo);
    form->addRow("Montant (DT) :", m_montantEdit);
    form->addRow("Date :", m_dateEdit);
    form->addRow(m_buttonBox);
}

void TransactionDialog::setData(const QMap<QString, QString> &data)
{
    m_typeCombo->setCurrentText(data["TYPE_TRAN"]);
    m_modeCombo->setCurrentText(data["MODE_PAIEMENT"]);
    m_statutCombo->setCurrentText(data["STATUT_TRAN"]);
    m_categoryCombo->setCurrentText(data["CATEGORIE_TRAN"]);
    m_montantEdit->setText(data["MONTANT_TRAN"]);
    m_dateEdit->setDate(QDate::fromString(data["DATE_TRAN"], "dd/MM/yyyy"));
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
    return data;
}
