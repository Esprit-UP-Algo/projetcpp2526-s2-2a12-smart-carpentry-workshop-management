#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QFrame>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadStyleSheet();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    this->setObjectName("window");
    this->setWindowTitle("Gestion Financière - Dashboard");
    this->resize(1400, 900);

    // Main Layout (Horizontal: Sidebar | Content)
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ===== SIDEBAR =====
    QFrame* sideBar = new QFrame;
    sideBar->setObjectName("sideBar");
    sideBar->setFixedWidth(260);
    
    QVBoxLayout* sideLayout = new QVBoxLayout(sideBar);
    sideLayout->setContentsMargins(20, 30, 20, 20);
    sideLayout->setSpacing(20);

    // Brand
    QLabel* brandLabel = new QLabel;
    brandLabel->setObjectName("brandLabel");
    // Try loading logo from multiple locations
    QString appDir = QCoreApplication::applicationDirPath();
    QString logoPath;
    
    // Try: same directory as executable (build/)
    if (QFile::exists(appDir + "/logo1.png")) {
        logoPath = appDir + "/logo1.png";
    }
    // Try: source directory (one level up from build/)
    else if (QFile::exists(appDir + "/../logo1.png")) {
        logoPath = appDir + "/../logo1.png";
    }
    // Try: project root (two levels up from build/)
    else if (QFile::exists(appDir + "/../../logo1.png")) {
        logoPath = appDir + "/../../logo1.png";
    }
    
    QPixmap logoPix(logoPath);
    if (!logoPix.isNull()) {
        // Bigger logo with smooth scaling
        QPixmap scaled = logoPix.scaled(360, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        brandLabel->setPixmap(scaled);
        brandLabel->setFixedHeight(scaled.height() + 16);
    } else {
        brandLabel->setText("Finance");
    }
    brandLabel->setAlignment(Qt::AlignCenter);
    brandLabel->setStyleSheet("background: transparent; padding: 8px 12px;");
    sideLayout->addWidget(brandLabel);

    // Nav
    sideLayout->addSpacing(20);
    QStringList navItems = {"Gestion Financière", "Gestion des Projets", "Gestion des Employés", "Gestion des Stocks "};
    for(const QString& item : navItems) {
        QPushButton* btn = new QPushButton(item);
        btn->setCheckable(true);
        if(item == "Tableau de bord") btn->setChecked(true);
        btn->setProperty("class", "navItem");
        sideLayout->addWidget(btn);
    }
    
    sideLayout->addStretch();

    // User Profile
    QFrame* profileFrame = new QFrame;
    profileFrame->setObjectName("profileFrame");
    QVBoxLayout* profileLayout = new QVBoxLayout(profileFrame);
    profileLayout->setContentsMargins(12, 14, 12, 14);
    profileLayout->setSpacing(6);
    
    // Profile Avatar (Circle Placeholder)
    QLabel* avatarLabel = new QLabel("A");
    avatarLabel->setStyleSheet("font-size: 40px; text-align: center; font-weight: 700; color: #4D362D; background-color: transparent; border: none;");
    avatarLabel->setAlignment(Qt::AlignCenter);
    
    // User Info
    QLabel* userNameLabel = new QLabel("Amine User");
    userNameLabel->setStyleSheet("font-weight: 700; font-size: 14px; color: #4D362D; background: transparent;");
    userNameLabel->setAlignment(Qt::AlignCenter);
    
    QLabel* userRoleLabel = new QLabel("Administrateur");
    userRoleLabel->setStyleSheet("font-weight: 500; font-size: 12px; color: #777777; background: transparent;");
    userRoleLabel->setAlignment(Qt::AlignCenter);
    
    // Logout Button
    QPushButton* logoutBtn = new QPushButton("Se déconnecter");
    logoutBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #FFFFFF;"
        "    border: 1px solid #E5E1DC;"
        "    color: #4D362D;"
        "    border-radius: 8px;"
        "    padding: 8px 12px;"
        "    font-weight: 600;"
        "    font-size: 12px;"
        "    transition: all 0.18s ease;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F7F6F4;"
        "    border-color: #C29B6D;"
        "}"
    );
    
    profileLayout->addWidget(avatarLabel);
    profileLayout->addWidget(userNameLabel);
    profileLayout->addWidget(userRoleLabel);
    profileLayout->addSpacing(8);
    profileLayout->addWidget(logoutBtn);
    sideLayout->addWidget(profileFrame);

    // ===== MAIN CONTENT AREA =====
    // We wrap main content in a scroll area just in case
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget* contentWidget = new QWidget;
    contentWidget->setObjectName("contentWidget");
    
    // Vertical layout for the main content
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(40, 40, 40, 40); // Increased margins
    contentLayout->setSpacing(30); // Increased spacing

    // Build the sections
    setupHeader(contentLayout);
    setupSummarySection(contentLayout);
    setupChartsSection(contentLayout); // Includes Quick Actions and Charts
    setupFormSection(contentLayout);
    setupTableSection(contentLayout);

    scrollArea->setWidget(contentWidget);

    // Add to split layout
    mainLayout->addWidget(sideBar);
    mainLayout->addWidget(scrollArea);
}

void MainWindow::setupHeader(QVBoxLayout* layout)
{
    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->setAlignment(Qt::AlignVCenter);
    
    QLabel* title = new QLabel("Tableau de bord");
    title->setObjectName("pageTitle");
    
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    
    // Search Box
    QLineEdit* search = new QLineEdit;
    search->setPlaceholderText("Rechercher...");
    search->setFixedWidth(280);
    search->setObjectName("searchBox");
    search->setMaximumHeight(40);
    
    QPushButton* bellBtn = new QPushButton("🔔");
    bellBtn->setFixedWidth(44);
    bellBtn->setFixedHeight(44);
    bellBtn->setProperty("class", "iconBtn");
    
    headerLayout->addWidget(search);
    headerLayout->addSpacing(12);
    headerLayout->addWidget(bellBtn);
    
    layout->addLayout(headerLayout);
    layout->addSpacing(10);
}

void MainWindow::setupSummarySection(QVBoxLayout* layout)
{
    QHBoxLayout* cardsLayout = new QHBoxLayout;
    cardsLayout->setSpacing(20);

    struct CardData { QString title; QString value; QString type; };
    QList<CardData> data = {
        {"Total Recettes", "12 450,00 €", "income"},
        {"Total Dépenses", "4 230,00 €", "expense"},
        {"Bénéfice Net", "+8 220,00 €", "profit"}
    };

    for(const auto& item : data) {
        QFrame* card = new QFrame;
        card->setProperty("class", "card summaryCard");
        card->setProperty("type", item.type); // To style via QSS
        
        QVBoxLayout* vLayout = new QVBoxLayout(card);
        QLabel* lblTitle = new QLabel(item.title);
        lblTitle->setProperty("class", "cardTitle");
        
        QLabel* lblValue = new QLabel(item.value);
        lblValue->setProperty("class", "cardValue");
        
        vLayout->addWidget(lblTitle);
        vLayout->addWidget(lblValue);
        
        cardsLayout->addWidget(card);
    }
    
    layout->addLayout(cardsLayout);
}

void MainWindow::setupChartsSection(QVBoxLayout* layout)
{
    QHBoxLayout* gridLayout = new QHBoxLayout;
    gridLayout->setSpacing(20);

    // 1. Quick Actions
    QFrame* actionsCard = new QFrame;
    actionsCard->setProperty("class", "card");
    QVBoxLayout* actLayout = new QVBoxLayout(actionsCard);
    
    QLabel* actTitle = new QLabel("Actions Rapides");
    actTitle->setProperty("class", "cardHeader");
    actLayout->addWidget(actTitle);
    
    QStringList actions = {"Exporter Factures", "Voir Statistiques", "Générer Rapport"};
    for(const QString& txt : actions) {
        QPushButton* btn = new QPushButton(txt);
        btn->setProperty("class", "actionBtn");
        actLayout->addWidget(btn);
    }
    actLayout->addStretch();
    
    // 2. Mockup Bar Chart
    QFrame* chartCard = new QFrame;
    chartCard->setProperty("class", "card");
    QVBoxLayout* chartLayout = new QVBoxLayout(chartCard);
    chartLayout->setSpacing(16);
    
    QLabel* chartTitle = new QLabel("Aperçu Financier");
    chartTitle->setProperty("class", "cardHeader");
    chartLayout->addWidget(chartTitle);

    // Chart Container with better layout
    QVBoxLayout* chartContainerLayout = new QVBoxLayout;
    
    // Chart Area
    QFrame* chartAreaFrame = new QFrame;
    chartAreaFrame->setStyleSheet("background-color: #FAFAF8; border-radius: 12px;");
    chartAreaFrame->setMinimumHeight(240);
    QVBoxLayout* chartAreaLayout = new QVBoxLayout(chartAreaFrame);
    chartAreaLayout->setContentsMargins(20, 20, 20, 20);
    
    // Bars with spacing and alignment
    QHBoxLayout* barsLayout = new QHBoxLayout;
    barsLayout->setAlignment(Qt::AlignBottom | Qt::AlignCenter);
    barsLayout->setSpacing(12);
    
    struct BarData {
        int height;
        QString label;
        bool isIncome;
    };
    
    QList<BarData> bars = {
        {60, "Sem 1", true},
        {40, "Sem 2", false},
        {75, "Sem 3", true},
        {55, "Sem 4", false},
        {90, "Sem 5", true},
        {30, "Sem 6", false},
        {80, "Sem 7", true}
    };
    
    for(const auto& bar : bars) {
        // Container for bar and label
        QVBoxLayout* barContainer = new QVBoxLayout;
        barContainer->setSpacing(8);
        barContainer->setAlignment(Qt::AlignCenter | Qt::AlignBottom);
        
        // Spacer at top
        barContainer->addStretch();
        
        // Bar
        QFrame* barFrame = new QFrame;
        barFrame->setFixedWidth(28);
        barFrame->setFixedHeight(bar.height);
        barFrame->setProperty("class", bar.isIncome ? "barIncome" : "barExpense");
        barFrame->setStyleSheet(
            QString("QFrame { "
                   "background-color: %1; "
                   "border-radius: 6px; "
                   "box-shadow: 0 -4px 8px %2; "
                   "}")
            .arg(bar.isIncome ? "#8A9A5B" : "#E57373",
                 bar.isIncome ? "rgba(138, 154, 91, 0.2)" : "rgba(229, 115, 115, 0.2)")
        );
        barContainer->addWidget(barFrame, 0, Qt::AlignCenter);
        
        // Label
        QLabel* barLabel = new QLabel(bar.label);
        barLabel->setStyleSheet("font-weight: 600; font-size: 11px; color: #777777;");
        barLabel->setAlignment(Qt::AlignCenter);
        barContainer->addWidget(barLabel);
        
        barsLayout->addLayout(barContainer);
    }
    
    chartAreaLayout->addLayout(barsLayout);
    chartContainerLayout->addWidget(chartAreaFrame);
    
    // Legend
    QHBoxLayout* legendLayout = new QHBoxLayout;
    legendLayout->addSpacing(20);
    
    QLabel* incomeLabel = new QLabel("● Recettes");
    incomeLabel->setStyleSheet("font-size: 12px; color: #8A9A5B; font-weight: 600;");
    
    QLabel* expenseLabel = new QLabel("● Dépenses");
    expenseLabel->setStyleSheet("font-size: 12px; color: #E57373; font-weight: 600;");
    
    legendLayout->addWidget(incomeLabel);
    legendLayout->addSpacing(20);
    legendLayout->addWidget(expenseLabel);
    legendLayout->addStretch();
    
    chartContainerLayout->addLayout(legendLayout);
    chartLayout->addLayout(chartContainerLayout);

    // Add to grid
    gridLayout->addWidget(actionsCard, 1);
    gridLayout->addWidget(chartCard, 2);
    
    layout->addLayout(gridLayout);
}

void MainWindow::setupFormSection(QVBoxLayout* layout)
{
    QFrame* formCard = new QFrame;
    formCard->setProperty("class", "card transactionForm");
    
    QVBoxLayout* formLayout = new QVBoxLayout(formCard);
    formLayout->setSpacing(16);
    
    QLabel* formTitle = new QLabel("Ajouter une transaction");
    formTitle->setProperty("class", "cardHeader");
    formLayout->addWidget(formTitle);
    
    // Form Grid
    QHBoxLayout* row1 = new QHBoxLayout;
    row1->setSpacing(20);
    
    QLabel* labelId = new QLabel("ID");
    labelId->setStyleSheet("font-weight: 600; color: #4D362D; font-size: 13px;");
    QLineEdit* idEdit = new QLineEdit("#TRX-8842");
    idEdit->setReadOnly(true);
    
    QLabel* labelAmount = new QLabel("Montant");
    labelAmount->setStyleSheet("font-weight: 600; color: #4D362D; font-size: 13px;");
    QLineEdit* amountEdit = new QLineEdit;
    amountEdit->setPlaceholderText("Montant €");
    
    QLabel* labelDate = new QLabel("Date");
    labelDate->setStyleSheet("font-weight: 600; color: #4D362D; font-size: 13px;");
    QDateEdit* dateEdit = new QDateEdit(QDate::currentDate());
    
    row1->addWidget(labelId); 
    row1->addWidget(idEdit);
    row1->addWidget(labelAmount);
    row1->addWidget(amountEdit);
    row1->addWidget(labelDate);
    row1->addWidget(dateEdit);

    QHBoxLayout* row2 = new QHBoxLayout;
    row2->setSpacing(20);
    
    QLabel* labelType = new QLabel("Type");
    labelType->setStyleSheet("font-weight: 600; color: #4D362D; font-size: 13px;");
    QComboBox* typeCombo = new QComboBox;
    typeCombo->addItems({"Facture", "Devis", "Acompte"});
    
    QLabel* labelMode = new QLabel("Mode");
    labelMode->setStyleSheet("font-weight: 600; color: #4D362D; font-size: 13px;");
    QComboBox* modeCombo = new QComboBox;
    modeCombo->addItems({"Virement", "Espèces", "Carte", "Chèque"});
    
    QLabel* labelCategory = new QLabel("Catégorie");
    labelCategory->setStyleSheet("font-weight: 600; color: #4D362D; font-size: 13px;");
    QComboBox* catCombo = new QComboBox;
    catCombo->addItems({"Recette", "Dépense"});
    
    row2->addWidget(labelType);
    row2->addWidget(typeCombo);
    row2->addWidget(labelMode);
    row2->addWidget(modeCombo);
    row2->addWidget(labelCategory);
    row2->addWidget(catCombo);
    
    QLineEdit* descEdit = new QLineEdit;
    descEdit->setPlaceholderText("Description...");
    descEdit->setMinimumHeight(44);
    
    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(12);
    btnLayout->addStretch();
    QPushButton* resetBtn = new QPushButton("Réinitialiser");
    QPushButton* addBtn = new QPushButton("Ajouter");
    addBtn->setProperty("class", "primaryBtn");
    resetBtn->setMinimumWidth(120);
    addBtn->setMinimumWidth(120);
    btnLayout->addWidget(resetBtn);
    btnLayout->addWidget(addBtn);

    formLayout->addLayout(row1);
    formLayout->addLayout(row2);
    formLayout->addWidget(descEdit);
    formLayout->addLayout(btnLayout);
    
    layout->addWidget(formCard);
}

void MainWindow::setupTableSection(QVBoxLayout* layout)
{
    QFrame* tableCard = new QFrame;
    tableCard->setProperty("class", "card");
    QVBoxLayout* cardLayout = new QVBoxLayout(tableCard);
    
    QLabel* title = new QLabel("Dernières Transactions");
    title->setProperty("class", "cardHeader");
    cardLayout->addWidget(title);
    
    QTableWidget* table = new QTableWidget(4, 7);
    QStringList headers = {"ID", "Date", "Description", "Type", "Mode", "Montant", "Statut"};
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(false);
    table->setFocusPolicy(Qt::NoFocus);
    table->setAlternatingRowColors(true);
    
    // Mock Data
    struct Trx { QString id; QString date; QString desc; QString type; QString mode; QString amount; QString status; };
    QList<Trx> data = {
        {"#TRX-8841", "04/02/2026", "Conception Site Web", "Facture", "Virement", "+1 500,00 €", "Payé"},
        {"#TRX-8840", "03/02/2026", "Achat Logiciel", "Facture", "Carte", "-250,00 €", "Payé"},
        {"#TRX-8839", "01/02/2026", "Consulting", "Devis", "Chèque", "800,00 €", "En attente"},
        {"#TRX-8838", "28/01/2026", "Fournitures Bureau", "Acompte", "Espèces", "-55,00 €", "Retard"}
    };
    
    for(int i=0; i<data.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(data[i].id));
        table->setItem(i, 1, new QTableWidgetItem(data[i].date));
        table->setItem(i, 2, new QTableWidgetItem(data[i].desc));
        table->setItem(i, 3, new QTableWidgetItem(data[i].type));
        table->setItem(i, 4, new QTableWidgetItem(data[i].mode));
        
        QTableWidgetItem* amtItem = new QTableWidgetItem(data[i].amount);
        if(data[i].amount.startsWith("+")) amtItem->setForeground(QColor("#27ae60"));
        else if(data[i].amount.startsWith("-")) amtItem->setForeground(QColor("#c0392b"));
        table->setItem(i, 5, amtItem);
        
        table->setItem(i, 6, new QTableWidgetItem(data[i].status));
    }
    
    cardLayout->addWidget(table);
    layout->addWidget(tableCard);
}

void MainWindow::loadStyleSheet()
{
    QFile file("style.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
        file.close();
    }
}
