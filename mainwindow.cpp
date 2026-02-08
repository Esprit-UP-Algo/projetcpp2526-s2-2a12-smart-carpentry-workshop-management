#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/modules/auth/loginpage.h"
#include "src/modules/auth/registerpage.h"
#include "src/core/session.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QKeyEvent>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isDarkMode(false)
    , currentScale(1.0)
    , currentTable(nullptr)
{
    ui->setupUi(this);
    setupUI();
    loadStyleSheet();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("Smart Carpentry Management");
    resize(1200, 800);
    
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create auth/main stack
    authStack = new QStackedWidget(centralWidget);
    mainLayout->addWidget(authStack);
    
    // Auth page setup
    setupAuth();
    
    // Main app page setup
    mainAppPage = new QWidget();
    QHBoxLayout *appLayout = new QHBoxLayout(mainAppPage);
    appLayout->setContentsMargins(0, 0, 0, 0);
    appLayout->setSpacing(0);
    
    createSidebar();
    
    contentArea = new QFrame(mainAppPage);
    contentArea->setObjectName("contentArea");
    contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    
    createNavbar();
    createContainer();
    createPages();
    
    contentLayout->addWidget(navbar);
    contentLayout->addWidget(container);
    
    appLayout->addWidget(sidebar);
    appLayout->addWidget(contentArea);
    
    authStack->addWidget(authPage);
    authStack->addWidget(mainAppPage);
    
    // Start with auth page
    authStack->setCurrentIndex(0);
}

void MainWindow::setupAuth()
{
    authPage = new QWidget();
    QVBoxLayout *authLayout = new QVBoxLayout(authPage);
    authLayout->setContentsMargins(0, 0, 0, 0);
    authLayout->setSpacing(0);
    
    QStackedWidget *authPages = new QStackedWidget(authPage);
    
    LoginPage *loginPage = new LoginPage();
    RegisterPage *registerPage = new RegisterPage();
    
    authPages->addWidget(loginPage);
    authPages->addWidget(registerPage);
    
    connect(loginPage, &LoginPage::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(loginPage, &LoginPage::switchToRegister, [authPages]() {
        authPages->setCurrentIndex(1);
    });
    connect(registerPage, &RegisterPage::switchToLogin, [authPages]() {
        authPages->setCurrentIndex(0);
    });
    
    authLayout->addWidget(authPages);
}

void MainWindow::onLoginSuccess(const Employee& employee)
{
    currentEmployee = employee;
    Session::setCurrentEmployee(employee);
    showMainApp();
}

void MainWindow::showMainApp()
{
    // Update profile with user info
    if (profileName) {
        profileName->setText(currentEmployee.getFullName());
    }
    authStack->setCurrentIndex(1);
    onSidebarButtonClicked(0);
}

void MainWindow::onLogout()
{
    Session::logout();
    authStack->setCurrentIndex(0);
}

QLabel* MainWindow::createRoundedAvatar(const QString& imagePath, int size)
{
    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(size, size);
    avatarLabel->setScaledContents(false);
    
    QPixmap sourcePixmap(imagePath);
    if (sourcePixmap.isNull()) {
        // Fallback to colored circle with letter
        QPixmap fallback(size, size);
        fallback.fill(Qt::transparent);
        
        QPainter painter(&fallback);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Draw gradient circle
        QLinearGradient gradient(0, 0, size, size);
        gradient.setColorAt(0, QColor("#8A9A5B"));
        gradient.setColorAt(1, QColor("#9aaa6b"));
        
        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, size, size);
        
        // Draw letter
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPixelSize(size / 2);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, "A");
        
        avatarLabel->setPixmap(fallback);
        return avatarLabel;
    }
    
    // Create rounded image
    QPixmap scaled = sourcePixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    
    QPixmap rounded(size, size);
    rounded.fill(Qt::transparent);
    
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    
    int x = (size - scaled.width()) / 2;
    int y = (size - scaled.height()) / 2;
    painter.drawPixmap(x, y, scaled);
    
    avatarLabel->setPixmap(rounded);
    return avatarLabel;
}

void MainWindow::createSidebar()
{
    sidebar = new QFrame(centralWidget);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(220);
    
    sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);
    
    // Logo area
    QFrame *logoFrame = new QFrame(sidebar);
    logoFrame->setObjectName("logoFrame");
    logoFrame->setFixedHeight(120);
    
    QVBoxLayout *logoLayout = new QVBoxLayout(logoFrame);
    logoLayout->setContentsMargins(15, 20, 15, 20);
    logoLayout->setAlignment(Qt::AlignCenter);
    
    logoLabel = createRoundedAvatar("src/assets/icons/logo1.png", 70);
    logoLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
    
    sidebarLayout->addWidget(logoFrame);
    
    // Sidebar menu items
    QStringList menuItems = {
        "Gestion des Projets",
        "Gestion des Employes", 
        "Gestion des Stocks",
        "Gestion Financiere",
        "Gestion des Designs"
    };
    
    for (int i = 0; i < menuItems.size(); ++i) {
        QPushButton *btn = new QPushButton(menuItems[i], sidebar);
        btn->setObjectName("sidebarButton");
        btn->setCheckable(true);
        btn->setFixedHeight(48);
        btn->setCursor(Qt::PointingHandCursor);
        
        if (i == 0) {
            btn->setChecked(true);
            btn->setProperty("active", true);
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
        }
        
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            onSidebarButtonClicked(i);
        });
        
        sidebarButtons.append(btn);
        sidebarLayout->addWidget(btn);
    }
    
    sidebarLayout->addStretch();
    
    // User Profile Card at bottom
    QFrame *profileFrame = new QFrame(sidebar);
    profileFrame->setObjectName("profileFrame");
    QVBoxLayout *profileLayout = new QVBoxLayout(profileFrame);
    profileLayout->setContentsMargins(12, 12, 12, 12);
    profileLayout->setSpacing(6);
    
    QLabel *avatarLabel = createRoundedAvatar("src/assets/icons/pfp.jpeg", 50);
    
    QLabel *userNameLabel = new QLabel("Employé");
    userNameLabel->setStyleSheet("font-weight: 600; font-size: 13px; background: transparent;");
    userNameLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *userRoleLabel = new QLabel("En attente");
    userRoleLabel->setStyleSheet("font-weight: 400; font-size: 11px; color: #888; background: transparent;");
    userRoleLabel->setAlignment(Qt::AlignCenter);
    
    QPushButton *logoutBtn = new QPushButton("Se déconnecter");
    logoutBtn->setObjectName("logoutButton");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setFixedHeight(32);
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);
    
    profileLayout->addWidget(avatarLabel, 0, Qt::AlignCenter);
    profileLayout->addWidget(userNameLabel);
    profileLayout->addWidget(userRoleLabel);
    profileLayout->addSpacing(6);
    profileLayout->addWidget(logoutBtn);
    sidebarLayout->addWidget(profileFrame);
    
    QLabel *footer = new QLabel("Version 1.0.0", sidebar);
    footer->setObjectName("sidebarFooter");
    footer->setAlignment(Qt::AlignCenter);
    footer->setFixedHeight(32);
    sidebarLayout->addWidget(footer);
}

void MainWindow::createNavbar()
{
    navbar = new QFrame(contentArea);
    navbar->setObjectName("navbar");
    navbar->setFixedHeight(65);
    
    navbarLayout = new QHBoxLayout(navbar);
    navbarLayout->setContentsMargins(25, 0, 25, 0);
    navbarLayout->setSpacing(15);
    
    pageTitle = new QLabel("Gestion des Projets", navbar);
    pageTitle->setObjectName("pageTitle");
    navbarLayout->addWidget(pageTitle);
    
    navbarLayout->addStretch();
    
    // Search bar
    QLineEdit *searchBar = new QLineEdit(navbar);
    searchBar->setObjectName("searchBar");
    searchBar->setPlaceholderText("Rechercher...");
    searchBar->setMinimumWidth(260);
    searchBar->setMaximumWidth(350);
    searchBar->setFixedHeight(38);
    connect(searchBar, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    // Dark mode toggle switch
    QWidget *darkModeContainer = new QWidget(navbar);
    QHBoxLayout *darkModeLayout = new QHBoxLayout(darkModeContainer);
    darkModeLayout->setContentsMargins(0, 0, 0, 0);
    darkModeLayout->setSpacing(8);
    
    QLabel *darkModeLabel = new QLabel("Passer en mode sombre", darkModeContainer);
    darkModeLabel->setObjectName("darkModeLabel");
    darkModeLabel->setStyleSheet("color: #718096; font-size: 12px;");
    
    darkModeToggle = new ToggleSwitch(darkModeContainer);
    connect(darkModeToggle, &ToggleSwitch::toggled, this, [this, darkModeLabel](bool checked) {
        if (checked) {
            darkModeLabel->setText("Passer en mode clair");
        } else {
            darkModeLabel->setText("Passer en mode sombre");
        }
        toggleDarkMode();
    });
    
    darkModeLayout->addWidget(darkModeLabel);
    darkModeLayout->addWidget(darkModeToggle);
    
    
    // Profile button
    profileBtn = new QPushButton(navbar);
    profileBtn->setObjectName("profileButton");
    profileBtn->setCursor(Qt::PointingHandCursor);
    profileBtn->setFixedSize(150, 42);
    
    QHBoxLayout *profileLayout = new QHBoxLayout(profileBtn);
    profileLayout->setContentsMargins(6, 3, 6, 3);
    profileLayout->setSpacing(8);
    
    QLabel *profilePhoto = createRoundedAvatar("src/assets/icons/pfp.jpeg", 32);
    
    profileName = new QLabel("Admin", profileBtn);
    profileName->setObjectName("profileName");
    
    QLabel *arrow = new QLabel("▼", profileBtn);
    arrow->setObjectName("dropdownArrow");
    
    profileLayout->addWidget(profilePhoto);
    profileLayout->addWidget(profileName);
    profileLayout->addWidget(arrow);
    profileLayout->addStretch();
    
    connect(profileBtn, &QPushButton::clicked, this, &MainWindow::showProfileMenu);
    
    navbarLayout->addWidget(searchBar);
    navbarLayout->addWidget(darkModeContainer);
    navbarLayout->addWidget(profileBtn);
}

void MainWindow::showProfileMenu()
{
    QMenu *menu = new QMenu(this);
    menu->setObjectName("dropdownMenu");
    
    QAction *profileAction = menu->addAction("Mon Profil");
    QAction *settingsAction = menu->addAction("Parametres");
    menu->addSeparator();
    QAction *logoutAction = menu->addAction("Deconnexion");
    
    connect(profileAction, &QAction::triggered, [this]() {
        QMessageBox::information(this, "Profil", "Ouvrir le profil utilisateur");
    });
    
    connect(settingsAction, &QAction::triggered, [this]() {
        QMessageBox::information(this, "Parametres", "Ouvrir les parametres");
    });
    
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogout);
    
    menu->exec(profileBtn->mapToGlobal(QPoint(0, profileBtn->height())));
}

void MainWindow::createContainer()
{
    container = new QFrame(contentArea);
    container->setObjectName("container");
    
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(25, 25, 25, 25);
    
    stackedWidget = new QStackedWidget(container);
    stackedWidget->setObjectName("stackedWidget");
    containerLayout->addWidget(stackedWidget);
}

void MainWindow::createPages()
{
    stackedWidget->addWidget(createProjectsPage());
    stackedWidget->addWidget(createEmployeesPage());
    stackedWidget->addWidget(createStocksPage());
    stackedWidget->addWidget(createFinancePage());
    stackedWidget->addWidget(createDesignsPage());
}

QWidget* MainWindow::createProjectsPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(18);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"PROJETS ACTIFS", "12", "active"},
        {"EN ATTENTE", "5", "pending"},
        {"TERMINES CE MOIS", "8", "completed"}
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
    
    // Action buttons
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("+ Nouveau Projet", page);
    QPushButton *editBtn = new QPushButton("Modifier", page);
    QPushButton *deleteBtn = new QPushButton("Supprimer", page);
    QPushButton *exportBtn = new QPushButton("Generer PDF", page);
    
    addBtn->setObjectName("actionButton");
    editBtn->setObjectName("actionButton");
    deleteBtn->setObjectName("actionButton");
    exportBtn->setObjectName("actionButton");
    
    addBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    exportBtn->setCursor(Qt::PointingHandCursor);
    
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddButtonClicked);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::onEditButtonClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteButtonClicked);
    
    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(editBtn);
    actionsLayout->addWidget(deleteBtn);
    actionsLayout->addWidget(exportBtn);
    actionsLayout->addStretch();
    
    // Table
    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("dataTable");
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"CLIENT", "TYPE", "DATE DEBUT", "DATE FIN", "STATUT", "BUDGET"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    
    // Sample data
    table->setRowCount(3);
    QStringList row1 = {"M. Dupont", "Meuble sur mesure", "01/02/2026", "15/03/2026", "En cours", "3500 EUR"};
    QStringList row2 = {"Mme Martin", "Renovation", "10/02/2026", "28/02/2026", "En attente", "2800 EUR"};
    QStringList row3 = {"Restaurant Le Bois", "Agencement", "15/01/2026", "30/01/2026", "Termine", "8500 EUR"};
    
    for (int col = 0; col < 6; ++col) {
        table->setItem(0, col, new QTableWidgetItem(row1[col]));
        table->setItem(1, col, new QTableWidgetItem(row2[col]));
        table->setItem(2, col, new QTableWidgetItem(row3[col]));
    }
    
    for (int row = 0; row < 3; ++row) {
        table->setRowHeight(row, 50);
    }
    
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
}

QWidget* MainWindow::createEmployeesPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(18);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"TOTAL EMPLOYES", "24", "total"},
        {"ACTIFS AUJOURD'HUI", "18", "active"},
        {"EN FORMATION", "3", "training"}
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
    
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("+ Nouvel Employe", page);
    QPushButton *editBtn = new QPushButton("Modifier", page);
    QPushButton *deleteBtn = new QPushButton("Supprimer", page);
    
    addBtn->setObjectName("actionButton");
    editBtn->setObjectName("actionButton");
    deleteBtn->setObjectName("actionButton");
    
    addBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addEmployee);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::editEmployee);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteEmployee);
    
    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(editBtn);
    actionsLayout->addWidget(deleteBtn);
    actionsLayout->addStretch();
    
    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("dataTable");
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"NOM", "FONCTION", "ANCIENNETE", "COMPETENCES", "STATUT", "PROJETS"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    
    // Sample data
    table->setRowCount(3);
    QStringList row1 = {"Jean Dupont", "Menuisier Senior", "5 ans", "Ébénisterie, Finition", "Actif", "3"};
    QStringList row2 = {"Marie Martin", "Designer", "2 ans", "CAO, Conception 3D", "Actif", "5"};
    QStringList row3 = {"Pierre Bernard", "Apprenti", "6 mois", "Assemblage", "Formation", "1"};
    
    for (int col = 0; col < 6; ++col) {
        table->setItem(0, col, new QTableWidgetItem(row1[col]));
        table->setItem(1, col, new QTableWidgetItem(row2[col]));
        table->setItem(2, col, new QTableWidgetItem(row3[col]));
    }
    
    for (int row = 0; row < 3; ++row) {
        table->setRowHeight(row, 50);
    }
    
    currentTable = table;
    
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
}

QWidget* MainWindow::createStocksPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(18);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"VALEUR STOCK", "45 280 €", "value"},
        {"ARTICLES", "156", "items"},
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
    
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("+ Nouveau Materiau", page);
    QPushButton *editBtn = new QPushButton("Modifier", page);
    QPushButton *alertBtn = new QPushButton("Alertes Stock", page);
    
    addBtn->setObjectName("actionButton");
    editBtn->setObjectName("actionButton");
    alertBtn->setObjectName("actionButton");
    
    addBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setCursor(Qt::PointingHandCursor);
    alertBtn->setCursor(Qt::PointingHandCursor);
    
    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(editBtn);
    actionsLayout->addWidget(alertBtn);
    actionsLayout->addStretch();
    
    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("dataTable");
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"MATERIAU", "CATEGORIE", "QUANTITE", "PRIX UNITAIRE", "FOURNISSEUR", "SEUIL"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    
    layout->addLayout(statsLayout);
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
}

QWidget* MainWindow::createFinancePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(18);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"REVENUS MENSUELS", "45 280 €", "income"},
        {"DEPENSES", "23 150 €", "expense"},
        {"MARGE BENEFICIAIRE", "22 130 €", "profit"}
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
    
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("+ Nouvelle Transaction", page);
    QPushButton *exportBtn = new QPushButton("Rapport Mensuel", page);
    
    addBtn->setObjectName("actionButton");
    exportBtn->setObjectName("actionButton");
    
    addBtn->setCursor(Qt::PointingHandCursor);
    exportBtn->setCursor(Qt::PointingHandCursor);
    
    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(exportBtn);
    actionsLayout->addStretch();
    
    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("dataTable");
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"CLIENT", "TYPE", "MONTANT", "STATUT", "DATE"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    
    layout->addLayout(statsLayout);
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
}

QWidget* MainWindow::createDesignsPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(18);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"DESIGNS ACTIFS", "15", "active"},
        {"EN VALIDATION", "4", "pending"},
        {"APPROUVES", "32", "approved"}
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
    
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("+ Nouveau Design", page);
    QPushButton *viewBtn = new QPushButton("Visualiser 3D", page);
    QPushButton *exportBtn = new QPushButton("Plans Technique", page);
    
    addBtn->setObjectName("actionButton");
    viewBtn->setObjectName("actionButton");
    exportBtn->setObjectName("actionButton");
    
    addBtn->setCursor(Qt::PointingHandCursor);
    viewBtn->setCursor(Qt::PointingHandCursor);
    exportBtn->setCursor(Qt::PointingHandCursor);
    
    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(viewBtn);
    actionsLayout->addWidget(exportBtn);
    actionsLayout->addStretch();
    
    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("dataTable");
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"NOM", "TYPE", "DIMENSIONS", "STYLE", "DATE", "STATUT"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    
    layout->addLayout(statsLayout);
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
}

void MainWindow::addEmployee()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un Employé");
    dialog.setMinimumWidth(400);
    
    QFormLayout *form = new QFormLayout(&dialog);
    
    QLineEdit *nameInput = new QLineEdit(&dialog);
    QLineEdit *functionInput = new QLineEdit(&dialog);
    QLineEdit *seniorityInput = new QLineEdit(&dialog);
    QLineEdit *skillsInput = new QLineEdit(&dialog);
    QComboBox *statusCombo = new QComboBox(&dialog);
    statusCombo->addItems({"Actif", "Formation", "Congé"});
    QLineEdit *projectsInput = new QLineEdit(&dialog);
    
    form->addRow("Nom:", nameInput);
    form->addRow("Fonction:", functionInput);
    form->addRow("Ancienneté:", seniorityInput);
    form->addRow("Compétences:", skillsInput);
    form->addRow("Statut:", statusCombo);
    form->addRow("Projets:", projectsInput);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        if (currentTable) {
            int row = currentTable->rowCount();
            currentTable->insertRow(row);
            currentTable->setItem(row, 0, new QTableWidgetItem(nameInput->text()));
            currentTable->setItem(row, 1, new QTableWidgetItem(functionInput->text()));
            currentTable->setItem(row, 2, new QTableWidgetItem(seniorityInput->text()));
            currentTable->setItem(row, 3, new QTableWidgetItem(skillsInput->text()));
            currentTable->setItem(row, 4, new QTableWidgetItem(statusCombo->currentText()));
            currentTable->setItem(row, 5, new QTableWidgetItem(projectsInput->text()));
            currentTable->setRowHeight(row, 50);
        }
    }
}

void MainWindow::editEmployee()
{
    if (!currentTable || currentTable->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un employé à modifier.");
        return;
    }
    
    int row = currentTable->currentRow();
    
    QDialog dialog(this);
    dialog.setWindowTitle("Modifier l'Employé");
    dialog.setMinimumWidth(400);
    
    QFormLayout *form = new QFormLayout(&dialog);
    
    QLineEdit *nameInput = new QLineEdit(currentTable->item(row, 0)->text(), &dialog);
    QLineEdit *functionInput = new QLineEdit(currentTable->item(row, 1)->text(), &dialog);
    QLineEdit *seniorityInput = new QLineEdit(currentTable->item(row, 2)->text(), &dialog);
    QLineEdit *skillsInput = new QLineEdit(currentTable->item(row, 3)->text(), &dialog);
    QComboBox *statusCombo = new QComboBox(&dialog);
    statusCombo->addItems({"Actif", "Formation", "Congé"});
    statusCombo->setCurrentText(currentTable->item(row, 4)->text());
    QLineEdit *projectsInput = new QLineEdit(currentTable->item(row, 5)->text(), &dialog);
    
    form->addRow("Nom:", nameInput);
    form->addRow("Fonction:", functionInput);
    form->addRow("Ancienneté:", seniorityInput);
    form->addRow("Compétences:", skillsInput);
    form->addRow("Statut:", statusCombo);
    form->addRow("Projets:", projectsInput);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        currentTable->item(row, 0)->setText(nameInput->text());
        currentTable->item(row, 1)->setText(functionInput->text());
        currentTable->item(row, 2)->setText(seniorityInput->text());
        currentTable->item(row, 3)->setText(skillsInput->text());
        currentTable->item(row, 4)->setText(statusCombo->currentText());
        currentTable->item(row, 5)->setText(projectsInput->text());
    }
}

void MainWindow::deleteEmployee()
{
    if (!currentTable || currentTable->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un employé à supprimer.");
        return;
    }
    
    int row = currentTable->currentRow();
    QString name = currentTable->item(row, 0)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmer la suppression",
        QString("Êtes-vous sûr de vouloir supprimer %1 ?").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        currentTable->removeRow(row);
    }
}

void MainWindow::onSidebarButtonClicked(int index)
{
    for (int i = 0; i < sidebarButtons.size(); ++i) {
        QPushButton *btn = sidebarButtons[i];
        btn->setChecked(i == index);
        btn->setProperty("active", i == index);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
    
    stackedWidget->setCurrentIndex(index);
    
    // Update currentTable reference for employees page
    if (index == 1) {  // Employees page
        QWidget *page = stackedWidget->widget(index);
        currentTable = page->findChild<QTableWidget*>("dataTable");
    }
    
    QStringList titles = {
        "Gestion des Projets",
        "Gestion des Employes",
        "Gestion des Stocks",
        "Gestion Financiere",
        "Gestion des Designs"
    };
    pageTitle->setText(titles[index]);
}

void MainWindow::onAddButtonClicked()
{
    QMessageBox::information(this, "Action", "Fonctionnalite d'ajout a implementer");
}

void MainWindow::onEditButtonClicked()
{
    QMessageBox::information(this, "Action", "Fonctionnalite de modification a implementer");
}

void MainWindow::onDeleteButtonClicked()
{
    QMessageBox::information(this, "Action", "Fonctionnalite de suppression a implementer");
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    qDebug() << "Recherche:" << text;
}

void MainWindow::toggleDarkMode()
{
    isDarkMode = darkModeToggle->isChecked();
    loadStyleSheet();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
            onScaleUp();
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Minus) {
            onScaleDown();
            event->accept();
            return;
        } else if (event->key() == Qt::Key_0) {
            onScaleReset();
            event->accept();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::onScaleUp()
{
    if (currentScale < 1.5) {
        currentScale += 0.1;
        applyScale(currentScale);
    }
}

void MainWindow::onScaleDown()
{
    if (currentScale > 0.7) {
        currentScale -= 0.1;
        applyScale(currentScale);
    }
}

void MainWindow::onScaleReset()
{
    currentScale = 1.0;
    applyScale(currentScale);
}

void MainWindow::applyScale(qreal scale)
{
    QFont font = qApp->font();
    font.setPointSizeF(font.pointSizeF() * scale / (currentScale == scale ? currentScale : (currentScale - (scale - currentScale))));
    qApp->setFont(font);
    
    // Update window size proportionally
    int newWidth = static_cast<int>(1200 * scale);
    int newHeight = static_cast<int>(800 * scale);
    resize(newWidth, newHeight);
}

void MainWindow::loadStyleSheet()
{
    QString filename = isDarkMode ? "style-dark.qss" : "style.qss";
    QFile styleFile(filename);
    
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        qApp->setStyleSheet(styleSheet);
        styleFile.close();
        qDebug() << "Stylesheet loaded:" << filename;
    } else {
        qDebug() << "Could not load stylesheet:" << filename;
    }
}
