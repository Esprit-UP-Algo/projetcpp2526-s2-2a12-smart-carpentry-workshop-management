#include "mainwindow.h"
#include "ui_mainwindow.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isDarkMode(false)
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
    resize(1400, 900);
    
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    createSidebar();
    
    contentArea = new QFrame(centralWidget);
    contentArea->setObjectName("contentArea");
    contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    
    createNavbar();
    createContainer();
    createPages();
    
    contentLayout->addWidget(navbar);
    contentLayout->addWidget(container);
    
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(contentArea);
}

void MainWindow::createSidebar()
{
    sidebar = new QFrame(centralWidget);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(260);
    
    sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);
    
    // Logo area with improved styling
    QFrame *logoFrame = new QFrame(sidebar);
    logoFrame->setObjectName("logoFrame");
    logoFrame->setFixedHeight(140);
    
    QVBoxLayout *logoLayout = new QVBoxLayout(logoFrame);
    logoLayout->setContentsMargins(20, 25, 20, 25);
    logoLayout->setAlignment(Qt::AlignCenter);
    
    logoLabel = new QLabel(logoFrame);
    logoLabel->setObjectName("logoImage");
    logoLabel->setFixedSize(90, 90);
    logoLabel->setScaledContents(false);
    logoLabel->setAlignment(Qt::AlignCenter);
    
    // Try to load icon.jpeg with proper circular masking
    QPixmap logoPixmap("icon.jpeg");
    if (!logoPixmap.isNull()) {
        // Scale the image
        QPixmap scaled = logoPixmap.scaled(90, 90, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        
        // Create circular mask
        QPixmap rounded(90, 90);
        rounded.fill(Qt::transparent);
        
        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        
        // Draw circle clipping path
        QPainterPath path;
        path.addEllipse(0, 0, 90, 90);
        painter.setClipPath(path);
        
        // Draw the image centered
        int x = (90 - scaled.width()) / 2;
        int y = (90 - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
        
        logoLabel->setPixmap(rounded);
    } else {
        logoLabel->setText("LOGO");
        logoLabel->setStyleSheet("background-color: #8A9A5B; color: white; font-weight: bold; border-radius: 45px; font-size: 16px;");
    }
    
    logoLayout->addWidget(logoLabel);
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
        btn->setFixedHeight(60);
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
    profileLayout->setContentsMargins(12, 14, 12, 14);
    profileLayout->setSpacing(6);
    
    // Profile Avatar
    QLabel *avatarLabel = new QLabel("A");
    avatarLabel->setStyleSheet("font-size: 32px; text-align: center; font-weight: 700; color: #FFFFFF; background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #8A9A5B, stop:1 #9aaa6b); border: none; border-radius: 30px; min-width: 60px; max-width: 60px; min-height: 60px; max-height: 60px;");
    avatarLabel->setAlignment(Qt::AlignCenter);
    
    // User Info
    QLabel *userNameLabel = new QLabel("Admin User");
    userNameLabel->setStyleSheet("font-weight: 700; font-size: 14px; color: #111827; background: transparent;");
    userNameLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *userRoleLabel = new QLabel("Administrateur");
    userRoleLabel->setStyleSheet("font-weight: 500; font-size: 12px; color: #6b7280; background: transparent;");
    userRoleLabel->setAlignment(Qt::AlignCenter);
    
    // Logout Button
    QPushButton *logoutBtn = new QPushButton("Se déconnecter");
    logoutBtn->setObjectName("logoutButton");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    
    profileLayout->addWidget(avatarLabel, 0, Qt::AlignCenter);
    profileLayout->addWidget(userNameLabel);
    profileLayout->addWidget(userRoleLabel);
    profileLayout->addSpacing(8);
    profileLayout->addWidget(logoutBtn);
    sidebarLayout->addWidget(profileFrame);
    
    QLabel *footer = new QLabel("Version 1.0.0", sidebar);
    footer->setObjectName("sidebarFooter");
    footer->setAlignment(Qt::AlignCenter);
    footer->setFixedHeight(40);
    sidebarLayout->addWidget(footer);
}

void MainWindow::createNavbar()
{
    navbar = new QFrame(contentArea);
    navbar->setObjectName("navbar");
    navbar->setFixedHeight(80);
    
    navbarLayout = new QHBoxLayout(navbar);
    navbarLayout->setContentsMargins(30, 0, 30, 0);
    navbarLayout->setSpacing(20);
    
    pageTitle = new QLabel("Gestion des Projets", navbar);
    pageTitle->setObjectName("pageTitle");
    navbarLayout->addWidget(pageTitle);
    
    navbarLayout->addStretch();
    
    // Search bar
    QLineEdit *searchBar = new QLineEdit(navbar);
    searchBar->setObjectName("searchBar");
    searchBar->setPlaceholderText("Rechercher...");
    searchBar->setMinimumWidth(300);
    searchBar->setMaximumWidth(400);
    searchBar->setFixedHeight(44);
    connect(searchBar, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    // Dark mode toggle
    QPushButton *darkModeBtn = new QPushButton("Mode Sombre", navbar);
    darkModeBtn->setObjectName("darkModeButton");
    darkModeBtn->setCursor(Qt::PointingHandCursor);
    darkModeBtn->setFixedSize(140, 44);
    darkModeBtn->setCheckable(true);
    connect(darkModeBtn, &QPushButton::clicked, this, &MainWindow::toggleDarkMode);
    
    // Notifications button
    notifBtn = new QPushButton("🔔", navbar);
    notifBtn->setObjectName("iconBtn");
    notifBtn->setCursor(Qt::PointingHandCursor);
    notifBtn->setFixedSize(44, 44);
    connect(notifBtn, &QPushButton::clicked, this, &MainWindow::showNotificationsMenu);
    
    // Profile button with photo
    profileBtn = new QPushButton(navbar);
    profileBtn->setObjectName("profileButton");
    profileBtn->setCursor(Qt::PointingHandCursor);
    profileBtn->setFixedSize(180, 50);
    
    // Create layout for profile button
    QHBoxLayout *profileLayout = new QHBoxLayout(profileBtn);
    profileLayout->setContentsMargins(8, 5, 8, 5);
    profileLayout->setSpacing(10);
    
    // Profile photo
    QLabel *profilePhoto = new QLabel(profileBtn);
    profilePhoto->setObjectName("profilePhoto");
    profilePhoto->setFixedSize(40, 40);
    profilePhoto->setScaledContents(false);
    
    QPixmap pfpPixmap("pfp.jpeg");
    if (!pfpPixmap.isNull()) {
        // Scale the image
        QPixmap scaled = pfpPixmap.scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        
        // Create circular mask
        QPixmap rounded(40, 40);
        rounded.fill(Qt::transparent);
        
        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        
        // Draw circle
        QPainterPath path;
        path.addEllipse(0, 0, 40, 40);
        painter.setClipPath(path);
        
        // Center the image
        int x = (40 - scaled.width()) / 2;
        int y = (40 - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
        
        profilePhoto->setPixmap(rounded);
    } else {
        profilePhoto->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #8A9A5B, stop:1 #9aaa6b); border-radius: 20px;");
    }
    
    QLabel *profileName = new QLabel("Admin", profileBtn);
    profileName->setObjectName("profileName");
    
    QLabel *arrow = new QLabel("▼", profileBtn);
    arrow->setObjectName("dropdownArrow");
    
    profileLayout->addWidget(profilePhoto);
    profileLayout->addWidget(profileName);
    profileLayout->addWidget(arrow);
    profileLayout->addStretch();
    
    connect(profileBtn, &QPushButton::clicked, this, &MainWindow::showProfileMenu);
    
    navbarLayout->addWidget(searchBar);
    navbarLayout->addWidget(darkModeBtn);
    navbarLayout->addWidget(notifBtn);
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
    
    connect(logoutAction, &QAction::triggered, [this]() {
        QMessageBox::information(this, "Deconnexion", "Deconnexion de l'application");
    });
    
    menu->exec(profileBtn->mapToGlobal(QPoint(0, profileBtn->height())));
}

void MainWindow::showNotificationsMenu()
{
    QMenu *menu = new QMenu(this);
    menu->setObjectName("dropdownMenu");
    
    QAction *notif1 = menu->addAction("Nouveau projet assigne");
    QAction *notif2 = menu->addAction("Stock faible: Bois de chene");
    QAction *notif3 = menu->addAction("Facture en attente");
    menu->addSeparator();
    QAction *viewAll = menu->addAction("Voir toutes les notifications");
    
    connect(viewAll, &QAction::triggered, [this]() {
        QMessageBox::information(this, "Notifications", "Afficher toutes les notifications");
    });
    
    menu->exec(notifBtn->mapToGlobal(QPoint(0, notifBtn->height())));
}

void MainWindow::createContainer()
{
    container = new QFrame(contentArea);
    container->setObjectName("container");
    
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(30, 30, 30, 30);
    
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
    layout->setSpacing(20);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"Projets Actifs", "12", "active"},
        {"En Attente", "5", "pending"},
        {"Termines ce Mois", "8", "completed"}
    };
    
    for (const auto& stat : stats) {
        QFrame *card = new QFrame(page);
        card->setObjectName("statCard");
        card->setProperty("type", stat.type);
        
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setSpacing(8);
        
        QLabel *title = new QLabel(stat.title, card);
        title->setObjectName("statTitle");
        
        QLabel *value = new QLabel(stat.value, card);
        value->setObjectName("statValue");
        
        cardLayout->addWidget(title);
        cardLayout->addWidget(value);
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
    table->setHorizontalHeaderLabels({"Client", "Type", "Date Debut", "Date Fin", "Statut", "Budget"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    
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
    
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
}

QWidget* MainWindow::createEmployeesPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"Total Employes", "24", "total"},
        {"Actifs Aujourd'hui", "18", "active"},
        {"En Formation", "3", "training"}
    };
    
    for (const auto& stat : stats) {
        QFrame *card = new QFrame(page);
        card->setObjectName("statCard");
        card->setProperty("type", stat.type);
        
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setSpacing(8);
        
        QLabel *title = new QLabel(stat.title, card);
        title->setObjectName("statTitle");
        
        QLabel *value = new QLabel(stat.value, card);
        value->setObjectName("statValue");
        
        cardLayout->addWidget(title);
        cardLayout->addWidget(value);
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
    
    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(editBtn);
    actionsLayout->addWidget(deleteBtn);
    actionsLayout->addStretch();
    
    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("dataTable");
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Nom", "Fonction", "Anciennete", "Competences", "Statut", "Projets"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
}

QWidget* MainWindow::createStocksPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"Valeur Stock", "45 280 EUR", "value"},
        {"Articles", "156", "items"},
        {"Alertes Stock", "8", "alerts"}
    };
    
    for (const auto& stat : stats) {
        QFrame *card = new QFrame(page);
        card->setObjectName("statCard");
        card->setProperty("type", stat.type);
        
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setSpacing(8);
        
        QLabel *title = new QLabel(stat.title, card);
        title->setObjectName("statTitle");
        
        QLabel *value = new QLabel(stat.value, card);
        value->setObjectName("statValue");
        
        cardLayout->addWidget(title);
        cardLayout->addWidget(value);
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
    table->setHorizontalHeaderLabels({"Materiau", "Categorie", "Quantite", "Prix Unitaire", "Fournisseur", "Seuil"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
}

QWidget* MainWindow::createFinancePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"Revenus Mensuels", "45 280 EUR", "income"},
        {"Depenses", "23 150 EUR", "expense"},
        {"Marge Beneficiaire", "22 130 EUR", "profit"}
    };
    
    for (const auto& stat : stats) {
        QFrame *card = new QFrame(page);
        card->setObjectName("statCard");
        card->setProperty("type", stat.type);
        
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setSpacing(8);
        
        QLabel *title = new QLabel(stat.title, card);
        title->setObjectName("statTitle");
        
        QLabel *value = new QLabel(stat.value, card);
        value->setObjectName("statValue");
        
        cardLayout->addWidget(title);
        cardLayout->addWidget(value);
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
    table->setHorizontalHeaderLabels({"Client", "Type", "Montant", "Statut", "Date"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    
    layout->addLayout(statsLayout);
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
}

QWidget* MainWindow::createDesignsPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    
    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);
    
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"Designs Actifs", "15", "active"},
        {"En Validation", "4", "pending"},
        {"Approuves", "32", "approved"}
    };
    
    for (const auto& stat : stats) {
        QFrame *card = new QFrame(page);
        card->setObjectName("statCard");
        card->setProperty("type", stat.type);
        
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setSpacing(8);
        
        QLabel *title = new QLabel(stat.title, card);
        title->setObjectName("statTitle");
        
        QLabel *value = new QLabel(stat.value, card);
        value->setObjectName("statValue");
        
        cardLayout->addWidget(title);
        cardLayout->addWidget(value);
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
    table->setHorizontalHeaderLabels({"Nom", "Type", "Dimensions", "Style", "Date", "Statut"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    
    layout->addLayout(statsLayout);
    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    
    return page;
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
    isDarkMode = !isDarkMode;
    loadStyleSheet();
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
