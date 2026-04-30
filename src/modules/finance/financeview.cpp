#include "financeview.h"
#include "financemodel.h"
#include "transactiondialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>
#include <QDebug>
#include <QScrollArea>
#include <QPainter>
#include <QGridLayout>
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>
#include <QPushButton>
#include <QDir>
#ifdef QT_SERIALPORT_LIB
#include "../../core/session.h"
#endif

FinanceView::FinanceView(QWidget *parent)
    : QWidget(parent)
    , m_model(nullptr)
#ifdef QT_SERIALPORT_LIB
    , m_arduinoHandler(new ArduinoHandler(this))
    , m_rfidPromptBox(nullptr)
#endif
{
    setupUI();
    setupConnections();
}

FinanceView::~FinanceView()
{
}

void FinanceView::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(18);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Section filtres
    QFrame *filterFrame = new QFrame();
    filterFrame->setObjectName("searchFrame");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(0, 0, 0, 10);
    filterLayout->setSpacing(10);

    QLabel *searchLabel = new QLabel("Recherche");
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("ref");
    m_searchEdit->setMinimumWidth(200);

    QLabel *typeLabel = new QLabel("Type :");
    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({"Tous", "Facture", "Devis", "Acompte"});

    QLabel *categoryLabel = new QLabel("Catégorie :");
    m_categoryCombo = new QComboBox();
    m_categoryCombo->addItems({"Toutes", "Recette", "Depense"});

    m_filterButton = new QPushButton("Filtrer");
    m_resetButton = new QPushButton("Réinitialiser");

    filterLayout->addWidget(searchLabel);
    filterLayout->addWidget(m_searchEdit);
    filterLayout->addWidget(typeLabel);
    filterLayout->addWidget(m_typeCombo);
    filterLayout->addWidget(categoryLabel);
    filterLayout->addWidget(m_categoryCombo);
    filterLayout->addWidget(m_filterButton);
    filterLayout->addWidget(m_resetButton);
    filterLayout->addStretch();

    mainLayout->addWidget(filterFrame);

    // Table des transactions
    m_table = new QTableWidget();
    m_table->setObjectName("financeTable");
    m_table->setColumnCount(8);

    QStringList headers = {
        "RÉFÉRENCE", "TYPE", "MODE PAIEMENT", "STATUT",
        "CATÉGORIE", "MONTANT (DT)", "DATE", "PROJET"
    };
    m_table->setHorizontalHeaderLabels(headers);

    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_table, 1);

    // Barre d'actions
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);

    m_addButton = new QPushButton("+ Nouvelle Transaction");
    m_deleteButton = new QPushButton("Supprimer");
    m_exportButton = new QPushButton("Exporter Excel");
    m_statsButton = new QPushButton("Statistiques détaillées");
    m_archiveButton = new QPushButton("Voir les Archives");  // Nouveau bouton

    actionLayout->addWidget(m_addButton);
    actionLayout->addWidget(m_deleteButton);
    actionLayout->addWidget(m_exportButton);
    actionLayout->addWidget(m_statsButton);
    actionLayout->addWidget(m_archiveButton);
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);
}

void FinanceView::setupConnections()
{
    connect(m_addButton, &QPushButton::clicked, this, &FinanceView::onAddClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &FinanceView::onDeleteClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &FinanceView::onExportClicked);
    connect(m_statsButton, &QPushButton::clicked, this, &FinanceView::onStatsClicked);
    connect(m_archiveButton, &QPushButton::clicked, this, &FinanceView::onArchiveClicked);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &FinanceView::onCellDoubleClicked);
    connect(m_filterButton, &QPushButton::clicked, this, &FinanceView::onFilterChanged);
    connect(m_resetButton, &QPushButton::clicked, this, &FinanceView::onResetFilters);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &FinanceView::onFilterChanged);
    connect(m_typeCombo, &QComboBox::currentTextChanged, this, &FinanceView::onFilterChanged);
    connect(m_categoryCombo, &QComboBox::currentTextChanged, this, &FinanceView::onFilterChanged);

#ifdef QT_SERIALPORT_LIB
    // Arduino connections
    connect(m_arduinoHandler, &ArduinoHandler::rfidCardRead, this, &FinanceView::onRFIDCardRead);
    connect(m_arduinoHandler, &ArduinoHandler::errorOccurred, this, &FinanceView::onRFIDError);
#endif
}

void FinanceView::setModel(FinanceModel *model)
{
    m_model = model;
    connect(m_model, &FinanceModel::dataChanged, this, &FinanceView::onModelDataChanged);
    connect(m_model, &FinanceModel::errorOccurred, this, &FinanceView::onModelError);
    loadTransactions();
}

void FinanceView::loadTransactions()
{
    if (m_model) {
        m_currentTransactions = m_model->loadTransactions();
        refreshTable();
    }
}

void FinanceView::refreshTable()
{
    m_table->setRowCount(0);

    for (int i = 0; i < m_currentTransactions.size(); ++i) {
        const auto &t = m_currentTransactions[i];
        m_table->insertRow(i);

        m_table->setItem(i, 0, new QTableWidgetItem(t.reference));
        m_table->setItem(i, 1, new QTableWidgetItem(t.type));
        m_table->setItem(i, 2, new QTableWidgetItem(t.modePaiement));
        m_table->setItem(i, 3, new QTableWidgetItem(t.statut));
        m_table->setItem(i, 4, new QTableWidgetItem(t.categorie));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(t.montant, 'f', 3)));
        m_table->setItem(i, 6, new QTableWidgetItem(t.date));
        m_table->setItem(i, 7, new QTableWidgetItem(t.nomProjet));

        m_table->setRowHeight(i, 52);
    }
}

void FinanceView::applyFilters()
{
    QString search = m_searchEdit->text().trimmed();
    QString type = m_typeCombo->currentText();
    QString category = m_categoryCombo->currentText();

    for (int row = 0; row < m_table->rowCount(); row++) {
        bool visible = true;

        if (!search.isEmpty()) {
            QTableWidgetItem *refItem = m_table->item(row, 0);
            if (!refItem || !refItem->text().contains(search, Qt::CaseInsensitive))
                visible = false;
        }

        if (visible && type != "Tous") {
            QTableWidgetItem *typeItem = m_table->item(row, 1);
            if (!typeItem || typeItem->text() != type)
                visible = false;
        }

        if (visible && category != "Toutes") {
            QTableWidgetItem *catItem = m_table->item(row, 4);
            if (!catItem || catItem->text() != category)
                visible = false;
        }

        m_table->setRowHidden(row, !visible);
    }
}

void FinanceView::onEditClicked(int row)
{
    if (row < 0 || row >= m_currentTransactions.size()) {
        QMessageBox::warning(this, "Erreur", "Aucune transaction sélectionnée.");
        return;
    }

    const auto &transaction = m_currentTransactions[row];

    TransactionDialog dialog(this);
    dialog.setWindowTitle("Modifier Transaction");

    QMap<QString, QString> data;
    data["TYPE_TRAN"] = transaction.type;
    data["MODE_PAIEMENT"] = transaction.modePaiement;
    data["STATUT_TRAN"] = transaction.statut;
    data["CATEGORIE_TRAN"] = transaction.categorie;
    data["MONTANT_TRAN"] = QString::number(transaction.montant, 'f', 3);
    data["DATE_TRAN"] = transaction.date;
    data["CONTRAT_PROJET"] = transaction.contratProjet;

    dialog.setData(data);

    if (dialog.exec() == QDialog::Accepted) {
        if (m_model && m_model->updateTransaction(transaction.reference, dialog.getData())) {
            loadTransactions();
            QMessageBox::information(this, "Succès", "Transaction modifiée avec succès !");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible de modifier la transaction.");
        }
    }
}

void FinanceView::onDeleteClicked()
{
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Aucune sélection",
                             "Veuillez sélectionner une transaction à supprimer.");
        return;
    }

    QString reference = m_table->item(row, 0)->text();
    m_pendingDeleteReference = reference;

#ifdef QT_SERIALPORT_LIB
    qDebug() << "QT_SERIALPORT_LIB is defined, using RFID authentication";
    // Show non-blocking RFID waiting prompt
    showRFIDWaitingPrompt();
    // Request RFID read
    m_arduinoHandler->requestRFIDRead();
#else
    qDebug() << "QT_SERIALPORT_LIB is NOT defined, using fallback confirmation dialog";
    // Fallback: show confirmation dialog
    QString projet = m_table->item(row, 7)->text();
    QString message;
    if (projet != "Aucun projet") {
        message = QString("Cette transaction est liée au projet '%1'.\n\n"
                          "La supprimer pourrait affecter les données du projet.\n\n"
                          "Confirmer la suppression ?")
                      .arg(projet);
    } else {
        message = "Supprimer cette transaction ?\n\nCette action est irréversible.";
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Confirmer la suppression");
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Question);

    QPushButton *ouiButton = msgBox.addButton("Oui", QMessageBox::YesRole);
    QPushButton *nonButton = msgBox.addButton("Non", QMessageBox::NoRole);
    msgBox.setDefaultButton(nonButton);

    msgBox.exec();

    if (msgBox.clickedButton() == ouiButton) {
        if (m_model && m_model->deleteTransaction(reference)) {
            loadTransactions();
            QMessageBox::information(this, "Succès",
                                     "Transaction supprimée avec succès !");
        } else {
            QMessageBox::warning(this, "Erreur",
                                 "Impossible de supprimer la transaction.");
        }
    }
    m_pendingDeleteReference.clear();
#endif
}

void FinanceView::onArchiveClicked()
{
    if (!m_model) return;
    
    auto archives = m_model->loadArchivedTransactions();
    
    QDialog *archiveDialog = new QDialog(this);
    archiveDialog->setWindowTitle("Transactions Supprimees - Archive");
    archiveDialog->setMinimumSize(1000, 600);
    archiveDialog->setStyleSheet(R"(
        QDialog {
            background: #f7f5f0;
        }
        QTableWidget {
            background: white;
            alternate-background-color: #f9f9f9;
            border: 1px solid #ddd;
            border-radius: 5px;
        }
        QHeaderView::section {
            background: #4a6a4e;
            color: white;
            padding: 8px;
            border: none;
        }
    )");
    
    QVBoxLayout *layout = new QVBoxLayout(archiveDialog);
    
    QLabel *title = new QLabel("Historique des Transactions Supprimees");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e2f; margin: 10px;");
    layout->addWidget(title);
    
    QLabel *info = new QLabel(QString("Total des suppressions : %1").arg(archives.size()));
    info->setStyleSheet("color: #666; margin-left: 10px;");
    layout->addWidget(info);
    
    QTableWidget *table = new QTableWidget();
    table->setColumnCount(9);
    QStringList headers = {"REFERENCE", "TYPE", "MODE", "STATUT", "CATEGORIE", 
                          "MONTANT", "DATE", "PROJET", "DATE SUPPRESSION"};
    table->setHorizontalHeaderLabels(headers);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    table->setRowCount(archives.size());
    for (int i = 0; i < archives.size(); ++i) {
        const auto &arch = archives[i];
        table->setItem(i, 0, new QTableWidgetItem(arch.reference));
        table->setItem(i, 1, new QTableWidgetItem(arch.type));
        table->setItem(i, 2, new QTableWidgetItem(arch.modePaiement));
        table->setItem(i, 3, new QTableWidgetItem(arch.statut));
        
        QTableWidgetItem *categorieItem = new QTableWidgetItem(arch.categorie);
        if (arch.categorie == "Recette") {
            categorieItem->setForeground(QBrush(QColor("#28a745")));
        } else {
            categorieItem->setForeground(QBrush(QColor("#dc3545")));
        }
        table->setItem(i, 4, categorieItem);
        
        table->setItem(i, 5, new QTableWidgetItem(QString::number(arch.montant, 'f', 3) + " DT"));
        table->setItem(i, 6, new QTableWidgetItem(arch.date));
        table->setItem(i, 7, new QTableWidgetItem(arch.projet));
        table->setItem(i, 8, new QTableWidgetItem(arch.deletedAt.toString("dd/MM/yyyy hh:mm:ss")));
    }
    
    layout->addWidget(table);
    
    bool isIntegrityValid = m_model->verifyLogIntegrity();
    
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    
    QLabel *integrityLabel = new QLabel();
    if (isIntegrityValid) {
        integrityLabel->setText("Archive non modifiee");
        integrityLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    } else {
        integrityLabel->setText("ATTENTION : L'archive a ete modifiee !");
        integrityLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
    }
    bottomLayout->addWidget(integrityLabel);
    
    QPushButton *exportArchiveBtn = new QPushButton("Exporter l'archive");
    connect(exportArchiveBtn, &QPushButton::clicked, [this, archives]() {
        QString fileName = QFileDialog::getSaveFileName(this, 
            "Exporter l'archive",
            QString("archive_suppressions_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
            "CSV (*.csv)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                QTextStream out(&file);
                out.setEncoding(QStringConverter::Utf8);
                out << "Reference;Type;Mode;Statut;Categorie;Montant;Date;Projet;Date suppression\n";
                for (const auto &arch : archives) {
                    out << arch.reference << ";"
                        << arch.type << ";"
                        << arch.modePaiement << ";"
                        << arch.statut << ";"
                        << arch.categorie << ";"
                        << arch.montant << ";"
                        << arch.date << ";"
                        << arch.projet << ";"
                        << arch.deletedAt.toString("dd/MM/yyyy hh:mm:ss") << "\n";
                }
                file.close();
                QMessageBox::information(this, "Succes", "Archive exportee avec succes !");
            }
        }
    });
    
    bottomLayout->addWidget(exportArchiveBtn);
    
    QPushButton *closeBtn = new QPushButton("Fermer");
    connect(closeBtn, &QPushButton::clicked, archiveDialog, &QDialog::accept);
    bottomLayout->addWidget(closeBtn);
    
    layout->addLayout(bottomLayout);
    
    archiveDialog->exec();
    delete archiveDialog;
}
void FinanceView::onExportClicked()
{
    if (!m_model) return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Exporter le rapport financier",
                                                    QString("rapport_financier_%1.xls").arg(QDate::currentDate().toString("yyyyMMdd")),
                                                    "Fichiers Excel (*.xls);;Fichiers HTML (*.html);;Tous les fichiers (*.*)");

    if (fileName.isEmpty()) return;

    auto stats = m_model->getStatistics();
    auto transactions = m_model->loadTransactions();

    exportToExcelNative(fileName, stats, transactions);
}

void FinanceView::onStatsClicked()
{
    if (!m_model) return;

    const auto stats = m_model->getStatistics();
    
    // Extraction des données principales
    struct FinancialData {
        double recettes = 0.0;
        double depenses = 0.0;
        double total = 0.0;
        int transactions = 0;
        double solde = 0.0;
        QMap<QString, double> parStatut;
        QMap<QString, double> parMode;
        QMap<QString, double> parCategorie;
        QMap<QString, double> parMois;
    };
    
    FinancialData data{
        .recettes = stats.totalRecettes,
        .depenses = stats.totalDepenses,
        .total = stats.totalGeneral,
        .transactions = stats.totalTransactions,
        .solde = stats.totalRecettes - stats.totalDepenses,
        .parStatut = stats.statsByStatus,
        .parMode = stats.statsByMode,
        .parCategorie = stats.statsByCategorie,
        .parMois = stats.statsByMonth
    };
    
    // Création du dialogue
    QDialog dialogue(this);
    dialogue.setWindowTitle("📊 Tableau de Bord Financier");
    dialogue.setMinimumSize(1200, 850);
    
    // Configuration du style modernisé
    dialogue.setStyleSheet(R"(
        QDialog {
            background: #F8F7F4;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Helvetica Neue', sans-serif;
        }
        
        /* Header */
        QWidget#headerWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1E3A2E, stop:1 #2A4A3A);
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        
        /* Cards */
        QFrame.card {
            background: white;
            border-radius: 12px;
            border: 1px solid #E8E5DF;
        }
        
        QFrame.card:hover {
            border-color: #C5BFAF;
            transition: border-color 0.2s;
        }
        
        /* KPI Cards */
        QFrame.kpi-card {
            background: white;
            border-radius: 10px;
            border: 1px solid #EFECE5;
        }
        
        QFrame.kpi-card[type="recettes"] {
            border-top: 3px solid #4CAF7D;
        }
        
        QFrame.kpi-card[type="depenses"] {
            border-top: 3px solid #E57373;
        }
        
        QFrame.kpi-card[type="solde"] {
            border-top: 3px solid #FFB74D;
        }
        
        QFrame.kpi-card[type="transactions"] {
            border-top: 3px solid #64B5F6;
        }
        
        /* Scroll Area */
        QScrollArea {
            border: none;
            background: transparent;
        }
        
        QScrollBar:vertical {
            background: #F0EDE8;
            width: 6px;
            border-radius: 3px;
        }
        
        QScrollBar::handle:vertical {
            background: #C5BFAF;
            border-radius: 3px;
            min-height: 30px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: #A89F8A;
        }
        
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
        
        /* Buttons */
        QPushButton {
            background: #2A4A3A;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 32px;
            font-size: 13px;
            font-weight: 600;
            letter-spacing: 0.5px;
        }
        
        QPushButton:hover {
            background: #1E3A2E;
        }
        
        QPushButton:pressed {
            background: #152D24;
        }
        
        /* Labels */
        QLabel.section-title {
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 1px;
            color: #6B7F6A;
            text-transform: uppercase;
            margin-bottom: 8px;
        }
        
        /* Separators */
        QFrame.separator-light {
            background: #EFECE5;
            max-height: 1px;
        }
        
        QFrame.separator-dark {
            background: #3A5C4A;
            max-height: 1px;
        }
    )");
    
    auto mainLayout = new QVBoxLayout(&dialogue);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // ==================== HEADER ====================
    auto header = new QWidget();
    header->setObjectName("headerWidget");
    header->setFixedHeight(85);
    
    auto headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(35, 0, 35, 0);
    
    auto titleContainer = new QVBoxLayout();
    titleContainer->setSpacing(6);
    
    auto titleLabel = new QLabel("📈 Analyse Financière");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: white;");
    
    auto subtitleLabel = new QLabel(
        QString("%1 transactions · Période totale · %2 DT")
            .arg(data.transactions)
            .arg(data.total, 0, 'f', 2)
    );
    subtitleLabel->setStyleSheet("font-size: 12px; color: #A8C4A0;");
    
    titleContainer->addWidget(titleLabel);
    titleContainer->addWidget(subtitleLabel);
    
    headerLayout->addLayout(titleContainer);
    headerLayout->addStretch();
    
    mainLayout->addWidget(header);
    
    // ==================== BODY ====================
    auto body = new QWidget();
    auto bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(35, 25, 35, 25);
    bodyLayout->setSpacing(20);
    
    // --- KPI Cards ---
    auto kpiGrid = new QGridLayout();
    kpiGrid->setSpacing(15);
    
    struct KPI {
        QString label;
        QString valeur;
        QString type;
        QString icon;
    };
    
    QList<KPI> kpiData = {
        {"Revenus Totaux", QString::number(data.recettes, 'f', 2) + " DT", "recettes", "💰"},
        {"Dépenses Totales", QString::number(data.depenses, 'f', 2) + " DT", "depenses", "💸"},
        {"Solde Net", QString::number(data.solde, 'f', 2) + " DT", "solde", "⚖️"},
        {"Transactions", QString::number(data.transactions), "transactions", "🔄"}
    };
    
    for (int i = 0; i < kpiData.size(); ++i) {
        auto card = new QFrame();
        card->setProperty("type", kpiData[i].type);
        card->setProperty("class", "kpi-card");
        card->setFixedHeight(95);
        
        auto cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(18, 15, 18, 15);
        
        auto leftContainer = new QVBoxLayout();
        leftContainer->setSpacing(6);
        
        auto labelContainer = new QHBoxLayout();
        labelContainer->setSpacing(6);
        
        auto iconLabel = new QLabel(kpiData[i].icon);
        iconLabel->setStyleSheet("font-size: 18px;");
        
        auto textLabel = new QLabel(kpiData[i].label);
        textLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #8B8A86; letter-spacing: 0.5px;");
        
        labelContainer->addWidget(iconLabel);
        labelContainer->addWidget(textLabel);
        labelContainer->addStretch();
        
        auto valueLabel = new QLabel(kpiData[i].valeur);
        valueLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #2D3E2D;");
        
        leftContainer->addLayout(labelContainer);
        leftContainer->addWidget(valueLabel);
        
        cardLayout->addLayout(leftContainer);
        
        kpiGrid->addWidget(card, 0, i);
    }
    
    bodyLayout->addLayout(kpiGrid);
    
    // ==================== SCROLLABLE CHARTS ====================
    auto scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    auto chartContainer = new QWidget();
    auto chartLayout = new QVBoxLayout(chartContainer);
    chartLayout->setSpacing(25);
    chartLayout->setContentsMargins(0, 0, 8, 0);
    
    // Fonction utilitaire pour créer une section
    auto createChartSection = [&](const QString& titre, QWidget* contenu) -> QWidget* {
        auto section = new QWidget();
        auto sectionLayout = new QVBoxLayout(section);
        sectionLayout->setContentsMargins(0, 0, 0, 0);
        sectionLayout->setSpacing(10);
        
        auto titreLabel = new QLabel(titre);
        titreLabel->setProperty("class", "section-title");
        
        auto container = new QFrame();
        container->setProperty("class", "card");
        
        auto containerLayout = new QVBoxLayout(container);
        containerLayout->setContentsMargins(0, 0, 0, 0);
        containerLayout->addWidget(contenu);
        
        sectionLayout->addWidget(titreLabel);
        sectionLayout->addWidget(container);
        
        return section;
    };
    
    // Graphique en barres amélioré
    auto createBarChart = [](const QString& titre, const QMap<QString, double>& donnees,
                              const QList<QColor>& couleurs) -> QWidget* {
        if (donnees.isEmpty()) return nullptr;
        
        double total = 0.0;
        for (double v : donnees) total += v;
        
        double maxValeur = 0.0;
        for (double v : donnees) maxValeur = qMax(maxValeur, v);
        maxValeur = qMax(maxValeur, 1.0);
        
        auto widget = new QWidget();
        auto layout = new QVBoxLayout(widget);
        layout->setContentsMargins(25, 18, 25, 18);
        layout->setSpacing(12);
        
        QList<QColor> palette = couleurs.isEmpty() ? 
            QList<QColor>{QColor("#4CAF7D"), QColor("#E57373"), QColor("#FFB74D"), QColor("#64B5F6"), QColor("#BA68C8")} :
            couleurs;
        
        int indexCouleur = 0;
        bool premier = true;
        
        for (auto it = donnees.begin(); it != donnees.end(); ++it) {
            if (!premier) {
                auto separateur = new QFrame();
                separateur->setProperty("class", "separator-light");
                separateur->setFixedHeight(1);
                layout->addWidget(separateur);
            }
            premier = false;
            
            const QString& cle = it.key();
            double valeur = it.value();
            double pourcentage = (total > 0) ? (valeur / total * 100.0) : 0.0;
            int largeurBarre = qMax(3, static_cast<int>((valeur / maxValeur) * 300));
            QColor couleur = palette[indexCouleur % palette.size()];
            indexCouleur++;
            
            auto itemWidget = new QWidget();
            auto itemLayout = new QVBoxLayout(itemWidget);
            itemLayout->setSpacing(6);
            
            // Ligne information
            auto infoLayout = new QHBoxLayout();
            infoLayout->setSpacing(8);
            
            auto nomLabel = new QLabel(cle);
            nomLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #3A3A3A;");
            
            auto valeurLabel = new QLabel(QString::number(valeur, 'f', 2) + " DT");
            valeurLabel->setStyleSheet(QString("font-size: 13px; font-weight: 700; color: %1;").arg(couleur.name()));
            
            auto pourcentLabel = new QLabel(QString("• %1%").arg(pourcentage, 0, 'f', 1));
            pourcentLabel->setStyleSheet("font-size: 11px; color: #B0A89A;");
            
            infoLayout->addWidget(nomLabel);
            infoLayout->addStretch();
            infoLayout->addWidget(valeurLabel);
            infoLayout->addWidget(pourcentLabel);
            
            // Barre de progression
            auto barreContainer = new QFrame();
            barreContainer->setFixedHeight(8);
            barreContainer->setStyleSheet("background: #F0EDE8; border-radius: 4px;");
            
            auto barreFill = new QFrame(barreContainer);
            barreFill->setFixedHeight(8);
            barreFill->setFixedWidth(largeurBarre);
            barreFill->setStyleSheet(QString("background: %1; border-radius: 4px;").arg(couleur.name()));
            
            itemLayout->addLayout(infoLayout);
            itemLayout->addWidget(barreContainer);
            
            layout->addWidget(itemWidget);
        }
        
        return widget;
    };
    
    // Graphique circulaire amélioré (donut)
    class DonutChart : public QWidget {
    public:
        struct Segment {
            double valeur;
            QColor couleur;
            QString label;
        };
        
        QList<Segment> segments;
        double total = 0.0;
        
        void addSegment(double valeur, const QColor& couleur, const QString& label) {
            segments.append({valeur, couleur, label});
            total += valeur;
        }
        
    protected:
        void paintEvent(QPaintEvent*) override {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);
            
            if (total == 0.0) return;
            
            int margin = 15;
            QRectF rect(margin, margin, width() - 2*margin, height() - 2*margin);
            double angleDepart = -90.0;
            
            for (const auto& segment : segments) {
                double angleBalayage = (segment.valeur / total) * 360.0;
                painter.setBrush(segment.couleur);
                painter.setPen(Qt::NoPen);
                painter.drawPie(rect, static_cast<int>(angleDepart * 16),
                               static_cast<int>(angleBalayage * 16));
                angleDepart += angleBalayage;
            }
            
            // Trou central
            double trouRatio = 0.55;
            QRectF trou = rect.adjusted(rect.width() * trouRatio / 2,
                                       rect.height() * trouRatio / 2,
                                       -rect.width() * trouRatio / 2,
                                       -rect.height() * trouRatio / 2);
            painter.setBrush(QColor("white"));
            painter.drawEllipse(trou);
        }
    };
    
    // Section Répartition
    auto repartitionWidget = new QWidget();
    auto repartitionLayout = new QHBoxLayout(repartitionWidget);
    repartitionLayout->setContentsMargins(25, 20, 25, 20);
    repartitionLayout->setSpacing(40);
    
    auto donutChart = new DonutChart();
    donutChart->setFixedSize(200, 200);
    donutChart->addSegment(data.recettes, QColor("#4CAF7D"), "Recettes");
    donutChart->addSegment(data.depenses, QColor("#E57373"), "Dépenses");
    
    auto legendContainer = new QWidget();
    auto legendLayout = new QVBoxLayout(legendContainer);
    legendLayout->setSpacing(15);
    
    auto addLegendItem = [&](const QString& label, double valeur, const QColor& couleur) {
        double pourcentage = (data.recettes + data.depenses > 0) ?
            (valeur / (data.recettes + data.depenses)) * 100.0 : 0.0;
        
        auto item = new QWidget();
        auto layout = new QVBoxLayout(item);
        layout->setSpacing(5);
        
        auto topLine = new QHBoxLayout();
        topLine->setSpacing(10);
        
        auto dot = new QFrame();
        dot->setFixedSize(10, 10);
        dot->setStyleSheet(QString("background: %1; border-radius: 5px;").arg(couleur.name()));
        
        auto labelWidget = new QLabel(label);
        labelWidget->setStyleSheet("font-size: 13px; font-weight: 600; color: #3A3A3A;");
        
        auto valueWidget = new QLabel(QString::number(valeur, 'f', 2) + " DT");
        valueWidget->setStyleSheet("font-size: 13px; color: #8B8A86;");
        
        auto percentWidget = new QLabel(QString("%1%").arg(pourcentage, 0, 'f', 1));
        percentWidget->setStyleSheet(QString("font-size: 15px; font-weight: 700; color: %1;").arg(couleur.name()));
        
        topLine->addWidget(dot);
        topLine->addWidget(labelWidget);
        topLine->addStretch();
        topLine->addWidget(valueWidget);
        topLine->addSpacing(15);
        topLine->addWidget(percentWidget);
        
        auto progressBar = new QFrame();
        progressBar->setFixedHeight(6);
        progressBar->setStyleSheet("background: #F0EDE8; border-radius: 3px;");
        
        auto progressFill = new QFrame(progressBar);
        progressFill->setFixedHeight(6);
        progressFill->setFixedWidth(static_cast<int>(pourcentage * 2.2));
        progressFill->setStyleSheet(QString("background: %1; border-radius: 3px;").arg(couleur.name()));
        
        layout->addLayout(topLine);
        layout->addWidget(progressBar);
        
        legendLayout->addWidget(item);
    };
    
    addLegendItem("Revenus", data.recettes, QColor("#4CAF7D"));
    
    auto separator = new QFrame();
    separator->setProperty("class", "separator-light");
    separator->setFixedHeight(1);
    legendLayout->addWidget(separator);
    
    addLegendItem("Dépenses", data.depenses, QColor("#E57373"));
    
    // Information solde
    auto soldeCard = new QFrame();
    soldeCard->setStyleSheet(QString("background: %1; border-radius: 8px; margin-top: 10px;")
        .arg(data.solde >= 0 ? "#E8F5E9" : "#FFEBEE"));
    auto soldeLayout = new QHBoxLayout(soldeCard);
    soldeLayout->setContentsMargins(15, 10, 15, 10);
    
    auto soldeLabel = new QLabel("SOLDE NET");
    soldeLabel->setStyleSheet("font-size: 10px; font-weight: 700; color: #6B7F6A; letter-spacing: 1px;");
    
    auto soldeValue = new QLabel(QString::number(data.solde, 'f', 2) + " DT");
    soldeValue->setStyleSheet(QString("font-size: 16px; font-weight: 700; color: %1;")
        .arg(data.solde >= 0 ? "#4CAF7D" : "#E57373"));
    
    soldeLayout->addWidget(soldeLabel);
    soldeLayout->addStretch();
    soldeLayout->addWidget(soldeValue);
    
    legendLayout->addWidget(soldeCard);
    legendLayout->addStretch();
    
    repartitionLayout->addWidget(donutChart, 0, Qt::AlignCenter);
    repartitionLayout->addWidget(legendContainer, 1);
    
    chartLayout->addWidget(createChartSection("📊 RÉPARTITION REVENUS / DÉPENSES", repartitionWidget));
    
    // Ajouter les graphiques en barres
    if (!data.parStatut.isEmpty()) {
        if (auto chart = createBarChart("🏷️ MONTANT PAR STATUT", data.parStatut, {})) {
            chartLayout->addWidget(createChartSection("Statut", chart));
        }
    }
    
    if (!data.parMode.isEmpty()) {
        if (auto chart = createBarChart("💳 MONTANT PAR MODE DE PAIEMENT", data.parMode, {})) {
            chartLayout->addWidget(createChartSection("Mode de paiement", chart));
        }
    }
    
    if (!data.parCategorie.isEmpty()) {
        if (auto chart = createBarChart("📁 MONTANT PAR CATÉGORIE", data.parCategorie, {})) {
            chartLayout->addWidget(createChartSection("Catégorie", chart));
        }
    }
    
    if (!data.parMois.isEmpty()) {
        QMap<QString, double> moisTries;
        QStringList moisList = data.parMois.keys();
        std::sort(moisList.begin(), moisList.end(),
            [](const QString& a, const QString& b) {
                return QDate::fromString("01/" + a, "dd/MM/yyyy") <
                       QDate::fromString("01/" + b, "dd/MM/yyyy");
            });
        
        for (const QString& mois : moisList) {
            moisTries[mois] = data.parMois[mois];
        }
        
        if (auto chart = createBarChart("📅 ÉVOLUTION MENSUELLE", moisTries, {})) {
            chartLayout->addWidget(createChartSection("Mois", chart));
        }
    }
    
    // Section Résumé supplémentaire
    auto resumeWidget = new QWidget();
    auto resumeLayout = new QGridLayout(resumeWidget);
    resumeLayout->setSpacing(15);
    
    double moyenne = (data.transactions > 0) ? (data.total / data.transactions) : 0.0;
    double tauxRevenus = (data.total > 0) ? (data.recettes / data.total * 100.0) : 0.0;
    
    struct ResumeItem {
        QString label;
        QString valeur;
        QString icone;
    };
    
    QList<ResumeItem> resumeData = {
        {"Transaction moyenne", QString::number(moyenne, 'f', 2) + " DT", "📊"},
        {"Taux de revenus", QString::number(tauxRevenus, 'f', 1) + "%", "📈"},
        {"Taux de dépenses", QString::number(100.0 - tauxRevenus, 'f', 1) + "%", "📉"}
    };
    
    for (int i = 0; i < resumeData.size(); ++i) {
        auto itemCard = new QFrame();
        itemCard->setProperty("class", "card");
        
        auto itemLayout = new QVBoxLayout(itemCard);
        itemLayout->setContentsMargins(15, 12, 15, 12);
        itemLayout->setSpacing(6);
        
        auto headerLayout = new QHBoxLayout();
        auto iconLabel = new QLabel(resumeData[i].icone);
        iconLabel->setStyleSheet("font-size: 16px;");
        auto labelWidget = new QLabel(resumeData[i].label);
        labelWidget->setStyleSheet("font-size: 11px; font-weight: 600; color: #8B8A86; letter-spacing: 0.5px;");
        headerLayout->addWidget(iconLabel);
        headerLayout->addWidget(labelWidget);
        headerLayout->addStretch();
        
        auto valueWidget = new QLabel(resumeData[i].valeur);
        valueWidget->setStyleSheet("font-size: 18px; font-weight: 700; color: #2D3E2D;");
        
        itemLayout->addLayout(headerLayout);
        itemLayout->addWidget(valueWidget);
        
        resumeLayout->addWidget(itemCard, 0, i);
    }
    
    chartLayout->addWidget(createChartSection("📌 RÉSUMÉ STATISTIQUE", resumeWidget));
    chartLayout->addStretch();
    
    scrollArea->setWidget(chartContainer);
    bodyLayout->addWidget(scrollArea);
    
    mainLayout->addWidget(body);
    
    // ==================== FOOTER ====================
    auto footer = new QWidget();
    footer->setFixedHeight(75);
    footer->setStyleSheet("background: white; border-top: 1px solid #EFECE5;");
    
    auto footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(35, 0, 35, 0);
    footerLayout->addStretch();
    
    auto closeButton = new QPushButton("Fermer le tableau");
    closeButton->setCursor(Qt::PointingHandCursor);
    QObject::connect(closeButton, &QPushButton::clicked, &dialogue, &QDialog::accept);
    
    footerLayout->addWidget(closeButton);
    mainLayout->addWidget(footer);
    
    dialogue.exec();
}
void FinanceView::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    onEditClicked(row);
}

void FinanceView::onFilterChanged()
{
    applyFilters();
}

void FinanceView::onResetFilters()
{
    m_searchEdit->clear();
    m_typeCombo->setCurrentIndex(0);
    m_categoryCombo->setCurrentIndex(0);
    for (int row = 0; row < m_table->rowCount(); row++)
        m_table->setRowHidden(row, false);
}

void FinanceView::onModelDataChanged()
{
    loadTransactions();
}

void FinanceView::onModelError(const QString &error)
{
    QMessageBox::warning(this, "Erreur Base de données", error);
}

void FinanceView::onAddClicked()
{
    TransactionDialog dialog(this);
    dialog.setWindowTitle("Nouvelle Transaction");

    if (dialog.exec() == QDialog::Accepted) {
        if (m_model && m_model->insertTransaction(dialog.getData())) {
            loadTransactions();
            QMessageBox::information(this, "Succès", "Transaction ajoutée avec succès !");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible d'ajouter la transaction.");
        }
    }
}

void FinanceView::exportToExcelNative(const QString &fileName,
                                      const FinanceModel::FinanceStats &stats,
                                      const QList<FinanceModel::Transaction> &transactions)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Erreur", "Impossible de créer le fichier.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    double soldeNet = stats.totalRecettes - stats.totalDepenses;
    double montantMoyen = stats.totalGeneral / qMax(1, stats.totalTransactions);

    out << "<html>\n"
        << "<head>\n"
        << "<meta charset=\"UTF-8\">\n"
        << "<title>Rapport Financier</title>\n"
        << "<style>\n"
        << "body { font-family: 'Segoe UI', Arial, sans-serif; margin: 20px; }\n"
        << "h1 { color: #2c3e2f; border-bottom: 2px solid #4a6a4e; padding-bottom: 10px; }\n"
        << "h2 { color: #4a6a4e; margin-top: 25px; }\n"
        << ".kpi-table { width: 100%; border-collapse: collapse; margin: 20px 0; background: #f8f9fa; }\n"
        << ".kpi-table td { padding: 15px; border: 1px solid #dee2e6; }\n"
        << ".kpi-label { font-weight: bold; background: #e9ecef; width: 200px; }\n"
        << ".kpi-value { font-size: 18px; font-weight: bold; }\n"
        << ".recette { color: #28a745; }\n"
        << ".depense { color: #dc3545; }\n"
        << "table { width: 100%; border-collapse: collapse; margin: 20px 0; }\n"
        << "th { background: #4a6a4e; color: white; padding: 10px; border: 1px solid #ddd; }\n"
        << "td { padding: 8px; border: 1px solid #ddd; }\n"
        << "tr:nth-child(even) { background: #f2f2f2; }\n"
        << ".stat-box { margin: 15px 0; padding: 10px; background: #f8f9fa; border-left: 4px solid #4a6a4e; }\n"
        << "footer { margin-top: 30px; padding-top: 10px; border-top: 1px solid #ddd; text-align: center; color: #666; }\n"
        << "</style>\n"
        << "</head>\n"
        << "<body>\n"
        << "<h1>📊 RAPPORT FINANCIER</h1>\n"
        << "<p>Généré le : " << QDate::currentDate().toString("dd/MM/yyyy") << "</p>\n\n"

        << "<h2>INDICATEURS CLÉS</h2>\n"
        << "<table class=\"kpi-table\">\n"
        << " <tr><td class=\"kpi-label\">Total Recettes</td><td class=\"kpi-value recette\">" << QString::number(stats.totalRecettes, 'f', 3) << " DT</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Total Dépenses</td><td class=\"kpi-value depense\">" << QString::number(stats.totalDepenses, 'f', 3) << " DT</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Solde Net</td><td class=\"kpi-value " << (soldeNet >= 0 ? "recette" : "depense") << "\">" << QString::number(soldeNet, 'f', 3) << " DT</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Nombre de Transactions</td><td class=\"kpi-value\">" << stats.totalTransactions << "</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Montant Moyen par Transaction</td><td class=\"kpi-value\">" << QString::number(montantMoyen, 'f', 3) << " DT</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Total Général</td><td class=\"kpi-value\">" << QString::number(stats.totalGeneral, 'f', 3) << " DT</td></tr>\n"
        << "</table>\n\n"

        << "<h2>📋 DÉTAIL DES TRANSACTIONS</h2>\n"
        << "<table>\n"
        << " <thead>\n"
        << "   <tr>\n"
        << "   <th>RÉFÉRENCE</th><th>TYPE</th><th>MODE PAIEMENT</th><th>STATUT</th><th>CATÉGORIE</th><th>MONTANT (DT)</th><th>DATE</th><th>PROJET</th>\n"
        << "   </tr>\n"
        << " </thead>\n"
        << " <tbody>\n";

    for (const auto &t : transactions) {
        QString rowClass = (t.categorie == "Recette") ? "recette" : "depense";
        out << "   <tr>\n"
            << "    <td>" << t.reference << "</td>\n"
            << "    <td>" << t.type << "</td>\n"
            << "    <td>" << t.modePaiement << "</td>\n"
            << "    <td>" << t.statut << "</td>\n"
            << "    <td>" << t.categorie << "</td>\n"
            << "   <td class=\"" << rowClass << "\">" << QString::number(t.montant, 'f', 3) << "</td>\n"
            << "    <td>" << t.date << "</td>\n"
            << "    <td>" << t.nomProjet << "</td>\n"
            << "   </tr>\n";
    }

    out << " </tbody>\n"
        << "</table>\n\n"

        << "<h2>📈 STATISTIQUES DÉTAILLÉES</h2>\n"

        << "<div class=\"stat-box\">\n"
        << "<h3>Par Statut</h3>\n"
        << "<table>\n"
        << "  <tr><th>Statut</th><th>Montant (DT)</th><th>Pourcentage</th></tr>\n";

    for (auto it = stats.statsByStatus.begin(); it != stats.statsByStatus.end(); ++it) {
        double pourcentage = stats.totalGeneral > 0 ? (it.value() / stats.totalGeneral) * 100 : 0;
        out << "  <tr><td>" << it.key() << "</td><td>" << QString::number(it.value(), 'f', 3) << "</td><td>" << QString::number(pourcentage, 'f', 1) << "%</td></tr>\n";
    }

    out << "</table>\n</div>\n\n"

        << "<div class=\"stat-box\">\n"
        << "<h3>Par Mode de Paiement</h3>\n"
        << "<table>\n"
        << "  <tr><th>Mode</th><th>Montant (DT)</th><th>Pourcentage</th></tr>\n";

    for (auto it = stats.statsByMode.begin(); it != stats.statsByMode.end(); ++it) {
        double pourcentage = stats.totalGeneral > 0 ? (it.value() / stats.totalGeneral) * 100 : 0;
        out << "  <tr><td>" << it.key() << "</td><td>" << QString::number(it.value(), 'f', 3) << "</td><td>" << QString::number(pourcentage, 'f', 1) << "%</td></tr>\n";
    }

    out << "</table>\n</div>\n\n"

        << "<div class=\"stat-box\">\n"
        << "<h3>Par Catégorie</h3>\n"
        << "<table>\n"
        << "  <tr><th>Catégorie</th><th>Montant (DT)</th><th>Pourcentage</th></tr>\n";

    for (auto it = stats.statsByCategorie.begin(); it != stats.statsByCategorie.end(); ++it) {
        double pourcentage = stats.totalGeneral > 0 ? (it.value() / stats.totalGeneral) * 100 : 0;
        out << "  <tr><td>" << it.key() << "</td><td>" << QString::number(it.value(), 'f', 3) << "</td><td>" << QString::number(pourcentage, 'f', 1) << "%</td></tr>\n";
    }

    out << "</table>\n</div>\n\n"

        << "<div class=\"stat-box\">\n"
        << "<h3>Évolution Mensuelle</h3>\n"
        << "<table>\n"
        << "  <tr><th>Mois</th><th>Montant (DT)</th></tr>\n";

    QStringList moisTries = stats.statsByMonth.keys();
    std::sort(moisTries.begin(), moisTries.end(), [](const QString &a, const QString &b) {
        return QDate::fromString("01/" + a, "dd/MM/yyyy") < QDate::fromString("01/" + b, "dd/MM/yyyy");
    });

    for (const QString &mois : moisTries) {
        out << "  <tr><td>" << mois << "</td><td>" << QString::number(stats.statsByMonth[mois], 'f', 3) << " DT</td></tr>\n";
    }

    out << "</table>\n</div>\n\n"

        << "<h2>📝 RÉSUMÉ</h2>\n"
        << "<table>\n"
        << "  <tr><td width=\"300\"><strong>Période</strong></td><td>Toutes les transactions</td></tr>\n"
        << "  <tr><td><strong>Performance</strong></td><td>" << (soldeNet >= 0 ? "📈 Bénéficiaire" : "📉 Déficitaire") << "</td></tr>\n"
        << "  <tr><td><strong>Ratio Recettes/Dépenses</strong></td><td>"
        << (stats.totalDepenses > 0 ? QString::number((stats.totalRecettes / stats.totalDepenses) * 100, 'f', 1) : "N/A")
        << "%</td></tr>\n"
        << "</table>\n\n"

        << "<footer>\n"
        << "  <p>Document généré automatiquement par le Système de Gestion Financière</p>\n"
        << "  <p>© " << QDate::currentDate().toString("yyyy") << " - Tous droits réservés</p>\n"
        << "</footer>\n"

        << "</body>\n"
        << "</html>";

    file.close();

    QString excelFileName = fileName;
    if (!excelFileName.endsWith(".xls", Qt::CaseInsensitive)) {
        excelFileName = fileName.left(fileName.lastIndexOf('.')) + ".xls";
        QFile::rename(fileName, excelFileName);
    }

    QMessageBox::information(this, "Export réussi",
                             QString("✅ Le rapport financier a été généré avec succès !\n\n"
                                     "📁 Fichier : %1\n\n"
                                     "📊 Le rapport contient :\n"
                                     "• Indicateurs clés (KPIs)\n"
                                     "• Détail de toutes les transactions\n"
                                     "• Statistiques par statut, mode, catégorie\n"
                                     "• Évolution mensuelle\n"
                                     "• Résumé financier\n\n"
                                     "💡 Ouvrez le fichier avec Excel pour une meilleure expérience.")
                                 .arg(excelFileName));
}

#ifdef QT_SERIALPORT_LIB
void FinanceView::closeRFIDPrompt()
{
#ifdef QT_SERIALPORT_LIB
    if (m_rfidPromptBox) {
        m_rfidPromptBox->close();
        m_rfidPromptBox->deleteLater();
        m_rfidPromptBox = nullptr;
    }
#endif
}

void FinanceView::showRFIDWaitingPrompt()
{
#ifdef QT_SERIALPORT_LIB
    if (m_rfidPromptBox) {
        m_rfidPromptBox->close();
        m_rfidPromptBox->deleteLater();
    }
    m_rfidPromptBox = new QMessageBox(QMessageBox::Information,
                                     "Authentification RFID",
                                     "En attente du scan de la carte RFID...\n\n"
                                     "Veuillez présenter votre carte.",
                                     QMessageBox::NoButton,
                                     this);
    m_rfidPromptBox->setModal(false);
    m_rfidPromptBox->show();
#endif
}

#ifdef QT_SERIALPORT_LIB
void FinanceView::onRFIDCardRead(const QString &uid)
{
    qDebug() << "RFID card read:" << uid;
    closeRFIDPrompt();

    // Verify RFID card access
    if (!m_model) {
        QMessageBox::warning(this, "Erreur", "Modèle non initialisé.");
        m_pendingDeleteReference.clear();
        return;
    }

    auto verificationResult = m_model->verifyRFIDCardAccess(uid);
    
    // Get current employee
    Employee* currentEmployee = Session::getCurrentEmployee();
    if (!currentEmployee) {
        QMessageBox::warning(this, "Erreur d'authentification",
                             "Aucun employé connecté.");
        m_pendingDeleteReference.clear();
        return;
    }
    
    // Check employee has "Chef Equipe" role
    QString employeeRole = currentEmployee->getPoste();
    qDebug() << "Current employee role:" << employeeRole;
    
    if (!verificationResult.isValid) {
        // Card not found or error
        QMessageBox::warning(this, "Carte RFID refusée",
                             verificationResult.errorMessage);
        m_pendingDeleteReference.clear();
        return;
    }
    
    // Verification result contains the card's role
    QString cardRole = verificationResult.errorMessage;
    qDebug() << "Card role:" << cardRole;
    
    // Check: Employee role must be "Chef Equipe"
    if (employeeRole != "Chef Equipe") {
        QMessageBox::warning(this, "Erreur d'autorisation",
                             QString("Incompatibilité de rôle :\n"
                                     "Rôle employé: %1\n"
                                     "Rôle requis: Chef Equipe\n\n"
                                     "Vous n'avez pas les droits nécessaires pour cette opération.")
                             .arg(employeeRole));
        m_pendingDeleteReference.clear();
        return;
    }
    
    // Check: Card role must match (already passed by existence check)
    // Double verify card code matches the one we're checking
    // This is implicit since we found the card with the exact RFID_CODE
    
    // All checks passed - proceed with deletion
    qDebug() << "RFID card authorized, proceeding with deletion";
    if (!m_pendingDeleteReference.isEmpty()) {
        if (m_model && m_model->deleteTransaction(m_pendingDeleteReference)) {
            loadTransactions();
            QMessageBox::information(this, "Succès",
                                     "Transaction supprimée avec succès !");
        } else {
            QMessageBox::warning(this, "Erreur",
                                 "Impossible de supprimer la transaction.");
        }
        m_pendingDeleteReference.clear();
    }
}

void FinanceView::onRFIDError(const QString &error)
{
    qDebug() << "RFID error:" << error;
    closeRFIDPrompt();
    QMessageBox::warning(this, "Erreur RFID",
                         QString("Erreur lors de la lecture de la carte RFID :\n\n%1").arg(error));
    m_pendingDeleteReference.clear();
}
#endif

#endif