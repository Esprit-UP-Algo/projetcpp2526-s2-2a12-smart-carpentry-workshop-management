#include "stockpage.h"
#include <QFormLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QDate>
#include <QTableWidgetSelectionRange>

StockPage::StockPage(QWidget *parent)
    : QWidget(parent)
    , currentRow(-1)
{
    setupUI();
}

void StockPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    stackedWidget = new QStackedWidget(this);
    stackedWidget->setObjectName("stockStackedWidget");

    // Add pages in order
    stackedWidget->addWidget(createStockListPage());  // index 0
    stackedWidget->addWidget(createAddPage());        // index 1
    stackedWidget->addWidget(createEditPage());       // index 2
    stackedWidget->addWidget(createViewPage());       // index 3

    mainLayout->addWidget(stackedWidget);
}

// ------------------------------------------------------------------
// Page 0 : Stock List
// ------------------------------------------------------------------
QWidget* StockPage::createStockListPage()
{
    QWidget *page = new QWidget();
    page->setObjectName("stockListPage");

    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(18);
    layout->setContentsMargins(25, 25, 25, 25);

    // ---------- Statistics ----------
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);

    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"VALEUR TOTALE", "45 280 DT", "value"},
        {"ARTICLES EN STOCK", "156", "items"},
        {"ALERTES STOCK", "8", "alerts"}
    };

    for (const auto& stat : stats) {
        QFrame *card = new QFrame(page);
        card->setObjectName("statCard");
        card->setProperty("type", stat.type);

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setSpacing(10);
        cardLayout->setContentsMargins(20, 20, 20, 20);

        QLabel *title = new QLabel(stat.title, card);
        title->setObjectName("statTitle");

        QLabel *value = new QLabel(stat.value, card);
        value->setObjectName("statValue");

        cardLayout->addWidget(title);
        cardLayout->addWidget(value);
        cardLayout->addStretch();
        statsLayout->addWidget(card);
    }

    layout->addLayout(statsLayout);

    // ---------- Action Buttons ----------
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    QPushButton *addBtn    = new QPushButton("+ Ajouter un matériau", page);
    QPushButton *editBtn   = new QPushButton("Modifier", page);
    QPushButton *deleteBtn = new QPushButton("Supprimer", page);
    QPushButton *alertBtn  = new QPushButton("Alertes Stock", page);

    addBtn->setObjectName("actionButton");
    editBtn->setObjectName("actionButton");
    deleteBtn->setObjectName("actionButton");
    alertBtn->setObjectName("actionButton");

    addBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    alertBtn->setCursor(Qt::PointingHandCursor);

    connect(addBtn, &QPushButton::clicked, this, &StockPage::onAddButtonClicked);
    connect(editBtn, &QPushButton::clicked, this, &StockPage::onEditButtonClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &StockPage::onDeleteButtonClicked);
    // alertBtn : no action (visual only)

    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(editBtn);
    actionsLayout->addWidget(deleteBtn);
    actionsLayout->addWidget(alertBtn);
    actionsLayout->addStretch();

    layout->addLayout(actionsLayout);

    // ---------- Table with 9 columns ----------
    stockTable = new QTableWidget(page);
    stockTable->setObjectName("dataTable");
    stockTable->setColumnCount(9);
    stockTable->setHorizontalHeaderLabels({
        "ID", "Nom", "Type",
        "Quantité en stock", "Prix unitaire",
        "Fournisseur", "Seuil alerte",
        "Date dernière commande", "Consommation mensuelle"
    });
    stockTable->horizontalHeader()->setStretchLastSection(true);
    stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    stockTable->verticalHeader()->setVisible(false);
    stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    stockTable->setSelectionMode(QAbstractItemView::SingleSelection);
    stockTable->setAlternatingRowColors(true);
    stockTable->setShowGrid(false);
    stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Sample data
    stockTable->setRowCount(4);
    QStringList row1 = {"M001", "Chêne massif", "Bois", "120 m²", "45.50 DT", "Bois & Cie", "20", "12/02/2026", "8 m²"};
    QStringList row2 = {"M002", "Contreplaqué", "Panneau", "85 feuilles", "28.00 DT", "Matériaux Moderne", "15", "05/02/2026", "12 feuilles"};
    QStringList row3 = {"M003", "Vis à bois", "Quincaillerie", "2500 unités", "0.12 DT", "Fixation Pro", "500", "20/01/2026", "350 unités"};
    QStringList row4 = {"M004", "Vernis mat", "Finition", "18 L", "32.80 DT", "Peinture Plus", "5", "15/02/2026", "2.5 L"};

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 9; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem();
            if (row == 0) item->setText(row1[col]);
            else if (row == 1) item->setText(row2[col]);
            else if (row == 2) item->setText(row3[col]);
            else if (row == 3) item->setText(row4[col]);
            stockTable->setItem(row, col, item);
        }
        stockTable->setRowHeight(row, 50);
    }

    // Double-click -> view details
    connect(stockTable, &QTableWidget::cellDoubleClicked,
            this, &StockPage::onViewTriggered);

    layout->addWidget(stockTable);
    layout->addStretch();

    return page;
}

// ------------------------------------------------------------------
// Page 1 : Add Material (empty form)
// ------------------------------------------------------------------
QWidget* StockPage::createAddPage()
{
    QWidget *page = new QWidget();
    page->setObjectName("addMaterialPage");

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // ---------- Carte encadrée centrée ----------
    QFrame *formCard = new QFrame(page);
    formCard->setObjectName("formCard");
    formCard->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(36, 32, 36, 32);
    cardLayout->setSpacing(24);

    // Titre
    QLabel *title = new QLabel("Ajouter un matériau", formCard);
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(title);

    // Formulaire
    QFormLayout *form = new QFormLayout();
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setSpacing(16);

    // Champs (inchangés)
    addId                = new QLineEdit(formCard);
    addNom               = new QLineEdit(formCard);
    addType              = new QLineEdit(formCard);
    addQuantite          = new QSpinBox(formCard);
    addQuantite->setRange(0, 999999);
    addPrixUnitaire      = new QDoubleSpinBox(formCard);
    addPrixUnitaire->setRange(0, 999999);
    addPrixUnitaire->setPrefix("DT ");
    addFournisseur       = new QLineEdit(formCard);
    addSeuilAlerte       = new QSpinBox(formCard);
    addSeuilAlerte->setRange(0, 999999);
    addDateCommande      = new QDateEdit(formCard);
    addDateCommande->setDate(QDate::currentDate());
    addDateCommande->setCalendarPopup(true);
    addConsoMensuelle    = new QLineEdit(formCard);
    addConsoMensuelle->setPlaceholderText("ex: 8 m²");

    // Ajout au formulaire
    form->addRow("ID :", addId);
    form->addRow("Nom :", addNom);
    form->addRow("Type :", addType);
    form->addRow("Quantité en stock :", addQuantite);
    form->addRow("Prix unitaire :", addPrixUnitaire);
    form->addRow("Fournisseur :", addFournisseur);
    form->addRow("Seuil alerte :", addSeuilAlerte);
    form->addRow("Date dernière commande :", addDateCommande);
    form->addRow("Consommation mensuelle :", addConsoMensuelle);

    cardLayout->addLayout(form);

    // Boutons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(16);
    buttonLayout->setAlignment(Qt::AlignCenter);

    QPushButton *saveBtn = new QPushButton("Enregistrer", formCard);
    saveBtn->setObjectName("saveButton");
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &StockPage::onSaveAddButtonClicked);

    QPushButton *backBtn = new QPushButton("← Retour à la liste", formCard);
    backBtn->setObjectName("backButton");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &StockPage::onBackToList);

    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(backBtn);
    cardLayout->addLayout(buttonLayout);
    cardLayout->addStretch();

    // Centrer la carte dans la page
    pageLayout->addWidget(formCard, 0, Qt::AlignCenter);
    pageLayout->addStretch();

    return page;
}
// ------------------------------------------------------------------
// Page 2 : Edit Material (pre-filled with selected row)
// ------------------------------------------------------------------
QWidget* StockPage::createEditPage()
{
    QWidget *page = new QWidget();
    page->setObjectName("editMaterialPage");

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // ---------- Carte encadrée centrée ----------
    QFrame *formCard = new QFrame(page);
    formCard->setObjectName("formCard");
    formCard->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(36, 32, 36, 32);
    cardLayout->setSpacing(24);

    // Titre
    QLabel *title = new QLabel("Modifier un matériau", formCard);
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(title);

    // Formulaire
    QFormLayout *form = new QFormLayout();
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setSpacing(16);

    // Champs (inchangés)
    editId                = new QLineEdit(formCard);
    editNom               = new QLineEdit(formCard);
    editType              = new QLineEdit(formCard);
    editQuantite          = new QSpinBox(formCard);
    editQuantite->setRange(0, 999999);
    editPrixUnitaire      = new QDoubleSpinBox(formCard);
    editPrixUnitaire->setRange(0, 999999);
    editPrixUnitaire->setPrefix("DT ");
    editFournisseur       = new QLineEdit(formCard);
    editSeuilAlerte       = new QSpinBox(formCard);
    editSeuilAlerte->setRange(0, 999999);
    editDateCommande      = new QDateEdit(formCard);
    editDateCommande->setCalendarPopup(true);
    editConsoMensuelle    = new QLineEdit(formCard);
    editConsoMensuelle->setPlaceholderText("ex: 8 m²");

    form->addRow("ID :", editId);
    form->addRow("Nom :", editNom);
    form->addRow("Type :", editType);
    form->addRow("Quantité en stock :", editQuantite);
    form->addRow("Prix unitaire :", editPrixUnitaire);
    form->addRow("Fournisseur :", editFournisseur);
    form->addRow("Seuil alerte :", editSeuilAlerte);
    form->addRow("Date dernière commande :", editDateCommande);
    form->addRow("Consommation mensuelle :", editConsoMensuelle);

    cardLayout->addLayout(form);

    // Boutons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(16);
    buttonLayout->setAlignment(Qt::AlignCenter);

    QPushButton *saveBtn = new QPushButton("Mettre à jour", formCard);
    saveBtn->setObjectName("saveButton");
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &StockPage::onSaveEditButtonClicked);

    QPushButton *backBtn = new QPushButton("← Retour à la liste", formCard);
    backBtn->setObjectName("backButton");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &StockPage::onBackToList);

    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(backBtn);
    cardLayout->addLayout(buttonLayout);
    cardLayout->addStretch();

    pageLayout->addWidget(formCard, 0, Qt::AlignCenter);
    pageLayout->addStretch();

    return page;
}

// ------------------------------------------------------------------
// Page 3 : View Material (read-only labels)
// ------------------------------------------------------------------
QWidget* StockPage::createViewPage()
{
    QWidget *page = new QWidget();
    page->setObjectName("viewMaterialPage");

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // ---------- Carte encadrée centrée, version "premium" ----------
    QFrame *formCard = new QFrame(page);
    formCard->setObjectName("formCard");
    formCard->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(40, 36, 40, 36);
    cardLayout->setSpacing(28);

    // --- En-tête avec icône et titre ---
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setSpacing(12);



    QLabel *title = new QLabel("Détails du matériau", formCard);
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);


    headerLayout->addWidget(title);
    cardLayout->addLayout(headerLayout);

    // --- Séparateur décoratif ---
    QFrame *separator = new QFrame(formCard);
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName("viewSeparator");
    separator->setStyleSheet("#viewSeparator { background-color: #E8E4DC; max-height: 1px; margin: 8px 0; }");
    cardLayout->addWidget(separator);

    // --- Formulaire en lecture seule (valeurs sous forme de "badges") ---
    QFormLayout *form = new QFormLayout();
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setSpacing(20);

    viewId              = new QLabel(formCard);
    viewNom             = new QLabel(formCard);
    viewType            = new QLabel(formCard);
    viewQuantite        = new QLabel(formCard);
    viewPrixUnitaire    = new QLabel(formCard);
    viewFournisseur     = new QLabel(formCard);
    viewSeuilAlerte     = new QLabel(formCard);
    viewDateCommande    = new QLabel(formCard);
    viewConsoMensuelle  = new QLabel(formCard);

    // Ajout d'un rendu "badge" via CSS (voir QSS plus bas)
    QList<QLabel*> allLabels = {viewId, viewNom, viewType, viewQuantite,
                                 viewPrixUnitaire, viewFournisseur, viewSeuilAlerte,
                                 viewDateCommande, viewConsoMensuelle};
    for (QLabel *lbl : allLabels) {
        lbl->setObjectName("viewValue");
        lbl->setTextFormat(Qt::RichText);  // permet d'intégrer des icônes si souhaité
    }

    form->addRow("ID :", viewId);
    form->addRow("Nom :", viewNom);
    form->addRow("Type :", viewType);
    form->addRow("Quantité en stock :", viewQuantite);
    form->addRow("Prix unitaire :", viewPrixUnitaire);
    form->addRow("Fournisseur :", viewFournisseur);
    form->addRow("Seuil alerte :", viewSeuilAlerte);
    form->addRow("Date dernière commande :", viewDateCommande);
    form->addRow("Consommation mensuelle :", viewConsoMensuelle);

    cardLayout->addLayout(form);

    // --- Bouton retour centré ---
    QPushButton *backBtn = new QPushButton("← Retour à la liste", formCard);
    backBtn->setObjectName("backButton");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet("min-width: 180px;"); // un peu plus large pour équilibrer
    connect(backBtn, &QPushButton::clicked, this, &StockPage::onBackToList);

    cardLayout->addWidget(backBtn, 0, Qt::AlignCenter);
    cardLayout->addStretch();

    pageLayout->addWidget(formCard, 0, Qt::AlignCenter);
    pageLayout->addStretch();

    return page;
}

// ------------------------------------------------------------------
// Helper : populate edit form from row data
// ------------------------------------------------------------------
void StockPage::populateEditForm(int row)
{
    if (!stockTable || row < 0 || row >= stockTable->rowCount())
        return;

    editId->setText(stockTable->item(row, 0)->text());
    editNom->setText(stockTable->item(row, 1)->text());
    editType->setText(stockTable->item(row, 2)->text());

    QString qty = stockTable->item(row, 3)->text();
    qty.remove(" m²").remove(" feuilles").remove(" unités").remove(" L").remove(" DT");
    editQuantite->setValue(qty.toInt());

    QString price = stockTable->item(row, 4)->text();
    price.remove(" DT");
    editPrixUnitaire->setValue(price.toDouble());

    editFournisseur->setText(stockTable->item(row, 5)->text());

    QString seuil = stockTable->item(row, 6)->text();
    editSeuilAlerte->setValue(seuil.toInt());

    QDate date = QDate::fromString(stockTable->item(row, 7)->text(), "dd/MM/yyyy");
    editDateCommande->setDate(date);

    editConsoMensuelle->setText(stockTable->item(row, 8)->text());
}

// ------------------------------------------------------------------
// Helper : populate view form from row data
// ------------------------------------------------------------------
void StockPage::populateViewForm(int row)
{
    if (!stockTable || row < 0 || row >= stockTable->rowCount())
        return;

    viewId->setText(stockTable->item(row, 0)->text());
    viewNom->setText(stockTable->item(row, 1)->text());
    viewType->setText(stockTable->item(row, 2)->text());
    viewQuantite->setText(stockTable->item(row, 3)->text());
    viewPrixUnitaire->setText(stockTable->item(row, 4)->text());
    viewFournisseur->setText(stockTable->item(row, 5)->text());
    viewSeuilAlerte->setText(stockTable->item(row, 6)->text());
    viewDateCommande->setText(stockTable->item(row, 7)->text());
    viewConsoMensuelle->setText(stockTable->item(row, 8)->text());
}

// ------------------------------------------------------------------
// Helper : clear add form
// ------------------------------------------------------------------
void StockPage::clearAddForm()
{
    addId->clear();
    addNom->clear();
    addType->clear();
    addQuantite->setValue(0);
    addPrixUnitaire->setValue(0.0);
    addFournisseur->clear();
    addSeuilAlerte->setValue(0);
    addDateCommande->setDate(QDate::currentDate());
    addConsoMensuelle->clear();
}

// ------------------------------------------------------------------
// Slots Implementation
// ------------------------------------------------------------------
void StockPage::onAddButtonClicked()
{
    clearAddForm();
    stackedWidget->setCurrentIndex(1); // Add page
}

void StockPage::onEditButtonClicked()
{
    QList<QTableWidgetItem*> selected = stockTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection",
                             "Veuillez sélectionner un matériau à modifier.");
        return;
    }
    currentRow = stockTable->currentRow();
    populateEditForm(currentRow);
    stackedWidget->setCurrentIndex(2); // Edit page
}

void StockPage::onDeleteButtonClicked()
{
    QList<QTableWidgetItem*> selected = stockTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection",
                             "Veuillez sélectionner un matériau à supprimer.");
        return;
    }

    int row = stockTable->currentRow();
    QString name = stockTable->item(row, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirmer la suppression",
        QString("Êtes-vous sûr de vouloir supprimer « %1 » ?").arg(name),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        stockTable->removeRow(row);
        // If table becomes empty, reset currentRow
        if (stockTable->rowCount() == 0)
            currentRow = -1;
    }
}

void StockPage::onViewTriggered()
{
    int row = stockTable->currentRow();
    if (row < 0) return;
    currentRow = row;
    populateViewForm(row);
    stackedWidget->setCurrentIndex(3); // View page
}

void StockPage::onBackToList()
{
    stackedWidget->setCurrentIndex(0); // List page
}

void StockPage::onSaveAddButtonClicked()
{
    // --- SIMPLE ADD (visual demonstration) ---
    int newRow = stockTable->rowCount();
    stockTable->insertRow(newRow);

    // Create items with formatted text
    QString qtyStr = QString::number(addQuantite->value()) + " m²"; // simplified
    QString priceStr = QString::number(addPrixUnitaire->value(), 'f', 2) + " DT";

    stockTable->setItem(newRow, 0, new QTableWidgetItem(addId->text()));
    stockTable->setItem(newRow, 1, new QTableWidgetItem(addNom->text()));
    stockTable->setItem(newRow, 2, new QTableWidgetItem(addType->text()));
    stockTable->setItem(newRow, 3, new QTableWidgetItem(qtyStr));
    stockTable->setItem(newRow, 4, new QTableWidgetItem(priceStr));
    stockTable->setItem(newRow, 5, new QTableWidgetItem(addFournisseur->text()));
    stockTable->setItem(newRow, 6, new QTableWidgetItem(QString::number(addSeuilAlerte->value())));
    stockTable->setItem(newRow, 7, new QTableWidgetItem(addDateCommande->date().toString("dd/MM/yyyy")));
    stockTable->setItem(newRow, 8, new QTableWidgetItem(addConsoMensuelle->text()));

    stockTable->setRowHeight(newRow, 50);

    // Return to list
    stackedWidget->setCurrentIndex(0);
}

void StockPage::onSaveEditButtonClicked()
{
    if (currentRow < 0 || currentRow >= stockTable->rowCount())
        return;

    // Update the selected row with edited values
    QString qtyStr = QString::number(editQuantite->value()) + " m²"; // simplified
    QString priceStr = QString::number(editPrixUnitaire->value(), 'f', 2) + " DT";

    stockTable->item(currentRow, 0)->setText(editId->text());
    stockTable->item(currentRow, 1)->setText(editNom->text());
    stockTable->item(currentRow, 2)->setText(editType->text());
    stockTable->item(currentRow, 3)->setText(qtyStr);
    stockTable->item(currentRow, 4)->setText(priceStr);
    stockTable->item(currentRow, 5)->setText(editFournisseur->text());
    stockTable->item(currentRow, 6)->setText(QString::number(editSeuilAlerte->value()));
    stockTable->item(currentRow, 7)->setText(editDateCommande->date().toString("dd/MM/yyyy"));
    stockTable->item(currentRow, 8)->setText(editConsoMensuelle->text());

    // Return to list
    stackedWidget->setCurrentIndex(0);
}
