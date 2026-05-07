#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/modules/auth/loginpage.h"
#include "src/modules/auth/forgotpasswordpage.h"
#include "src/modules/employees/employeemanagementpage.h"
#include "src/core/session.h"
#include "src/modules/stock/stockpage.h"
#include "src/modules/projects/projectmanagementpage.h"
#include "src/modules/finance/financeview.h"
#include "src/modules/finance/financemodel.h"
#include "produit3ddialog.h"
#include "produitbarcodedialog.h"
#include "src/modules/chat/chatbotwidget.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include <QCoreApplication>
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
#include <QDoubleValidator>
#include <QIntValidator>
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
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QMap>
#include <QRegularExpression>
#include <algorithm>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QTextCursor>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <cmath>
#include <numeric>
#include <QLinearGradient>
#include <QStandardPaths>
#include <QBuffer>

// ─────────────────────────────────────────────────────────────────────────────
// Helper : retourne tous les ID_PROJET disponibles en BD
// Utilise la première connexion ouverte, quel que soit son nom
// ─────────────────────────────────────────────────────────────────────────────
static QStringList getAllProjetIds()
{
    QStringList ids;

    QSqlDatabase db;
    QStringList connNames = QSqlDatabase::connectionNames();
    qDebug() << "[getAllProjetIds] Connexions disponibles :" << connNames;

    for (const QString& name : connNames) {
        QSqlDatabase c = QSqlDatabase::database(name);
        if (c.isOpen()) { db = c; break; }
    }
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "[getAllProjetIds] Aucune connexion BD ouverte !";
        return ids;
    }

    QSqlQuery q(db);
    if (!q.exec("SELECT ID_PROJET FROM PROJET ORDER BY ID_PROJET")) {
        qDebug() << "[getAllProjetIds] Erreur SQL :" << q.lastError().databaseText();
        return ids;
    }
    while (q.next()) {
        QString v = q.value(0).toString().trimmed();
        if (!v.isEmpty()) ids << v;
    }
    qDebug() << "[getAllProjetIds] Projets trouvés :" << ids;
    return ids;
}

// ─────────────────────────────────────────────────────────────────────────────
// MainWindow
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , produitDB(ProduitDatabase::instance())
    , ui(new Ui::MainWindow)
    , isDarkMode(false)
    , currentScale(1.0)
    , currentTable(nullptr)
    , m_chatbotWidget(nullptr)
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
    setMinimumSize(1200, 700);
    setMaximumSize(2000, 900);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    authStack = new QStackedWidget(centralWidget);
    mainLayout->addWidget(authStack);

    setupAuth();

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
    authStack->setCurrentIndex(0);
}

void MainWindow::setupAuth()
{
    authPage = new QWidget();
    QVBoxLayout *authLayout = new QVBoxLayout(authPage);
    authLayout->setContentsMargins(0, 0, 0, 0);
    authLayout->setSpacing(0);

    QStackedWidget *authPages = new QStackedWidget(authPage);
    LoginPage          *loginPage  = new LoginPage();
    ForgotPasswordPage *forgotPage = new ForgotPasswordPage();

    authPages->addWidget(loginPage);
    authPages->addWidget(forgotPage);

    connect(loginPage,  &LoginPage::loginSuccess,          this,       &MainWindow::onLoginSuccess);
    connect(loginPage,  &LoginPage::switchToForgotPassword,[authPages](){ authPages->setCurrentIndex(1); });
    connect(forgotPage, &ForgotPasswordPage::switchToLogin,[authPages](){ authPages->setCurrentIndex(0); });

    authLayout->addWidget(authPages);
}

void MainWindow::setupChatbot()
{
    // Le chatbot est créé dans createChatbotPage()
    // Cette méthode reste disponible pour une initialisation future
}

void MainWindow::onLoginSuccess(const Employee& employee)
{
    currentEmployee = employee;
    Session::setCurrentEmployee(employee);
    showMainApp();
}

void MainWindow::showMainApp()
{
    if (profileName)
        profileName->setText(currentEmployee.getFullName());

    for (QLabel* lbl : profileBtn->findChildren<QLabel*>()) {
        if (lbl->width() == 38 && lbl->height() == 38) {
            int size = 38;
            QPixmap avatar(size, size);
            avatar.fill(Qt::transparent);

            if (currentEmployee.hasPhoto()) {
                QPixmap src;
                src.loadFromData(currentEmployee.getPhoto());
                QPixmap scaled = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                QPainter painter(&avatar);
                painter.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addEllipse(0, 0, size, size);
                painter.setClipPath(path);
                painter.drawPixmap(0, 0, scaled);
            } else {
                QString initial = currentEmployee.getPrenom().isEmpty()
                ? "?" : QString(currentEmployee.getPrenom().at(0).toUpper());
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
            }
            lbl->setPixmap(avatar);
            break;
        }
    }

    authStack->setCurrentIndex(1);

    struct { int idx; bool allowed; } perms[] = {
        { 0, currentEmployee.canAccessProjet()       },
        { 1, currentEmployee.canAccessEmploye()      },
        { 2, currentEmployee.canAccessMateriau()     },
        { 3, currentEmployee.canAccessTransactions() },
        { 4, currentEmployee.canAccessProduit()      },
        { 5, true }, // Assistant IA — accessible à tous
    };
    int firstAllowed = -1;
    for (auto& p : perms) {
        if (p.idx < sidebarButtons.size()) {
            sidebarButtons[p.idx]->setVisible(p.allowed);
            if (p.allowed && firstAllowed == -1) firstAllowed = p.idx;
        }
    }
    if (firstAllowed >= 0)
        onSidebarButtonClicked(firstAllowed);
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
        QPixmap fallback(size, size);
        fallback.fill(Qt::transparent);
        QPainter painter(&fallback);
        painter.setRenderHint(QPainter::Antialiasing);
        QLinearGradient gradient(0, 0, size, size);
        gradient.setColorAt(0, QColor("#8A9A5B"));
        gradient.setColorAt(1, QColor("#9aaa6b"));
        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, size, size);
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPixelSize(size / 2);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, "A");
        avatarLabel->setPixmap(fallback);
        return avatarLabel;
    }

    QPixmap scaled = sourcePixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap rounded(size, size);
    rounded.fill(Qt::transparent);
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap((size - scaled.width()) / 2, (size - scaled.height()) / 2, scaled);
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

    QLabel *sidebarLogo = new QLabel(sidebar);
    QPixmap logoPixmap("src/assets/icons/logo1.png");
    if (!logoPixmap.isNull())
        sidebarLogo->setPixmap(logoPixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    sidebarLogo->setAlignment(Qt::AlignCenter);
    sidebarLogo->setContentsMargins(0, 16, 0, 8);
    sidebarLayout->addWidget(sidebarLogo, 0, Qt::AlignCenter);

    QStringList menuItems = {
        "Gestion des Projets", "Gestion des Employes",
        "Gestion des Stocks",  "Gestion Financiere", "Gestion des Designs",
        "Assistant IA"
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
        connect(btn, &QPushButton::clicked, this, [this, i]() { onSidebarButtonClicked(i); });
        sidebarButtons.append(btn);
        sidebarLayout->addWidget(btn);
    }
    sidebarLayout->addStretch();

    QLabel *footer = new QLabel("Version 1.0.0", sidebar);
    footer->setObjectName("sidebarFooter");
    footer->setAlignment(Qt::AlignCenter);
    footer->setFixedHeight(24);
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

    QWidget *darkModeContainer = new QWidget(navbar);
    QHBoxLayout *darkModeLayout = new QHBoxLayout(darkModeContainer);
    darkModeLayout->setContentsMargins(0, 0, 0, 0);
    darkModeLayout->setSpacing(8);

    QLabel *darkModeLabel = new QLabel("Passer en mode sombre", darkModeContainer);
    darkModeLabel->setObjectName("darkModeLabel");
    darkModeLabel->setStyleSheet("color: #718096; font-size: 12px;");

    darkModeToggle = new ToggleSwitch(darkModeContainer);
    connect(darkModeToggle, &ToggleSwitch::toggled, this, [this, darkModeLabel](bool checked) {
        darkModeLabel->setText(checked ? "Passer en mode clair" : "Passer en mode sombre");
        toggleDarkMode();
    });
    darkModeLayout->addWidget(darkModeLabel);
    darkModeLayout->addWidget(darkModeToggle);

    profileBtn = new QPushButton(navbar);
    profileBtn->setObjectName("profileButton");
    profileBtn->setCursor(Qt::PointingHandCursor);
    profileBtn->setFixedSize(42, 42);
    profileBtn->setStyleSheet(
        "QPushButton#profileButton { border-radius: 21px; border: 2px solid #8A9A5B; "
        "background: transparent; padding: 0; }"
        "QPushButton#profileButton:hover { border-color: #9aaa6b; }");

    QHBoxLayout *profileLayout = new QHBoxLayout(profileBtn);
    profileLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *profilePhoto = createRoundedAvatar("src/assets/icons/pfp.jpeg", 38);
    profileName = new QLabel("", profileBtn);
    profileName->hide();
    profileLayout->addWidget(profilePhoto, 0, Qt::AlignCenter);
    connect(profileBtn, &QPushButton::clicked, this, &MainWindow::showProfileMenu);

    navbarLayout->addWidget(darkModeContainer);
    navbarLayout->addWidget(profileBtn);
}

void MainWindow::showProfileMenu()
{
    QMenu *menu = new QMenu(this);
    menu->setObjectName("dropdownMenu");
    QAction *nameAction = menu->addAction(currentEmployee.getFullName());
    nameAction->setEnabled(false);
    QFont f = nameAction->font(); f.setBold(true); nameAction->setFont(f);
    menu->addSeparator();
    connect(menu->addAction("Se deconnecter"), &QAction::triggered, this, &MainWindow::onLogout);
    QPoint pos = profileBtn->mapToGlobal(QPoint(profileBtn->width(), profileBtn->height() + 4));
    pos.setX(pos.x() - menu->sizeHint().width());
    menu->exec(pos);
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
    stackedWidget->addWidget(new ProjectManagementPage(this));  // 0
    stackedWidget->addWidget(createEmployeesPage());            // 1
    stackedWidget->addWidget(new StockPage(this));              // 2
    stackedWidget->addWidget(createFinancePage());              // 3
    stackedWidget->addWidget(createProductsPage());             // 4
    stackedWidget->addWidget(createChatbotPage());              // 5
}

QWidget* MainWindow::createChatbotPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_chatbotWidget = new ChatbotWidget(page);
    layout->addWidget(m_chatbotWidget);
    return page;
}

QWidget* MainWindow::createEmployeesPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(new EmployeeManagementPage(page));
    return page;
}

QWidget* MainWindow::createFinancePage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    FinanceModel *model = new FinanceModel(page);
    FinanceView  *view  = new FinanceView(page);
    view->setModel(model);
    layout->addWidget(view);
    return page;
}

// ─────────────────────────────────────────────────────────────────────────────
// Page Produits — colonnes calées sur la vraie BD
// ID_PROD | NOM_PROD | CATEGORIE_PROD | PRIX_PROD | DIMENSIONS | MAT_UTILISE
// DATE_CREATION | IMAGE | PRODUIT_PROJET
// ─────────────────────────────────────────────────────────────────────────────

QWidget* MainWindow::createProductsPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(14);
    layout->setContentsMargins(0, 0, 0, 0);

    // ── Stat cards ───────────────────────────────────────────────────────────
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(12);

    auto makeCard = [&](const QString& title, const QString& objName,
                        const QString& type) -> QFrame*
    {
        QFrame *card = new QFrame(page);
        card->setObjectName("statCard");
        card->setProperty("type", type);

        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setSpacing(4);
        cl->setContentsMargins(20, 16, 20, 16);

        QLabel *t = new QLabel(title, card);
        t->setObjectName("statTitle");

        QLabel *v = new QLabel("—", card);
        v->setObjectName(objName);
        v->setStyleSheet("font-size:26px; font-weight:bold; color:#2d3748;");

        cl->addWidget(t);
        cl->addWidget(v);
        cl->addStretch();
        return card;
    };

    statsLayout->addWidget(makeCard("TOTAL PRODUITS",   "stat_total",   "active"));
    statsLayout->addWidget(makeCard("PRIX MOYEN (DT)",  "stat_avg",     "pending"));
    statsLayout->addWidget(makeCard("NOUVEAUTÉS MOIS",  "stat_new",     "new"));
    statsLayout->addWidget(makeCard("VALEUR CATALOGUE", "stat_catalog", "active"));
    layout->addLayout(statsLayout);

    // ── Toolbar ──────────────────────────────────────────────────────────────
    QHBoxLayout *toolBar = new QHBoxLayout();
    toolBar->setSpacing(8);

    QPushButton *addBtn    = new QPushButton("+ Nouveau",      page);
    QPushButton *editBtn   = new QPushButton("Modifier",       page);
    QPushButton *deleteBtn = new QPushButton("Supprimer",      page);
    QPushButton *exportBtn = new QPushButton("Export PDF",     page);
    QPushButton *statsBtn  = new QPushButton("Statistiques",   page);

    for (auto *b : {addBtn, editBtn, deleteBtn, exportBtn, statsBtn}) {
        b->setObjectName("actionButton");
        b->setCursor(Qt::PointingHandCursor);
        b->setMinimumHeight(38);
    }
    exportBtn->setStyleSheet(
        "QPushButton{background-color:#2b6cb0;color:white;border:none;"
        "border-radius:6px;padding:0 14px;font-weight:bold;}"
        "QPushButton:hover{background-color:#2c5282;}");
    statsBtn->setStyleSheet(
        "QPushButton{background-color:#8A9A5B;color:white;border:none;"
        "border-radius:6px;padding:0 14px;font-weight:bold;}"
        "QPushButton:hover{background-color:#6a8040;}");

    connect(addBtn,    &QPushButton::clicked, this, &MainWindow::onAddProductClicked);
    connect(editBtn,   &QPushButton::clicked, this, &MainWindow::onEditProductClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteProductClicked);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::onExportProductsPDF);
    connect(statsBtn,  &QPushButton::clicked, this, &MainWindow::onShowProductStats);

    toolBar->addWidget(addBtn);
    toolBar->addWidget(editBtn);
    toolBar->addWidget(deleteBtn);
    toolBar->addSpacing(8);

    // Séparateur visuel
    QFrame *sep = new QFrame(page);
    sep->setFrameShape(QFrame::VLine);
    sep->setFixedHeight(28);
    sep->setStyleSheet("color:#e2e8f0;");
    toolBar->addWidget(sep);

    toolBar->addWidget(exportBtn);
    toolBar->addWidget(statsBtn);
    toolBar->addStretch();

    // Recherche + tri
    QLineEdit *searchBar = new QLineEdit(page);
    searchBar->setObjectName("productSearch");
    searchBar->setPlaceholderText("Rechercher par nom ou type/catégorie...");
    searchBar->setMinimumHeight(38);
    searchBar->setMinimumWidth(260);
    searchBar->setClearButtonEnabled(true);

    QComboBox *sortCombo = new QComboBox(page);
    sortCombo->setObjectName("productSortCombo");
    sortCombo->setMinimumHeight(38);
    sortCombo->addItem("Tri : Date (récent)",  "date_desc");
    sortCombo->addItem("Tri : Date (ancien)",  "date_asc");

    toolBar->addWidget(searchBar);
    toolBar->addWidget(sortCombo);
    layout->addLayout(toolBar);

    // ── Corps : table + panneau détail ───────────────────────────────────────
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(0);
    bodyLayout->setContentsMargins(0, 0, 0, 0);

    // ── Table ────────────────────────────────────────────────────────────────
    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("productsTable");
    table->setColumnCount(10);
    table->setHorizontalHeaderLabels({
        "ID", "NOM", "CATÉGORIE", "PRIX (DT)",
        "DIMENSIONS", "MATÉRIAUX", "DATE CRÉATION", "IMAGE", "PROJET", ""
    });
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setDefaultSectionSize(105);
    table->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
    table->setColumnWidth(7, 56);   // colonne image miniature
    table->setColumnWidth(9, 0);    // colonne cachée (réservée)
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(false);

    bodyLayout->addWidget(table, 1);

    // ── Panneau détail ───────────────────────────────────────────────────────
    buildDetailPanel(page);
    bodyLayout->addWidget(m_detailPanel);

    layout->addLayout(bodyLayout, 1);

    // ── Connexions ───────────────────────────────────────────────────────────
    auto applyFilter = [this, searchBar, sortCombo]() {
        onProductSearchOrSort(searchBar->text(), sortCombo->currentData().toString());
    };
    connect(searchBar, &QLineEdit::textChanged,       this, [applyFilter](const QString&) { applyFilter(); });
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [applyFilter](int) { applyFilter(); });

    // Sélection de ligne → panneau détail
    connect(table, &QTableWidget::currentCellChanged,
            this, [this](int row, int, int, int){ onProductRowSelected(row); });

    return page;
}

// ─────────────────────────────────────────────────────────────────────────────
// loadProduits — recharge la table depuis la BD
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::loadProduits()
{
    QWidget *productsPage = stackedWidget->widget(4);
    if (!productsPage) return;

    QTableWidget *table = productsPage->findChild<QTableWidget*>("productsTable");
    if (!table) return;

    m_allProduits = produitDB.getAllProduits();   // cache interne
    populateProductTable(table, m_allProduits);
    updateProductStatCards(productsPage, m_allProduits);
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper : style bordure rouge/vert sur un QLineEdit
// ─────────────────────────────────────────────────────────────────────────────
static void setFieldError(QLineEdit* f, bool error)
{
    f->setStyleSheet(error
                         ? "border:2px solid #e53e3e;border-radius:4px;padding:4px;"
                         : "border:2px solid #68d391;border-radius:4px;padding:4px;");
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper : valide tous les champs et retourne la liste des erreurs
// ─────────────────────────────────────────────────────────────────────────────
static QStringList validateProduitFields(QLineEdit* nom, QLineEdit* prix,
                                         QLineEdit* dim,  QLineEdit* mat,
                                         QLineEdit* img)
{
    QStringList errors;

    // NOM — obligatoire, 2 à 50 caractères, lettres/chiffres/espaces/tirets
    QString nomVal = nom->text().trimmed();
    if (nomVal.isEmpty()) {
        errors << "• Nom du produit : champ obligatoire.";
        setFieldError(nom, true);
    } else if (nomVal.length() < 2) {
        errors << "• Nom du produit : minimum 2 caractères.";
        setFieldError(nom, true);
    } else if (nomVal.length() > 50) {
        errors << "• Nom du produit : maximum 50 caractères.";
        setFieldError(nom, true);
    } else if (!QRegularExpression("^[\\w\\séàâêîôùûçèäëïöü' \\-]+$",
                                   QRegularExpression::CaseInsensitiveOption).match(nomVal).hasMatch()) {
        errors << "• Nom du produit : caractères invalides détectés.";
        setFieldError(nom, true);
    } else {
        setFieldError(nom, false);
    }

    // PRIX — obligatoire, nombre > 0, max 999 999
    QString prixVal = prix->text().trimmed();
    if (prixVal.isEmpty()) {
        errors << "• Prix : champ obligatoire.";
        setFieldError(prix, true);
    } else {
        bool ok = false;
        double v = prixVal.replace(',', '.').toDouble(&ok);
        if (!ok || v <= 0.0) {
            errors << "• Prix : doit être un nombre positif (ex: 150.00).";
            setFieldError(prix, true);
        } else if (v > 999999.0) {
            errors << "• Prix : valeur trop élevée (max 999 999).";
            setFieldError(prix, true);
        } else {
            setFieldError(prix, false);
        }
    }

    // DIMENSIONS — obligatoire, format libre mais non vide, max 20 chars
    QString dimVal = dim->text().trimmed();
    if (dimVal.isEmpty()) {
        errors << "• Dimensions : champ obligatoire (ex: 200 x 90 x 75 cm).";
        setFieldError(dim, true);
    } else if (dimVal.length() > 20) {
        errors << "• Dimensions : maximum 20 caractères.";
        setFieldError(dim, true);
    } else {
        setFieldError(dim, false);
    }

    // MATÉRIAUX — obligatoire, max 20 chars
    QString matVal = mat->text().trimmed();
    if (matVal.isEmpty()) {
        errors << "• Matériaux : champ obligatoire (ex: Chêne massif).";
        setFieldError(mat, true);
    } else if (matVal.length() > 20) {
        errors << "• Matériaux : maximum 20 caractères.";
        setFieldError(mat, true);
    } else {
        setFieldError(mat, false);
    }

    // IMAGE — optionnel, mais si renseigné : extension valide + max 20 chars
    QString imgVal = img->text().trimmed();
    if (!imgVal.isEmpty()) {
        if (imgVal.length() > 20) {
            errors << "• Image : maximum 20 caractères.";
            setFieldError(img, true);
        } else if (!QRegularExpression("\\.(jpg|jpeg|png|gif|bmp)$",
                                       QRegularExpression::CaseInsensitiveOption).match(imgVal).hasMatch()) {
            errors << "• Image : extension invalide (jpg, jpeg, png, gif, bmp).";
            setFieldError(img, true);
        } else {
            setFieldError(img, false);
        }
    } else {
        img->setStyleSheet(""); // optionnel → neutre
    }

    return errors;
}

void MainWindow::onAddProductClicked()
{
    QStringList projetIds = getAllProjetIds();
    if (projetIds.isEmpty()) {
        QMessageBox::critical(this, "Erreur",
                              "Aucun projet trouvé en base de données.\n"
                              "Veuillez d'abord créer un projet.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un Nouveau Produit");
    dialog.setMinimumWidth(560);
    dialog.setStyleSheet("QDialog{background-color:white;}");

    QVBoxLayout *mainLay = new QVBoxLayout(&dialog);
    mainLay->setSpacing(18); mainLay->setContentsMargins(30,30,30,30);

    QLabel *titleLbl = new QLabel("Nouveau Produit", &dialog);
    titleLbl->setStyleSheet("font-size:18px;font-weight:bold;color:#2c3e50;");
    titleLbl->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(titleLbl);

    // Label d'erreur global (caché par défaut)
    QLabel *errorLbl = new QLabel(&dialog);
    errorLbl->setStyleSheet("background:#fff5f5;color:#c53030;border:1px solid #fc8181;"
                            "border-radius:6px;padding:10px;font-size:12px;");
    errorLbl->setWordWrap(true);
    errorLbl->hide();
    mainLay->addWidget(errorLbl);

    QFormLayout *form = new QFormLayout();
    form->setSpacing(12); form->setLabelAlignment(Qt::AlignRight);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    QString fieldStyle = "border:1px solid #cbd5e0;border-radius:4px;padding:4px;";

    QLineEdit *nomEdit  = new QLineEdit(&dialog);
    nomEdit->setPlaceholderText("Nom du produit (2-50 caractères)");
    nomEdit->setMinimumHeight(35); nomEdit->setStyleSheet(fieldStyle);

    QComboBox *catEdit  = new QComboBox(&dialog);
    catEdit->addItems({"Meuble","Menuiserie","Décoration","Porte","Fenêtre","Escalier","Autre"});
    catEdit->setMinimumHeight(35);

    QLineEdit *prixEdit = new QLineEdit(&dialog);
    prixEdit->setPlaceholderText("Ex: 150.00  (> 0, max 999999)");
    prixEdit->setMinimumHeight(35); prixEdit->setStyleSheet(fieldStyle);

    QLineEdit *dimEdit  = new QLineEdit(&dialog);
    dimEdit->setPlaceholderText("Ex: 200x90x75  (max 20 car.)");
    dimEdit->setMinimumHeight(35); dimEdit->setStyleSheet(fieldStyle);

    QLineEdit *matEdit  = new QLineEdit(&dialog);
    matEdit->setPlaceholderText("Ex: Chêne massif  (max 20 car.)");
    matEdit->setMinimumHeight(35); matEdit->setStyleSheet(fieldStyle);

    QDateEdit *dateEdit = new QDateEdit(&dialog);
    dateEdit->setDate(QDate::currentDate()); dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd/MM/yyyy"); dateEdit->setMinimumHeight(35);

    QLineEdit *imageEdit = new QLineEdit(&dialog);
    imageEdit->setPlaceholderText("Optionnel — ex: photo.jpg  (max 20 car.)");
    imageEdit->setMinimumHeight(35); imageEdit->setMaxLength(20);
    imageEdit->setStyleSheet(fieldStyle);

    // ── Preview image ──────────────────────────────────────────────────────────
    QLabel *imgPreview = new QLabel(&dialog);
    imgPreview->setFixedSize(120, 80);
    imgPreview->setAlignment(Qt::AlignCenter);
    imgPreview->setStyleSheet("border:1px solid #cbd5e0; border-radius:4px; background:#f7fafc; color:#a0aec0; font-size:11px;");
    imgPreview->setText("Aucune image");

    connect(imageEdit, &QLineEdit::textChanged, &dialog, [imgPreview](const QString& fn){
        QString trimmed = fn.trimmed();
        QString appDir = QCoreApplication::applicationDirPath();
        QStringList candidates = {
            "src/assets/icons/" + trimmed,
            appDir + "/src/assets/icons/" + trimmed,
            appDir + "/../src/assets/icons/" + trimmed,
            appDir + "/../../src/assets/icons/" + trimmed,
            appDir + "/../../../src/assets/icons/" + trimmed,
        };
        QPixmap pm;
        for (const QString& c : candidates)
            if (pm.load(c)) break;

        if (!pm.isNull())
            imgPreview->setPixmap(pm.scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        else
            imgPreview->setText(trimmed.isEmpty() ? "Aucune image" : "❌ Introuvable");
    });

    QComboBox *projetCombo = new QComboBox(&dialog);
    projetCombo->addItems(projetIds); projetCombo->setMinimumHeight(35);

    form->addRow("Nom du produit * :", nomEdit);
    form->addRow("Catégorie * :",      catEdit);
    form->addRow("Prix (DT) * :",      prixEdit);
    form->addRow("Dimensions * :",     dimEdit);
    form->addRow("Matériaux * :",      matEdit);
    form->addRow("Date création :",    dateEdit);
    form->addRow("Image (nom) :",      imageEdit);
    form->addRow("Aperçu :",           imgPreview);
    form->addRow("Projet (FK) * :",    projetCombo);

    QLabel *reqLbl = new QLabel("* Champs obligatoires", &dialog);
    reqLbl->setStyleSheet("color:#718096;font-size:11px;");

    mainLay->addLayout(form);
    mainLay->addWidget(reqLbl);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, &dialog);
    bb->button(QDialogButtonBox::Ok)->setText("Ajouter");
    bb->button(QDialogButtonBox::Ok)->setMinimumHeight(40);
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton{background-color:#8A9A5B;color:white;border:none;border-radius:5px;"
        "padding:8px 20px;font-weight:bold;}QPushButton:hover{background-color:#9aaa6b;}");
    bb->button(QDialogButtonBox::Cancel)->setText("Annuler");
    bb->button(QDialogButtonBox::Cancel)->setMinimumHeight(40);
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton{background-color:#e2e8f0;color:#4a5568;border:none;border-radius:5px;"
        "padding:8px 20px;}QPushButton:hover{background-color:#cbd5e0;}");
    mainLay->addWidget(bb);

    // Validation en temps réel sur chaque champ
    auto liveCheck = [&]() {
        QStringList e = validateProduitFields(nomEdit, prixEdit, dimEdit, matEdit, imageEdit);
        if (e.isEmpty()) { errorLbl->hide(); }
        else { errorLbl->setText(e.join("\n")); errorLbl->show(); }
        dialog.adjustSize();
    };
    connect(nomEdit,   &QLineEdit::textChanged, &dialog, liveCheck);
    connect(prixEdit,  &QLineEdit::textChanged, &dialog, liveCheck);
    connect(dimEdit,   &QLineEdit::textChanged, &dialog, liveCheck);
    connect(matEdit,   &QLineEdit::textChanged, &dialog, liveCheck);
    connect(imageEdit, &QLineEdit::textChanged, &dialog, liveCheck);

    connect(bb, &QDialogButtonBox::accepted, &dialog, [&]() {
        QStringList errors = validateProduitFields(nomEdit, prixEdit, dimEdit, matEdit, imageEdit);
        if (!errors.isEmpty()) {
            errorLbl->setText(errors.join("\n"));
            errorLbl->show();
            dialog.adjustSize();
            return; // ne pas fermer le dialog
        }
        dialog.accept();
    });
    connect(bb, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    Produit p;
    p.setNom        (nomEdit->text().trimmed());
    p.setCategorie  (catEdit->currentText());
    p.setPrix       (prixEdit->text().trimmed().replace(',','.').toDouble());
    p.setDimensions (dimEdit->text().trimmed());
    p.setMatUtilise (matEdit->text().trimmed());
    p.setDateCreation(dateEdit->date());
    p.setImage      (imageEdit->text().trimmed().toUtf8());
    p.setProjetId   (projetCombo->currentText());

    if (produitDB.addProduit(p)) {
        QMessageBox::information(this, "Succès", "Produit ajouté avec succès !");
        loadProduits();
    } else {
        QMessageBox::critical(this, "Erreur BD",
                              "Ajout échoué !\nVérifiez que l'ID projet existe bien en base.");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// MODIFICATION
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onEditProductClicked()
{
    QWidget *productsPage = stackedWidget->widget(4);
    QTableWidget *table = productsPage ? productsPage->findChild<QTableWidget*>("productsTable") : nullptr;

    if (!table || table->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection",
                             "Veuillez sélectionner un produit dans la liste avant de modifier.");
        return;
    }

    int row = table->currentRow();
    QString realId = table->item(row,0) ? table->item(row,0)->data(Qt::UserRole).toString() : "";
    if (realId.isEmpty()) { QMessageBox::critical(this,"Erreur","ID produit introuvable."); return; }

    Produit existing = produitDB.getProduit(realId);

    QStringList projetIds = getAllProjetIds();
    if (projetIds.isEmpty()) {
        QMessageBox::critical(this, "Erreur", "Aucun projet trouvé en base de données.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Modifier le Produit");
    dialog.setMinimumWidth(560);
    dialog.setStyleSheet("QDialog{background-color:white;}");

    QVBoxLayout *mainLay = new QVBoxLayout(&dialog);
    mainLay->setSpacing(18); mainLay->setContentsMargins(30,30,30,30);

    QLabel *titleLbl = new QLabel("Modifier le Produit", &dialog);
    titleLbl->setStyleSheet("font-size:18px;font-weight:bold;color:#2c3e50;");
    titleLbl->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(titleLbl);

    QLabel *errorLbl = new QLabel(&dialog);
    errorLbl->setStyleSheet("background:#fff5f5;color:#c53030;border:1px solid #fc8181;"
                            "border-radius:6px;padding:10px;font-size:12px;");
    errorLbl->setWordWrap(true);
    errorLbl->hide();
    mainLay->addWidget(errorLbl);

    QFormLayout *form = new QFormLayout();
    form->setSpacing(12); form->setLabelAlignment(Qt::AlignRight);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    QString fieldStyle = "border:1px solid #cbd5e0;border-radius:4px;padding:4px;";

    QLineEdit *idDisplay = new QLineEdit(existing.getId(), &dialog);
    idDisplay->setReadOnly(true); idDisplay->setMinimumHeight(35);
    idDisplay->setStyleSheet("background-color:#f0f0f0;color:#888;border-radius:4px;padding:4px;");

    QLineEdit *nomEdit  = new QLineEdit(existing.getNom(), &dialog);
    nomEdit->setMinimumHeight(35); nomEdit->setStyleSheet(fieldStyle);

    QComboBox *catEdit  = new QComboBox(&dialog);
    catEdit->addItems({"Meuble","Menuiserie","Décoration","Porte","Fenêtre","Escalier","Autre"});
    catEdit->setCurrentText(existing.getCategorie()); catEdit->setMinimumHeight(35);

    QLineEdit *prixEdit = new QLineEdit(QString::number(existing.getPrix(),'f',2), &dialog);
    prixEdit->setMinimumHeight(35); prixEdit->setStyleSheet(fieldStyle);

    QLineEdit *dimEdit  = new QLineEdit(existing.getDimensions(), &dialog);
    dimEdit->setMinimumHeight(35); dimEdit->setStyleSheet(fieldStyle);

    QLineEdit *matEdit  = new QLineEdit(existing.getMatUtilise(), &dialog);
    matEdit->setMinimumHeight(35); matEdit->setStyleSheet(fieldStyle);

    QDateEdit *dateEdit = new QDateEdit(&dialog);
    dateEdit->setDate(existing.getDateCreation().isValid() ? existing.getDateCreation() : QDate::currentDate());
    dateEdit->setCalendarPopup(true); dateEdit->setDisplayFormat("dd/MM/yyyy"); dateEdit->setMinimumHeight(35);

    QLineEdit *imageEdit = new QLineEdit(QString::fromUtf8(existing.getImage()), &dialog);
    imageEdit->setPlaceholderText("Optionnel — ex: photo.jpg  (max 20 car.)");
    imageEdit->setMinimumHeight(35); imageEdit->setMaxLength(20);
    imageEdit->setStyleSheet(fieldStyle);

    QComboBox *projetCombo = new QComboBox(&dialog);
    projetCombo->addItems(projetIds);
    int pidx = projetIds.indexOf(existing.getProjetId());
    if (pidx >= 0) projetCombo->setCurrentIndex(pidx);
    projetCombo->setMinimumHeight(35);

    form->addRow("ID Produit :",       idDisplay);
    form->addRow("Nom du produit * :", nomEdit);
    form->addRow("Catégorie * :",      catEdit);
    form->addRow("Prix (DT) * :",      prixEdit);
    form->addRow("Dimensions * :",     dimEdit);
    form->addRow("Matériaux * :",      matEdit);
    form->addRow("Date création :",    dateEdit);
    form->addRow("Image (nom) :",      imageEdit);
    form->addRow("Projet (FK) * :",    projetCombo);

    QLabel *reqLbl = new QLabel("* Champs obligatoires", &dialog);
    reqLbl->setStyleSheet("color:#718096;font-size:11px;");

    mainLay->addLayout(form);
    mainLay->addWidget(reqLbl);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, &dialog);
    bb->button(QDialogButtonBox::Ok)->setText("Modifier");
    bb->button(QDialogButtonBox::Ok)->setMinimumHeight(40);
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton{background-color:#8A9A5B;color:white;border:none;border-radius:5px;"
        "padding:8px 20px;font-weight:bold;}QPushButton:hover{background-color:#9aaa6b;}");
    bb->button(QDialogButtonBox::Cancel)->setText("Annuler");
    bb->button(QDialogButtonBox::Cancel)->setMinimumHeight(40);
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton{background-color:#e2e8f0;color:#4a5568;border:none;border-radius:5px;"
        "padding:8px 20px;}QPushButton:hover{background-color:#cbd5e0;}");
    mainLay->addWidget(bb);

    auto liveCheck = [&]() {
        QStringList e = validateProduitFields(nomEdit, prixEdit, dimEdit, matEdit, imageEdit);
        if (e.isEmpty()) { errorLbl->hide(); }
        else { errorLbl->setText(e.join("\n")); errorLbl->show(); }
        dialog.adjustSize();
    };
    connect(nomEdit,   &QLineEdit::textChanged, &dialog, liveCheck);
    connect(prixEdit,  &QLineEdit::textChanged, &dialog, liveCheck);
    connect(dimEdit,   &QLineEdit::textChanged, &dialog, liveCheck);
    connect(matEdit,   &QLineEdit::textChanged, &dialog, liveCheck);
    connect(imageEdit, &QLineEdit::textChanged, &dialog, liveCheck);

    connect(bb, &QDialogButtonBox::accepted, &dialog, [&]() {
        QStringList errors = validateProduitFields(nomEdit, prixEdit, dimEdit, matEdit, imageEdit);
        if (!errors.isEmpty()) {
            errorLbl->setText(errors.join("\n"));
            errorLbl->show();
            dialog.adjustSize();
            return;
        }
        dialog.accept();
    });
    connect(bb, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    Produit p;
    p.setId          (realId);
    p.setNom         (nomEdit->text().trimmed());
    p.setCategorie   (catEdit->currentText());
    p.setPrix        (prixEdit->text().trimmed().replace(',','.').toDouble());
    p.setDimensions  (dimEdit->text().trimmed());
    p.setMatUtilise  (matEdit->text().trimmed());
    p.setDateCreation(dateEdit->date());
    p.setImage       (imageEdit->text().trimmed().toUtf8());
    p.setProjetId    (projetCombo->currentText());

    if (produitDB.updateProduit(p)) {
        QMessageBox::information(this, "Succès", "Produit modifié avec succès !");
        loadProduits();
    } else {
        QMessageBox::critical(this, "Erreur BD",
                              "Modification échouée !\nVérifiez la console pour le détail.");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SUPPRESSION
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onDeleteProductClicked()
{
    QWidget *productsPage = stackedWidget->widget(4);
    QTableWidget *table = productsPage ? productsPage->findChild<QTableWidget*>("productsTable") : nullptr;

    if (!table || table->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection",
                             "Veuillez sélectionner un produit dans la liste avant de supprimer.");
        return;
    }

    int row = table->currentRow();
    QString id  = table->item(row,0) ? table->item(row,0)->data(Qt::UserRole).toString() : "";
    QString nom = table->item(row,1) ? table->item(row,1)->text() : id;

    if (id.isEmpty()) {
        QMessageBox::critical(this, "Erreur", "Impossible de récupérer l'ID du produit sélectionné.");
        return;
    }

    // Boîte de confirmation renforcée avec détails du produit
    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("Confirmer la suppression");
    confirmBox.setIcon(QMessageBox::Warning);
    confirmBox.setText(QString("<b>Supprimer le produit suivant ?</b>"));
    confirmBox.setInformativeText(
        QString("Nom : <b>%1</b><br>ID : %2<br><br>"
                "<span style='color:#e53e3e;'>Cette action est irreversible.</span>")
            .arg(nom, id));
    QPushButton *btnSupp = confirmBox.addButton("Supprimer", QMessageBox::DestructiveRole);
    btnSupp->setStyleSheet("background-color:#e53e3e;color:white;border:none;"
                           "border-radius:4px;padding:6px 16px;font-weight:bold;");
    QPushButton *btnAnn  = confirmBox.addButton("Annuler",   QMessageBox::RejectRole);
    btnAnn->setStyleSheet("background-color:#e2e8f0;color:#4a5568;border:none;"
                          "border-radius:4px;padding:6px 16px;");
    confirmBox.setDefaultButton(btnAnn);
    confirmBox.exec();

    if (confirmBox.clickedButton() != btnSupp) return;

    if (produitDB.deleteProduit(id)) {
        QMessageBox::information(this, "Succès",
                                 QString("Le produit \"%1\" a été supprimé avec succès.").arg(nom));
        loadProduits();
    } else {
        QMessageBox::critical(this, "Erreur BD",
                              "Suppression échouée !\n"
                              "Ce produit est peut-être lié à d'autres enregistrements.");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Divers
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::addEmployee()    {}
void MainWindow::editEmployee()   {}
void MainWindow::deleteEmployee() {}

void MainWindow::onSidebarButtonClicked(int index)
{
    for (int i = 0; i < sidebarButtons.size(); ++i) {
        QPushButton *btn = sidebarButtons[i];
        btn->setChecked(i == index);
        btn->setProperty("active", i == index);
        btn->style()->unpolish(btn); btn->style()->polish(btn);
    }
    stackedWidget->setCurrentIndex(index);

    if (index == 4) loadProduits();   // recharge à chaque visite

    if (index == 1) {
        QWidget *page = stackedWidget->widget(index);
        currentTable = page->findChild<QTableWidget*>("dataTable");
    }

    QStringList titles = {
        "Gestion des Projets","Gestion des Employes",
        "Gestion des Stocks","Gestion Financiere","Gestion des Designs",
        "Assistant IA"
    };
    if (index >= 0 && index < titles.size())
        pageTitle->setText(titles[index]);
}

void MainWindow::toggleDarkMode() { isDarkMode = darkModeToggle->isChecked(); loadStyleSheet(); }

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if      (event->key()==Qt::Key_Plus||event->key()==Qt::Key_Equal) { onScaleUp();    event->accept(); return; }
        else if (event->key()==Qt::Key_Minus)                              { onScaleDown();  event->accept(); return; }
        else if (event->key()==Qt::Key_0)                                  { onScaleReset(); event->accept(); return; }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::onScaleUp()    { if (currentScale<1.5) { currentScale+=0.1; applyScale(currentScale); } }
void MainWindow::onScaleDown()  { if (currentScale>0.7) { currentScale-=0.1; applyScale(currentScale); } }
void MainWindow::onScaleReset() { currentScale=1.0; applyScale(currentScale); }

void MainWindow::applyScale(qreal scale)
{
    // On repart toujours d'une taille de base fixe (9pt) multipliée par scale.
    // On ne lit PAS qApp->font().pointSize() car il peut valoir -1 quand la
    // police système est définie en pixels (cas fréquent sur Windows/MinGW),
    // ce qui provoquerait un QFont::setPointSize: Point size <= 0.
    QFont font = qApp->font();
    const qreal basePointSize = 9.0;   // taille de référence toujours > 0
    qreal newSize = basePointSize * scale;
    if (newSize > 0.0)
        font.setPointSizeF(newSize);
    qApp->setFont(font);
    resize(static_cast<int>(1000 * scale), static_cast<int>(500 * scale));
}

void MainWindow::loadStyleSheet()
{
    QString filename = isDarkMode ? "style-dark.qss" : "style.qss";

    // Essai dans plusieurs emplacements candidats
    QStringList candidates = {
        filename,
        QCoreApplication::applicationDirPath() + "/" + filename,
        QCoreApplication::applicationDirPath() + "/../" + filename,
        "src/" + filename,
        "../" + filename,
    };

    QString sheet;
    QString usedPath;
    for (const QString& path : candidates) {
        QFile f(path);
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&f);
            sheet = stream.readAll();
            f.close();
            usedPath = path;
            break;
        }
    }

    if (sheet.isEmpty()) {
        qDebug() << "[StyleSheet] Fichier introuvable dans tous les emplacements cherchés :"
                 << candidates;
        qApp->setStyleSheet("");
        return;
    }

    qDebug() << "[StyleSheet] Chargé depuis :" << usedPath;
    qApp->setStyleSheet(sheet);
}

QFrame* MainWindow::createSeparator()
{
    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    return sep;
}

void MainWindow::populateProductTable(QTableWidget *table,
                                      const QList<Produit>& list)
{
    table->setRowCount(0);

    for (int i = 0; i < list.size(); ++i) {
        const Produit& p = list[i];
        table->insertRow(i);

        auto item = [](const QString& txt,
                       Qt::Alignment align = Qt::AlignVCenter | Qt::AlignLeft)
        {
            QTableWidgetItem *it = new QTableWidgetItem(txt);
            it->setTextAlignment(align);
            return it;
        };

        // ID
        QTableWidgetItem *idItem = item(p.getId(), Qt::AlignCenter | Qt::AlignVCenter);
        idItem->setData(Qt::UserRole, p.getId());
        table->setItem(i, 0, idItem);

        // Nom
        table->setItem(i, 1, item(p.getNom()));

        // Catégorie
        table->setItem(i, 2, item(p.getCategorie(), Qt::AlignCenter | Qt::AlignVCenter));

        // Prix
        QTableWidgetItem *prixItem = item(
            QString::number(p.getPrix(), 'f', 2) + " DT",
            Qt::AlignRight | Qt::AlignVCenter);
        prixItem->setData(Qt::UserRole + 1, p.getPrix());
        table->setItem(i, 3, prixItem);

        // Dimensions
        table->setItem(i, 4, item(p.getDimensions()));

        // Matériaux
        table->setItem(i, 5, item(p.getMatUtilise()));

        // Date création
        QTableWidgetItem *dateItem = item(
            p.getDateCreation().isValid()
                ? p.getDateCreation().toString("dd/MM/yyyy") : "—",
            Qt::AlignCenter | Qt::AlignVCenter);
        dateItem->setData(Qt::UserRole + 2, p.getDateCreation());
        table->setItem(i, 6, dateItem);

        // ── Image miniature ──────────────────────────────────────────────────
        if (p.hasImage()) {
            QPixmap pm;
            bool loaded = pm.loadFromData(p.getImage());
            if (!loaded) {
                pm.load(QString::fromUtf8(p.getImage()));
            }
            if (!pm.isNull()) {
                QLabel *imgLbl = new QLabel();
                imgLbl->setAlignment(Qt::AlignCenter);
                QPixmap thumb = pm.scaled(44, 44, Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
                QPixmap rounded(44, 44);
                rounded.fill(Qt::transparent);
                QPainter painter(&rounded);
                painter.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addRoundedRect(0, 0, 44, 44, 6, 6);
                painter.setClipPath(path);
                painter.drawPixmap(0, 0, thumb);
                imgLbl->setPixmap(rounded);
                imgLbl->setToolTip(QString::fromUtf8(p.getImage()));
                table->setCellWidget(i, 7, imgLbl);
            } else {
                QLabel *ph = new QLabel();
                ph->setAlignment(Qt::AlignCenter);
                QPixmap pm2(44, 44);
                pm2.fill(Qt::transparent);
                QPainter pa(&pm2);
                pa.setRenderHint(QPainter::Antialiasing);
                pa.setBrush(QColor("#e2e8f0"));
                pa.setPen(Qt::NoPen);
                pa.drawRoundedRect(0, 0, 44, 44, 6, 6);
                pa.setPen(QColor("#a0aec0"));
                QFont f; f.setPixelSize(16); f.setBold(true);
                pa.setFont(f);
                pa.drawText(QRect(0, 0, 44, 44), Qt::AlignCenter,
                            p.getNom().isEmpty() ? "?" : QString(p.getNom().at(0).toUpper()));
                ph->setPixmap(pm2);
                ph->setToolTip("Nom fichier : " + QString::fromUtf8(p.getImage()));
                table->setCellWidget(i, 7, ph);
            }
        } else {
            QLabel *ph = new QLabel();
            ph->setAlignment(Qt::AlignCenter);
            QPixmap pm2(44, 44);
            pm2.fill(Qt::transparent);
            QPainter pa(&pm2);
            pa.setRenderHint(QPainter::Antialiasing);
            pa.setBrush(QColor("#f0f0f0"));
            pa.setPen(Qt::NoPen);
            pa.drawRoundedRect(0, 0, 44, 44, 6, 6);
            pa.setPen(QColor("#cbd5e0"));
            QFont f; f.setPixelSize(20);
            pa.setFont(f);
            pa.drawText(QRect(0, 0, 44, 44), Qt::AlignCenter, "—");
            ph->setPixmap(pm2);
            table->setCellWidget(i, 7, ph);
        }

        // Projet
        table->setItem(i, 8, item(p.getProjetId(), Qt::AlignCenter | Qt::AlignVCenter));

        table->setRowHeight(i, 52);
    }

    // Vider le panneau détail quand la liste change
    clearDetailPanel();
}

void MainWindow::onProductSearchOrSort(const QString& text, const QString& sortKey)
{
    QWidget *productsPage = stackedWidget->widget(4);
    if (!productsPage) return;
    QTableWidget *table = productsPage->findChild<QTableWidget*>("productsTable");
    if (!table) return;

    // ── Filtrage ─────────────────────────────────────────────────────────────
    QList<Produit> filtered;
    const QString q = text.trimmed().toLower();
    for (const Produit& p : m_allProduits) {
        if (q.isEmpty()
            || p.getNom().toLower().contains(q)
            || p.getCategorie().toLower().contains(q))
        {
            filtered.append(p);
        }
    }

    // ── Tri ──────────────────────────────────────────────────────────────────
    auto cmp = [&](const Produit& a, const Produit& b) -> bool {
        if (sortKey == "date_desc") return a.getDateCreation() > b.getDateCreation();
        if (sortKey == "date_asc")  return a.getDateCreation() < b.getDateCreation();
        if (sortKey == "prix_desc") return a.getPrix() > b.getPrix();
        if (sortKey == "prix_asc")  return a.getPrix() < b.getPrix();
        if (sortKey == "nom_asc")   return a.getNom().toLower() < b.getNom().toLower();
        if (sortKey == "nom_desc")  return a.getNom().toLower() > b.getNom().toLower();
        return false;
    };
    std::sort(filtered.begin(), filtered.end(), cmp);

    populateProductTable(table, filtered);
}

void MainWindow::updateProductStatCards(QWidget *productsPage,
                                        const QList<Produit>& list)
{
    int total   = list.size();
    double sum  = 0.0;
    int newThis = 0;
    QDate today = QDate::currentDate();

    for (const Produit& p : list) {
        sum += p.getPrix();
        if (p.getDateCreation().year()  == today.year() &&
            p.getDateCreation().month() == today.month())
            ++newThis;
    }
    double avg = total > 0 ? sum / total : 0.0;

    auto setCard = [&](const QString& objName, const QString& val) {
        if (QLabel *lbl = productsPage->findChild<QLabel*>(objName))
            lbl->setText(val);
    };

    setCard("stat_total",   QString::number(total));
    setCard("stat_avg",     total > 0 ? QString::number(avg, 'f', 2) : "—");
    setCard("stat_new",     QString::number(newThis));
    setCard("stat_catalog", total > 0
                                ? QString::number(sum, 'f', 2) + " DT" : "—");
}

// ─────────────────────────────────────────────────────────────────────────────
// Nice-scale : calcule un pas "rond" pour l'axe Y
// ─────────────────────────────────────────────────────────────────────────────
struct NiceScale { double scaleMin, scaleMax, tickStep; int nbTicks; };

static NiceScale computeNiceScale(double dataMax, int targetTicks = 5)
{
    if (dataMax <= 0) return {0, 1, 1, 1};

    double rawStep  = dataMax / targetTicks;
    double magnitude = std::pow(10.0, std::floor(std::log10(rawStep)));
    double normStep  = rawStep / magnitude;

    double niceStep;
    if      (normStep <= 1.0) niceStep = 1.0;
    else if (normStep <= 2.0) niceStep = 2.0;
    else if (normStep <= 2.5) niceStep = 2.5;
    else if (normStep <= 5.0) niceStep = 5.0;
    else                      niceStep = 10.0;

    double tickStep = niceStep * magnitude;
    double scaleMax = std::ceil(dataMax / tickStep) * tickStep;
    int nb = static_cast<int>(std::round(scaleMax / tickStep));
    if (nb < 2) { scaleMax = tickStep * 2; nb = 2; }

    return {0.0, scaleMax, tickStep, nb};
}

static QPixmap drawBarChart(
    const QMap<QString, int>& data,
    int w, int h,
    const QString& title,
    const QColor& barColor = QColor("#8A9A5B"))
{
    QPixmap pm(w, h);
    pm.fill(QColor("#f8fafc"));
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    const int marginL = 56, marginR = 16, marginT = 36, marginB = 52;
    const int chartW  = w - marginL - marginR;
    const int chartH  = h - marginT - marginB;

    p.setPen(QColor("#1a202c"));
    QFont tf; tf.setBold(true); tf.setPixelSize(13); p.setFont(tf);
    p.drawText(QRect(0, 6, w, 24), Qt::AlignCenter, title);

    if (data.isEmpty()) { p.end(); return pm; }

    int maxVal = *std::max_element(data.cbegin(), data.cend());
    if (maxVal == 0) { p.end(); return pm; }

    NiceScale ns = computeNiceScale(static_cast<double>(maxVal), 5);
    double scaleMax  = ns.scaleMax;
    double tickStep  = ns.tickStep;
    int    nbTicks   = ns.nbTicks;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#ffffff"));
    p.drawRect(marginL, marginT, chartW, chartH);

    QFont gf; gf.setPixelSize(10); p.setFont(gf);
    for (int i = 0; i <= nbTicks; ++i) {
        double val = i * tickStep;
        int    y   = marginT + chartH - static_cast<int>(chartH * val / scaleMax);

        if (i == 0) {
            p.setPen(QPen(QColor("#94a3b8"), 1.5));
        } else {
            p.setPen(QPen(QColor("#e2e8f0"), 1.0, Qt::DashLine));
        }
        p.drawLine(marginL, y, marginL + chartW, y);

        p.setPen(QPen(QColor("#94a3b8"), 1.5));
        p.drawLine(marginL - 5, y, marginL, y);

        p.setPen(QColor("#4a5568"));
        QString label = (tickStep >= 1.0 && std::fmod(val, 1.0) == 0.0)
                            ? QString::number(static_cast<int>(val))
                            : QString::number(val, 'g', 3);
        p.drawText(QRect(0, y - 9, marginL - 8, 18),
                   Qt::AlignRight | Qt::AlignVCenter, label);
    }

    p.setPen(QPen(QColor("#64748b"), 1.5));
    p.drawLine(marginL, marginT, marginL, marginT + chartH);

    QList<QColor> palette = {
        QColor("#8A9A5B"), QColor("#2b6cb0"), QColor("#d69e2e"),
        QColor("#e53e3e"), QColor("#38a169"), QColor("#805ad5"),
        QColor("#dd6b20")
    };

    int nbars = data.size();
    int barW  = qMax(10, (chartW - (nbars + 1) * 10) / nbars);
    int gap   = (chartW - nbars * barW) / (nbars + 1);
    int idx   = 0;

    for (auto it = data.cbegin(); it != data.cend(); ++it, ++idx) {
        double ratio = static_cast<double>(it.value()) / scaleMax;
        int barH = static_cast<int>(chartH * ratio);
        int x    = marginL + gap + idx * (barW + gap);
        int y    = marginT + chartH - barH;

        QColor c = palette[idx % palette.size()];

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 18));
        p.drawRoundedRect(x + 2, y + 2, barW, barH, 4, 4);

        QLinearGradient grad(x, y, x, y + barH);
        grad.setColorAt(0, c.lighter(120));
        grad.setColorAt(1, c);
        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(x, y, barW, barH, 4, 4);

        QString valStr = QString::number(it.value());
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 200));
        p.drawRoundedRect(x + barW/2 - 14, y - 20, 28, 16, 4, 4);
        p.setPen(c.darker(140));
        QFont vf; vf.setPixelSize(10); vf.setBold(true); p.setFont(vf);
        p.drawText(QRect(x + barW/2 - 14, y - 20, 28, 16),
                   Qt::AlignCenter, valStr);

        p.setPen(QColor("#4a5568"));
        QFont llf; llf.setPixelSize(9); p.setFont(llf);
        QString label = it.key();
        if (label.length() > 10) label = label.left(9) + "…";
        p.save();
        p.translate(x + barW / 2, marginT + chartH + 8);
        p.rotate(-28);
        p.drawText(QRect(-32, 0, 64, 14), Qt::AlignLeft, label);
        p.restore();
    }

    p.end();
    return pm;
}

static QPixmap drawPieChart(
    const QMap<QString, int>& data,
    int w, int h,
    const QString& title)
{
    QPixmap pm(w, h);
    pm.fill(QColor("#f8fafc"));
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(QColor("#1a202c"));
    QFont tf; tf.setBold(true); tf.setPixelSize(13); p.setFont(tf);
    p.drawText(QRect(0, 6, w, 24), Qt::AlignCenter, title);

    if (data.isEmpty()) return pm;

    int total = 0;
    for (int v : data) total += v;
    if (total == 0) return pm;

    QList<QColor> palette = {
        QColor("#8A9A5B"), QColor("#2b6cb0"), QColor("#d69e2e"),
        QColor("#e53e3e"), QColor("#38a169"), QColor("#805ad5"),
        QColor("#dd6b20")
    };

    int diameter = qMin(w - 160, h - 60);
    int cx = 24 + diameter / 2;
    int cy = 30 + (h - 36) / 2;
    QRectF pieRect(cx - diameter / 2, cy - diameter / 2, diameter, diameter);

    int startAngle = 0;
    int idx = 0;
    QFont lf; lf.setPixelSize(10); p.setFont(lf);

    for (auto it = data.cbegin(); it != data.cend(); ++it, ++idx) {
        int span = static_cast<int>(360.0 * it.value() / total * 16);
        QColor c = palette[idx % palette.size()];

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 18));
        p.drawPie(pieRect.adjusted(2, 2, 2, 2), startAngle, span);

        p.setBrush(c);
        p.drawPie(pieRect, startAngle, span);

        int ly = 40 + idx * 22;
        p.fillRect(cx + diameter / 2 + 16, ly, 14, 14, c);
        p.setPen(QColor("#2d3748"));
        double pct = 100.0 * it.value() / total;
        p.drawText(cx + diameter / 2 + 34, ly + 12,
                   QString("%1  %2%").arg(it.key()).arg(pct, 0, 'f', 1));

        startAngle += span;
    }

    p.end();
    return pm;
}

static QPixmap drawPriceHistogram(const QList<Produit>& list, int w, int h)
{
    QMap<QString, int> buckets;
    QVector<QPair<double,double>> ranges = {{0,100},{100,300},{300,600},{600,1000},{1000,1e9}};
    QStringList labels = {"< 100 DT","100-300 DT","300-600 DT","600-1000 DT","> 1000 DT"};
    for (const QString& l : labels) buckets[l] = 0;
    for (const Produit& prod : list) {
        double prix = prod.getPrix();
        for (int i = 0; i < ranges.size(); ++i) {
            if (prix >= ranges[i].first && prix < ranges[i].second) {
                buckets[labels[i]]++;
                break;
            }
        }
    }
    QMap<QString,int> ordered;
    for (const QString& l : labels) ordered.insert(l, buckets[l]);
    return drawBarChart(ordered, w, h, "Répartition des Prix", QColor("#2b6cb0"));
}

void MainWindow::onShowProductStats()
{
    const QList<Produit>& list = m_allProduits;
    if (list.isEmpty()) {
        QMessageBox::information(this, "Statistiques",
                                 "Aucun produit en base de données.");
        return;
    }

    int total = list.size();
    QMap<QString, int> byCategorie;
    QMap<QString, int> byMat;
    QMap<int,    int>  byYear;
    double sum = 0.0, minPrix = list[0].getPrix(), maxPrix = list[0].getPrix();

    for (const Produit& prod : list) {
        byCategorie[prod.getCategorie()]++;
        byMat[prod.getMatUtilise()]++;
        if (prod.getDateCreation().isValid())
            byYear[prod.getDateCreation().year()]++;
        sum    += prod.getPrix();
        minPrix = std::min(minPrix, prod.getPrix());
        maxPrix = std::max(maxPrix, prod.getPrix());
    }
    double avg = sum / total;

    QString topCat = byCategorie.begin().key(); int topCatN = byCategorie.begin().value();
    for (auto it = byCategorie.begin(); it != byCategorie.end(); ++it)
        if (it.value() > topCatN) { topCat = it.key(); topCatN = it.value(); }

    QString topMat = byMat.begin().key(); int topMatN = byMat.begin().value();
    for (auto it = byMat.begin(); it != byMat.end(); ++it)
        if (it.value() > topMatN) { topMat = it.key(); topMatN = it.value(); }

    QDialog dlg(this);
    dlg.setWindowTitle("Statistiques des Produits — WoodFlow");
    dlg.setMinimumSize(900, 680);
    dlg.setStyleSheet("QDialog { background: #f7fafc; }"
                      "QScrollArea { border: none; background: transparent; }"
                      "QLabel#sectionTitle { font-size:13px; font-weight:bold; color:#8A9A5B;"
                      "  border-bottom: 2px solid #8A9A5B; padding-bottom:4px; margin-top:12px; }"
                      "QFrame#kpiCard { background: white; border-radius: 10px;"
                      "  border: 1px solid #e2e8f0; }");

    QVBoxLayout *rootLay = new QVBoxLayout(&dlg);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);

    QFrame *header = new QFrame(&dlg);
    header->setFixedHeight(70);
    header->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                          "  stop:0 #1a202c, stop:1 #2d3748);"
                          "border-radius: 0px;");
    QHBoxLayout *hdrLay = new QHBoxLayout(header);
    hdrLay->setContentsMargins(24, 0, 24, 0);

    QLabel *logoHdr = new QLabel(header);
    QPixmap logoPm("src/assets/icons/logo1.png");
    if (!logoPm.isNull())
        logoHdr->setPixmap(logoPm.scaled(44, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoHdr->setFixedSize(50, 50);

    QLabel *hdrTitle = new QLabel("Tableau de bord — Gestion des Produits", header);
    hdrTitle->setStyleSheet("color:white;font-size:17px;font-weight:bold;");

    QLabel *hdrSub = new QLabel(QString("Analyse sur %1 produits · %2")
                                    .arg(total)
                                    .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy")),
                                header);
    hdrSub->setStyleSheet("color:#a0aec0;font-size:11px;");

    QVBoxLayout *hdrTextLay = new QVBoxLayout();
    hdrTextLay->addWidget(hdrTitle);
    hdrTextLay->addWidget(hdrSub);
    hdrLay->addWidget(logoHdr);
    hdrLay->addLayout(hdrTextLay);
    hdrLay->addStretch();
    rootLay->addWidget(header);

    QScrollArea *scroll = new QScrollArea(&dlg);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QWidget *content = new QWidget();
    content->setStyleSheet("background: #f7fafc;");
    QVBoxLayout *lay = new QVBoxLayout(content);
    lay->setContentsMargins(24, 20, 24, 20);
    lay->setSpacing(14);
    scroll->setWidget(content);
    rootLay->addWidget(scroll);

    auto makeKpi = [&](const QString& label,
                       const QString& val, const QString& accent) -> QFrame*
    {
        QFrame *card = new QFrame(content);
        card->setObjectName("kpiCard");
        card->setFixedHeight(90);
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 10, 16, 10);
        cl->setSpacing(4);

        QLabel *lblLbl = new QLabel(label, card);
        lblLbl->setStyleSheet("color:#718096; font-size:11px; font-weight:bold;");

        QLabel *valLbl = new QLabel(val, card);
        valLbl->setStyleSheet(QString("color:%1; font-size:22px; font-weight:bold;").arg(accent));

        cl->addWidget(lblLbl);
        cl->addWidget(valLbl);
        cl->addStretch();
        return card;
    };
    QHBoxLayout *kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(12);
    kpiRow->addWidget(makeKpi("TOTAL PRODUITS",   QString::number(total),                     "#2d3748"));
    kpiRow->addWidget(makeKpi("PRIX MOYEN",       QString::number(avg,    'f', 2) + " DT",   "#2b6cb0"));
    kpiRow->addWidget(makeKpi("PRIX MIN",         QString::number(minPrix,'f', 2) + " DT",   "#38a169"));
    kpiRow->addWidget(makeKpi("PRIX MAX",         QString::number(maxPrix,'f', 2) + " DT",   "#e53e3e"));
    kpiRow->addWidget(makeKpi("VALEUR CATALOGUE", QString::number(sum,    'f', 0) + " DT",   "#8A9A5B"));
    lay->addLayout(kpiRow);

    QLabel *secCharts = new QLabel("Graphiques analytiques", content);
    secCharts->setObjectName("sectionTitle");
    lay->addWidget(secCharts);

    QHBoxLayout *chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(12);

    QFrame *chartCatFrame = new QFrame(content);
    chartCatFrame->setObjectName("kpiCard");
    QVBoxLayout *ccl = new QVBoxLayout(chartCatFrame);
    ccl->setContentsMargins(8, 8, 8, 8);
    QLabel *chartCatLbl = new QLabel(chartCatFrame);
    chartCatLbl->setPixmap(drawBarChart(byCategorie, 380, 210, "Produits par catégorie"));
    ccl->addWidget(chartCatLbl);
    chartsRow->addWidget(chartCatFrame);

    QFrame *chartPieFrame = new QFrame(content);
    chartPieFrame->setObjectName("kpiCard");
    QVBoxLayout *cpl = new QVBoxLayout(chartPieFrame);
    cpl->setContentsMargins(8, 8, 8, 8);
    QLabel *chartPieLbl = new QLabel(chartPieFrame);
    chartPieLbl->setPixmap(drawPieChart(byCategorie, 380, 210, "Part par catégorie (%)"));
    cpl->addWidget(chartPieLbl);
    chartsRow->addWidget(chartPieFrame);

    lay->addLayout(chartsRow);

    QFrame *priceFrame = new QFrame(content);
    priceFrame->setObjectName("kpiCard");
    QHBoxLayout *pfl = new QHBoxLayout(priceFrame);
    pfl->setContentsMargins(8, 8, 8, 8);
    QLabel *priceChartLbl = new QLabel(priceFrame);
    priceChartLbl->setPixmap(drawPriceHistogram(list, 780, 180));
    pfl->addWidget(priceChartLbl);
    lay->addWidget(priceFrame);

    QFrame *matFrame = new QFrame(content);
    matFrame->setObjectName("kpiCard");
    QHBoxLayout *mfl = new QHBoxLayout(matFrame);
    mfl->setContentsMargins(8, 8, 8, 8);
    QLabel *matChartLbl = new QLabel(matFrame);
    matChartLbl->setPixmap(drawBarChart(byMat, 780, 180, "Produits par matériau", QColor("#d69e2e")));
    mfl->addWidget(matChartLbl);
    lay->addWidget(matFrame);

    QLabel *secDetail = new QLabel("Détail analytique", content);
    secDetail->setObjectName("sectionTitle");
    lay->addWidget(secDetail);

    auto kv = [&](const QString& k, const QString& v, const QString& color = "#2d3748") {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *kl = new QLabel(k + " :", content);
        kl->setStyleSheet("color:#4a5568;font-size:12px;");
        QLabel *vl = new QLabel(v, content);
        vl->setStyleSheet(QString("font-weight:bold;font-size:12px;color:%1;").arg(color));
        vl->setAlignment(Qt::AlignRight);
        row->addWidget(kl); row->addStretch(); row->addWidget(vl);
        lay->addLayout(row);
    };

    kv("Catégorie dominante",
       QString("%1  (%2 produits, %3 %)")
           .arg(topCat).arg(topCatN)
           .arg(100.0 * topCatN / total, 0, 'f', 1),
       "#8A9A5B");

    kv("Matériau le plus utilisé",
       QString("%1  (%2 produits)").arg(topMat).arg(topMatN),
       "#2b6cb0");

    if (!byYear.isEmpty()) {
        QLabel *secYear = new QLabel("Créations par année", content);
        secYear->setObjectName("sectionTitle");
        lay->addWidget(secYear);
        for (auto it = byYear.cbegin(); it != byYear.cend(); ++it)
            kv(QString::number(it.key()), QString::number(it.value()) + " produit(s)");
    }

    QPushButton *closeBtn = new QPushButton("Fermer", content);
    closeBtn->setFixedHeight(42);
    closeBtn->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #8A9A5B,stop:1 #9aaa6b);"
        "color:white;border:none;border-radius:8px;"
        "padding:0 28px;font-weight:bold;font-size:13px;}"
        "QPushButton:hover{background:#9aaa6b;}");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    lay->addWidget(closeBtn, 0, Qt::AlignCenter);

    dlg.exec();
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers PDF internes
// ─────────────────────────────────────────────────────────────────────────────
static void drawRoundRect(QPainter& p, const QRectF& r, qreal rx, const QColor& fill,
                          const QColor& border = Qt::transparent, qreal bw = 0)
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(fill);
    p.setPen(border == Qt::transparent ? Qt::NoPen : QPen(border, bw));
    p.drawRoundedRect(r, rx, rx);
    p.restore();
}

static void drawText(QPainter& p, const QRectF& r, const QString& text,
                     const QFont& font, const QColor& color,
                     int flags = Qt::AlignLeft | Qt::AlignVCenter)
{
    p.save();
    p.setFont(font);
    p.setPen(color);
    p.drawText(r, flags, text);
    p.restore();
}

static QPixmap loadProduitImage(const Produit& prod)
{
    QPixmap pm;

    if (prod.hasImage() && !prod.getImage().isEmpty())
        pm.loadFromData(prod.getImage());

    if (pm.isNull() && prod.hasImage()) {
        QString fn = QString::fromUtf8(prod.getImage()).trimmed();
        if (!fn.isEmpty()) {
            QString appDir = QCoreApplication::applicationDirPath();
            QStringList candidates = {
                "src/assets/icons/" + fn,
                appDir + "/src/assets/icons/" + fn,
                appDir + "/../src/assets/icons/" + fn,
                appDir + "/../../src/assets/icons/" + fn,
                appDir + "/../../../src/assets/icons/" + fn,
                appDir + "/" + fn,
                fn,
            };
            for (const QString& c : candidates)
                if (pm.load(c)) {
                    qDebug() << "[loadProduitImage] Chargé depuis:" << c;
                    break;
                }
        }
    }
    return pm;
}

static QPixmap makePlaceholder(const Produit& prod, int w, int h,
                               const QMap<QString,QColor>& catAccent,
                               const QMap<QString,QColor>& catBg)
{
    QPixmap ph(w, h);
    ph.fill(Qt::transparent);
    QPainter pa(&ph);
    pa.setRenderHint(QPainter::Antialiasing);

    QColor bg     = catBg    .value(prod.getCategorie(), QColor("#F9FAFB"));
    QColor accent = catAccent.value(prod.getCategorie(), QColor("#6B7280"));

    pa.setBrush(bg);
    pa.setPen(Qt::NoPen);
    pa.drawRoundedRect(0, 0, w, h, 14, 14);

    QRadialGradient rg(w/2, h/2, h*0.38);
    rg.setColorAt(0, accent.lighter(170));
    rg.setColorAt(1, accent.lighter(140));
    pa.setBrush(rg);
    pa.setPen(Qt::NoPen);
    int cr = int(h * 0.38);
    pa.drawEllipse(w/2 - cr, h/2 - cr - 10, cr*2, cr*2);

    QString initials;
    for (const QString& word : prod.getNom().split(' ', Qt::SkipEmptyParts))
        if (initials.length() < 2) initials += word.at(0).toUpper();
    if (initials.isEmpty()) initials = "?";

    pa.setPen(accent.darker(115));
    QFont fi; fi.setPixelSize(int(h * 0.30)); fi.setBold(true); pa.setFont(fi);
    pa.drawText(QRect(0, h/2 - cr - 10, w, cr*2), Qt::AlignCenter, initials);

    pa.setPen(accent.darker(140));
    QFont fc; fc.setPixelSize(int(h * 0.10)); fc.setBold(true); pa.setFont(fc);
    pa.drawText(QRect(0, h - int(h*0.18), w, int(h*0.15)),
                Qt::AlignCenter, prod.getCategorie());
    pa.end();
    return ph;
}

// ═══════════════════════════════════════════════════════════════════════════════
void MainWindow::onExportProductsPDF()
// ═══════════════════════════════════════════════════════════════════════════════
{
    const QList<Produit>& list = m_allProduits;
    if (list.isEmpty()) {
        QMessageBox::information(this, "Export PDF", "Aucun produit à exporter.");
        return;
    }

    QString defaultPath =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/Catalogue_WoodFlow_"
        + QDate::currentDate().toString("yyyyMMdd") + ".pdf";

    QString filePath = QFileDialog::getSaveFileName(
        this, "Enregistrer le catalogue PDF", defaultPath,
        "Fichiers PDF (*.pdf)");
    if (filePath.isEmpty()) return;
    if (!filePath.endsWith(".pdf", Qt::CaseInsensitive)) filePath += ".pdf";

    static const QMap<QString,QColor> CAT_ACCENT = {
                                                     {"Meuble",     QColor("#3B82F6")}, {"Menuiserie", QColor("#10B981")},
                                                     {"Décoration", QColor("#F59E0B")}, {"Porte",      QColor("#8B5CF6")},
                                                     {"Fenêtre",    QColor("#14B8A6")}, {"Escalier",   QColor("#EF4444")},
                                                     {"Autre",      QColor("#6B7280")},
                                                     };
    static const QMap<QString,QColor> CAT_BG = {
                                                 {"Meuble",     QColor("#EFF6FF")}, {"Menuiserie", QColor("#ECFDF5")},
                                                 {"Décoration", QColor("#FFFBEB")}, {"Porte",      QColor("#F5F3FF")},
                                                 {"Fenêtre",    QColor("#F0FDFA")}, {"Escalier",   QColor("#FFF1F2")},
                                                 {"Autre",      QColor("#F9FAFB")},
                                                 };

    double sum = 0, minP = list[0].getPrix(), maxP = list[0].getPrix();
    QMap<QString,int> byCat;
    for (const Produit& prod : list) {
        sum  += prod.getPrix();
        minP  = std::min(minP, prod.getPrix());
        maxP  = std::max(maxP, prod.getPrix());
        byCat[prod.getCategorie()]++;
    }
    double avg = sum / list.size();

    QMap<QString, QPixmap> prodImages;
    for (const Produit& prod : list) {
        QPixmap pm = loadProduitImage(prod);
        if (pm.isNull())
            pm = makePlaceholder(prod, 400, 300, CAT_ACCENT, CAT_BG);
        prodImages[prod.getId()] = pm;
    }

    QPixmap logoPixmap;
    logoPixmap.load("src/assets/icons/logo1.png");
    if (logoPixmap.isNull())
        logoPixmap.load(QCoreApplication::applicationDirPath() + "/src/assets/icons/logo1.png");

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);

    QPainter p(&printer);
    if (!p.isActive()) {
        QMessageBox::critical(this, "Erreur", "Impossible d'initialiser le PDF.");
        return;
    }
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    const QRect page = printer.pageRect(QPrinter::DevicePixel).toRect();
    const int W  = page.width();
    const int H  = page.height();

    auto font = [&](int ptSize, bool bold = false, bool italic = false) -> QFont {
        QFont f("Arial", ptSize);
        f.setBold(bold);
        f.setItalic(italic);
        return f;
    };

    auto drawBadge = [&](int x, int y, int w, int h,
                         const QString& text, const QColor& bg, const QColor& fg)
    {
        drawRoundRect(p, QRectF(x, y, w, h), h/2.5, bg);
        p.save();
        p.setFont(font(8, true));
        p.setPen(fg);
        p.drawText(QRect(x, y, w, h), Qt::AlignCenter, text);
        p.restore();
    };

    auto hline = [&](int y, int x1, int x2, const QColor& c = QColor("#E2E8F0"), qreal w = 1.5) {
        p.save();
        p.setPen(QPen(c, w));
        p.drawLine(x1, y, x2, y);
        p.restore();
    };

    int pageNum = 0;
    auto drawFooter = [&]() {
        ++pageNum;
        int fy = H - int(H * 0.038);
        hline(fy, int(W*0.05), int(W*0.95), QColor("#CBD5E0"), 1.0);
        p.save();
        p.setFont(font(7));
        p.setPen(QColor("#94A3B8"));
        p.drawText(QRect(int(W*0.05), fy+4, int(W*0.5), int(H*0.03)),
                   Qt::AlignLeft | Qt::AlignTop,
                   "WoodFlow — Smart Carpentry Management · Catalogue confidentiel");
        p.drawText(QRect(int(W*0.5), fy+4, int(W*0.45), int(H*0.03)),
                   Qt::AlignRight | Qt::AlignTop,
                   QString("Page %1  ·  Généré le %2")
                       .arg(pageNum)
                       .arg(QDate::currentDate().toString("dd/MM/yyyy")));
        p.restore();
    };

    // PAGE 1 — COUVERTURE
    {
        QLinearGradient bg(0, 0, 0, H);
        bg.setColorAt(0.0, QColor("#0F172A"));
        bg.setColorAt(0.6, QColor("#1E3A5F"));
        bg.setColorAt(1.0, QColor("#0F172A"));
        p.fillRect(0, 0, W, H, bg);

        QLinearGradient topBar(0, 0, W, 0);
        topBar.setColorAt(0.0, QColor("#8A9A5B"));
        topBar.setColorAt(0.5, QColor("#2563EB"));
        topBar.setColorAt(1.0, QColor("#D97706"));
        p.fillRect(0, 0, W, int(H*0.012), topBar);

        p.save();
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255,255,255,12));
        p.drawEllipse(int(W*0.55), int(H*0.05), int(W*0.55), int(W*0.55));
        p.setBrush(QColor(255,255,255,8));
        p.drawEllipse(int(W*0.6), int(H*0.3), int(W*0.45), int(W*0.45));
        p.restore();

        int logoY = int(H * 0.09);
        if (!logoPixmap.isNull()) {
            QPixmap lpm = logoPixmap.scaled(int(W*0.28), int(H*0.10),
                                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
            p.drawPixmap(int(W*0.07), logoY, lpm);
        } else {
            p.save();
            p.setFont(font(36, true));
            p.setPen(QColor("#8A9A5B"));
            p.drawText(int(W*0.07), logoY + int(H*0.07), "WF");
            p.restore();
        }

        int titleY = int(H * 0.28);
        p.save();
        p.setFont(font(34, true));
        p.setPen(QColor("#FFFFFF"));
        p.drawText(QRect(int(W*0.07), titleY, int(W*0.86), int(H*0.12)),
                   Qt::AlignLeft | Qt::AlignVCenter, "WoodFlow");
        p.restore();

        p.save();
        p.setFont(font(22, false));
        p.setPen(QColor("#94A3B8"));
        p.drawText(QRect(int(W*0.07), titleY + int(H*0.10), int(W*0.86), int(H*0.07)),
                   Qt::AlignLeft | Qt::AlignVCenter, "Catalogue Officiel des Produits");
        p.restore();

        hline(int(H*0.46), int(W*0.07), int(W*0.93), QColor("#334155"), 1.5);

        p.save();
        p.setFont(font(10));
        p.setPen(QColor("#64748B"));
        p.drawText(QRect(int(W*0.07), int(H*0.47), int(W*0.86), int(H*0.04)),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QDateTime::currentDateTime().toString("dddd dd MMMM yyyy 'à' hh:mm")
                       + "  ·  Confidentiel — usage interne");
        p.restore();

        struct KpiData { QString val; QString label; QColor color; };
        QList<KpiData> kpis = {
                               { QString::number(list.size()),           "PRODUITS",          QColor("#8A9A5B") },
                               { QString::number(sum, 'f', 0) + " DT",  "VALEUR CATALOGUE",  QColor("#D97706") },
                               { QString::number(avg, 'f', 2) + " DT",  "PRIX MOYEN",        QColor("#3B82F6") },
                               { QString::number(minP,'f', 2) + " DT",  "PRIX MINIMUM",      QColor("#10B981") },
                               { QString::number(maxP,'f', 2) + " DT",  "PRIX MAXIMUM",      QColor("#EF4444") },
                               };

        int kpiY    = int(H * 0.55);
        int kpiH    = int(H * 0.15);
        int kpiGap  = int(W * 0.018);
        int kpiW    = (int(W * 0.86) - kpiGap * 4) / 5;
        int kpiX0   = int(W * 0.07);

        for (int k = 0; k < kpis.size(); ++k) {
            int kx = kpiX0 + k * (kpiW + kpiGap);
            drawRoundRect(p, QRectF(kx, kpiY, kpiW, kpiH), 10, QColor("#1E293B"));
            p.save();
            p.setBrush(kpis[k].color);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(kx, kpiY, kpiW, 5, 3, 3);
            p.restore();
            p.save();
            p.setFont(font(11, true));
            p.setPen(kpis[k].color);
            p.drawText(QRect(kx, kpiY + 12, kpiW, int(kpiH*0.55)),
                       Qt::AlignCenter | Qt::AlignVCenter, kpis[k].val);
            p.restore();
            p.save();
            p.setFont(font(7, true));
            p.setPen(QColor("#64748B"));
            p.drawText(QRect(kx, kpiY + int(kpiH*0.6), kpiW, int(kpiH*0.35)),
                       Qt::AlignCenter | Qt::AlignVCenter, kpis[k].label);
            p.restore();
        }

        int catY = int(H * 0.75);
        QList<QPair<QString,QColor>> catDefs = {
                                                 {"Meuble",     QColor("#3B82F6")}, {"Menuiserie", QColor("#10B981")},
                                                 {"Décoration", QColor("#F59E0B")}, {"Porte",      QColor("#8B5CF6")},
                                                 {"Fenêtre",    QColor("#14B8A6")}, {"Escalier",   QColor("#EF4444")},
                                                 {"Autre",      QColor("#6B7280")},
                                                 };
        QList<QPair<QString,QColor>> activeCats;
        for (const auto& cd : catDefs)
            if (byCat.value(cd.first, 0) > 0)
                activeCats.append(cd);

        if (!activeCats.isEmpty()) {
            int catBoxH  = int(H * 0.12);
            int catGap   = int(W * 0.015);
            int catBoxW  = (int(W * 0.86) - catGap * (activeCats.size()-1)) / activeCats.size();
            int catX0    = int(W * 0.07);

            for (int c = 0; c < activeCats.size(); ++c) {
                int cx = catX0 + c * (catBoxW + catGap);
                QColor col = activeCats[c].second;
                drawRoundRect(p, QRectF(cx, catY, catBoxW, catBoxH), 8, col);
                int n = byCat.value(activeCats[c].first, 0);
                p.save();
                p.setFont(font(18, true));
                p.setPen(Qt::white);
                p.drawText(QRect(cx, catY+4, catBoxW, int(catBoxH*0.55)),
                           Qt::AlignCenter, QString::number(n));
                p.setFont(font(7, true));
                p.setPen(QColor(255,255,255,210));
                p.drawText(QRect(cx, catY + int(catBoxH*0.58), catBoxW, int(catBoxH*0.35)),
                           Qt::AlignCenter, activeCats[c].first);
                p.restore();
            }
        }

        drawFooter();
    }

    // PAGE 2 — TABLEAU RÉCAPITULATIF
    printer.newPage();
    {
        p.fillRect(0, 0, W, int(H*0.10), QColor("#0F172A"));
        QLinearGradient hdrBar(0, 0, W, 0);
        hdrBar.setColorAt(0, QColor("#8A9A5B"));
        hdrBar.setColorAt(1, QColor("#2563EB"));
        p.fillRect(0, int(H*0.10), W, int(H*0.006), hdrBar);

        p.save();
        p.setFont(font(18, true));
        p.setPen(Qt::white);
        p.drawText(QRect(int(W*0.05), 0, int(W*0.7), int(H*0.10)),
                   Qt::AlignLeft | Qt::AlignVCenter, "Spécifications Techniques");
        p.setFont(font(11));
        p.setPen(QColor("#8A9A5B"));
        p.drawText(QRect(int(W*0.72), 0, int(W*0.23), int(H*0.10)),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString("%1 produits").arg(list.size()));
        p.restore();

        int tX = int(W * 0.035);
        int tW = int(W * 0.93);
        int tY = int(H * 0.115);
        int rowH = int((H * 0.84) / (list.size() + 1));
        rowH = qMin(rowH, int(H * 0.072));
        rowH = qMax(rowH, int(H * 0.042));

        struct Col { QString hdr; int pct; int align; };
        QList<Col> cols = {
                           {"#",         4,  Qt::AlignCenter},
                           {"Produit",  28,  Qt::AlignLeft},
                           {"Catégorie",14,  Qt::AlignCenter},
                           {"Prix DT",   12, Qt::AlignRight},
                           {"Dimensions",14, Qt::AlignLeft},
                           {"Matériaux", 14, Qt::AlignLeft},
                           {"Création",  10, Qt::AlignCenter},
                           {"Projet",    4,  Qt::AlignCenter},
                           };
        QList<int> colX, colW;
        int cx = tX + 4;
        for (const Col& c_ : cols) {
            colX.append(cx);
            int cw = tW * c_.pct / 100;
            colW.append(cw);
            cx += cw;
        }

        p.fillRect(tX, tY, tW, rowH, QColor("#0F172A"));
        p.save();
        p.setFont(font(8, true));
        p.setPen(Qt::white);
        for (int c_ = 0; c_ < cols.size(); ++c_) {
            p.drawText(QRect(colX[c_]+4, tY, colW[c_]-8, rowH),
                       cols[c_].align | Qt::AlignVCenter, cols[c_].hdr);
        }
        p.restore();

        for (int i = 0; i < list.size(); ++i) {
            const Produit& prod = list[i];
            int ry = tY + rowH * (i + 1);
            if (ry + rowH > int(H * 0.96)) break;

            QColor rowBg = (i % 2 == 0) ? Qt::white : QColor("#F8FAFC");
            p.fillRect(tX, ry, tW, rowH, rowBg);
            hline(ry + rowH, tX, tX + tW, QColor("#E2E8F0"), 0.8);

            QColor accentCol = CAT_ACCENT.value(prod.getCategorie(), QColor("#6B7280"));
            p.fillRect(tX, ry, 4, rowH, accentCol);

            p.save();
            p.setFont(font(8));
            p.setPen(QColor("#94A3B8"));
            p.drawText(QRect(colX[0]+4, ry, colW[0]-8, rowH), Qt::AlignCenter|Qt::AlignVCenter,
                       QString::number(i+1));

            p.setPen(QColor("#0F172A"));
            p.setFont(font(9, true));
            p.drawText(QRect(colX[1]+4, ry, colW[1]-8, rowH), Qt::AlignLeft|Qt::AlignVCenter,
                       prod.getNom());
            p.restore();

            int badgeW = colW[2] - 12;
            int badgeH = int(rowH * 0.55);
            int badgeX = colX[2] + 6;
            int badgeY = ry + (rowH - badgeH) / 2;
            drawRoundRect(p, QRectF(badgeX, badgeY, badgeW, badgeH), badgeH/2.5, accentCol);
            p.save();
            p.setFont(font(7, true));
            p.setPen(Qt::white);
            p.drawText(QRect(badgeX, badgeY, badgeW, badgeH), Qt::AlignCenter, prod.getCategorie());
            p.restore();

            p.save();
            p.setFont(font(9, true));
            p.setPen(QColor("#2563EB"));
            p.drawText(QRect(colX[3]+4, ry, colW[3]-8, rowH), Qt::AlignRight|Qt::AlignVCenter,
                       QString::number(prod.getPrix(),'f',2) + " DT");

            p.setFont(font(8));
            p.setPen(QColor("#475569"));
            p.drawText(QRect(colX[4]+4, ry, colW[4]-8, rowH), Qt::AlignLeft|Qt::AlignVCenter,
                       prod.getDimensions().isEmpty() ? "—" : prod.getDimensions());
            p.drawText(QRect(colX[5]+4, ry, colW[5]-8, rowH), Qt::AlignLeft|Qt::AlignVCenter,
                       prod.getMatUtilise().isEmpty() ? "—" : prod.getMatUtilise());

            p.setPen(QColor("#94A3B8"));
            p.drawText(QRect(colX[6]+4, ry, colW[6]-8, rowH), Qt::AlignCenter|Qt::AlignVCenter,
                       prod.getDateCreation().isValid()
                           ? prod.getDateCreation().toString("dd/MM/yy") : "—");
            p.drawText(QRect(colX[7]+4, ry, colW[7]-8, rowH), Qt::AlignCenter|Qt::AlignVCenter,
                       prod.getProjetId());
            p.restore();
        }

        p.save();
        p.setPen(QPen(QColor("#CBD5E0"), 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawRect(tX, tY, tW, rowH * (qMin(list.size(), int((H*0.84)/rowH)) + 1));
        p.restore();

        drawFooter();
    }

    // PAGES PRODUITS — 1 fiche par page
    for (const Produit& prod : list) {
        printer.newPage();

        QColor accent = CAT_ACCENT.value(prod.getCategorie(), QColor("#6B7280"));

        int hdrH = int(H * 0.155);
        QLinearGradient hdrGrad(0, 0, W, hdrH);
        hdrGrad.setColorAt(0, accent.darker(120));
        hdrGrad.setColorAt(1, accent);
        p.fillRect(0, 0, W, hdrH, hdrGrad);

        QLinearGradient tb(0,0,W,0);
        tb.setColorAt(0, Qt::white); tb.setColorAt(1, QColor(255,255,255,80));
        p.fillRect(0, 0, W, int(H*0.006), tb);

        p.save();
        p.setFont(font(22, true));
        p.setPen(Qt::white);
        p.drawText(QRect(int(W*0.045), int(hdrH*0.12), int(W*0.65), int(hdrH*0.52)),
                   Qt::AlignLeft | Qt::AlignVCenter, prod.getNom());

        p.setFont(font(9));
        p.setPen(QColor(255,255,255,180));
        p.drawText(QRect(int(W*0.045), int(hdrH*0.60), int(W*0.65), int(hdrH*0.28)),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QString("Référence : %1   ·   Projet : %2")
                       .arg(prod.getId()).arg(prod.getProjetId()));
        p.restore();

        p.save();
        p.setFont(font(26, true));
        p.setPen(Qt::white);
        p.drawText(QRect(int(W*0.55), int(hdrH*0.08), int(W*0.40), int(hdrH*0.58)),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(prod.getPrix(), 'f', 2) + " DT");
        p.restore();

        int bdgW = int(W * 0.18);
        int bdgH = int(hdrH * 0.25);
        int bdgX = int(W * 0.77);
        int bdgY = int(hdrH * 0.66);
        drawRoundRect(p, QRectF(bdgX, bdgY, bdgW, bdgH), bdgH/2.5,
                      QColor(255,255,255,50), QColor(255,255,255,150), 1.0);
        p.save();
        p.setFont(font(9, true));
        p.setPen(Qt::white);
        p.drawText(QRect(bdgX, bdgY, bdgW, bdgH), Qt::AlignCenter, prod.getCategorie());
        p.restore();

        p.fillRect(0, hdrH, W, H - hdrH, QColor("#F8FAFC"));

        int bodyY   = hdrH + int(H * 0.025);
        int bodyH   = int(H * 0.72) - int(H * 0.025);
        int imgZoneW = int(W * 0.50);
        int imgZoneH = bodyH;
        int imgPad   = int(W * 0.035);

        drawRoundRect(p, QRectF(int(W*0.025), bodyY,
                                imgZoneW - int(W*0.015), imgZoneH - int(H*0.01)),
                      12, Qt::white, QColor("#E2E8F0"), 1.0);

        QPixmap pm = prodImages.value(prod.getId());
        if (!pm.isNull()) {
            int imgMaxW = imgZoneW - imgPad*2 - int(W*0.015);
            int imgMaxH = imgZoneH - imgPad*2 - int(H*0.01);
            QPixmap scaled = pm.scaled(imgMaxW, imgMaxH,
                                       Qt::KeepAspectRatio, Qt::SmoothTransformation);
            int ix = int(W*0.025) + imgPad + (imgMaxW - scaled.width())  / 2;
            int iy = bodyY + imgPad + (imgMaxH - scaled.height()) / 2;

            p.save();
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0,0,0,18));
            p.drawRoundedRect(ix+4, iy+4, scaled.width(), scaled.height(), 8, 8);
            p.restore();

            p.drawPixmap(ix, iy, scaled);
        }

        int infoX = int(W * 0.535);
        int infoW = int(W * 0.435);
        int infoY = bodyY;

        p.save();
        p.setFont(font(11, true));
        p.setPen(QColor("#64748B"));
        p.drawText(QRect(infoX, infoY, infoW, int(H*0.045)),
                   Qt::AlignLeft | Qt::AlignVCenter, "FICHE TECHNIQUE");
        p.restore();
        hline(infoY + int(H*0.044), infoX, infoX + infoW, accent, 2.5);

        struct InfoRow { QString key; QString val; };
        QList<InfoRow> rows = {
                               { "Catégorie",       prod.getCategorie() },
                               { "Dimensions",      prod.getDimensions().isEmpty() ? "—" : prod.getDimensions() },
                               { "Matériaux",       prod.getMatUtilise().isEmpty() ? "—" : prod.getMatUtilise() },
                               { "Date de création",prod.getDateCreation().isValid()
                                                        ? prod.getDateCreation().toString("dd MMMM yyyy") : "—" },
                               { "Projet associé",  prod.getProjetId() },
                               { "Identifiant",     prod.getId() },
                               };

        int rowInfoH = int(H * 0.082);
        int rowInfoY = infoY + int(H * 0.055);

        for (const InfoRow& row : rows) {
            if (&row - &rows[0]) {
                bool alt = (&row - &rows[0]) % 2 != 0;
                if (alt) drawRoundRect(p, QRectF(infoX, rowInfoY, infoW, rowInfoH),
                                  6, QColor("#F1F5F9"));
            }
            p.save();
            p.setFont(font(8, true));
            p.setPen(QColor("#94A3B8"));
            p.drawText(QRect(infoX + 8, rowInfoY, infoW, int(rowInfoH*0.45)),
                       Qt::AlignLeft | Qt::AlignVCenter, row.key.toUpper());
            p.setFont(font(11, true));
            p.setPen(QColor("#0F172A"));
            p.drawText(QRect(infoX + 8, rowInfoY + int(rowInfoH*0.44),
                             infoW - 8, int(rowInfoH*0.52)),
                       Qt::AlignLeft | Qt::AlignVCenter, row.val);
            p.restore();
            hline(rowInfoY + rowInfoH, infoX, infoX + infoW, QColor("#E2E8F0"), 0.8);
            rowInfoY += rowInfoH;
        }

        int priceCardY = rowInfoY + int(H * 0.025);
        int priceCardH = int(H * 0.085);

        drawRoundRect(p, QRectF(infoX, priceCardY, infoW, priceCardH), 10, accent);
        p.save();
        p.setFont(font(8, true));
        p.setPen(QColor(255,255,255,180));
        p.drawText(QRect(infoX + 14, priceCardY + 8, infoW - 14, int(priceCardH*0.38)),
                   Qt::AlignLeft | Qt::AlignVCenter, "PRIX UNITAIRE");
        p.setFont(font(20, true));
        p.setPen(Qt::white);
        p.drawText(QRect(infoX, priceCardY + int(priceCardH*0.35), infoW - 14, int(priceCardH*0.58)),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(prod.getPrix(), 'f', 2) + " DT");
        p.restore();

        int footBandY = int(H * 0.92);
        p.fillRect(0, footBandY, W, int(H * 0.005), accent);

        drawFooter();
    }

    p.end();

    QMessageBox mb(this);
    mb.setWindowTitle("Export PDF réussi ✓");
    mb.setIcon(QMessageBox::Information);
    mb.setText(QString(
                   "<b style='font-size:13px;'>Catalogue PDF généré avec succès !</b><br/><br/>"
                   "<span style='color:#4a5568;'>%1</span><br/><br/>"
                   "<span style='color:#718096;font-size:11px;'>%2 produits · %3 pages au total</span>")
                   .arg(filePath).arg(list.size()).arg(list.size() + 2));
    QPushButton *openBtn = mb.addButton("  Ouvrir le PDF  ", QMessageBox::ActionRole);
    mb.addButton("Fermer", QMessageBox::RejectRole);
    mb.exec();
    if (mb.clickedButton() == openBtn)
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

void MainWindow::buildDetailPanel(QWidget *parent)
{
    m_detailPanel = new QFrame(parent);
    m_detailPanel->setObjectName("detailPanel");
    m_detailPanel->setFixedWidth(240);
    m_detailPanel->setStyleSheet(
        "QFrame#detailPanel {"
        "  background:white;"
        "  border-left:1px solid #e2e8f0;"
        "}"
        "QPushButton#advBtn {"
        "  background:white; border:1px solid #e2e8f0;"
        "  border-radius:7px; padding:9px 10px;"
        "  text-align:left; font-size:12px; color:#2d3748;"
        "}"
        "QPushButton#advBtn:hover { background:#f7fafc; border-color:#cbd5e0; }"
        "QPushButton#advBtn:disabled { color:#a0aec0; background:#fafafa; }");

    QVBoxLayout *lay = new QVBoxLayout(m_detailPanel);
    lay->setContentsMargins(14, 14, 14, 14);
    lay->setSpacing(10);

    m_detailImg = new QLabel(m_detailPanel);
    m_detailImg->setFixedSize(212, 150);
    m_detailImg->setAlignment(Qt::AlignCenter);
    m_detailImg->setStyleSheet(
        "background:#f0f4f8; border-radius:8px; color:#a0aec0; font-size:11px;");
    m_detailImg->setText("Sélectionnez\nun produit");
    lay->addWidget(m_detailImg);

    m_detailName = new QLabel("—", m_detailPanel);
    m_detailName->setStyleSheet("font-size:13px;font-weight:bold;color:#1a202c;");
    m_detailName->setWordWrap(true);
    lay->addWidget(m_detailName);

    m_detailRef = new QLabel("", m_detailPanel);
    m_detailRef->setStyleSheet("font-size:10px;color:#718096;");
    lay->addWidget(m_detailRef);

    m_detailPrice = new QLabel("", m_detailPanel);
    m_detailPrice->setStyleSheet("font-size:18px;font-weight:bold;color:#2b6cb0;margin-top:2px;");
    lay->addWidget(m_detailPrice);

    QFrame *infoGrid = new QFrame(m_detailPanel);
    infoGrid->setStyleSheet("background:#f7fafc;border-radius:6px;border:1px solid #e2e8f0;");
    QGridLayout *grid = new QGridLayout(infoGrid);
    grid->setContentsMargins(10, 8, 10, 8);
    grid->setSpacing(6);
    grid->setColumnStretch(1, 1);

    auto addRow = [&](int row, const QString& label, QLabel*& valLbl) {
        QLabel *lbl = new QLabel(label, infoGrid);
        lbl->setStyleSheet("font-size:10px;color:#718096;");
        valLbl = new QLabel("—", infoGrid);
        valLbl->setStyleSheet("font-size:11px;font-weight:500;color:#2d3748;");
        valLbl->setWordWrap(true);
        grid->addWidget(lbl,    row, 0);
        grid->addWidget(valLbl, row, 1);
    };
    addRow(0, "Catégorie", m_detailCat);
    addRow(1, "Matériau",  m_detailMat);
    addRow(2, "Dimensions",m_detailDim);
    addRow(3, "Création",  m_detailDate);
    lay->addWidget(infoGrid);

    QFrame *sepLine = new QFrame(m_detailPanel);
    sepLine->setFrameShape(QFrame::HLine);
    sepLine->setStyleSheet("color:#e2e8f0;");
    lay->addWidget(sepLine);

    QLabel *advLabel = new QLabel("FONCTIONNALITÉS", m_detailPanel);
    advLabel->setStyleSheet("font-size:10px;font-weight:bold;color:#a0aec0;letter-spacing:0.05em;");
    lay->addWidget(advLabel);

    m_btn3D = new QPushButton(m_detailPanel);
    m_btn3D->setObjectName("advBtn");
    m_btn3D->setMinimumHeight(44);
    m_btn3D->setDisabled(true);
    {
        QHBoxLayout *bl = new QHBoxLayout(m_btn3D);
        bl->setContentsMargins(6, 0, 6, 0);
        bl->setSpacing(8);
        QLabel *ic = new QLabel("3D", m_btn3D);
        ic->setStyleSheet("font-size:10px; font-weight:bold; color:#2b6cb0; background:#ebf8ff; border-radius:4px; padding:2px 4px;");
        ic->setFixedWidth(26);
        QVBoxLayout *tl = new QVBoxLayout();
        QLabel *t1 = new QLabel("Visualisation 3D", m_btn3D);
        t1->setStyleSheet("font-size:12px;font-weight:bold;color:#2d3748;");
        QLabel *t2 = new QLabel("Rotation · Dimensions", m_btn3D);
        t2->setStyleSheet("font-size:10px;color:#718096;");
        tl->addWidget(t1); tl->addWidget(t2);
        bl->addWidget(ic);
        bl->addLayout(tl);
        bl->addStretch();
        QLabel *arr = new QLabel("›", m_btn3D);
        arr->setStyleSheet("font-size:14px;color:#cbd5e0;");
        bl->addWidget(arr);
    }
    connect(m_btn3D, &QPushButton::clicked, this, &MainWindow::onShow3DProduct);
    lay->addWidget(m_btn3D);

    m_btnBarcode = new QPushButton(m_detailPanel);
    m_btnBarcode->setObjectName("advBtn");
    m_btnBarcode->setMinimumHeight(44);
    m_btnBarcode->setDisabled(true);
    {
        QHBoxLayout *bl = new QHBoxLayout(m_btnBarcode);
        bl->setContentsMargins(6, 0, 6, 0);
        bl->setSpacing(8);
        QLabel *ic = new QLabel("QR", m_btnBarcode);
        ic->setStyleSheet("font-size:10px; font-weight:bold; color:#38a169; background:#f0fff4; border-radius:4px; padding:2px 4px;");
        ic->setFixedWidth(26);
        QVBoxLayout *tl = new QVBoxLayout();
        QLabel *t1 = new QLabel("Code-barres / QR", m_btnBarcode);
        t1->setStyleSheet("font-size:12px;font-weight:bold;color:#2d3748;");
        QLabel *t2 = new QLabel("Générer · Imprimer", m_btnBarcode);
        t2->setStyleSheet("font-size:10px;color:#718096;");
        tl->addWidget(t1); tl->addWidget(t2);
        bl->addWidget(ic);
        bl->addLayout(tl);
        bl->addStretch();
        QLabel *arr = new QLabel("›", m_btnBarcode);
        arr->setStyleSheet("font-size:14px;color:#cbd5e0;");
        bl->addWidget(arr);
    }
    connect(m_btnBarcode, &QPushButton::clicked, this, &MainWindow::onShowBarcode);
    lay->addWidget(m_btnBarcode);

    lay->addStretch();
}

void MainWindow::refreshDetailPanel(const Produit& p)
{
    m_selectedProduit = p;

    bool imgLoaded = false;
    if (p.hasImage()) {
        QPixmap pm = loadProduitImage(p);
        if (!pm.isNull()) {
            imgLoaded = true;
            QPixmap dest(212, 150);
            dest.fill(Qt::white);
            QPixmap scaled = pm.scaled(212, 150, Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
            QPainter pa(&dest);
            pa.setRenderHint(QPainter::Antialiasing);
            QPainterPath clip;
            clip.addRoundedRect(0, 0, 212, 150, 8, 8);
            pa.setClipPath(clip);
            pa.drawPixmap((212 - scaled.width())  / 2,
                          (150 - scaled.height()) / 2, scaled);
            m_detailImg->setPixmap(dest);
            m_detailImg->setText("");
        }
    }

    if (!imgLoaded) {
        QPixmap ph(212, 150);
        ph.fill(Qt::transparent);
        QPainter pa(&ph);
        pa.setRenderHint(QPainter::Antialiasing);

        static const QMap<QString, QColor> catColors = {
                                                        {"Meuble",      QColor("#EBF4FF")},
                                                        {"Menuiserie",  QColor("#E6FFFA")},
                                                        {"Décoration",  QColor("#FFFFF0")},
                                                        {"Porte",       QColor("#FFF5F5")},
                                                        {"Fenêtre",     QColor("#F0FFF4")},
                                                        {"Escalier",    QColor("#FFFAF0")},
                                                        {"Autre",       QColor("#FAF5FF")},
                                                        };
        QColor bg = catColors.value(p.getCategorie(), QColor("#F7FAFC"));
        pa.setBrush(bg);
        pa.setPen(Qt::NoPen);
        pa.drawRoundedRect(0, 0, 212, 150, 8, 8);

        QString initials;
        QStringList words = p.getNom().split(' ', Qt::SkipEmptyParts);
        for (int i = 0; i < qMin(2, words.size()); ++i)
            initials += words[i].at(0).toUpper();
        if (initials.isEmpty()) initials = "?";

        pa.setPen(QColor("#a0aec0"));
        QFont f; f.setPixelSize(42); f.setBold(true);
        pa.setFont(f);
        pa.drawText(QRect(0, 0, 212, 120), Qt::AlignCenter, initials);

        pa.setPen(QColor("#718096"));
        QFont f2; f2.setPixelSize(11);
        pa.setFont(f2);
        pa.drawText(QRect(0, 126, 212, 18), Qt::AlignCenter, p.getCategorie());

        m_detailImg->setPixmap(ph);
        m_detailImg->setText("");
    }

    m_detailName->setText(p.getNom());
    m_detailRef->setText(QString("Réf. #%1  ·  Projet #%2")
                             .arg(p.getId()).arg(p.getProjetId()));
    m_detailPrice->setText(QString("%1 DT")
                               .arg(p.getPrix(), 0, 'f', 2));
    m_detailCat->setText(p.getCategorie());
    m_detailMat->setText(p.getMatUtilise());
    m_detailDim->setText(p.getDimensions().isEmpty() ? "—" : p.getDimensions());
    m_detailDate->setText(p.getDateCreation().isValid()
                              ? p.getDateCreation().toString("dd/MM/yyyy") : "—");

    m_btn3D->setEnabled(true);
    m_btnBarcode->setEnabled(true);
}

void MainWindow::clearDetailPanel()
{
    m_selectedProduit = Produit();

    m_detailImg->setPixmap(QPixmap());
    m_detailImg->setText("Sélectionnez\nun produit");
    m_detailName->setText("—");
    m_detailRef->setText("");
    m_detailPrice->setText("");
    m_detailCat->setText("—");
    m_detailMat->setText("—");
    m_detailDim->setText("—");
    m_detailDate->setText("—");

    m_btn3D->setEnabled(false);
    m_btnBarcode->setEnabled(false);
}

void MainWindow::onProductRowSelected(int row)
{
    if (row < 0) { clearDetailPanel(); return; }

    QWidget *productsPage = stackedWidget->widget(4);
    if (!productsPage) return;
    QTableWidget *table = productsPage->findChild<QTableWidget*>("productsTable");
    if (!table || row >= table->rowCount()) return;

    QTableWidgetItem *idItem = table->item(row, 0);
    if (!idItem) return;

    QString id = idItem->data(Qt::UserRole).toString();
    if (id.isEmpty()) return;

    for (const Produit& p : m_allProduits) {
        if (p.getId() == id) {
            refreshDetailPanel(p);
            return;
        }
    }

    // Fallback : charger depuis BD
    refreshDetailPanel(produitDB.getProduit(id));
}

void MainWindow::onShow3DProduct()
{
    if (m_selectedProduit.getId().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection",
                             "Veuillez sélectionner un produit dans la liste.");
        return;
    }
    Produit3DDialog dlg(m_selectedProduit, this);
    dlg.exec();
}

void MainWindow::onShowBarcode()
{
    if (m_selectedProduit.getId().isEmpty()) {
        QMessageBox::warning(this, "Aucune sélection",
                             "Veuillez sélectionner un produit dans la liste.");
        return;
    }
    ProduitBarcodeDialog dlg(m_selectedProduit, this);
    dlg.exec();
}
