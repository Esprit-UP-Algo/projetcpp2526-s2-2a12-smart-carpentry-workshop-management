#include "projectmanagementpage.h"
#include "projectstatspage.h"
#include "projectcalendarpage.h"
#include "projectemailalerts.h"
#include "src/database/projectdatabase.h"
#include "src/common/projectvalidators.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QDialog>
#include <QBrush>
#include <QColor>
#include <QFrame>
#include <QMap>

// ============================================================
//  Column indices
// ============================================================
enum Col {
    COL_ID      = 0,
    COL_NOM     = 1,
    COL_CLIENT  = 2,
    COL_TYPE    = 3,
    COL_DATE    = 4,
    COL_DL      = 5,
    COL_STATUS  = 6,
    COL_BUDGET  = 7,
    COL_ADRESSE = 8,
    COL_CHEF    = 9,
    COL_COUNT   = 10
};

// ============================================================
//  Constructor
// ============================================================
ProjectManagementPage::ProjectManagementPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    refreshTable();
}

// ============================================================
//  UI Setup
// ============================================================
void ProjectManagementPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    mainLayout->setSpacing(16);

    // ── Top action row: Stats + Calendar ────────────────────
    QHBoxLayout *topActions = new QHBoxLayout();
    topActions->setSpacing(10);

    m_statsBtn    = new QPushButton("Statistiques", this);
    m_calendarBtn = new QPushButton("Calendrier",   this);

    for (auto btn : {m_statsBtn, m_calendarBtn}) {
        btn->setFixedHeight(38);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton{background:#5b6f8a;color:white;border:none;"
            "border-radius:8px;font-size:13px;font-weight:700;padding:0 20px;}"
            "QPushButton:hover{background:#6b7f9a;}");
    }

    topActions->addWidget(m_statsBtn);
    topActions->addWidget(m_calendarBtn);
    topActions->addStretch();
    mainLayout->addLayout(topActions);

    m_emailAlertBtn = new QPushButton("Alertes Deadline", this);
    m_emailAlertBtn->setFixedHeight(38);
    m_emailAlertBtn->setCursor(Qt::PointingHandCursor);
    m_emailAlertBtn->setStyleSheet(
        "QPushButton{background:#e67e22;color:white;border:none;"
        "border-radius:8px;font-size:13px;font-weight:700;padding:0 20px;}"
        "QPushButton:hover{background:#f39c12;}");
    topActions->addWidget(m_emailAlertBtn);


    // ── Toolbar ──────────────────────────────────────────────
    QHBoxLayout *toolbar = new QHBoxLayout();
    toolbar->setSpacing(10);

    m_addBtn       = new QPushButton("+ Nouveau Projet", this);
    m_editBtn      = new QPushButton("Modifier",       this);
    m_deleteBtn    = new QPushButton("Supprimer",      this);
    m_exportPdfBtn = new QPushButton("Contrat Client", this);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Rechercher projet ou client…");
    m_searchEdit->setMinimumWidth(220);
    m_searchEdit->setFixedHeight(36);



    for (auto btn : {m_addBtn, m_editBtn, m_deleteBtn, m_exportPdfBtn}) {
        btn->setObjectName("actionButton");
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(36);
    }

    toolbar->addWidget(m_addBtn);
    toolbar->addWidget(m_editBtn);
    toolbar->addWidget(m_deleteBtn);
    toolbar->addWidget(m_exportPdfBtn);
    toolbar->addStretch();
    toolbar->addWidget(m_searchEdit);


    mainLayout->addLayout(toolbar);

    // ── Table ────────────────────────────────────────────────
    m_table = new QTableWidget(this);
    m_table->setObjectName("dataTable");
    m_table->setColumnCount(COL_COUNT);
    m_table->setHorizontalHeaderLabels({
        "ID", "NOM PROJET", "CLIENT", "TYPE",
        "DATE DÉBUT", "DEADLINE", "STATUT",
        "BUDGET (DT)", "ADRESSE CHANTIER", "CHEF DE PROJET"
    });
    m_table->setColumnHidden(COL_ID, true);   // ID stocké mais caché
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(false);
    mainLayout->addWidget(m_table);

    // ── Signals ──────────────────────────────────────────────
    connect(m_addBtn,       &QPushButton::clicked, this, &ProjectManagementPage::onAddButtonClicked);
    connect(m_editBtn,      &QPushButton::clicked, this, &ProjectManagementPage::onEditButtonClicked);
    connect(m_deleteBtn,    &QPushButton::clicked, this, &ProjectManagementPage::onDeleteButtonClicked);
    connect(m_exportPdfBtn, &QPushButton::clicked, this, &ProjectManagementPage::onExportPdfClicked);
    connect(m_searchEdit,   &QLineEdit::textChanged, this, &ProjectManagementPage::onSearchTextChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &ProjectManagementPage::onRowDoubleClicked);
    connect(m_statsBtn, &QPushButton::clicked, this, &ProjectManagementPage::onStatsClicked);
    connect(m_calendarBtn, &QPushButton::clicked, this, &ProjectManagementPage::onCalendarClicked);
    connect(m_emailAlertBtn, &QPushButton::clicked, this, &ProjectManagementPage::onEmailAlertClicked);
}

// ============================================================
//  Table helpers
// ============================================================
void ProjectManagementPage::colorizeStatusItem(QTableWidgetItem* item, const QString& status)
{
    if (status == "En cours")       item->setForeground(QBrush(QColor("#27ae60")));
    else if (status == "En attente") item->setForeground(QBrush(QColor("#f39c12")));
    else if (status == "Terminé")   item->setForeground(QBrush(QColor("#2980b9")));
    else if (status == "Annulé")    item->setForeground(QBrush(QColor("#e74c3c")));
}

void ProjectManagementPage::populateTable(const QList<Projet>& projets)
{
    m_table->setRowCount(0);
    ProjectDatabase& db = ProjectDatabase::instance();

    for (const Projet& p : projets) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setRowHeight(row, 46);

        auto cell = [&](int col, const QString& text) {
            auto* it = new QTableWidgetItem(text);
            it->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            m_table->setItem(row, col, it);
        };

        cell(COL_ID,      QString::number(p.getId()));
        cell(COL_NOM,     p.getNom());
        cell(COL_CLIENT,  p.getClient());
        cell(COL_TYPE,    p.getType());
        cell(COL_DATE,    p.getDateProjet().toString("dd/MM/yyyy"));
        cell(COL_DL,      p.getDeadline().toString("dd/MM/yyyy"));

        auto* statusItem = new QTableWidgetItem(p.getStatus());
        statusItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        colorizeStatusItem(statusItem, p.getStatus());
        m_table->setItem(row, COL_STATUS, statusItem);

        cell(COL_BUDGET,  QString::number(p.getBudget(), 'f', 2));
        cell(COL_ADRESSE, p.getAdresse());

        // Chef de projet : afficher le nom
        QString chefName = (p.getChefProjet() != 0)
                               ? db.getEmployeeName(p.getChefProjet())
                               : "—";
        cell(COL_CHEF, chefName);
    }
}

void ProjectManagementPage::refreshTable()
{
    populateTable(ProjectDatabase::instance().getAllProjets());
}

// ============================================================
//  Shared dialog for Add / Edit
// ============================================================
bool ProjectManagementPage::showProjectDialog(const QString& title,
                                              ProjectFormData& data,
                                              bool isEdit)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(540);
    dialog.setStyleSheet("QDialog { background-color: #f8fafc; }"
                         "QLabel  { color: #374151; font-size: 13px; }"
                         "QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox {"
                         "  background:white; border:1px solid #d1d5db;"
                         "  border-radius:6px; padding:4px 8px; min-height:32px; }"
                         "QLineEdit:focus, QComboBox:focus, QDateEdit:focus {"
                         "  border-color:#8A9A5B; }"
                         /* Error label style */
                         ".errorLabel { color: #ef4444; font-size: 11px; padding-top: 2px; }");

    QVBoxLayout *vl = new QVBoxLayout(&dialog);
    vl->setSpacing(18);
    vl->setContentsMargins(30, 28, 30, 24);

    QLabel *titleLbl = new QLabel(title, &dialog);
    titleLbl->setStyleSheet("font-size:17px;font-weight:bold;color:#2c3e50;");
    titleLbl->setAlignment(Qt::AlignCenter);
    vl->addWidget(titleLbl);

    // Form layout
    QFormLayout *form = new QFormLayout();
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // Fields
    QLineEdit      *eNom     = new QLineEdit(data.nom, &dialog);
    QLineEdit      *eClient  = new QLineEdit(data.client, &dialog);
    QLineEdit      *eAdresse = new QLineEdit(data.adresse, &dialog);

    QComboBox      *cType    = new QComboBox(&dialog);
    cType->addItems({"Meuble sur mesure","Rénovation","Agencement",
                     "Restaurant","Bureau","Cuisine","Salle de bain","Autre"});
    cType->setCurrentText(data.type);

    QDateEdit      *dDebut   = new QDateEdit(data.dateProjet.isValid()
                                          ? data.dateProjet : QDate::currentDate(), &dialog);
    dDebut->setCalendarPopup(true);
    dDebut->setDisplayFormat("dd/MM/yyyy");

    QDateEdit      *dFin     = new QDateEdit(data.deadline.isValid()
                                        ? data.deadline : QDate::currentDate().addDays(30), &dialog);
    dFin->setCalendarPopup(true);
    dFin->setDisplayFormat("dd/MM/yyyy");

    QComboBox      *cStatus  = new QComboBox(&dialog);
    cStatus->addItems({"En cours","En attente","Terminé","Annulé"});
    cStatus->setCurrentText(data.status.isEmpty() ? "En cours" : data.status);

    QDoubleSpinBox *sBudget  = new QDoubleSpinBox(&dialog);
    sBudget->setRange(0, 99999999);
    sBudget->setDecimals(2);
    sBudget->setSuffix(" DT");
    sBudget->setValue(data.budget);

    // Chef de projet combo
    QComboBox *cChef = new QComboBox(&dialog);
    cChef->addItem("— Aucun —", 0);
    QMap<int, QString> employees = ProjectDatabase::instance().getAllEmployees();
    for (auto it = employees.cbegin(); it != employees.cend(); ++it)
        cChef->addItem(it.value(), it.key());
    int idx = cChef->findData(data.chefProjetId);
    if (idx >= 0) cChef->setCurrentIndex(idx);

    // Error labels
    QLabel *errorNom = new QLabel(&dialog);
    QLabel *errorClient = new QLabel(&dialog);
    QLabel *errorAdresse = new QLabel(&dialog);
    QLabel *errorType = new QLabel(&dialog);
    QLabel *errorDateDebut = new QLabel(&dialog);
    QLabel *errorDeadline = new QLabel(&dialog);
    QLabel *errorStatus = new QLabel(&dialog);
    QLabel *errorBudget = new QLabel(&dialog);
    QLabel *errorChef = new QLabel(&dialog);

    // Set error label properties
    for (QLabel *lbl : {errorNom, errorClient, errorAdresse, errorType,
                        errorDateDebut, errorDeadline, errorStatus, errorBudget, errorChef}) {
        lbl->setProperty("class", "errorLabel");
        lbl->setStyleSheet("color: #ef4444; font-size: 11px; padding-top: 2px;");
        lbl->hide();
    }

    // Helper to add a row with field and its error label
    auto addFormRow = [&](const QString& label, QWidget* field, QLabel* errorLabel) {
        QVBoxLayout *fieldLayout = new QVBoxLayout();
        fieldLayout->setSpacing(0);
        fieldLayout->setContentsMargins(0, 0, 0, 0);
        fieldLayout->addWidget(field);
        fieldLayout->addWidget(errorLabel);
        form->addRow(label, fieldLayout);
    };

    addFormRow("Nom du projet *:",    eNom, errorNom);
    addFormRow("Client *:",           eClient, errorClient);
    addFormRow("Adresse chantier:",   eAdresse, errorAdresse);
    addFormRow("Type de projet:",     cType, errorType);
    addFormRow("Date de début:",      dDebut, errorDateDebut);
    addFormRow("Deadline:",           dFin, errorDeadline);
    addFormRow("Statut:",             cStatus, errorStatus);
    addFormRow("Budget (DT):",        sBudget, errorBudget);
    addFormRow("Chef de projet:",     cChef, errorChef);

    vl->addLayout(form);

    // Buttons
    QDialogButtonBox *bb = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    bb->button(QDialogButtonBox::Ok)->setText(isEdit ? "Modifier" : "Ajouter");
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton{background:#8A9A5B;color:white;border:none;border-radius:6px;"
        "padding:8px 22px;font-weight:bold;}"
        "QPushButton:hover{background:#9aaa6b;}");
    bb->button(QDialogButtonBox::Cancel)->setText("Annuler");
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton{background:#e2e8f0;color:#4a5568;border:none;border-radius:6px;padding:8px 18px;}"
        "QPushButton:hover{background:#cbd5e0;}");
    vl->addWidget(bb);

    // Validation function
    auto validate = [&]() -> bool {
        bool ok = true;
        QString errorMsg;
        // Clear previous errors
        for (QLabel *lbl : {errorNom, errorClient, errorAdresse, errorType,
                            errorDateDebut, errorDeadline, errorStatus, errorBudget, errorChef}) {
            lbl->hide();
            lbl->clear();
        }

        // Project name
        if (!ProjectValidators::validateProjectName(eNom->text().trimmed(), errorMsg,
                                                    isEdit ? data.id : -1)) {
            errorNom->setText(errorMsg);
            errorNom->show();
            ok = false;
        }

        // Client
        if (!ProjectValidators::validateClientName(eClient->text().trimmed(), errorMsg)) {
            errorClient->setText(errorMsg);
            errorClient->show();
            ok = false;
        }

        // Type
        if (!ProjectValidators::validateType(cType->currentText(), errorMsg)) {
            errorType->setText(errorMsg);
            errorType->show();
            ok = false;
        }

        // Dates
        if (!ProjectValidators::validateDates(dDebut->date(), dFin->date(), errorMsg)) {
            errorDeadline->setText(errorMsg);
            errorDeadline->show();
            ok = false;
        }

        // Status
        if (!ProjectValidators::validateStatus(cStatus->currentText(), errorMsg)) {
            errorStatus->setText(errorMsg);
            errorStatus->show();
            ok = false;
        }

        // Budget
        if (!ProjectValidators::validateBudget(sBudget->value(), errorMsg)) {
            errorBudget->setText(errorMsg);
            errorBudget->show();
            ok = false;
        }

        // Adresse
        if (!ProjectValidators::validateAdresse(eAdresse->text().trimmed(), errorMsg)) {
            errorAdresse->setText(errorMsg);
            errorAdresse->show();
            ok = false;
        }

        // Chef
        if (!ProjectValidators::validateChefProjet(cChef->currentData().toInt(), errorMsg)) {
            errorChef->setText(errorMsg);
            errorChef->show();
            ok = false;
        }

        return ok;
    };

    // Override the OK button to run validation
    QPushButton *okButton = bb->button(QDialogButtonBox::Ok);
    connect(okButton, &QPushButton::clicked, &dialog, [&]() {
        if (validate()) {
            dialog.accept();
        }
    });
    // Cancel button just closes
    connect(bb->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            &dialog, &QDialog::reject);

    // Run the dialog
    if (dialog.exec() != QDialog::Accepted)
        return false;

    // After acceptance, populate data (validation already passed)
    data.nom         = eNom->text().trimmed();
    data.client      = eClient->text().trimmed();
    data.adresse     = eAdresse->text().trimmed();
    data.type        = cType->currentText();
    data.dateProjet  = dDebut->date();
    data.deadline    = dFin->date();
    data.status      = cStatus->currentText();
    data.budget      = sBudget->value();
    data.chefProjetId = cChef->currentData().toInt();

    return true;
}

// ============================================================
//  CRUD Slots
// ============================================================
void ProjectManagementPage::onAddButtonClicked()
{
    ProjectFormData fd;
    fd.nom = fd.client = fd.adresse = fd.type = fd.status = "";
    fd.dateProjet  = QDate::currentDate();
    fd.deadline    = QDate::currentDate().addDays(30);
    fd.budget      = 0.0;
    fd.chefProjetId = 0;

    if (!showProjectDialog("Nouveau Projet", fd, false))
        return;

    Projet p(fd.nom, fd.dateProjet, fd.deadline, fd.status,
             fd.budget, fd.adresse, fd.client, fd.type, fd.chefProjetId);

    if (ProjectDatabase::instance().addProjet(p)) {
        QMessageBox::information(this, "Succès", "Projet ajouté avec succès !");
        refreshTable();
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible d'ajouter le projet.\nVérifiez la connexion à la base de données.");
    }
}

void ProjectManagementPage::onEditButtonClicked()
{
    if (m_table->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Sélection requise",
                             "Veuillez sélectionner un projet à modifier.");
        return;
    }

    int row = m_table->currentRow();
    int id  = m_table->item(row, COL_ID)->text().toInt();
    Projet existing = ProjectDatabase::instance().getProjet(id);

    ProjectFormData fd;
    fd.id           = id;
    fd.nom          = existing.getNom();
    fd.client       = existing.getClient();
    fd.adresse      = existing.getAdresse();
    fd.type         = existing.getType();
    fd.dateProjet   = existing.getDateProjet();
    fd.deadline     = existing.getDeadline();
    fd.status       = existing.getStatus();
    fd.budget       = existing.getBudget();
    fd.chefProjetId = existing.getChefProjet();

    if (!showProjectDialog("Modifier le Projet", fd, true))
        return;

    existing.setNom(fd.nom);
    existing.setClient(fd.client);
    existing.setAdresse(fd.adresse);
    existing.setType(fd.type);
    existing.setDateProjet(fd.dateProjet);
    existing.setDeadline(fd.deadline);
    existing.setStatus(fd.status);
    existing.setBudget(fd.budget);
    existing.setChefProjet(fd.chefProjetId);

    if (ProjectDatabase::instance().updateProjet(existing)) {
        QMessageBox::information(this, "Succès", "Projet modifié avec succès !");
        refreshTable();
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de modifier le projet.");
    }
}

void ProjectManagementPage::onDeleteButtonClicked()
{
    if (m_table->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Sélection requise",
                             "Veuillez sélectionner un projet à supprimer.");
        return;
    }

    int row    = m_table->currentRow();
    int id     = m_table->item(row, COL_ID)->text().toInt();
    QString nom = m_table->item(row, COL_NOM) ? m_table->item(row, COL_NOM)->text() : "ce projet";
    QString cli = m_table->item(row, COL_CLIENT) ? m_table->item(row, COL_CLIENT)->text() : "";

    auto ret = QMessageBox::question(
        this, "Confirmer la suppression",
        QString("Supprimer le projet « %1 » (client : %2) ?\n\nCette action est irréversible.")
            .arg(nom, cli),
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (ProjectDatabase::instance().deleteProjet(id)) {
            QMessageBox::information(this, "Succès", "Projet supprimé avec succès !");
            refreshTable();
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer le projet.");
        }
    }
}

// ============================================================
//  Export PDF — Contrat Client
// ============================================================
void ProjectManagementPage::onExportPdfClicked()
{
    if (m_table->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Sélection requise",
                             "Veuillez sélectionner un projet pour générer le contrat.");
        return;
    }

    int row = m_table->currentRow();
    int id  = m_table->item(row, COL_ID)->text().toInt();
    Projet p = ProjectDatabase::instance().getProjet(id);

    QString chefName = (p.getChefProjet() != 0)
                           ? ProjectDatabase::instance().getEmployeeName(p.getChefProjet())
                           : "À définir";

    QString filePath = QFileDialog::getSaveFileName(
        this, "Enregistrer le contrat", "",
        "Fichiers PDF (*.pdf)");

    if (filePath.isEmpty()) return;
    if (!filePath.endsWith(".pdf", Qt::CaseInsensitive))
        filePath += ".pdf";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    QString today = QDate::currentDate().toString("dd MMMM yyyy");

    QString html = QString(R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8"/>
<style>
  body {
    font-family: 'Georgia', serif;
    font-size: 11pt;
    color: #1a1a1a;
    margin: 0;
    padding: 0;
    line-height: 1.5;
  }

  .header {
    text-align: center;
    padding: 15px 0 5px 0;
    border-bottom: 2px solid #555;
  }
  .header h1 {
    margin: 0;
    font-size: 18pt;
    font-weight: bold;
    color: #2c3e50;
  }
  .header p {
    margin: 3px 0 0 0;
    font-size: 9pt;
    color: #555;
  }

  .ref-bar {
    display: flex;
    justify-content: space-between;
    font-size: 9pt;
    margin: 15px 30px 15px 30px;
    color: #333;
  }

  .body {
    padding: 0 30px;
  }

  h2 {
    font-size: 12pt;
    font-weight: bold;
    margin-top: 20px;
    padding-bottom: 2px;
    border-bottom: 1px solid #ccc;
    color: #2c3e50;
  }

  table {
    width: 100%;
    border-collapse: collapse;
    margin-top: 8px;
    font-size: 10.5pt;
  }

  td {
    padding: 4px 8px;
    vertical-align: top;
  }

  tr:nth-child(even) td {
    background: #f9f9f9;
  }

  .label {
    font-weight: bold;
    width: 180px;
    color: #2c3e50;
  }

  .clause {
    margin: 6px 0;
  }

  .signature {
    margin-top: 35px;
    display: flex;
    justify-content: space-between;
  }

  .sig-block {
    text-align: center;
    width: 45%;
    border-top: 1px solid #555;
    padding-top: 6px;
    font-size: 10pt;
    color: #333;
  }

  .footer {
    margin-top: 25px;
    font-size: 8pt;
    color: #777;
    text-align: center;
    border-top: 1px solid #ddd;
    padding-top: 6px;
  }

  .status {
    display: inline-block;
    font-size: 9pt;
    padding: 2px 8px;
    border-radius: 3px;
    color: white;
    background-color: %1;
  }

</style>
</head>
<body>

<div class="header">
  <h1>WoodFlow SARL</h1>
  <p>Solutions Menuiserie &amp; Agencement — El Gazela, Tunis, Tunisie </p>
</div>

<div class="ref-bar">
  <span>Réf. contrat : WF-%2-%3</span>
  <span>Date d'émission : %4</span>
</div>

<div class="body">

<h2>CONTRAT DE PRESTATION DE SERVICES</h2>
<p class="clause">
Entre la société <b>WoodFlow </b> (« Le Prestataire ») et <b>%5</b> (« Le Client »), il est convenu ce qui suit :
</p>

<h2>1. Identification des Parties</h2>
<table>
<tr><td class="label">Nom du client</td><td>%5</td></tr>
<tr><td class="label">Adresse chantier</td><td>%6</td></tr>
<tr><td class="label">Chef de projet assigné</td><td>%7</td></tr>
</table>

<h2>2. Objet du Contrat</h2>
<table>
<tr><td class="label">Nom du projet</td><td>%8</td></tr>
<tr><td class="label">Type de prestation</td><td>%9</td></tr>
<tr><td class="label">Statut actuel</td><td><span class="status">%10</span></td></tr>
</table>

<h2>3. Durée et Calendrier</h2>
<table>
<tr><td class="label">Date de démarrage</td><td>%11</td></tr>
<tr><td class="label">Date de livraison prévue</td><td>%12</td></tr>
</table>

<h2>4. Conditions Financières</h2>
<table>
<tr><td class="label">Budget contractuel</td><td><b>%13 DT</b></td></tr>
<tr><td class="label">Modalités de paiement</td><td>30 % à la signature · 40 % à mi-parcours · 30 % à la réception</td></tr>
<tr><td class="label">Mode de règlement accepté</td><td>Virement bancaire / Chèque certifié</td></tr>
</table>

<h2>5. Obligations du Prestataire</h2>
<p class="clause">Le Prestataire s'engage à réaliser les travaux selon les règles de l'art et informer le Client de tout imprévu.</p>

<h2>6. Obligations du Client</h2>
<p class="clause">Le Client s'engage à donner accès au chantier aux dates convenues, régler les échéances et désigner un interlocuteur unique.</p>

<h2>7. Clause de Résiliation</h2>
<p class="clause">En cas de résiliation après démarrage, les prestations réalisées restent acquises au Prestataire. Pénalité : <b>15 %</b> du montant restant.</p>

<h2>8. Droit Applicable &amp; Litiges</h2>
<p class="clause">Le contrat est soumis au droit tunisien. Tribunal de Commerce de Tunis compétent en cas de litige.</p>

<div class="signature">
  <div class="sig-block">
    <b>Le Prestataire</b><br/>WoodFlow SARL<br/><br/><br/>Signature &amp; Cachet
  </div>
  <div class="sig-block">
    <b>Le Client</b><br/>%5<br/><br/><br/>Signature
  </div>
</div>

<div class="footer">
Document généré automatiquement par WoodFlow le %4 · Toute reproduction interdite sans accord écrit.
</div>

</div>
</body>
</html>
)")
                       // badge color
                       .arg(p.getStatus() == "En cours"  ? "#27ae60" :
                                p.getStatus() == "En attente"? "#f39c12" :
                                p.getStatus() == "Terminé"   ? "#2980b9" : "#e74c3c")   // %1
                       .arg(p.getId())                                                // %2
                       .arg(QDate::currentDate().toString("yyyyMMdd"))               // %3
                       .arg(today)                                                    // %4
                       .arg(p.getClient().toHtmlEscaped())                           // %5
                       .arg(p.getAdresse().isEmpty() ? "Non spécifiée" : p.getAdresse().toHtmlEscaped()) // %6
                       .arg(chefName.toHtmlEscaped())                                // %7
                       .arg(p.getNom().toHtmlEscaped())                              // %8
                       .arg(p.getType().toHtmlEscaped())                             // %9
                       .arg(p.getStatus().toHtmlEscaped())                           // %10
                       .arg(p.getDateProjet().toString("dd/MM/yyyy"))                // %11
                       .arg(p.getDeadline().toString("dd/MM/yyyy"))                  // %12
                       .arg(QString::number(p.getBudget(), 'f', 2));                 // %13

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Contrat généré",
                             QString("Le contrat client a été enregistré :\n%1").arg(filePath));
}

// ============================================================
//  Search
// ============================================================
void ProjectManagementPage::onSearchTextChanged(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        refreshTable();
    } else {
        populateTable(ProjectDatabase::instance().searchByNomOrClient(text.trimmed()));
    }
}

// ============================================================
//  Row double-click → details popup
// ============================================================
void ProjectManagementPage::onRowDoubleClicked(int row, int /*column*/)
{
    if (row < 0 || row >= m_table->rowCount()) return;

    auto get = [&](int col) -> QString {
        auto* it = m_table->item(row, col);
        return it ? it->text() : "—";
    };

    QString status = get(COL_STATUS);
    QString statusColor =
        status == "En cours"  ? "#27ae60" :
            status == "En attente"? "#f39c12" :
            status == "Terminé"   ? "#2980b9" : "#e74c3c";

    QDialog dlg(this);
    dlg.setWindowTitle("Détails du Projet");
    dlg.setMinimumWidth(420);
    dlg.setStyleSheet("QDialog{background:#f8fafc;} QLabel{color:#374151;}");

    QVBoxLayout *vl = new QVBoxLayout(&dlg);
    vl->setContentsMargins(24, 24, 24, 20);
    vl->setSpacing(10);

    QLabel *hdr = new QLabel(get(COL_NOM), &dlg);
    hdr->setStyleSheet("font-size:16px;font-weight:bold;color:#8A9A5B;");
    hdr->setAlignment(Qt::AlignCenter);
    vl->addWidget(hdr);

    auto addRow = [&](const QString& lbl, const QString& val,
                      const QString& color = QString()) {
        QHBoxLayout *hl = new QHBoxLayout();
        QLabel *l = new QLabel(lbl + ":", &dlg);
        l->setStyleSheet("font-weight:600;min-width:140px;");
        QLabel *v = new QLabel(val, &dlg);
        if (!color.isEmpty())
            v->setStyleSheet(QString("color:%1;font-weight:bold;").arg(color));
        hl->addWidget(l);
        hl->addWidget(v, 1);
        vl->addLayout(hl);
    };

    addRow("Client",          get(COL_CLIENT));
    addRow("Adresse chantier",get(COL_ADRESSE));
    addRow("Type",            get(COL_TYPE));
    addRow("Chef de projet",  get(COL_CHEF));
    addRow("Date de début",   get(COL_DATE));
    addRow("Deadline",        get(COL_DL));
    addRow("Statut",          status, statusColor);
    addRow("Budget",          get(COL_BUDGET) + " DT");

    QPushButton *closeBtn = new QPushButton("Fermer", &dlg);
    closeBtn->setStyleSheet(
        "QPushButton{background:#8A9A5B;color:white;border:none;border-radius:6px;"
        "padding:8px 22px;font-weight:bold;}"
        "QPushButton:hover{background:#9aaa6b;}");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    vl->addSpacing(6);
    vl->addWidget(closeBtn, 0, Qt::AlignCenter);

    dlg.exec();
}

void ProjectManagementPage::onStatsClicked()
{
    ProjectStatsPage dlg(this);
    dlg.exec();
}

void ProjectManagementPage::onCalendarClicked()
{
    ProjectCalendarPage dlg(this);
    dlg.exec();
}

void ProjectManagementPage::onEmailAlertClicked()
{
    ProjectEmailAlertDialog dlg(this);
    dlg.exec();
}
