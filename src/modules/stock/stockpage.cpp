#include "stockpage.h"
#include "src/database/stockdatabase.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QFrame>
#include <QDebug>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
StockPage::StockPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    refreshStats();
    refreshTable(StockDatabase::instance().getAllMaterials());
}

// ---------------------------------------------------------------------------
// UI Setup
// ---------------------------------------------------------------------------
void StockPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    mainLayout->setSpacing(18);

    // ── Stat cards ──────────────────────────────────────────────────────────
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);

    struct StatCard { QString title; QLabel** labelPtr; QString type; };
    QList<StatCard> cards = {
        {"VALEUR TOTALE",            &totalValueLabel,  "value"},
        {"ARTICLES EN STOCK",        &totalCountLabel,  "items"},
        {"ALERTES STOCK",            &alertCountLabel,  "alerts"},
        {"CONSOMMATION MOYS / MOIS", &renewalRateLabel, "renew"}
    };

    for (auto& card : cards) {
        QFrame *frame = new QFrame(this);
        frame->setObjectName("statCard");
        frame->setProperty("type", card.type);

        QVBoxLayout *cl = new QVBoxLayout(frame);
        cl->setSpacing(10);
        cl->setContentsMargins(20, 20, 20, 20);

        QLabel *titleLbl = new QLabel(card.title, frame);
        titleLbl->setObjectName("statTitle");

        *card.labelPtr = new QLabel("—", frame);
        (*card.labelPtr)->setObjectName("statValue");

        cl->addWidget(titleLbl);
        cl->addWidget(*card.labelPtr);
        cl->addStretch();
        statsLayout->addWidget(frame);
    }

    mainLayout->addLayout(statsLayout);

    // ── Search + Sort bar ───────────────────────────────────────────────────
    QHBoxLayout *searchSortLayout = new QHBoxLayout();
    searchSortLayout->setSpacing(12);

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Rechercher par nom ou type...");
    searchEdit->setFixedHeight(35);
    connect(searchEdit, &QLineEdit::textChanged, this, &StockPage::onSearchTextChanged);

    sortCombo = new QComboBox(this);
    sortCombo->addItems({
        "Tri par défaut",
        "Nom (A → Z)",
        "Nom (Z → A)",
        "Conso. mensuelle ↑",
        "Conso. mensuelle ↓"
    });
    sortCombo->setFixedHeight(35);
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StockPage::onSortChanged);

    searchSortLayout->addWidget(searchEdit, 3);
    searchSortLayout->addWidget(sortCombo, 1);
    searchSortLayout->addStretch();
    mainLayout->addLayout(searchSortLayout);

    // ── Action buttons ──────────────────────────────────────────────────────
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(12);

    QPushButton *addBtn       = new QPushButton("+ Ajouter un matériau", this);
    QPushButton *editBtn      = new QPushButton("Modifier", this);
    QPushButton *deleteBtn    = new QPushButton("Supprimer", this);
    QPushButton *alertBtn     = new QPushButton("Voir alertes", this);
    QPushButton *exportPdfBtn = new QPushButton("Exporter alertes PDF", this);

    for (auto btn : {addBtn, editBtn, deleteBtn, alertBtn, exportPdfBtn}) {
        btn->setObjectName("actionButton");
        btn->setCursor(Qt::PointingHandCursor);
    }

    connect(addBtn,       &QPushButton::clicked, this, &StockPage::onAddButtonClicked);
    connect(editBtn,      &QPushButton::clicked, this, &StockPage::onEditButtonClicked);
    connect(deleteBtn,    &QPushButton::clicked, this, &StockPage::onDeleteButtonClicked);
    connect(alertBtn,     &QPushButton::clicked, this, &StockPage::onShowAlertsClicked);
    connect(exportPdfBtn, &QPushButton::clicked, this, &StockPage::onExportAlertPdfClicked);

    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(editBtn);
    actionsLayout->addWidget(deleteBtn);
    actionsLayout->addWidget(exportPdfBtn);
    actionsLayout->addWidget(alertBtn);
    actionsLayout->addStretch();
    mainLayout->addLayout(actionsLayout);

    // ── Table ───────────────────────────────────────────────────────────────
    stockTable = new QTableWidget(this);
    stockTable->setObjectName("dataTable");
    stockTable->setColumnCount(10);
    stockTable->setHorizontalHeaderLabels({
        "ID", "Nom", "Type", "Quantité", "Prix unit. (DT)",
        "Fournisseur", "Seuil alerte", "Dernière commande", "Conso. mensuelle", "Unité"
    });
    stockTable->horizontalHeader()->setStretchLastSection(true);
    stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    stockTable->verticalHeader()->setVisible(false);
    stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    stockTable->setSelectionMode(QAbstractItemView::SingleSelection);
    stockTable->setAlternatingRowColors(true);
    stockTable->setShowGrid(false);
    stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Hide ID column — kept for internal use
    stockTable->setColumnHidden(0, true);

    connect(stockTable, &QTableWidget::cellDoubleClicked,
            this, &StockPage::onViewTriggered);

    mainLayout->addWidget(stockTable);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
void StockPage::refreshTable(const QList<StockMaterial>& materials)
{
    stockTable->setRowCount(materials.size());
    for (int i = 0; i < materials.size(); ++i) {
        const StockMaterial& m = materials[i];

        auto item = [](const QString& text) {
            return new QTableWidgetItem(text);
        };

        stockTable->setItem(i, 0, item(QString::number(m.getId())));
        stockTable->setItem(i, 1, item(m.getNom()));
        stockTable->setItem(i, 2, item(m.getType()));
        stockTable->setItem(i, 3, item(QString::number(m.getQuantite(), 'f', 2)));
        stockTable->setItem(i, 4, item(QString::number(m.getPrixUnitaire(), 'f', 2)));
        stockTable->setItem(i, 5, item(m.getFournisseur()));
        stockTable->setItem(i, 6, item(QString::number(m.getSeuilAlerte(), 'f', 2)));
        stockTable->setItem(i, 7, item(m.getLastOrder().toString("dd/MM/yyyy")));
        stockTable->setItem(i, 8, item(QString::number(m.getConsoMensuelle(), 'f', 2)));
        stockTable->setItem(i, 9, item(m.getUnite()));

        stockTable->setRowHeight(i, 48);

        // Highlight rows below alert threshold in light red
        if (m.isBelowAlert()) {
            for (int c = 0; c < 10; ++c) {
                if (stockTable->item(i, c))
                    stockTable->item(i, c)->setBackground(QColor(255, 220, 220));
            }
        }
    }
}

void StockPage::refreshStats()
{
    StockDatabase& db = StockDatabase::instance();
    totalValueLabel ->setText(QString("%1 DT").arg(db.getTotalValue(),  0, 'f', 2));
    totalCountLabel ->setText(QString::number(db.getTotalCount()));
    alertCountLabel ->setText(QString::number(db.getAlertCount()));
    renewalRateLabel->setText(QString("%1").arg(db.getAverageRenewalRate(), 0, 'f', 2));
}

StockMaterial StockPage::materialFromCurrentRow() const
{
    int row = stockTable->currentRow();
    if (row < 0) return StockMaterial();

    StockMaterial m;
    m.setId(stockTable->item(row, 0)->text().toInt());
    m.setNom(stockTable->item(row, 1)->text());
    m.setType(stockTable->item(row, 2)->text());
    m.setQuantite(stockTable->item(row, 3)->text().toDouble());
    m.setPrixUnitaire(stockTable->item(row, 4)->text().toDouble());
    m.setFournisseur(stockTable->item(row, 5)->text());
    m.setSeuilAlerte(stockTable->item(row, 6)->text().toDouble());
    m.setLastOrder(QDate::fromString(stockTable->item(row, 7)->text(), "dd/MM/yyyy"));
    m.setConsoMensuelle(stockTable->item(row, 8)->text().toDouble());
    m.setUnite(stockTable->item(row, 9)->text());
    return m;
}

// Helper: build the form fields and return the dialog pointer
// Used by Add and Edit to avoid code duplication
static QDialog* buildMaterialDialog(QWidget* parent, const QString& windowTitle,
                                    QLineEdit*& nomEdit, QLineEdit*& typeEdit,
                                    QDoubleSpinBox*& qteEdit, QDoubleSpinBox*& prixEdit,
                                    QLineEdit*& fournEdit, QDoubleSpinBox*& seuilEdit,
                                    QDateEdit*& dateEdit, QDoubleSpinBox*& consoEdit,
                                    QLineEdit*& uniteEdit)
{
    QDialog* dialog = new QDialog(parent);
    dialog->setObjectName("stockDialog");
    dialog->setWindowTitle(windowTitle);
    dialog->setMinimumWidth(520);
    dialog->setModal(true);

    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    QLabel* title = new QLabel(windowTitle, dialog);
    title->setObjectName("dialogTitle");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QFormLayout* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setSpacing(14);

    nomEdit   = new QLineEdit(dialog);
    typeEdit  = new QLineEdit(dialog);
    qteEdit   = new QDoubleSpinBox(dialog);   qteEdit->setRange(0, 9999999); qteEdit->setDecimals(2);
    prixEdit  = new QDoubleSpinBox(dialog);   prixEdit->setRange(0, 9999999); prixEdit->setDecimals(2); prixEdit->setSuffix(" DT");
    fournEdit = new QLineEdit(dialog);
    seuilEdit = new QDoubleSpinBox(dialog);   seuilEdit->setRange(0, 9999999); seuilEdit->setDecimals(2);
    dateEdit  = new QDateEdit(dialog);        dateEdit->setDate(QDate::currentDate()); dateEdit->setCalendarPopup(true); dateEdit->setDisplayFormat("dd/MM/yyyy");
    consoEdit = new QDoubleSpinBox(dialog);   consoEdit->setRange(0, 9999999); consoEdit->setDecimals(2);
    uniteEdit = new QLineEdit(dialog);

    form->addRow("Nom *",                   nomEdit);
    form->addRow("Type",                    typeEdit);
    form->addRow("Quantité en stock",       qteEdit);
    form->addRow("Prix unitaire",           prixEdit);
    form->addRow("Fournisseur",             fournEdit);
    form->addRow("Seuil d'alerte",          seuilEdit);
    form->addRow("Date dernière commande",  dateEdit);
    form->addRow("Consommation mensuelle",  consoEdit);
     form->addRow("Unité", uniteEdit);

    mainLayout->addLayout(form);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    buttons->button(QDialogButtonBox::Ok)->setText("Enregistrer");
    buttons->button(QDialogButtonBox::Ok)->setObjectName("saveButton");
    buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");
    buttons->button(QDialogButtonBox::Cancel)->setObjectName("cancelButton");
    QObject::connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    mainLayout->addWidget(buttons, 0, Qt::AlignCenter);

    return dialog;
}

// ---------------------------------------------------------------------------
// CRUD slots
// ---------------------------------------------------------------------------
void StockPage::onAddButtonClicked()
{
    QLineEdit *nomEdit, *typeEdit, *fournEdit, *uniteEdit;;
    QDoubleSpinBox *qteEdit, *prixEdit, *seuilEdit, *consoEdit;
    QDateEdit *dateEdit;

    QDialog* dialog = buildMaterialDialog(this, "Ajouter un matériau",
                                          nomEdit, typeEdit, qteEdit, prixEdit,
                                          fournEdit, seuilEdit, dateEdit, consoEdit, uniteEdit);

    if (dialog->exec() == QDialog::Accepted) {
        if (nomEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Champ obligatoire", "Le nom du matériau est obligatoire.");
            delete dialog;
            return;
        }

        StockMaterial mat;
        mat.setNom(nomEdit->text().trimmed());
        mat.setType(typeEdit->text().trimmed());
        mat.setQuantite(qteEdit->value());
        mat.setPrixUnitaire(prixEdit->value());
        mat.setFournisseur(fournEdit->text().trimmed());
        mat.setSeuilAlerte(seuilEdit->value());
        mat.setLastOrder(dateEdit->date());
        mat.setConsoMensuelle(consoEdit->value());
        mat.setUnite(uniteEdit->text().trimmed());

        if (StockDatabase::instance().addMaterial(mat)) {
            QMessageBox::information(this, "Succès", "Matériau ajouté avec succès.");
            refreshStats();
            refreshTable(StockDatabase::instance().getAllMaterials());
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible d'ajouter le matériau. Vérifiez la connexion.");
        }
    }
    delete dialog;
}

void StockPage::onEditButtonClicked()
{
    if (stockTable->currentRow() < 0) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un matériau à modifier.");
        return;
    }

    StockMaterial current = materialFromCurrentRow();

    QLineEdit *nomEdit, *typeEdit, *fournEdit , *uniteEdit ;
    QDoubleSpinBox *qteEdit, *prixEdit, *seuilEdit, *consoEdit;
    QDateEdit *dateEdit;

    QDialog* dialog = buildMaterialDialog(this, "Modifier un matériau",
                                          nomEdit, typeEdit, qteEdit, prixEdit,
                                          fournEdit, seuilEdit, dateEdit, consoEdit,
                                          uniteEdit);

    // Pre-fill with current values
    nomEdit->setText(current.getNom());
    typeEdit->setText(current.getType());
    qteEdit->setValue(current.getQuantite());
    prixEdit->setValue(current.getPrixUnitaire());
    fournEdit->setText(current.getFournisseur());
    seuilEdit->setValue(current.getSeuilAlerte());
    if (!current.getLastOrder().isNull())
        dateEdit->setDate(current.getLastOrder());
    consoEdit->setValue(current.getConsoMensuelle());

    if (dialog->exec() == QDialog::Accepted) {
        if (nomEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Champ obligatoire", "Le nom du matériau est obligatoire.");
            delete dialog;
            return;
        }

        current.setNom(nomEdit->text().trimmed());
        current.setType(typeEdit->text().trimmed());
        current.setQuantite(qteEdit->value());
        current.setPrixUnitaire(prixEdit->value());
        current.setFournisseur(fournEdit->text().trimmed());
        current.setSeuilAlerte(seuilEdit->value());
        current.setLastOrder(dateEdit->date());
        current.setConsoMensuelle(consoEdit->value());
        current.setUnite(uniteEdit->text().trimmed());

        if (StockDatabase::instance().updateMaterial(current)) {
            QMessageBox::information(this, "Succès", "Matériau mis à jour avec succès.");
            refreshStats();
            refreshTable(StockDatabase::instance().getAllMaterials());
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de mettre à jour le matériau.");
        }
    }
    delete dialog;
}

void StockPage::onDeleteButtonClicked()
{
    if (stockTable->currentRow() < 0) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un matériau à supprimer.");
        return;
    }

    StockMaterial current = materialFromCurrentRow();

    auto reply = QMessageBox::question(
        this,
        "Confirmer la suppression",
        QString("Êtes-vous sûr de vouloir supprimer « %1 » ?").arg(current.getNom()),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        if (StockDatabase::instance().deleteMaterial(current.getId())) {
            QMessageBox::information(this, "Succès", "Matériau supprimé avec succès.");
            refreshStats();
            refreshTable(StockDatabase::instance().getAllMaterials());
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer le matériau.");
        }
    }
}

void StockPage::onViewTriggered(int row, int)
{
    if (row < 0) return;

    QStringList labels = {
        "ID", "Nom", "Type", "Quantité en stock", "Prix unitaire (DT)",
        "Fournisseur", "Seuil alerte", "Dernière commande", "Conso. mensuelle","Unité"
    };

    QDialog dialog(this);
    dialog.setObjectName("stockViewDialog");
    dialog.setWindowTitle("Détails du matériau");
    dialog.setMinimumWidth(460);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    QLabel *title = new QLabel("Détails du matériau", &dialog);
    title->setObjectName("dialogTitle");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setSpacing(12);

    for (int i = 1; i < stockTable->columnCount(); ++i) {  // skip hidden ID col 0
        QLabel *val = new QLabel(stockTable->item(row, i)->text(), &dialog);
        val->setObjectName("viewValue");
        form->addRow(labels[i] + " :", val);
    }
    mainLayout->addLayout(form);

    QPushButton *closeBtn = new QPushButton("Fermer", &dialog);
    closeBtn->setObjectName("backButton");
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignCenter);

    dialog.exec();
}

void StockPage::onShowAlertsClicked()
{
    QList<StockMaterial> all = StockDatabase::instance().getAllMaterials();
    QStringList alertLines;
    for (const auto& m : all) {
        if (m.isBelowAlert())
            alertLines << QString("• %1 — stock: %2 (seuil: %3)")
                              .arg(m.getNom())
                              .arg(m.getQuantite())
                              .arg(m.getSeuilAlerte());
    }

    if (alertLines.isEmpty()) {
        QMessageBox::information(this, "Alertes Stock", "Aucun matériau en dessous du seuil d'alerte.");
    } else {
        QMessageBox::warning(this, "Alertes Stock",
                             QString("%1 matériau(x) en alerte :\n\n").arg(alertLines.size())
                                 + alertLines.join("\n"));
    }
}

void StockPage::onExportAlertPdfClicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter les alertes en PDF", "alertes_stock.pdf", "Fichiers PDF (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Landscape);

    QList<StockMaterial> all = StockDatabase::instance().getAllMaterials();
    QString html = "<html><body style='font-family:Arial;'>";
    html += "<h2 style='color:#c0392b;'>Rapport d'alertes Stock — WoodFlow</h2>";
    html += "<p>Date : " + QDate::currentDate().toString("dd/MM/yyyy") + "</p>";
    html += "<table border='1' cellspacing='0' cellpadding='5' width='100%'>";
    html += "<tr style='background:#c0392b;color:white;'>"
            "<th>Nom</th><th>Type</th><th>Quantité</th>"
            "<th>Seuil</th><th>Fournisseur</th><th>Conso. mensuelle</th></tr>";

    bool hasAlerts = false;
    for (const auto& m : all) {
        if (m.isBelowAlert()) {
            hasAlerts = true;
            html += QString("<tr style='background:#ffd5d5;'>"
                            "<td>%1</td><td>%2</td><td>%3</td>"
                            "<td>%4</td><td>%5</td><td>%6</td></tr>")
                        .arg(m.getNom()).arg(m.getType())
                        .arg(m.getQuantite()).arg(m.getSeuilAlerte())
                        .arg(m.getFournisseur()).arg(m.getConsoMensuelle());
        }
    }

    if (!hasAlerts)
        html += "<tr><td colspan='6'>Aucun article en alerte.</td></tr>";

    html += "</table></body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Export PDF",
                             "Rapport d'alertes exporté avec succès :\n" + fileName);
}

// ---------------------------------------------------------------------------
// Search & Sort
// ---------------------------------------------------------------------------
void StockPage::onSearchTextChanged(const QString& text)
{
    if (text.trimmed().isEmpty()) {
        // Re-apply current sort instead of bare getAllMaterials
        onSortChanged(sortCombo->currentIndex());
        return;
    }
    QList<StockMaterial> results = StockDatabase::instance().searchByNomOrType(text.trimmed());
    refreshTable(results);
}

void StockPage::onSortChanged(int index)
{
    // If search is active, filter first then sort in memory
    QString searchText = searchEdit->text().trimmed();
    QList<StockMaterial> base = searchText.isEmpty()
                                    ? StockDatabase::instance().getAllMaterials()
                                    : StockDatabase::instance().searchByNomOrType(searchText);

    switch (index) {
    case 0: // Default (ID order, already from getAllMaterials)
        break;
    case 1: // Nom A→Z
        std::sort(base.begin(), base.end(), [](const StockMaterial& a, const StockMaterial& b){
            return a.getNom() < b.getNom();
        });
        break;
    case 2: // Nom Z→A
        std::sort(base.begin(), base.end(), [](const StockMaterial& a, const StockMaterial& b){
            return a.getNom() > b.getNom();
        });
        break;
    case 3: // Conso mensuelle ↑
        std::sort(base.begin(), base.end(), [](const StockMaterial& a, const StockMaterial& b){
            return a.getConsoMensuelle() < b.getConsoMensuelle();
        });
        break;
    case 4: // Conso mensuelle ↓
        std::sort(base.begin(), base.end(), [](const StockMaterial& a, const StockMaterial& b){
            return a.getConsoMensuelle() > b.getConsoMensuelle();
        });
        break;
    default:
        break;
    }

    refreshTable(base);
}
