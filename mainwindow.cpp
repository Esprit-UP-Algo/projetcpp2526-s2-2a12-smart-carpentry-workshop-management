#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/modules/auth/loginpage.h"
#include "src/modules/auth/registerpage.h"
#include "src/modules/employees/employeemanagementpage.h"
#include "src/core/session.h"
#include "src/modules/stock/stockpage.h"
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
#include <QDateEdit>
#include <QComboBox>
#include <QDate>
#include <QMenu>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFileDialog>
#include <QDate>
#include <QMap>
#include <algorithm>



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

    setMinimumSize(1200, 700);  // Taille minimum
    setMaximumSize(2000, 900);
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
    // Update navbar profile name
    if (profileName)
        profileName->setText(currentEmployee.getFullName());

    // Redraw navbar avatar circle with employee's first initial
    for (QLabel* lbl : profileBtn->findChildren<QLabel*>()) {
        if (lbl->width() == 32 && lbl->height() == 32) {
            QString initial = currentEmployee.getPrenom().isEmpty()
                ? "?" : QString(currentEmployee.getPrenom().at(0).toUpper());
            int size = 32;
            QPixmap avatar(size, size);
            avatar.fill(Qt::transparent);
            QPainter painter(&avatar);
            painter.setRenderHint(QPainter::Antialiasing);
            QLinearGradient g(0, 0, size, size);
            g.setColorAt(0, QColor("#8A9A5B"));
            g.setColorAt(1, QColor("#6a8040"));
            painter.setBrush(g);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(0, 0, size, size);
            painter.setPen(Qt::white);
            QFont font = painter.font();
            font.setPixelSize(size / 2);
            font.setBold(true);
            painter.setFont(font);
            painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, initial);
            lbl->setPixmap(avatar);
            break;
        }
    }

    // Update sidebar bottom card name + role
    for (QLabel* lbl : sidebar->findChildren<QLabel*>()) {
        if (lbl->styleSheet().contains("font-weight: 600") && lbl->styleSheet().contains("13px"))
            lbl->setText(currentEmployee.getFullName());
        if (lbl->styleSheet().contains("font-weight: 400") && lbl->styleSheet().contains("11px"))
            lbl->setText(currentEmployee.getPoste().isEmpty() ? "Employe" : currentEmployee.getPoste());
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
    stackedWidget->addWidget(new StockPage(this));
    stackedWidget->addWidget(createFinancePage());
    stackedWidget->addWidget(createProductsPage());
}

QWidget* MainWindow::createProjectsPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(18);

    // ────────────────────────────────────────────────
    // Statistics cards
    // ────────────────────────────────────────────────
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

    // ────────────────────────────────────────────────
    // Action buttons
    // ────────────────────────────────────────────────
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

    // ────────────────────────────────────────────────
    // Projects Table
    // ────────────────────────────────────────────────
    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("dataTable");
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({
        "CLIENT",
        "TYPE",
        "DATE DEBUT",
        "DATE FIN",
        "STATUT",
        "BUDGET",
        "NOM PROJET",
        "ADRESSE"
    });

    // Table appearance
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);

    // ────────────────────────────────────────────────
    // Sample data
    // ────────────────────────────────────────────────
    table->setRowCount(3);

    // Row 1 - M. Dupont
    table->setItem(0, 0, new QTableWidgetItem("M. Dupont"));
    table->setItem(0, 1, new QTableWidgetItem("Meuble sur mesure"));
    table->setItem(0, 2, new QTableWidgetItem("01/02/2026"));
    table->setItem(0, 3, new QTableWidgetItem("15/03/2026"));

    QTableWidgetItem *statut1 = new QTableWidgetItem("En cours");
    statut1->setForeground(QBrush(QColor("#2ecc71")));
    table->setItem(0, 4, statut1);

    table->setItem(0, 5, new QTableWidgetItem("3500 EUR"));
    table->setItem(0, 6, new QTableWidgetItem("Rénovation Salon"));
    table->setItem(0, 7, new QTableWidgetItem("15 Rue de Paris, 75001 Paris"));

    // Row 2 - Mme Martin
    table->setItem(1, 0, new QTableWidgetItem("Mme Martin"));
    table->setItem(1, 1, new QTableWidgetItem("Rénovation"));
    table->setItem(1, 2, new QTableWidgetItem("10/02/2026"));
    table->setItem(1, 3, new QTableWidgetItem("28/02/2026"));

    QTableWidgetItem *statut2 = new QTableWidgetItem("En attente");
    statut2->setForeground(QBrush(QColor("#f39c12")));
    table->setItem(1, 4, statut2);

    table->setItem(1, 5, new QTableWidgetItem("2800 EUR"));
    table->setItem(1, 6, new QTableWidgetItem("Cuisine Moderne"));
    table->setItem(1, 7, new QTableWidgetItem("8 Rue des Lilas, 69002 Lyon"));

    // Row 3 - Restaurant Le Bois
    table->setItem(2, 0, new QTableWidgetItem("Restaurant Le Bois"));
    table->setItem(2, 1, new QTableWidgetItem("Agencement"));
    table->setItem(2, 2, new QTableWidgetItem("15/01/2026"));
    table->setItem(2, 3, new QTableWidgetItem("30/01/2026"));

    QTableWidgetItem *statut3 = new QTableWidgetItem("Terminé");
    statut3->setForeground(QBrush(QColor("#3498db")));
    table->setItem(2, 4, statut3);

    table->setItem(2, 5, new QTableWidgetItem("8500 EUR"));
    table->setItem(2, 6, new QTableWidgetItem("Agencement Restaurant"));
    table->setItem(2, 7, new QTableWidgetItem("45 Cours Gambetta, 33000 Bordeaux"));

    // Set row heights
    for (int row = 0; row < 3; ++row) {
        table->setRowHeight(row, 50);
    }

    // ────────────────────────────────────────────────
    // DOUBLE-CLICK TO VIEW PROJECT DETAILS
    // ────────────────────────────────────────────────
    connect(table, &QTableWidget::cellDoubleClicked, this, [this, table](int row, int) {
        // Safety checks
        if (!table || row < 0 || row >= table->rowCount()) {
            return;
        }

        // Get data from table with null checks
        QString nomProjet = table->item(row, 6) ? table->item(row, 6)->text() : "Non spécifié";
        QString client = table->item(row, 0) ? table->item(row, 0)->text() : "Non spécifié";
        QString adresse = table->item(row, 7) ? table->item(row, 7)->text() : "Non spécifiée";
        QString type = table->item(row, 1) ? table->item(row, 1)->text() : "Non spécifié";
        QString dateDebut = table->item(row, 2) ? table->item(row, 2)->text() : "Non spécifiée";
        QString dateFin = table->item(row, 3) ? table->item(row, 3)->text() : "Non spécifiée";
        QString statut = table->item(row, 4) ? table->item(row, 4)->text() : "Non spécifié";
        QString budget = table->item(row, 5) ? table->item(row, 5)->text() : "Non spécifié";

        // Determine color based on status
        QString statutColor;
        if (statut == "En cours") {
            statutColor = "#2ecc71";
        } else if (statut == "En attente") {
            statutColor = "#f39c12";
        } else if (statut == "Terminé") {
            statutColor = "#3498db";
        } else {
            statutColor = "#e74c3c";
        }

        // Create and show details dialog
        QMessageBox details(this);
        details.setWindowTitle("Détails du Projet");
        details.setIcon(QMessageBox::Information);
        details.setText(QString(
                            "<div style='font-family: Arial, sans-serif;'>"
                            "<h3 style='color: #8A9A5B; margin-bottom: 15px;'>%1</h3>"
                            "<table cellspacing='12' style='font-size: 13px;'>"
                            "<tr><td style='font-weight: bold; color: #4a5568;'>Client:</td><td style='color: #2d3748;'>%2</td></tr>"
                            "<tr><td style='font-weight: bold; color: #4a5568;'>Adresse chantier:</td><td style='color: #2d3748;'>%3</td></tr>"
                            "<tr><td style='font-weight: bold; color: #4a5568;'>Type:</td><td style='color: #2d3748;'>%4</td></tr>"
                            "<tr><td style='font-weight: bold; color: #4a5568;'>Date début:</td><td style='color: #2d3748;'>%5</td></tr>"
                            "<tr><td style='font-weight: bold; color: #4a5568;'>Date fin:</td><td style='color: #2d3748;'>%6</td></tr>"
                            "<tr><td style='font-weight: bold; color: #4a5568;'>Statut:</td><td><span style='font-weight: bold; color: %7;'>%8</span></td></tr>"
                            "<tr><td style='font-weight: bold; color: #4a5568;'>Budget:</td><td style='color: #2d3748; font-weight: bold;'>%9</td></tr>"
                            "</table>"
                            "</div>"
                            ).arg(nomProjet, client, adresse, type, dateDebut, dateFin, statutColor, statut, budget));

        details.exec();
    });

    // ────────────────────────────────────────────────
    // Add everything to layout
    // ────────────────────────────────────────────────
    layout->addLayout(actionsLayout);
    layout->addWidget(table);

    return page;
}
QWidget* MainWindow::createEmployeesPage()
{
    // Uses EmployeeManagementPage which reads live data from Oracle DB
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    EmployeeManagementPage *empPage = new EmployeeManagementPage(page);
    layout->addWidget(empPage);
    return page;
}


QWidget* MainWindow::createFinancePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setSpacing(18);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ────────────────────────────────────────────────
    // Cartes statistiques principales
    // ────────────────────────────────────────────────
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);

    struct StatInfo {
        QString title;
        QString property;
    };

    const QList<StatInfo> stats = {
        {"REVENUS",  "income"},
        {"DÉPENSES", "expense"},
        {"BÉNÉFICE", "profit"}
    };

    QList<QLabel*> valueLabels;

    for (const auto &s : stats) {
        QFrame *card = new QFrame(page);
        card->setObjectName("statCard");
        card->setProperty("type", s.property);

        QVBoxLayout *cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(20, 20, 20, 20);
        cardLay->setSpacing(10);

        QLabel *titleLbl = new QLabel(s.title, card);
        titleLbl->setObjectName("statTitle");

        QLabel *valLbl = new QLabel("0 DT", card);
        valLbl->setObjectName("statValue");
        valueLabels.append(valLbl);

        cardLay->addWidget(titleLbl);
        cardLay->addWidget(valLbl);
        cardLay->addStretch();

        statsLayout->addWidget(card);
    }

    mainLayout->addLayout(statsLayout);

    // ────────────────────────────────────────────────
    // Barre de filtres
    // ────────────────────────────────────────────────
    QFrame *filterFrame = new QFrame(page);
    filterFrame->setObjectName("searchFrame");

    QHBoxLayout *filterLay = new QHBoxLayout(filterFrame);
    filterLay->setContentsMargins(0, 0, 0, 10);
    filterLay->setSpacing(10);

    QLabel *lblSearch = new QLabel("Rechercher :", filterFrame);
    lblSearch->setObjectName("searchLabel");

    QLineEdit *searchEdit = new QLineEdit(filterFrame);
    searchEdit->setObjectName("financeSearch");
    searchEdit->setPlaceholderText("Client, type, catégorie...");
    searchEdit->setMinimumWidth(260);

    QComboBox *cbType = new QComboBox(filterFrame);
    cbType->setObjectName("filterCombo");
    cbType->addItems({"Tous", "Facture", "Devis", "Acompte"});

    QComboBox *cbCategory = new QComboBox(filterFrame);
    cbCategory->setObjectName("filterCombo");
    cbCategory->addItems({"Toutes", "Recette", "Dépense"});

    QComboBox *cbStatus = new QComboBox(filterFrame);
    cbStatus->setObjectName("filterCombo");
    cbStatus->addItems({"Tous", "Payé", "En attente", "Retard"});

    QDateEdit *debutDate = new QDateEdit(filterFrame);
    debutDate->setObjectName("dateFilter");
    debutDate->setCalendarPopup(true);
    debutDate->setDate(QDate::currentDate().addMonths(-1));
    debutDate->setDisplayFormat("dd/MM/yyyy");

    QDateEdit *finDate = new QDateEdit(filterFrame);
    finDate->setObjectName("dateFilter");
    finDate->setCalendarPopup(true);
    finDate->setDate(QDate::currentDate());
    finDate->setDisplayFormat("dd/MM/yyyy");

    QPushButton *btnFiltrer = new QPushButton("Filtrer", filterFrame);
    btnFiltrer->setObjectName("searchButton");
    btnFiltrer->setCursor(Qt::PointingHandCursor);

    QPushButton *btnReset = new QPushButton("Réinitialiser", filterFrame);
    btnReset->setObjectName("resetButton");
    btnReset->setCursor(Qt::PointingHandCursor);

    filterLay->addWidget(lblSearch);
    filterLay->addWidget(searchEdit);
    filterLay->addWidget(cbType);
    filterLay->addWidget(cbCategory);
    filterLay->addWidget(cbStatus);
    filterLay->addWidget(new QLabel("Du :"));
    filterLay->addWidget(debutDate);
    filterLay->addWidget(new QLabel("Au :"));
    filterLay->addWidget(finDate);
    filterLay->addWidget(btnFiltrer);
    filterLay->addWidget(btnReset);
    filterLay->addStretch();

    mainLayout->addWidget(filterFrame);

    // ────────────────────────────────────────────────
    // Tableau
    // ────────────────────────────────────────────────
    financeTable = new QTableWidget(page);
    financeTable->setObjectName("financeTable");
    financeTable->setColumnCount(6);
    financeTable->setHorizontalHeaderLabels({
        "CLIENT", "TYPE", "CATÉGORIE", "MONTANT (DT)", "STATUT", "DATE"
    });

    financeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    financeTable->verticalHeader()->setVisible(false);
    financeTable->setAlternatingRowColors(true);
    financeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    financeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    financeTable->setShowGrid(false);
    financeTable->setSortingEnabled(false);

    // Données exemple
    const QList<QStringList> exemples = {
        {"M. Dupont",       "Facture",   "Recette",   "3500",  "Payé",       "01/02/2026"},
        {"Mme Martin",      "Devis",     "Recette",   "2800",  "En attente", "10/02/2026"},
        {"Restaurant Le Bois","Acompte", "Recette",   "1500",  "Payé",       "15/01/2026"},
        {"M. Bernard",      "Facture",   "Dépense",   "1200",  "Retard",     "05/02/2026"},
        {"SARL Dubois",     "Facture",   "Recette",   "5200",  "Payé",       "20/01/2026"}
    };

    financeTable->setRowCount(exemples.size());
    for (int r = 0; r < exemples.size(); ++r) {
        for (int c = 0; c < 6; ++c) {
            QString txt = exemples[r][c];
            if (c == 3) txt += " DT";
            financeTable->setItem(r, c, new QTableWidgetItem(txt));
        }
        financeTable->setRowHeight(r, 52);
    }

    mainLayout->addWidget(financeTable, 1);

    // ────────────────────────────────────────────────
    // Mise à jour statistiques
    // ────────────────────────────────────────────────
    auto updateStats = [this, valueLabels]() {
        double revenus = 0.0;
        double depenses = 0.0;

        for (int r = 0; r < financeTable->rowCount(); ++r) {
            if (financeTable->isRowHidden(r)) continue;

            auto itemMontant = financeTable->item(r, 3);
            if (!itemMontant) continue;

            QString m = itemMontant->text()
                            .replace(" DT", "")
                            .replace(" ", "")
                            .trimmed();

            bool ok;
            double montant = m.toDouble(&ok);
            if (!ok) continue;

            QString cat = financeTable->item(r, 2)->text();

            if (cat == "Recette") revenus += montant;
            else if (cat == "Dépense") depenses += montant;
        }

        if (valueLabels.size() >= 3) {
            valueLabels[0]->setText(QString("%L1 DT").arg(revenus, 0, 'f', 2));
            valueLabels[1]->setText(QString("%L1 DT").arg(depenses, 0, 'f', 2));
            valueLabels[2]->setText(QString("%L1 DT").arg(revenus - depenses, 0, 'f', 2));
        }
    };

    updateStats();

    // ────────────────────────────────────────────────
    // Barre d'actions
    // ────────────────────────────────────────────────
    QHBoxLayout *actionBar = new QHBoxLayout();
    actionBar->setSpacing(12);

    QPushButton *btnAjouter   = new QPushButton("+ Nouvelle Transaction", page);
    QPushButton *btnSupprimer = new QPushButton("Supprimer", page);
    QPushButton *btnExporter  = new QPushButton("Exporter CSV", page);
    QPushButton *btnStats     = new QPushButton("Statistiques détaillées", page);

    for (auto b : {btnAjouter, btnSupprimer, btnExporter, btnStats}) {
        b->setObjectName("actionButton");
        b->setCursor(Qt::PointingHandCursor);
    }

    QComboBox *triCombo = new QComboBox(page);
    triCombo->setObjectName("sortCombo");
    triCombo->addItems({
        "Tri par défaut",
        "Montant ↑",
        "Montant ↓",
        "Date ↓ (récent)",
        "Date ↑ (ancien)"
    });
    triCombo->setMinimumWidth(210);

    actionBar->addWidget(btnAjouter);
    actionBar->addWidget(btnSupprimer);
    actionBar->addWidget(btnExporter);
    actionBar->addWidget(btnStats);
    actionBar->addSpacing(16);
    actionBar->addWidget(new QLabel("Trier par :"));
    actionBar->addWidget(triCombo);
    actionBar->addStretch();

    mainLayout->addLayout(actionBar);

    // ────────────────────────────────────────────────
    // Logique filtres
    // ────────────────────────────────────────────────
    auto doFilter = [=]() {
        QString recherche = searchEdit->text().trimmed().toLower();
        QString typeSel   = cbType->currentText();
        QString catSel    = cbCategory->currentText();
        QString statutSel = cbStatus->currentText();
        QDate d1 = debutDate->date();
        QDate d2 = finDate->date();

        for (int r = 0; r < financeTable->rowCount(); ++r) {
            bool visible = true;

            if (!recherche.isEmpty()) {
                bool trouve = false;
                for (int c = 0; c < 3; ++c) {
                    auto it = financeTable->item(r, c);
                    if (it && it->text().toLower().contains(recherche)) {
                        trouve = true;
                        break;
                    }
                }
                if (!trouve) visible = false;
            }

            if (visible && typeSel   != "Tous"   && financeTable->item(r,1)->text() != typeSel)   visible = false;
            if (visible && catSel    != "Toutes" && financeTable->item(r,2)->text() != catSel)   visible = false;
            if (visible && statutSel != "Tous"   && financeTable->item(r,4)->text() != statutSel) visible = false;

            if (visible) {
                auto dateIt = financeTable->item(r, 5);
                if (dateIt) {
                    QDate dt = QDate::fromString(dateIt->text(), "dd/MM/yyyy");
                    if (dt.isValid() && (dt < d1 || dt > d2)) visible = false;
                }
            }

            financeTable->setRowHidden(r, !visible);
        }

        updateStats();
    };

    // ────────────────────────────────────────────────
    // Logique tri (simple réorganisation visuelle)
    // ────────────────────────────────────────────────
    auto doSort = [=](int idx) {
        if (idx == 0) return;

        struct Ligne {
            int row;
            double montant = 0.0;
            QDate date;
        };

        QList<Ligne> lignes;
        for (int r = 0; r < financeTable->rowCount(); ++r) {
            if (!financeTable->isRowHidden(r)) {
                auto mIt = financeTable->item(r, 3);
                auto dIt = financeTable->item(r, 5);
                if (mIt && dIt) {
                    QString mStr = mIt->text().replace(" DT","").trimmed();
                    lignes << Ligne{r, mStr.toDouble(), QDate::fromString(dIt->text(), "dd/MM/yyyy")};
                }
            }
        }

        std::stable_sort(lignes.begin(), lignes.end(), [idx](const Ligne &a, const Ligne &b){
            if (idx == 1) return a.montant < b.montant;
            if (idx == 2) return a.montant > b.montant;
            if (idx == 3) return a.date > b.date;
            if (idx == 4) return a.date < b.date;
            return false;
        });

        int nouvelleLigne = 0;
        for (const auto &l : lignes) {
            for (int c = 0; c < 6; ++c) {
                auto item = financeTable->takeItem(l.row, c);
                financeTable->setItem(nouvelleLigne, c, item);
            }
            financeTable->setRowHeight(nouvelleLigne, 52);
            nouvelleLigne++;
        }
    };

    // ────────────────────────────────────────────────
    // Connexions
    // ────────────────────────────────────────────────
    connect(btnFiltrer, &QPushButton::clicked, doFilter);
    connect(btnReset, &QPushButton::clicked, [=](){
        searchEdit->clear();
        cbType->setCurrentIndex(0);
        cbCategory->setCurrentIndex(0);
        cbStatus->setCurrentIndex(0);
        debutDate->setDate(QDate::currentDate().addMonths(-1));
        finDate->setDate(QDate::currentDate());
        for (int r = 0; r < financeTable->rowCount(); ++r) {
            financeTable->setRowHidden(r, false);
        }
        updateStats();
    });

    connect(triCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), doSort);

    // Ajouter transaction
    connect(btnAjouter, &QPushButton::clicked, [=](){
        QDialog dlg(this);
        dlg.setWindowTitle("Nouvelle Transaction");
        dlg.setMinimumWidth(420);

        QFormLayout form(&dlg);

        QLineEdit *clientEdit = new QLineEdit(&dlg);
        QComboBox *typeCb = new QComboBox(&dlg);
        QComboBox *catCb = new QComboBox(&dlg);
        QLineEdit *montantEdit = new QLineEdit(&dlg);
        QComboBox *statutCb = new QComboBox(&dlg);
        QDateEdit *dateEdit = new QDateEdit(QDate::currentDate(), &dlg);

        typeCb->addItems({"Facture", "Devis", "Acompte"});
        catCb->addItems({"Recette", "Dépense"});
        statutCb->addItems({"Payé", "En attente", "Retard"});
        dateEdit->setCalendarPopup(true);
        dateEdit->setDisplayFormat("dd/MM/yyyy");

        form.addRow("Client :", clientEdit);
        form.addRow("Type :", typeCb);
        form.addRow("Catégorie :", catCb);
        form.addRow("Montant (DT) :", montantEdit);
        form.addRow("Statut :", statutCb);
        form.addRow("Date :", dateEdit);

        QDialogButtonBox *box = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        form.addRow(box);

        connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

        if (dlg.exec() == QDialog::Accepted) {
            int row = financeTable->rowCount();
            financeTable->insertRow(row);

            financeTable->setItem(row, 0, new QTableWidgetItem(clientEdit->text()));
            financeTable->setItem(row, 1, new QTableWidgetItem(typeCb->currentText()));
            financeTable->setItem(row, 2, new QTableWidgetItem(catCb->currentText()));
            financeTable->setItem(row, 3, new QTableWidgetItem(montantEdit->text() + " DT"));
            financeTable->setItem(row, 4, new QTableWidgetItem(statutCb->currentText()));
            financeTable->setItem(row, 5, new QTableWidgetItem(
                dateEdit->date().toString("dd/MM/yyyy")));

            financeTable->setRowHeight(row, 52);
            updateStats();
        }
    });

    // Modifier (double-clic)
    connect(financeTable, &QTableWidget::cellDoubleClicked,
            [=](int row, int){
        QDialog dlg(this);
        dlg.setWindowTitle("Modifier Transaction");
        dlg.setMinimumWidth(420);

        QFormLayout form(&dlg);

        QString montantTxt = financeTable->item(row,3)->text()
                                 .replace(" DT","").trimmed();

        QLineEdit *clientEdit = new QLineEdit(financeTable->item(row,0)->text(), &dlg);
        QComboBox *typeCb = new QComboBox(&dlg);
        QComboBox *catCb = new QComboBox(&dlg);
        QLineEdit *montantEdit = new QLineEdit(montantTxt, &dlg);
        QComboBox *statutCb = new QComboBox(&dlg);
        QDateEdit *dateEdit = new QDateEdit(&dlg);

        typeCb->addItems({"Facture", "Devis", "Acompte"});
        typeCb->setCurrentText(financeTable->item(row,1)->text());

        catCb->addItems({"Recette", "Dépense"});
        catCb->setCurrentText(financeTable->item(row,2)->text());

        statutCb->addItems({"Payé", "En attente", "Retard"});
        statutCb->setCurrentText(financeTable->item(row,4)->text());

        dateEdit->setDate(QDate::fromString(financeTable->item(row,5)->text(), "dd/MM/yyyy"));
        dateEdit->setCalendarPopup(true);
        dateEdit->setDisplayFormat("dd/MM/yyyy");

        form.addRow("Client :", clientEdit);
        form.addRow("Type :", typeCb);
        form.addRow("Catégorie :", catCb);
        form.addRow("Montant (DT) :", montantEdit);
        form.addRow("Statut :", statutCb);
        form.addRow("Date :", dateEdit);

        QDialogButtonBox *box = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        form.addRow(box);

        connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

        if (dlg.exec() == QDialog::Accepted) {
            financeTable->item(row,0)->setText(clientEdit->text());
            financeTable->item(row,1)->setText(typeCb->currentText());
            financeTable->item(row,2)->setText(catCb->currentText());
            financeTable->item(row,3)->setText(montantEdit->text() + " DT");
            financeTable->item(row,4)->setText(statutCb->currentText());
            financeTable->item(row,5)->setText(dateEdit->date().toString("dd/MM/yyyy"));
            updateStats();
        }
    });

    // Supprimer
    connect(btnSupprimer, &QPushButton::clicked, [=](){
        int row = financeTable->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Aucune sélection", "Sélectionnez une ligne.");
            return;
        }

        if (QMessageBox::question(this, "Confirmer",
                                  "Supprimer cette transaction ?",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            financeTable->removeRow(row);
            updateStats();
        }
    });

    // Exporter CSV
    connect(btnExporter, &QPushButton::clicked, [=](){
        QString fichier = QFileDialog::getSaveFileName(this,
            "Exporter en CSV",
            "transactions_" + QDate::currentDate().toString("yyyyMMdd") + ".csv",
            "Fichiers CSV (*.csv);;Tous les fichiers (*.*)");

        if (fichier.isEmpty()) return;

        QFile file(fichier);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Erreur", "Impossible d'écrire le fichier.");
            return;
        }

        QTextStream out(&file);
        QStringList entetes;
        for (int c = 0; c < financeTable->columnCount(); ++c) {
            entetes << financeTable->horizontalHeaderItem(c)->text();
        }
        out << entetes.join(";") << "\n";

        for (int r = 0; r < financeTable->rowCount(); ++r) {
            if (financeTable->isRowHidden(r)) continue;
            QStringList ligne;
            for (int c = 0; c < financeTable->columnCount(); ++c) {
                auto it = financeTable->item(r, c);
                ligne << (it ? it->text() : "");
            }
            out << ligne.join(";") << "\n";
        }

        file.close();
        QMessageBox::information(this, "Succès", "Export terminé.");
    });

    // Statistiques détaillées
    connect(btnStats, &QPushButton::clicked, [=](){
        QDialog *dlg = new QDialog(this);
        dlg->setWindowTitle("Statistiques détaillées");
        dlg->setMinimumSize(520, 480);

        QVBoxLayout *lay = new QVBoxLayout(dlg);

        double totRevenus = 0, totDepenses = 0;
        double totPaye = 0, totAttente = 0, totRetard = 0;
        int cntPaye = 0, cntAttente = 0, cntRetard = 0;

        QMap<QString, double> revenusMensuels, depensesMensuels;

        for (int r = 0; r < financeTable->rowCount(); ++r) {
            if (financeTable->isRowHidden(r)) continue;

            auto montantIt = financeTable->item(r, 3);
            auto catIt     = financeTable->item(r, 2);
            auto statutIt  = financeTable->item(r, 4);
            auto dateIt    = financeTable->item(r, 5);

            if (!montantIt || !catIt || !statutIt || !dateIt) continue;

            QString mStr = montantIt->text().replace(" DT","").trimmed();
            double montant = mStr.toDouble();

            QString cat = catIt->text();
            QString statut = statutIt->text();
            QString mois = dateIt->text().right(7); // MM/yyyy

            if (cat == "Recette") {
                totRevenus += montant;
                revenusMensuels[mois] += montant;
            } else if (cat == "Dépense") {
                totDepenses += montant;
                depensesMensuels[mois] += montant;
            }

            if (statut == "Payé")       { totPaye += montant; cntPaye++;     }
            else if (statut == "En attente") { totAttente += montant; cntAttente++; }
            else if (statut == "Retard")     { totRetard += montant; cntRetard++;   }
        }

        // Résumé général
        QFrame *resume = new QFrame(dlg);
        resume->setObjectName("statsSummary");
        QGridLayout *gridResume = new QGridLayout(resume);

        gridResume->addWidget(new QLabel("<b>Résumé financier</b>"), 0, 0, 1, 2);
        gridResume->addWidget(new QLabel("Total revenus :"), 1, 0);
        gridResume->addWidget(new QLabel(QString("%L1 DT").arg(totRevenus,0,'f',2)), 1, 1);
        gridResume->addWidget(new QLabel("Total dépenses :"), 2, 0);
        gridResume->addWidget(new QLabel(QString("%L1 DT").arg(totDepenses,0,'f',2)), 2, 1);
        gridResume->addWidget(new QLabel("Bénéfice net :"), 3, 0);
        QLabel *profitLbl = new QLabel(QString("%L1 DT").arg(totRevenus - totDepenses,0,'f',2));
        profitLbl->setStyleSheet("color:#27ae60; font-weight:bold;");
        gridResume->addWidget(profitLbl, 3, 1);

        lay->addWidget(resume);

        // Statut des paiements
        QFrame *statutFrame = new QFrame(dlg);
        statutFrame->setObjectName("statsPayment");
        QGridLayout *gridStatut = new QGridLayout(statutFrame);

        gridStatut->addWidget(new QLabel("<b>Statut des paiements</b>"), 0, 0, 1, 3);
        gridStatut->addWidget(new QLabel("Statut"), 1, 0);
        gridStatut->addWidget(new QLabel("Montant"), 1, 1);
        gridStatut->addWidget(new QLabel("Nombre"), 1, 2);

        gridStatut->addWidget(new QLabel("Payé"), 2, 0);
        gridStatut->addWidget(new QLabel(QString("%L1 DT").arg(totPaye,0,'f',2)), 2, 1);
        gridStatut->addWidget(new QLabel(QString::number(cntPaye)), 2, 2);

        gridStatut->addWidget(new QLabel("En attente"), 3, 0);
        gridStatut->addWidget(new QLabel(QString("%L1 DT").arg(totAttente,0,'f',2)), 3, 1);
        gridStatut->addWidget(new QLabel(QString::number(cntAttente)), 3, 2);

        gridStatut->addWidget(new QLabel("Retard"), 4, 0);
        gridStatut->addWidget(new QLabel(QString("%L1 DT").arg(totRetard,0,'f',2)), 4, 1);
        gridStatut->addWidget(new QLabel(QString::number(cntRetard)), 4, 2);

        lay->addWidget(statutFrame);

        // Évolution mensuelle
        QFrame *mensuelFrame = new QFrame(dlg);
        mensuelFrame->setObjectName("statsMonthly");
        QVBoxLayout *mensuelLay = new QVBoxLayout(mensuelFrame);

        mensuelLay->addWidget(new QLabel("<b>Évolution mensuelle</b>"));

        QStringList moisListe = revenusMensuels.keys();
        std::sort(moisListe.begin(), moisListe.end());

        for (const QString &m : moisListe) {
            QFrame *ligne = new QFrame(mensuelFrame);
            QHBoxLayout *l = new QHBoxLayout(ligne);
            l->addWidget(new QLabel(m));
            l->addStretch();
            l->addWidget(new QLabel(QString("Revenus : %L1 DT").arg(revenusMensuels[m],0,'f',2)));
            l->addWidget(new QLabel(QString("Dépenses : %L1 DT").arg(depensesMensuels.value(m,0),0,'f',2)));
            mensuelLay->addWidget(ligne);
        }

        lay->addWidget(mensuelFrame);

        QPushButton *close = new QPushButton("Fermer", dlg);
        close->setObjectName("actionButton");
        connect(close, &QPushButton::clicked, dlg, &QDialog::accept);
        lay->addWidget(close, 0, Qt::AlignRight);

        dlg->exec();
    });

    return page;
}
QWidget* MainWindow::createProductsPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(18);

    // Statistics cards
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);

    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {
        {"PRODUITS ACTIFS", "45", "active"},
        {"EN RÉAPPROVISIONNEMENT", "8", "pending"},
        {"NOUVEAUTÉS CE MOIS", "12", "new"}
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
    QPushButton *addBtn = new QPushButton("+ Nouveau Produit", page);
    QPushButton *editBtn = new QPushButton("Modifier", page);
    QPushButton *deleteBtn = new QPushButton("Supprimer", page);
    QPushButton *exportBtn = new QPushButton("Exporter PDF", page);

    addBtn->setObjectName("actionButton");
    editBtn->setObjectName("actionButton");
    deleteBtn->setObjectName("actionButton");
    exportBtn->setObjectName("actionButton");

    addBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    exportBtn->setCursor(Qt::PointingHandCursor);

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddProductClicked);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::onEditProductClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteProductClicked);

    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(editBtn);
    actionsLayout->addWidget(deleteBtn);
    actionsLayout->addWidget(exportBtn);
    actionsLayout->addStretch();

    // Search bar for products
    QLineEdit *searchBar = new QLineEdit(page);
    searchBar->setObjectName("productSearch");
    searchBar->setPlaceholderText("Rechercher un produit par ID, nom ou catégorie...");
    searchBar->setMinimumHeight(38);
    searchBar->setMaximumWidth(400);

    connect(searchBar, &QLineEdit::textChanged, this, [this](const QString &text) {
        QWidget *productsPage = stackedWidget->widget(4);
        QTableWidget *table = productsPage->findChild<QTableWidget*>("productsTable");
        if (table) {
            for (int row = 0; row < table->rowCount(); ++row) {
                bool match = false;
                for (int col = 0; col < 3; ++col) { // Chercher dans ID, Nom, Catégorie
                    if (table->item(row, col) && table->item(row, col)->text().contains(text, Qt::CaseInsensitive)) {
                        match = true;
                        break;
                    }
                }
                table->setRowHidden(row, !match);
            }
        }
    });

    actionsLayout->addWidget(searchBar);

    // Table
    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("productsTable");
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({
        "ID PRODUIT",
        "NOM",
        "CATÉGORIE",
        "PRIX",
        "DIMENSIONS",
        "MATÉRIAUX",
        "DATE CRÉATION",
        "IMAGE"
    });

    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);

    // Sample data
    table->setRowCount(4);

    // Produit 1
    table->setItem(0, 0, new QTableWidgetItem("PROD-001"));
    table->setItem(0, 1, new QTableWidgetItem("Table à manger en chêne"));
    table->setItem(0, 2, new QTableWidgetItem("Meuble"));
    table->setItem(0, 3, new QTableWidgetItem("850 EUR"));
    table->setItem(0, 4, new QTableWidgetItem("200 x 90 x 75 cm"));
    table->setItem(0, 5, new QTableWidgetItem("Chêne massif, Vernis"));
    table->setItem(0, 6, new QTableWidgetItem("15/01/2026"));
    table->setItem(0, 7, new QTableWidgetItem("assets/images/table_chene.jpg"));

    // Produit 2
    table->setItem(1, 0, new QTableWidgetItem("PROD-002"));
    table->setItem(1, 1, new QTableWidgetItem("Bibliothèque moderne"));
    table->setItem(1, 2, new QTableWidgetItem("Meuble"));
    table->setItem(1, 3, new QTableWidgetItem("650 EUR"));
    table->setItem(1, 4, new QTableWidgetItem("120 x 30 x 180 cm"));
    table->setItem(1, 5, new QTableWidgetItem("MDF, Verre trempé"));
    table->setItem(1, 6, new QTableWidgetItem("20/01/2026"));
    table->setItem(1, 7, new QTableWidgetItem("assets/images/bibliotheque.jpg"));

    // Produit 3
    table->setItem(2, 0, new QTableWidgetItem("PROD-003"));
    table->setItem(2, 1, new QTableWidgetItem("Porte d'entrée bois"));
    table->setItem(2, 2, new QTableWidgetItem("Porte"));
    table->setItem(2, 3, new QTableWidgetItem("1200 EUR"));
    table->setItem(2, 4, new QTableWidgetItem("90 x 210 cm"));
    table->setItem(2, 5, new QTableWidgetItem("Chêne, Double vitrage"));
    table->setItem(2, 6, new QTableWidgetItem("05/02/2026"));
    table->setItem(2, 7, new QTableWidgetItem("assets/images/porte_chene.jpg"));

    // Produit 4
    table->setItem(3, 0, new QTableWidgetItem("PROD-004"));
    table->setItem(3, 1, new QTableWidgetItem("Étagère murale"));
    table->setItem(3, 2, new QTableWidgetItem("Décoration"));
    table->setItem(3, 3, new QTableWidgetItem("120 EUR"));
    table->setItem(3, 4, new QTableWidgetItem("80 x 20 x 20 cm"));
    table->setItem(3, 5, new QTableWidgetItem("Bois de pin"));
    table->setItem(3, 6, new QTableWidgetItem("10/02/2026"));
    table->setItem(3, 7, new QTableWidgetItem("assets/images/etagere.jpg"));

    for (int row = 0; row < 4; ++row) {
        table->setRowHeight(row, 50);
    }

    // Double-click to view details
    connect(table, &QTableWidget::cellDoubleClicked, this, [this, table](int row, int) {
        if (!table || row < 0 || row >= table->rowCount()) return;

        QString idProd = table->item(row, 0) ? table->item(row, 0)->text() : "Non spécifié";
        QString nomProd = table->item(row, 1) ? table->item(row, 1)->text() : "Non spécifié";
        QString categorie = table->item(row, 2) ? table->item(row, 2)->text() : "Non spécifié";
        QString prix = table->item(row, 3) ? table->item(row, 3)->text() : "Non spécifié";
        QString dimensions = table->item(row, 4) ? table->item(row, 4)->text() : "Non spécifié";
        QString materiaux = table->item(row, 5) ? table->item(row, 5)->text() : "Non spécifié";
        QString dateCreation = table->item(row, 6) ? table->item(row, 6)->text() : "Non spécifié";
        QString image = table->item(row, 7) ? table->item(row, 7)->text() : "Non spécifié";

        QMessageBox details(this);
        details.setWindowTitle("Détails du Produit");
        details.setIcon(QMessageBox::Information);
        details.setText(QString(
                            "<div style='font-family: Arial;'>"
                            "<h3 style='color: #8A9A5B;'>%1</h3>"
                            "<table cellspacing='10'>"
                            "<tr><td><b>ID Produit:</b></td><td>%2</td></tr>"
                            "<tr><td><b>Nom:</b></td><td>%3</td></tr>"
                            "<tr><td><b>Catégorie:</b></td><td>%4</td></tr>"
                            "<tr><td><b>Prix:</b></td><td style='color: #27ae60; font-weight: bold;'>%5</td></tr>"
                            "<tr><td><b>Dimensions:</b></td><td>%6</td></tr>"
                            "<tr><td><b>Matériaux utilisés:</b></td><td>%7</td></tr>"
                            "<tr><td><b>Date création:</b></td><td>%8</td></tr>"
                            "<tr><td><b>Image:</b></td><td>%9</td></tr>"
                            "</table>"
                            "</div>"
                            ).arg(nomProd, idProd, nomProd, categorie, prix, dimensions, materiaux, dateCreation, image));

        details.exec();
    });

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

// Replace these functions in mainwindow.cpp

void MainWindow::onAddButtonClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un Nouveau Projet");
    dialog.setMinimumWidth(500);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel *titleLabel = new QLabel("Nouveau Projet", &dialog);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Form layout
    QFormLayout *form = new QFormLayout();
    form->setSpacing(15);
    form->setLabelAlignment(Qt::AlignRight);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // Form fields
    QLineEdit *nomProjet = new QLineEdit(&dialog);
    nomProjet->setPlaceholderText("Nom du projet");
    nomProjet->setMinimumHeight(35);

    QLineEdit *client = new QLineEdit(&dialog);
    client->setPlaceholderText("Nom du client");
    client->setMinimumHeight(35);

    QLineEdit *adresse = new QLineEdit(&dialog);
    adresse->setPlaceholderText("Adresse complète du chantier");
    adresse->setMinimumHeight(35);

    QComboBox *typeProjet = new QComboBox(&dialog);
    typeProjet->addItems({"Meuble sur mesure", "Rénovation", "Agencement", "Restaurant", "Bureau", "Cuisine", "Salle de bain"});
    typeProjet->setMinimumHeight(35);

    QDateEdit *dateDebut = new QDateEdit(&dialog);
    dateDebut->setDate(QDate::currentDate());
    dateDebut->setCalendarPopup(true);
    dateDebut->setDisplayFormat("dd/MM/yyyy");
    dateDebut->setMinimumHeight(35);

    QDateEdit *dateFin = new QDateEdit(&dialog);
    dateFin->setDate(QDate::currentDate().addDays(30));
    dateFin->setCalendarPopup(true);
    dateFin->setDisplayFormat("dd/MM/yyyy");
    dateFin->setMinimumHeight(35);

    QComboBox *statut = new QComboBox(&dialog);
    statut->addItems({"En cours", "En attente", "Terminé", "Annulé"});
    statut->setMinimumHeight(35);

    QLineEdit *budget = new QLineEdit(&dialog);
    budget->setPlaceholderText("0.00 EUR");
    budget->setMinimumHeight(35);

    // Add rows to form
    form->addRow("Nom du projet:", nomProjet);
    form->addRow("Client:", client);
    form->addRow("Adresse chantier:", adresse);
    form->addRow("Type:", typeProjet);
    form->addRow("Date début:", dateDebut);
    form->addRow("Date fin:", dateFin);
    form->addRow("Statut:", statut);
    form->addRow("Budget:", budget);

    mainLayout->addLayout(form);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Ajouter");
    buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton { background-color: #8A9A5B; color: white; border: none; border-radius: 5px; padding: 8px 20px; font-weight: bold; }"
        "QPushButton:hover { background-color: #9aaa6b; }"
        );
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Annuler");
    buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton { background-color: #e2e8f0; color: #4a5568; border: none; border-radius: 5px; padding: 8px 20px; }"
        "QPushButton:hover { background-color: #cbd5e0; }"
        );

    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        // Get the projects page and its table
        QWidget *projectsPage = stackedWidget->widget(0);
        QTableWidget *table = projectsPage->findChild<QTableWidget*>("dataTable");

        if (table) {
            int row = table->rowCount();
            table->insertRow(row);

            // Format budget
            QString budgetText = budget->text();
            if (!budgetText.contains("EUR")) {
                budgetText = budgetText + " EUR";
            }

            // Set values in table
            table->setItem(row, 0, new QTableWidgetItem(client->text()));
            table->setItem(row, 1, new QTableWidgetItem(typeProjet->currentText()));
            table->setItem(row, 2, new QTableWidgetItem(dateDebut->date().toString("dd/MM/yyyy")));
            table->setItem(row, 3, new QTableWidgetItem(dateFin->date().toString("dd/MM/yyyy")));

            // Create statut item with color
            QTableWidgetItem *statutItem = new QTableWidgetItem(statut->currentText());
            if (statut->currentText() == "En cours") {
                statutItem->setForeground(QBrush(QColor("#2ecc71")));
            } else if (statut->currentText() == "En attente") {
                statutItem->setForeground(QBrush(QColor("#f39c12")));
            } else if (statut->currentText() == "Terminé") {
                statutItem->setForeground(QBrush(QColor("#3498db")));
            } else {
                statutItem->setForeground(QBrush(QColor("#e74c3c")));
            }
            table->setItem(row, 4, statutItem);

            table->setItem(row, 5, new QTableWidgetItem(budgetText));

            // Add hidden columns for additional data
            table->setColumnCount(8); // Increase column count
            if (table->columnCount() > 6) {
                table->setHorizontalHeaderLabels({"CLIENT", "TYPE", "DATE DEBUT", "DATE FIN", "STATUT", "BUDGET", "NOM PROJET", "ADRESSE"});
                table->setItem(row, 6, new QTableWidgetItem(nomProjet->text()));
                table->setItem(row, 7, new QTableWidgetItem(adresse->text()));
            }

            table->setRowHeight(row, 50);

            QMessageBox::information(this, "Succès", "Projet ajouté avec succès!");
        }
    }
}

void MainWindow::onEditButtonClicked()
{
    // Get the projects page and its table
    QWidget *projectsPage = stackedWidget->widget(0);
    QTableWidget *table = projectsPage->findChild<QTableWidget*>("dataTable");

    if (!table || table->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un projet à modifier.");
        return;
    }

    int row = table->currentRow();

    // Get existing values
    QString clientValue = table->item(row, 0) ? table->item(row, 0)->text() : "";
    QString typeValue = table->item(row, 1) ? table->item(row, 1)->text() : "";
    QString dateDebutValue = table->item(row, 2) ? table->item(row, 2)->text() : QDate::currentDate().toString("dd/MM/yyyy");
    QString dateFinValue = table->item(row, 3) ? table->item(row, 3)->text() : QDate::currentDate().addDays(30).toString("dd/MM/yyyy");
    QString statutValue = table->item(row, 4) ? table->item(row, 4)->text() : "En cours";
    QString budgetValue = table->item(row, 5) ? table->item(row, 5)->text().replace(" EUR", "") : "";

    // Get hidden values if available
    QString nomProjetValue = (table->columnCount() > 6 && table->item(row, 6)) ? table->item(row, 6)->text() : "";
    QString adresseValue = (table->columnCount() > 7 && table->item(row, 7)) ? table->item(row, 7)->text() : "";

    QDialog dialog(this);
    dialog.setWindowTitle("Modifier le Projet");
    dialog.setMinimumWidth(500);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel *titleLabel = new QLabel("Modifier le Projet", &dialog);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Form layout
    QFormLayout *form = new QFormLayout();
    form->setSpacing(15);
    form->setLabelAlignment(Qt::AlignRight);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // Form fields with existing values
    QLineEdit *nomProjet = new QLineEdit(nomProjetValue, &dialog);
    nomProjet->setPlaceholderText("Nom du projet");
    nomProjet->setMinimumHeight(35);

    QLineEdit *client = new QLineEdit(clientValue, &dialog);
    client->setPlaceholderText("Nom du client");
    client->setMinimumHeight(35);

    QLineEdit *adresse = new QLineEdit(adresseValue, &dialog);
    adresse->setPlaceholderText("Adresse complète du chantier");
    adresse->setMinimumHeight(35);

    QComboBox *typeProjet = new QComboBox(&dialog);
    typeProjet->addItems({"Meuble sur mesure", "Rénovation", "Agencement", "Restaurant", "Bureau", "Cuisine", "Salle de bain"});
    typeProjet->setCurrentText(typeValue);
    typeProjet->setMinimumHeight(35);

    QDateEdit *dateDebut = new QDateEdit(&dialog);
    dateDebut->setDate(QDate::fromString(dateDebutValue, "dd/MM/yyyy"));
    dateDebut->setCalendarPopup(true);
    dateDebut->setDisplayFormat("dd/MM/yyyy");
    dateDebut->setMinimumHeight(35);

    QDateEdit *dateFin = new QDateEdit(&dialog);
    dateFin->setDate(QDate::fromString(dateFinValue, "dd/MM/yyyy"));
    dateFin->setCalendarPopup(true);
    dateFin->setDisplayFormat("dd/MM/yyyy");
    dateFin->setMinimumHeight(35);

    QComboBox *statut = new QComboBox(&dialog);
    statut->addItems({"En cours", "En attente", "Terminé", "Annulé"});
    statut->setCurrentText(statutValue);
    statut->setMinimumHeight(35);

    QLineEdit *budget = new QLineEdit(budgetValue, &dialog);
    budget->setPlaceholderText("0.00 EUR");
    budget->setMinimumHeight(35);

    // Add rows to form
    form->addRow("Nom du projet:", nomProjet);
    form->addRow("Client:", client);
    form->addRow("Adresse chantier:", adresse);
    form->addRow("Type:", typeProjet);
    form->addRow("Date début:", dateDebut);
    form->addRow("Date fin:", dateFin);
    form->addRow("Statut:", statut);
    form->addRow("Budget:", budget);

    mainLayout->addLayout(form);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Modifier");
    buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton { background-color: #8A9A5B; color: white; border: none; border-radius: 5px; padding: 8px 20px; font-weight: bold; }"
        "QPushButton:hover { background-color: #9aaa6b; }"
        );
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Annuler");
    buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton { background-color: #e2e8f0; color: #4a5568; border: none; border-radius: 5px; padding: 8px 20px; }"
        "QPushButton:hover { background-color: #cbd5e0; }"
        );

    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        // Update table with new values
        QString budgetText = budget->text();
        if (!budgetText.contains("EUR")) {
            budgetText = budgetText + " EUR";
        }

        table->item(row, 0)->setText(client->text());
        table->item(row, 1)->setText(typeProjet->currentText());
        table->item(row, 2)->setText(dateDebut->date().toString("dd/MM/yyyy"));
        table->item(row, 3)->setText(dateFin->date().toString("dd/MM/yyyy"));

        // Update statut with color
        QTableWidgetItem *statutItem = table->item(row, 4);
        statutItem->setText(statut->currentText());
        if (statut->currentText() == "En cours") {
            statutItem->setForeground(QBrush(QColor("#2ecc71")));
        } else if (statut->currentText() == "En attente") {
            statutItem->setForeground(QBrush(QColor("#f39c12")));
        } else if (statut->currentText() == "Terminé") {
            statutItem->setForeground(QBrush(QColor("#3498db")));
        } else {
            statutItem->setForeground(QBrush(QColor("#e74c3c")));
        }

        table->item(row, 5)->setText(budgetText);

        // Update hidden columns
        if (table->columnCount() > 6) {
            if (table->item(row, 6)) {
                table->item(row, 6)->setText(nomProjet->text());
            } else {
                table->setItem(row, 6, new QTableWidgetItem(nomProjet->text()));
            }

            if (table->columnCount() > 7) {
                if (table->item(row, 7)) {
                    table->item(row, 7)->setText(adresse->text());
                } else {
                    table->setItem(row, 7, new QTableWidgetItem(adresse->text()));
                }
            }
        }

        QMessageBox::information(this, "Succès", "Projet modifié avec succès!");
    }
}

void MainWindow::onDeleteButtonClicked()
{
    // Get the projects page and its table
    QWidget *projectsPage = stackedWidget->widget(0);
    QTableWidget *table = projectsPage->findChild<QTableWidget*>("dataTable");

    if (!table || table->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un projet à supprimer.");
        return;
    }

    int row = table->currentRow();
    QString clientName = table->item(row, 0) ? table->item(row, 0)->text() : "";
    QString projectName = (table->columnCount() > 6 && table->item(row, 6)) ? table->item(row, 6)->text() : "Projet sans nom";

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmer la suppression",
        QString("Êtes-vous sûr de vouloir supprimer le projet \"%1\" pour le client %2 ?\n\nCette action est irréversible.")
            .arg(projectName).arg(clientName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        table->removeRow(row);
        QMessageBox::information(this, "Succès", "Projet supprimé avec succès!");
    }
}


// Don't forget to call setupProjectsTable() in createProjctsPage() or after table creation





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
    int newWidth = static_cast<int>(1000 * scale);
    int newHeight = static_cast<int>(500 * scale);
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
// Ajoutez ces fonctions dans mainwindow.cpp

void MainWindow::onAddProductClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un Nouveau Produit");
    dialog.setMinimumWidth(550);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel *titleLabel = new QLabel("Nouveau Produit", &dialog);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Form layout
    QFormLayout *form = new QFormLayout();
    form->setSpacing(15);
    form->setLabelAlignment(Qt::AlignRight);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // Form fields
    QLineEdit *idProd = new QLineEdit(&dialog);
    idProd->setPlaceholderText("PROD-001");
    idProd->setMinimumHeight(35);

    QLineEdit *nomProd = new QLineEdit(&dialog);
    nomProd->setPlaceholderText("Nom du produit");
    nomProd->setMinimumHeight(35);

    QComboBox *categorieProd = new QComboBox(&dialog);
    categorieProd->addItems({"Meuble", "Menuiserie", "Décoration", "Porte", "Fenêtre", "Escalier", "Autre"});
    categorieProd->setMinimumHeight(35);

    QLineEdit *prixProd = new QLineEdit(&dialog);
    prixProd->setPlaceholderText("0.00 EUR");
    prixProd->setMinimumHeight(35);

    QLineEdit *dimensions = new QLineEdit(&dialog);
    dimensions->setPlaceholderText("L x l x H (ex: 200 x 80 x 75 cm)");
    dimensions->setMinimumHeight(35);

    QLineEdit *matUtilise = new QLineEdit(&dialog);
    matUtilise->setPlaceholderText("Bois, MDF, Verre, Métal...");
    matUtilise->setMinimumHeight(35);

    QDateEdit *dateCreation = new QDateEdit(&dialog);
    dateCreation->setDate(QDate::currentDate());
    dateCreation->setCalendarPopup(true);
    dateCreation->setDisplayFormat("dd/MM/yyyy");
    dateCreation->setMinimumHeight(35);

    QLineEdit *image = new QLineEdit(&dialog);
    image->setPlaceholderText("chemin/vers/image.jpg");
    image->setMinimumHeight(35);

    // Bouton pour parcourir les images
    QPushButton *browseBtn = new QPushButton("Parcourir...", &dialog);
    browseBtn->setMinimumHeight(35);
    browseBtn->setCursor(Qt::PointingHandCursor);
    browseBtn->setStyleSheet(
        "QPushButton { background-color: #e2e8f0; color: #4a5568; border: none; border-radius: 5px; padding: 8px 15px; }"
        "QPushButton:hover { background-color: #cbd5e0; }"
        );

    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(image);
    imageLayout->addWidget(browseBtn);

    connect(browseBtn, &QPushButton::clicked, [&dialog, image]() {
        QString fileName = QFileDialog::getOpenFileName(&dialog,
                                                        "Sélectionner une image",
                                                        QDir::homePath(),
                                                        "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (!fileName.isEmpty()) {
            image->setText(fileName);
        }
    });

    // Add rows to form
    form->addRow("ID Produit:", idProd);
    form->addRow("Nom du produit:", nomProd);
    form->addRow("Catégorie:", categorieProd);
    form->addRow("Prix:", prixProd);
    form->addRow("Dimensions:", dimensions);
    form->addRow("Matériaux utilisés:", matUtilise);
    form->addRow("Date création:", dateCreation);
    form->addRow("Image:", imageLayout);

    mainLayout->addLayout(form);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Ajouter");
    buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton { background-color: #8A9A5B; color: white; border: none; border-radius: 5px; padding: 8px 20px; font-weight: bold; }"
        "QPushButton:hover { background-color: #9aaa6b; }"
        );
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Annuler");
    buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton { background-color: #e2e8f0; color: #4a5568; border: none; border-radius: 5px; padding: 8px 20px; }"
        "QPushButton:hover { background-color: #cbd5e0; }"
        );

    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        // Get the products page and its table
        QWidget *productsPage = stackedWidget->widget(4); // Index 4 pour la page produits
        QTableWidget *table = productsPage->findChild<QTableWidget*>("productsTable");

        if (table) {
            int row = table->rowCount();
            table->insertRow(row);

            // Format price
            QString prixText = prixProd->text();
            if (!prixText.contains("EUR")) {
                prixText = prixText + " EUR";
            }

            // Set values in table
            table->setItem(row, 0, new QTableWidgetItem(idProd->text()));
            table->setItem(row, 1, new QTableWidgetItem(nomProd->text()));
            table->setItem(row, 2, new QTableWidgetItem(categorieProd->currentText()));
            table->setItem(row, 3, new QTableWidgetItem(prixText));
            table->setItem(row, 4, new QTableWidgetItem(dimensions->text()));
            table->setItem(row, 5, new QTableWidgetItem(matUtilise->text()));
            table->setItem(row, 6, new QTableWidgetItem(dateCreation->date().toString("dd/MM/yyyy")));
            table->setItem(row, 7, new QTableWidgetItem(image->text()));

            table->setRowHeight(row, 50);

            QMessageBox::information(this, "Succès", "Produit ajouté avec succès!");
        }
    }
}

void MainWindow::onEditProductClicked()
{
    // Get the products page and its table
    QWidget *productsPage = stackedWidget->widget(4);
    QTableWidget *table = productsPage->findChild<QTableWidget*>("productsTable");

    if (!table || table->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un produit à modifier.");
        return;
    }

    int row = table->currentRow();

    // Get existing values
    QString idProd = table->item(row, 0) ? table->item(row, 0)->text() : "";
    QString nomProd = table->item(row, 1) ? table->item(row, 1)->text() : "";
    QString categorieProd = table->item(row, 2) ? table->item(row, 2)->text() : "";
    QString prixProd = table->item(row, 3) ? table->item(row, 3)->text().replace(" EUR", "") : "";
    QString dimensions = table->item(row, 4) ? table->item(row, 4)->text() : "";
    QString matUtilise = table->item(row, 5) ? table->item(row, 5)->text() : "";
    QString dateCreation = table->item(row, 6) ? table->item(row, 6)->text() : QDate::currentDate().toString("dd/MM/yyyy");
    QString image = table->item(row, 7) ? table->item(row, 7)->text() : "";

    QDialog dialog(this);
    dialog.setWindowTitle("Modifier le Produit");
    dialog.setMinimumWidth(550);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel *titleLabel = new QLabel("Modifier le Produit", &dialog);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Form layout
    QFormLayout *form = new QFormLayout();
    form->setSpacing(15);
    form->setLabelAlignment(Qt::AlignRight);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // Form fields with existing values
    QLineEdit *idProdEdit = new QLineEdit(idProd, &dialog);
    idProdEdit->setPlaceholderText("PROD-001");
    idProdEdit->setMinimumHeight(35);

    QLineEdit *nomProdEdit = new QLineEdit(nomProd, &dialog);
    nomProdEdit->setPlaceholderText("Nom du produit");
    nomProdEdit->setMinimumHeight(35);

    QComboBox *categorieProdEdit = new QComboBox(&dialog);
    categorieProdEdit->addItems({"Meuble", "Menuiserie", "Décoration", "Porte", "Fenêtre", "Escalier", "Autre"});
    categorieProdEdit->setCurrentText(categorieProd);
    categorieProdEdit->setMinimumHeight(35);

    QLineEdit *prixProdEdit = new QLineEdit(prixProd, &dialog);
    prixProdEdit->setPlaceholderText("0.00 EUR");
    prixProdEdit->setMinimumHeight(35);

    QLineEdit *dimensionsEdit = new QLineEdit(dimensions, &dialog);
    dimensionsEdit->setPlaceholderText("L x l x H (ex: 200 x 80 x 75 cm)");
    dimensionsEdit->setMinimumHeight(35);

    QLineEdit *matUtiliseEdit = new QLineEdit(matUtilise, &dialog);
    matUtiliseEdit->setPlaceholderText("Bois, MDF, Verre, Métal...");
    matUtiliseEdit->setMinimumHeight(35);

    QDateEdit *dateCreationEdit = new QDateEdit(&dialog);
    dateCreationEdit->setDate(QDate::fromString(dateCreation, "dd/MM/yyyy"));
    dateCreationEdit->setCalendarPopup(true);
    dateCreationEdit->setDisplayFormat("dd/MM/yyyy");
    dateCreationEdit->setMinimumHeight(35);

    QLineEdit *imageEdit = new QLineEdit(image, &dialog);
    imageEdit->setPlaceholderText("chemin/vers/image.jpg");
    imageEdit->setMinimumHeight(35);

    // Bouton pour parcourir les images
    QPushButton *browseBtn = new QPushButton("Parcourir...", &dialog);
    browseBtn->setMinimumHeight(35);
    browseBtn->setCursor(Qt::PointingHandCursor);
    browseBtn->setStyleSheet(
        "QPushButton { background-color: #e2e8f0; color: #4a5568; border: none; border-radius: 5px; padding: 8px 15px; }"
        "QPushButton:hover { background-color: #cbd5e0; }"
        );

    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(imageEdit);
    imageLayout->addWidget(browseBtn);

    connect(browseBtn, &QPushButton::clicked, [&dialog, imageEdit]() {
        QString fileName = QFileDialog::getOpenFileName(&dialog,
                                                        "Sélectionner une image",
                                                        QDir::homePath(),
                                                        "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (!fileName.isEmpty()) {
            imageEdit->setText(fileName);
        }
    });

    // Add rows to form
    form->addRow("ID Produit:", idProdEdit);
    form->addRow("Nom du produit:", nomProdEdit);
    form->addRow("Catégorie:", categorieProdEdit);
    form->addRow("Prix:", prixProdEdit);
    form->addRow("Dimensions:", dimensionsEdit);
    form->addRow("Matériaux utilisés:", matUtiliseEdit);
    form->addRow("Date création:", dateCreationEdit);
    form->addRow("Image:", imageLayout);

    mainLayout->addLayout(form);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Modifier");
    buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton { background-color: #8A9A5B; color: white; border: none; border-radius: 5px; padding: 8px 20px; font-weight: bold; }"
        "QPushButton:hover { background-color: #9aaa6b; }"
        );
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Annuler");
    buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton { background-color: #e2e8f0; color: #4a5568; border: none; border-radius: 5px; padding: 8px 20px; }"
        "QPushButton:hover { background-color: #cbd5e0; }"
        );

    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        // Update table with new values
        QString prixText = prixProdEdit->text();
        if (!prixText.contains("EUR")) {
            prixText = prixText + " EUR";
        }

        table->item(row, 0)->setText(idProdEdit->text());
        table->item(row, 1)->setText(nomProdEdit->text());
        table->item(row, 2)->setText(categorieProdEdit->currentText());
        table->item(row, 3)->setText(prixText);
        table->item(row, 4)->setText(dimensionsEdit->text());
        table->item(row, 5)->setText(matUtiliseEdit->text());
        table->item(row, 6)->setText(dateCreationEdit->date().toString("dd/MM/yyyy"));
        table->item(row, 7)->setText(imageEdit->text());

        QMessageBox::information(this, "Succès", "Produit modifié avec succès!");
    }
}

void MainWindow::onDeleteProductClicked()
{
    // Get the products page and its table
    QWidget *productsPage = stackedWidget->widget(4);
    QTableWidget *table = productsPage->findChild<QTableWidget*>("productsTable");

    if (!table || table->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un produit à supprimer.");
        return;
    }

    int row = table->currentRow();
    QString idProd = table->item(row, 0) ? table->item(row, 0)->text() : "";
    QString nomProd = table->item(row, 1) ? table->item(row, 1)->text() : "Produit sans nom";

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmer la suppression",
        QString("Êtes-vous sûr de vouloir supprimer le produit \"%1\" (ID: %2) ?\n\nCette action est irréversible.")
            .arg(nomProd).arg(idProd),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        table->removeRow(row);
        QMessageBox::information(this, "Succès", "Produit supprimé avec succès!");
    }
}
