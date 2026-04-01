#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/modules/auth/loginpage.h"
#include "src/modules/auth/forgotpasswordpage.h"
#include "src/modules/employees/employeemanagementpage.h"
#include "src/core/session.h"
#include "src/modules/stock/stockpage.h"
#include "src/modules/projects/projectmanagementpage.h"
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

    authPages->addWidget(loginPage);   // index 0
    authPages->addWidget(forgotPage);  // index 1

    connect(loginPage, &LoginPage::loginSuccess,
            this, &MainWindow::onLoginSuccess);

    connect(loginPage, &LoginPage::switchToForgotPassword, [authPages]() {
        authPages->setCurrentIndex(1);
    });

    connect(forgotPage, &ForgotPasswordPage::switchToLogin, [authPages]() {
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
    if (profileName)
        profileName->setText(currentEmployee.getFullName());

    for (QLabel* lbl : profileBtn->findChildren<QLabel*>()) {
        if (lbl->width() == 38 && lbl->height() == 38) {
            int size = 38;
            QPixmap avatar(size, size);
            avatar.fill(Qt::transparent);

            if (currentEmployee.hasPhoto()) {
                // Use employee photo — clip to circle
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
                // Fallback: initial letter on green gradient
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

    // Sidebar bottom shows logo only — no name/role labels

    authStack->setCurrentIndex(1);

    // Apply permissions — hide sidebar buttons the employee cannot access
    // Sidebar order: 0=Projets, 1=Employes, 2=Stocks, 3=Finance, 4=Designs/Produits
    struct { int idx; bool allowed; } perms[] = {
                 { 0, currentEmployee.canAccessProjet()       },
                 { 1, currentEmployee.canAccessEmploye()      },
                 { 2, currentEmployee.canAccessMateriau()     },
                 { 3, currentEmployee.canAccessTransactions() },
                 { 4, currentEmployee.canAccessProduit()      },
                 };
    int firstAllowed = 0;
    for (auto& p : perms) {
        if (p.idx < sidebarButtons.size()) {
            sidebarButtons[p.idx]->setVisible(p.allowed);
            if (p.allowed && firstAllowed == 0) firstAllowed = p.idx;
        }
    }

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

    // Logo at the top
    QLabel *sidebarLogo = new QLabel(sidebar);
    QPixmap logoPixmap("src/assets/icons/logo1.png");
    if (!logoPixmap.isNull())
        sidebarLogo->setPixmap(logoPixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    sidebarLogo->setAlignment(Qt::AlignCenter);
    sidebarLogo->setContentsMargins(0, 16, 0, 8);
    sidebarLayout->addWidget(sidebarLogo, 0, Qt::AlignCenter);

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
        if (checked) {
            darkModeLabel->setText("Passer en mode clair");
        } else {
            darkModeLabel->setText("Passer en mode sombre");
        }
        toggleDarkMode();
    });

    darkModeLayout->addWidget(darkModeLabel);
    darkModeLayout->addWidget(darkModeToggle);

    // Profile button — circular avatar only, click opens dropdown with logout
    profileBtn = new QPushButton(navbar);
    profileBtn->setObjectName("profileButton");
    profileBtn->setCursor(Qt::PointingHandCursor);
    profileBtn->setFixedSize(42, 42);
    profileBtn->setStyleSheet(
        "QPushButton#profileButton { border-radius: 21px; border: 2px solid #8A9A5B; "
        "background: transparent; padding: 0; }"
        "QPushButton#profileButton:hover { border-color: #9aaa6b; }"
        );

    QHBoxLayout *profileLayout = new QHBoxLayout(profileBtn);
    profileLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *profilePhoto = createRoundedAvatar("src/assets/icons/pfp.jpeg", 38);
    profileName = new QLabel("", profileBtn); // kept for showMainApp compatibility
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

    // Header — show employee name
    QAction *nameAction = menu->addAction(currentEmployee.getFullName());
    nameAction->setEnabled(false);
    QFont f = nameAction->font(); f.setBold(true); nameAction->setFont(f);
    menu->addSeparator();
    QAction *logoutAction = menu->addAction("Se deconnecter");

    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogout);

    // Position menu below the button, aligned to its right edge — stays inside window
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
    stackedWidget->addWidget(new ProjectManagementPage(this));
    stackedWidget->addWidget(createEmployeesPage());
    stackedWidget->addWidget(new StockPage(this));
    stackedWidget->addWidget(createFinancePage());
    stackedWidget->addWidget(createProductsPage());
}



QWidget* MainWindow::createEmployeesPage()
{
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

    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);

    struct StatInfo { QString title; QString property; };
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

    financeTable = new QTableWidget(page);
    financeTable->setObjectName("financeTable");
    financeTable->setColumnCount(6);
    financeTable->setHorizontalHeaderLabels({"CLIENT", "TYPE", "CATÉGORIE", "MONTANT (DT)", "STATUT", "DATE"});
    financeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    financeTable->verticalHeader()->setVisible(false);
    financeTable->setAlternatingRowColors(true);
    financeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    financeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    financeTable->setShowGrid(false);
    financeTable->setSortingEnabled(false);

    const QList<QStringList> exemples = {
        {"M. Dupont",         "Facture",  "Recette",  "3500", "Payé",       "01/02/2026"},
        {"Mme Martin",        "Devis",    "Recette",  "2800", "En attente", "10/02/2026"},
        {"Restaurant Le Bois","Acompte",  "Recette",  "1500", "Payé",       "15/01/2026"},
        {"M. Bernard",        "Facture",  "Dépense",  "1200", "Retard",     "05/02/2026"},
        {"SARL Dubois",       "Facture",  "Recette",  "5200", "Payé",       "20/01/2026"}
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

    auto updateStats = [this, valueLabels]() {
        double revenus = 0.0, depenses = 0.0;
        for (int r = 0; r < financeTable->rowCount(); ++r) {
            if (financeTable->isRowHidden(r)) continue;
            auto itemMontant = financeTable->item(r, 3);
            if (!itemMontant) continue;
            QString m = itemMontant->text().replace(" DT", "").replace(" ", "").trimmed();
            bool ok; double montant = m.toDouble(&ok);
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

    QHBoxLayout *actionBar = new QHBoxLayout();
    actionBar->setSpacing(12);
    QPushButton *btnAjouter   = new QPushButton("+ Nouvelle Transaction", page);
    QPushButton *btnSupprimer = new QPushButton("Supprimer", page);
    QPushButton *btnExporter  = new QPushButton("Exporter CSV", page);
    QPushButton *btnStats     = new QPushButton("Statistiques détaillées", page);
    for (auto b : {btnAjouter, btnSupprimer, btnExporter, btnStats}) {
        b->setObjectName("actionButton"); b->setCursor(Qt::PointingHandCursor);
    }
    QComboBox *triCombo = new QComboBox(page);
    triCombo->setObjectName("sortCombo");
    triCombo->addItems({"Tri par défaut","Montant ↑","Montant ↓","Date ↓ (récent)","Date ↑ (ancien)"});
    triCombo->setMinimumWidth(210);
    actionBar->addWidget(btnAjouter);
    actionBar->addWidget(btnSupprimer);
    actionBar->addWidget(btnExporter);
    actionBar->addWidget(btnStats);
    actionBar->addSpacing(16);
    actionBar->addWidget(new QLabel("Trier par :"));
    actionBar->addWidget(triCombo);
    actionBar->addStretch();

    auto doFilter = [=]() {
        QString recherche = searchEdit->text().trimmed().toLower();
        QString typeSel = cbType->currentText(), catSel = cbCategory->currentText(), statutSel = cbStatus->currentText();
        QDate d1 = debutDate->date(), d2 = finDate->date();
        for (int r = 0; r < financeTable->rowCount(); ++r) {
            bool visible = true;
            if (!recherche.isEmpty()) {
                bool trouve = false;
                for (int c = 0; c < 3; ++c) { auto it = financeTable->item(r,c); if (it && it->text().toLower().contains(recherche)) { trouve=true; break; } }
                if (!trouve) visible = false;
            }
            if (visible && typeSel   != "Tous"   && financeTable->item(r,1)->text() != typeSel)   visible = false;
            if (visible && catSel    != "Toutes" && financeTable->item(r,2)->text() != catSel)    visible = false;
            if (visible && statutSel != "Tous"   && financeTable->item(r,4)->text() != statutSel) visible = false;
            if (visible) { auto dateIt = financeTable->item(r,5); if (dateIt) { QDate dt = QDate::fromString(dateIt->text(),"dd/MM/yyyy"); if (dt.isValid() && (dt<d1||dt>d2)) visible=false; } }
            financeTable->setRowHidden(r, !visible);
        }
        updateStats();
    };

    connect(btnFiltrer, &QPushButton::clicked, doFilter);
    connect(btnReset, &QPushButton::clicked, [=](){
        searchEdit->clear(); cbType->setCurrentIndex(0); cbCategory->setCurrentIndex(0); cbStatus->setCurrentIndex(0);
        debutDate->setDate(QDate::currentDate().addMonths(-1)); finDate->setDate(QDate::currentDate());
        for (int r = 0; r < financeTable->rowCount(); ++r) financeTable->setRowHidden(r, false);
        updateStats();
    });
    connect(triCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int idx){
        if (idx==0) return;
        struct Ligne { int row; double montant=0.0; QDate date; };
        QList<Ligne> lignes;
        for (int r=0; r<financeTable->rowCount(); ++r) {
            if (!financeTable->isRowHidden(r)) {
                auto mIt=financeTable->item(r,3); auto dIt=financeTable->item(r,5);
                if (mIt&&dIt) { QString mStr=mIt->text().replace(" DT","").trimmed(); lignes<<Ligne{r,mStr.toDouble(),QDate::fromString(dIt->text(),"dd/MM/yyyy")}; }
            }
        }
        std::stable_sort(lignes.begin(),lignes.end(),[idx](const Ligne&a,const Ligne&b){
            if(idx==1) return a.montant<b.montant; if(idx==2) return a.montant>b.montant;
            if(idx==3) return a.date>b.date; if(idx==4) return a.date<b.date; return false;
        });
        int nl=0;
        for (const auto&l:lignes){ for(int c=0;c<6;++c){ auto item=financeTable->takeItem(l.row,c); financeTable->setItem(nl,c,item); } financeTable->setRowHeight(nl,52); nl++; }
    });
    connect(btnAjouter, &QPushButton::clicked, [=](){
        QDialog dlg(this); dlg.setWindowTitle("Nouvelle Transaction"); dlg.setMinimumWidth(420);
        QFormLayout form(&dlg);
        QLineEdit *clientEdit=new QLineEdit(&dlg); QComboBox *typeCb=new QComboBox(&dlg); QComboBox *catCb=new QComboBox(&dlg);
        QLineEdit *montantEdit=new QLineEdit(&dlg); QComboBox *statutCb=new QComboBox(&dlg); QDateEdit *dateEdit=new QDateEdit(QDate::currentDate(),&dlg);
        typeCb->addItems({"Facture","Devis","Acompte"}); catCb->addItems({"Recette","Dépense"}); statutCb->addItems({"Payé","En attente","Retard"});
        dateEdit->setCalendarPopup(true); dateEdit->setDisplayFormat("dd/MM/yyyy");
        form.addRow("Client :",clientEdit); form.addRow("Type :",typeCb); form.addRow("Catégorie :",catCb);
        form.addRow("Montant (DT) :",montantEdit); form.addRow("Statut :",statutCb); form.addRow("Date :",dateEdit);
        QDialogButtonBox *box=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dlg); form.addRow(box);
        connect(box,&QDialogButtonBox::accepted,&dlg,&QDialog::accept); connect(box,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if (dlg.exec()==QDialog::Accepted) {
            int row=financeTable->rowCount(); financeTable->insertRow(row);
            financeTable->setItem(row,0,new QTableWidgetItem(clientEdit->text()));
            financeTable->setItem(row,1,new QTableWidgetItem(typeCb->currentText()));
            financeTable->setItem(row,2,new QTableWidgetItem(catCb->currentText()));
            financeTable->setItem(row,3,new QTableWidgetItem(montantEdit->text()+" DT"));
            financeTable->setItem(row,4,new QTableWidgetItem(statutCb->currentText()));
            financeTable->setItem(row,5,new QTableWidgetItem(dateEdit->date().toString("dd/MM/yyyy")));
            financeTable->setRowHeight(row,52); updateStats();
        }
    });
    connect(financeTable,&QTableWidget::cellDoubleClicked,[=](int row,int){
        QDialog dlg(this); dlg.setWindowTitle("Modifier Transaction"); dlg.setMinimumWidth(420);
        QFormLayout form(&dlg);
        QString montantTxt=financeTable->item(row,3)->text().replace(" DT","").trimmed();
        QLineEdit *clientEdit=new QLineEdit(financeTable->item(row,0)->text(),&dlg);
        QComboBox *typeCb=new QComboBox(&dlg); QComboBox *catCb=new QComboBox(&dlg);
        QLineEdit *montantEdit=new QLineEdit(montantTxt,&dlg); QComboBox *statutCb=new QComboBox(&dlg); QDateEdit *dateEdit=new QDateEdit(&dlg);
        typeCb->addItems({"Facture","Devis","Acompte"}); typeCb->setCurrentText(financeTable->item(row,1)->text());
        catCb->addItems({"Recette","Dépense"}); catCb->setCurrentText(financeTable->item(row,2)->text());
        statutCb->addItems({"Payé","En attente","Retard"}); statutCb->setCurrentText(financeTable->item(row,4)->text());
        dateEdit->setDate(QDate::fromString(financeTable->item(row,5)->text(),"dd/MM/yyyy")); dateEdit->setCalendarPopup(true); dateEdit->setDisplayFormat("dd/MM/yyyy");
        form.addRow("Client :",clientEdit); form.addRow("Type :",typeCb); form.addRow("Catégorie :",catCb);
        form.addRow("Montant (DT) :",montantEdit); form.addRow("Statut :",statutCb); form.addRow("Date :",dateEdit);
        QDialogButtonBox *box=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dlg); form.addRow(box);
        connect(box,&QDialogButtonBox::accepted,&dlg,&QDialog::accept); connect(box,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
        if (dlg.exec()==QDialog::Accepted) {
            financeTable->item(row,0)->setText(clientEdit->text()); financeTable->item(row,1)->setText(typeCb->currentText());
            financeTable->item(row,2)->setText(catCb->currentText()); financeTable->item(row,3)->setText(montantEdit->text()+" DT");
            financeTable->item(row,4)->setText(statutCb->currentText()); financeTable->item(row,5)->setText(dateEdit->date().toString("dd/MM/yyyy"));
            updateStats();
        }
    });
    connect(btnSupprimer,&QPushButton::clicked,[=](){
        int row=financeTable->currentRow();
        if (row<0) { QMessageBox::warning(this,"Aucune sélection","Sélectionnez une ligne."); return; }
        if (QMessageBox::question(this,"Confirmer","Supprimer cette transaction ?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
        { financeTable->removeRow(row); updateStats(); }
    });
    connect(btnExporter,&QPushButton::clicked,[=](){
        QString fichier=QFileDialog::getSaveFileName(this,"Exporter en CSV","transactions_"+QDate::currentDate().toString("yyyyMMdd")+".csv","Fichiers CSV (*.csv);;Tous (*.*)");
        if (fichier.isEmpty()) return;
        QFile file(fichier);
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) { QMessageBox::warning(this,"Erreur","Impossible d'écrire."); return; }
        QTextStream out(&file);
        QStringList entetes; for(int c=0;c<financeTable->columnCount();++c) entetes<<financeTable->horizontalHeaderItem(c)->text();
        out<<entetes.join(";")<<"\n";
        for(int r=0;r<financeTable->rowCount();++r){ if(financeTable->isRowHidden(r)) continue; QStringList ligne; for(int c=0;c<financeTable->columnCount();++c){auto it=financeTable->item(r,c);ligne<<(it?it->text():"");} out<<ligne.join(";")<<"\n"; }
        file.close(); QMessageBox::information(this,"Succès","Export terminé.");
    });
    connect(btnStats,&QPushButton::clicked,[=](){
        QDialog *dlg=new QDialog(this); dlg->setWindowTitle("Statistiques détaillées"); dlg->setMinimumSize(520,480);
        QVBoxLayout *lay=new QVBoxLayout(dlg);
        double totRevenus=0,totDepenses=0,totPaye=0,totAttente=0,totRetard=0; int cntPaye=0,cntAttente=0,cntRetard=0;
        QMap<QString,double> revenusMensuels,depensesMensuels;
        for(int r=0;r<financeTable->rowCount();++r){
            if(financeTable->isRowHidden(r)) continue;
            auto montantIt=financeTable->item(r,3); auto catIt=financeTable->item(r,2); auto statutIt=financeTable->item(r,4); auto dateIt=financeTable->item(r,5);
            if(!montantIt||!catIt||!statutIt||!dateIt) continue;
            double montant=montantIt->text().replace(" DT","").trimmed().toDouble();
            QString cat=catIt->text(),statut=statutIt->text(),mois=dateIt->text().right(7);
            if(cat=="Recette"){totRevenus+=montant;revenusMensuels[mois]+=montant;}else if(cat=="Dépense"){totDepenses+=montant;depensesMensuels[mois]+=montant;}
            if(statut=="Payé"){totPaye+=montant;cntPaye++;}else if(statut=="En attente"){totAttente+=montant;cntAttente++;}else if(statut=="Retard"){totRetard+=montant;cntRetard++;}
        }
        QFrame *resume=new QFrame(dlg); resume->setObjectName("statsSummary"); QGridLayout *gr=new QGridLayout(resume);
        gr->addWidget(new QLabel("<b>Résumé financier</b>"),0,0,1,2);
        gr->addWidget(new QLabel("Total revenus :"),1,0); gr->addWidget(new QLabel(QString("%L1 DT").arg(totRevenus,0,'f',2)),1,1);
        gr->addWidget(new QLabel("Total dépenses :"),2,0); gr->addWidget(new QLabel(QString("%L1 DT").arg(totDepenses,0,'f',2)),2,1);
        gr->addWidget(new QLabel("Bénéfice net :"),3,0);
        QLabel *profitLbl=new QLabel(QString("%L1 DT").arg(totRevenus-totDepenses,0,'f',2)); profitLbl->setStyleSheet("color:#27ae60;font-weight:bold;"); gr->addWidget(profitLbl,3,1);
        lay->addWidget(resume);
        QPushButton *close=new QPushButton("Fermer",dlg); close->setObjectName("actionButton"); connect(close,&QPushButton::clicked,dlg,&QDialog::accept); lay->addWidget(close,0,Qt::AlignRight);
        dlg->exec();
    });

    mainLayout->addLayout(actionBar);
    return page;
}

QWidget* MainWindow::createProductsPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(18);

    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);
    struct StatData { QString title; QString value; QString type; };
    QList<StatData> stats = {{"PRODUITS ACTIFS","45","active"},{"EN RÉAPPROVISIONNEMENT","8","pending"},{"NOUVEAUTÉS CE MOIS","12","new"}};
    for (const auto& stat : stats) {
        QFrame *card = new QFrame(page); card->setObjectName("statCard"); card->setProperty("type", stat.type);
        QVBoxLayout *cardLayout = new QVBoxLayout(card); cardLayout->setSpacing(10); cardLayout->setContentsMargins(20,20,20,20);
        QLabel *title = new QLabel(stat.title, card); title->setObjectName("statTitle");
        QLabel *value = new QLabel(stat.value, card); value->setObjectName("statValue");
        cardLayout->addWidget(title); cardLayout->addWidget(value); cardLayout->addStretch(); statsLayout->addWidget(card);
    }
    layout->addLayout(statsLayout);

    QHBoxLayout *actionsLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("+ Nouveau Produit", page);
    QPushButton *editBtn = new QPushButton("Modifier", page);
    QPushButton *deleteBtn = new QPushButton("Supprimer", page);
    QPushButton *exportBtn = new QPushButton("Exporter PDF", page);
    for (auto b : {addBtn,editBtn,deleteBtn,exportBtn}) { b->setObjectName("actionButton"); b->setCursor(Qt::PointingHandCursor); }
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddProductClicked);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::onEditProductClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteProductClicked);
    actionsLayout->addWidget(addBtn); actionsLayout->addWidget(editBtn); actionsLayout->addWidget(deleteBtn); actionsLayout->addWidget(exportBtn);

    QLineEdit *searchBar = new QLineEdit(page);
    searchBar->setObjectName("productSearch");
    searchBar->setPlaceholderText("Rechercher un produit par ID, nom ou catégorie...");
    searchBar->setMinimumHeight(38); searchBar->setMaximumWidth(400);
    connect(searchBar, &QLineEdit::textChanged, this, [this](const QString &text) {
        QWidget *productsPage = stackedWidget->widget(4);
        QTableWidget *table = productsPage->findChild<QTableWidget*>("productsTable");
        if (table) { for(int row=0;row<table->rowCount();++row){ bool match=false; for(int col=0;col<3;++col){ if(table->item(row,col)&&table->item(row,col)->text().contains(text,Qt::CaseInsensitive)){match=true;break;} } table->setRowHidden(row,!match); } }
    });
    actionsLayout->addWidget(searchBar); actionsLayout->addStretch();

    QTableWidget *table = new QTableWidget(page);
    table->setObjectName("productsTable");
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"ID PRODUIT","NOM","CATÉGORIE","PRIX","DIMENSIONS","MATÉRIAUX","DATE CRÉATION","IMAGE"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true); table->setShowGrid(false);

    table->setRowCount(4);
    table->setItem(0,0,new QTableWidgetItem("PROD-001")); table->setItem(0,1,new QTableWidgetItem("Table à manger en chêne")); table->setItem(0,2,new QTableWidgetItem("Meuble")); table->setItem(0,3,new QTableWidgetItem("850 EUR")); table->setItem(0,4,new QTableWidgetItem("200 x 90 x 75 cm")); table->setItem(0,5,new QTableWidgetItem("Chêne massif, Vernis")); table->setItem(0,6,new QTableWidgetItem("15/01/2026")); table->setItem(0,7,new QTableWidgetItem("assets/images/table_chene.jpg"));
    table->setItem(1,0,new QTableWidgetItem("PROD-002")); table->setItem(1,1,new QTableWidgetItem("Bibliothèque moderne")); table->setItem(1,2,new QTableWidgetItem("Meuble")); table->setItem(1,3,new QTableWidgetItem("650 EUR")); table->setItem(1,4,new QTableWidgetItem("120 x 30 x 180 cm")); table->setItem(1,5,new QTableWidgetItem("MDF, Verre trempé")); table->setItem(1,6,new QTableWidgetItem("20/01/2026")); table->setItem(1,7,new QTableWidgetItem("assets/images/bibliotheque.jpg"));
    table->setItem(2,0,new QTableWidgetItem("PROD-003")); table->setItem(2,1,new QTableWidgetItem("Porte d'entrée bois")); table->setItem(2,2,new QTableWidgetItem("Porte")); table->setItem(2,3,new QTableWidgetItem("1200 EUR")); table->setItem(2,4,new QTableWidgetItem("90 x 210 cm")); table->setItem(2,5,new QTableWidgetItem("Chêne, Double vitrage")); table->setItem(2,6,new QTableWidgetItem("05/02/2026")); table->setItem(2,7,new QTableWidgetItem("assets/images/porte_chene.jpg"));
    table->setItem(3,0,new QTableWidgetItem("PROD-004")); table->setItem(3,1,new QTableWidgetItem("Étagère murale")); table->setItem(3,2,new QTableWidgetItem("Décoration")); table->setItem(3,3,new QTableWidgetItem("120 EUR")); table->setItem(3,4,new QTableWidgetItem("80 x 20 x 20 cm")); table->setItem(3,5,new QTableWidgetItem("Bois de pin")); table->setItem(3,6,new QTableWidgetItem("10/02/2026")); table->setItem(3,7,new QTableWidgetItem("assets/images/etagere.jpg"));
    for (int row=0;row<4;++row) table->setRowHeight(row,50);

    layout->addLayout(actionsLayout);
    layout->addWidget(table);
    return page;
}

void MainWindow::addEmployee() {}
void MainWindow::editEmployee() {}
void MainWindow::deleteEmployee() {}

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
    if (index == 1) {
        QWidget *page = stackedWidget->widget(index);
        currentTable = page->findChild<QTableWidget*>("dataTable");
    }
    QStringList titles = {"Gestion des Projets","Gestion des Employes","Gestion des Stocks","Gestion Financiere","Gestion des Designs"};
    pageTitle->setText(titles[index]);
}


void MainWindow::toggleDarkMode() { isDarkMode = darkModeToggle->isChecked(); loadStyleSheet(); }

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key()==Qt::Key_Plus||event->key()==Qt::Key_Equal) { onScaleUp(); event->accept(); return; }
        else if (event->key()==Qt::Key_Minus) { onScaleDown(); event->accept(); return; }
        else if (event->key()==Qt::Key_0) { onScaleReset(); event->accept(); return; }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::onScaleUp() { if (currentScale<1.5) { currentScale+=0.1; applyScale(currentScale); } }
void MainWindow::onScaleDown() { if (currentScale>0.7) { currentScale-=0.1; applyScale(currentScale); } }
void MainWindow::onScaleReset() { currentScale=1.0; applyScale(currentScale); }

void MainWindow::applyScale(qreal scale)
{
    QFont font=qApp->font(); font.setPointSizeF(font.pointSizeF()*scale/(currentScale==scale?currentScale:(currentScale-(scale-currentScale)))); qApp->setFont(font);
    resize(static_cast<int>(1000*scale), static_cast<int>(500*scale));
}

void MainWindow::loadStyleSheet()
{
    QString filename = isDarkMode ? "style-dark.qss" : "style.qss";
    QFile styleFile(filename);
    if (styleFile.open(QFile::ReadOnly|QFile::Text)) {
        QTextStream stream(&styleFile); qApp->setStyleSheet(stream.readAll()); styleFile.close();
        qDebug() << "Stylesheet loaded:" << filename;
    } else { qDebug() << "Could not load stylesheet:" << filename; }
}

void MainWindow::onAddProductClicked()
{
    QDialog dialog(this); dialog.setWindowTitle("Ajouter un Nouveau Produit"); dialog.setMinimumWidth(550); dialog.setStyleSheet("QDialog{background-color:white;}");
    QVBoxLayout *mainLayout=new QVBoxLayout(&dialog); mainLayout->setSpacing(20); mainLayout->setContentsMargins(30,30,30,30);
    QLabel *titleLabel=new QLabel("Nouveau Produit",&dialog); titleLabel->setStyleSheet("font-size:18px;font-weight:bold;color:#2c3e50;"); titleLabel->setAlignment(Qt::AlignCenter); mainLayout->addWidget(titleLabel);
    QFormLayout *form=new QFormLayout(); form->setSpacing(15); form->setLabelAlignment(Qt::AlignRight); form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    QLineEdit *idProd=new QLineEdit(&dialog); idProd->setPlaceholderText("PROD-001"); idProd->setMinimumHeight(35);
    QLineEdit *nomProd=new QLineEdit(&dialog); nomProd->setPlaceholderText("Nom du produit"); nomProd->setMinimumHeight(35);
    QComboBox *categorieProd=new QComboBox(&dialog); categorieProd->addItems({"Meuble","Menuiserie","Décoration","Porte","Fenêtre","Escalier","Autre"}); categorieProd->setMinimumHeight(35);
    QLineEdit *prixProd=new QLineEdit(&dialog); prixProd->setPlaceholderText("0.00 EUR"); prixProd->setMinimumHeight(35);
    QLineEdit *dimensions=new QLineEdit(&dialog); dimensions->setPlaceholderText("L x l x H"); dimensions->setMinimumHeight(35);
    QLineEdit *matUtilise=new QLineEdit(&dialog); matUtilise->setPlaceholderText("Bois, MDF, Verre..."); matUtilise->setMinimumHeight(35);
    QDateEdit *dateCreation=new QDateEdit(&dialog); dateCreation->setDate(QDate::currentDate()); dateCreation->setCalendarPopup(true); dateCreation->setDisplayFormat("dd/MM/yyyy"); dateCreation->setMinimumHeight(35);
    QLineEdit *image=new QLineEdit(&dialog); image->setPlaceholderText("chemin/vers/image.jpg"); image->setMinimumHeight(35);
    QPushButton *browseBtn=new QPushButton("Parcourir...",&dialog); browseBtn->setMinimumHeight(35); browseBtn->setCursor(Qt::PointingHandCursor);
    browseBtn->setStyleSheet("QPushButton{background-color:#e2e8f0;color:#4a5568;border:none;border-radius:5px;padding:8px 15px;}QPushButton:hover{background-color:#cbd5e0;}");
    QHBoxLayout *imageLayout=new QHBoxLayout(); imageLayout->addWidget(image); imageLayout->addWidget(browseBtn);
    connect(browseBtn,&QPushButton::clicked,[&dialog,image](){ QString f=QFileDialog::getOpenFileName(&dialog,"Sélectionner une image",QDir::homePath(),"Images (*.png *.jpg *.jpeg *.bmp *.gif)"); if(!f.isEmpty()) image->setText(f); });
    form->addRow("ID Produit:",idProd); form->addRow("Nom du produit:",nomProd); form->addRow("Catégorie:",categorieProd);
    form->addRow("Prix:",prixProd); form->addRow("Dimensions:",dimensions); form->addRow("Matériaux:",matUtilise);
    form->addRow("Date création:",dateCreation); form->addRow("Image:",imageLayout);
    mainLayout->addLayout(form);
    QDialogButtonBox *buttonBox=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Ajouter"); buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet("QPushButton{background-color:#8A9A5B;color:white;border:none;border-radius:5px;padding:8px 20px;font-weight:bold;}QPushButton:hover{background-color:#9aaa6b;}");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Annuler"); buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet("QPushButton{background-color:#e2e8f0;color:#4a5568;border:none;border-radius:5px;padding:8px 20px;}QPushButton:hover{background-color:#cbd5e0;}");
    mainLayout->addWidget(buttonBox);
    connect(buttonBox,&QDialogButtonBox::accepted,&dialog,&QDialog::accept); connect(buttonBox,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    if (dialog.exec()==QDialog::Accepted) {
        QWidget *productsPage=stackedWidget->widget(4); QTableWidget *table=productsPage->findChild<QTableWidget*>("productsTable");
        if (table) {
            int row=table->rowCount(); table->insertRow(row);
            QString prixText=prixProd->text(); if(!prixText.contains("EUR")) prixText+=" EUR";
            table->setItem(row,0,new QTableWidgetItem(idProd->text())); table->setItem(row,1,new QTableWidgetItem(nomProd->text()));
            table->setItem(row,2,new QTableWidgetItem(categorieProd->currentText())); table->setItem(row,3,new QTableWidgetItem(prixText));
            table->setItem(row,4,new QTableWidgetItem(dimensions->text())); table->setItem(row,5,new QTableWidgetItem(matUtilise->text()));
            table->setItem(row,6,new QTableWidgetItem(dateCreation->date().toString("dd/MM/yyyy"))); table->setItem(row,7,new QTableWidgetItem(image->text()));
            table->setRowHeight(row,50); QMessageBox::information(this,"Succès","Produit ajouté avec succès!");
        }
    }
}

void MainWindow::onEditProductClicked()
{
    QWidget *productsPage=stackedWidget->widget(4); QTableWidget *table=productsPage->findChild<QTableWidget*>("productsTable");
    if (!table||table->selectedItems().isEmpty()) { QMessageBox::warning(this,"Aucune sélection","Veuillez sélectionner un produit à modifier."); return; }
    int row=table->currentRow();
    QString idProd=table->item(row,0)?table->item(row,0)->text():"";
    QString nomProd=table->item(row,1)?table->item(row,1)->text():"";
    QString categorieProd=table->item(row,2)?table->item(row,2)->text():"";
    QString prixProd=table->item(row,3)?table->item(row,3)->text().replace(" EUR",""):"";
    QString dimensions=table->item(row,4)?table->item(row,4)->text():"";
    QString matUtilise=table->item(row,5)?table->item(row,5)->text():"";
    QString dateCreation=table->item(row,6)?table->item(row,6)->text():QDate::currentDate().toString("dd/MM/yyyy");
    QString image=table->item(row,7)?table->item(row,7)->text():"";
    QDialog dialog(this); dialog.setWindowTitle("Modifier le Produit"); dialog.setMinimumWidth(550); dialog.setStyleSheet("QDialog{background-color:white;}");
    QVBoxLayout *mainLayout=new QVBoxLayout(&dialog); mainLayout->setSpacing(20); mainLayout->setContentsMargins(30,30,30,30);
    QLabel *titleLabel=new QLabel("Modifier le Produit",&dialog); titleLabel->setStyleSheet("font-size:18px;font-weight:bold;color:#2c3e50;"); titleLabel->setAlignment(Qt::AlignCenter); mainLayout->addWidget(titleLabel);
    QFormLayout *form=new QFormLayout(); form->setSpacing(15); form->setLabelAlignment(Qt::AlignRight); form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    QLineEdit *idProdEdit=new QLineEdit(idProd,&dialog); idProdEdit->setMinimumHeight(35);
    QLineEdit *nomProdEdit=new QLineEdit(nomProd,&dialog); nomProdEdit->setMinimumHeight(35);
    QComboBox *categorieProdEdit=new QComboBox(&dialog); categorieProdEdit->addItems({"Meuble","Menuiserie","Décoration","Porte","Fenêtre","Escalier","Autre"}); categorieProdEdit->setCurrentText(categorieProd); categorieProdEdit->setMinimumHeight(35);
    QLineEdit *prixProdEdit=new QLineEdit(prixProd,&dialog); prixProdEdit->setMinimumHeight(35);
    QLineEdit *dimensionsEdit=new QLineEdit(dimensions,&dialog); dimensionsEdit->setMinimumHeight(35);
    QLineEdit *matUtiliseEdit=new QLineEdit(matUtilise,&dialog); matUtiliseEdit->setMinimumHeight(35);
    QDateEdit *dateCreationEdit=new QDateEdit(&dialog); dateCreationEdit->setDate(QDate::fromString(dateCreation,"dd/MM/yyyy")); dateCreationEdit->setCalendarPopup(true); dateCreationEdit->setDisplayFormat("dd/MM/yyyy"); dateCreationEdit->setMinimumHeight(35);
    QLineEdit *imageEdit=new QLineEdit(image,&dialog); imageEdit->setMinimumHeight(35);
    QPushButton *browseBtn=new QPushButton("Parcourir...",&dialog); browseBtn->setMinimumHeight(35); browseBtn->setCursor(Qt::PointingHandCursor);
    browseBtn->setStyleSheet("QPushButton{background-color:#e2e8f0;color:#4a5568;border:none;border-radius:5px;padding:8px 15px;}QPushButton:hover{background-color:#cbd5e0;}");
    QHBoxLayout *imageLayout=new QHBoxLayout(); imageLayout->addWidget(imageEdit); imageLayout->addWidget(browseBtn);
    connect(browseBtn,&QPushButton::clicked,[&dialog,imageEdit](){ QString f=QFileDialog::getOpenFileName(&dialog,"Sélectionner une image",QDir::homePath(),"Images (*.png *.jpg *.jpeg *.bmp *.gif)"); if(!f.isEmpty()) imageEdit->setText(f); });
    form->addRow("ID Produit:",idProdEdit); form->addRow("Nom du produit:",nomProdEdit); form->addRow("Catégorie:",categorieProdEdit);
    form->addRow("Prix:",prixProdEdit); form->addRow("Dimensions:",dimensionsEdit); form->addRow("Matériaux:",matUtiliseEdit);
    form->addRow("Date création:",dateCreationEdit); form->addRow("Image:",imageLayout);
    mainLayout->addLayout(form);
    QDialogButtonBox *buttonBox=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Modifier"); buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet("QPushButton{background-color:#8A9A5B;color:white;border:none;border-radius:5px;padding:8px 20px;font-weight:bold;}QPushButton:hover{background-color:#9aaa6b;}");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Annuler"); buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(40);
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet("QPushButton{background-color:#e2e8f0;color:#4a5568;border:none;border-radius:5px;padding:8px 20px;}QPushButton:hover{background-color:#cbd5e0;}");
    mainLayout->addWidget(buttonBox);
    connect(buttonBox,&QDialogButtonBox::accepted,&dialog,&QDialog::accept); connect(buttonBox,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    if (dialog.exec()==QDialog::Accepted) {
        QString prixText=prixProdEdit->text(); if(!prixText.contains("EUR")) prixText+=" EUR";
        table->item(row,0)->setText(idProdEdit->text()); table->item(row,1)->setText(nomProdEdit->text());
        table->item(row,2)->setText(categorieProdEdit->currentText()); table->item(row,3)->setText(prixText);
        table->item(row,4)->setText(dimensionsEdit->text()); table->item(row,5)->setText(matUtiliseEdit->text());
        table->item(row,6)->setText(dateCreationEdit->date().toString("dd/MM/yyyy")); table->item(row,7)->setText(imageEdit->text());
        QMessageBox::information(this,"Succès","Produit modifié avec succès!");
    }
}

void MainWindow::onDeleteProductClicked()
{
    QWidget *productsPage=stackedWidget->widget(4); QTableWidget *table=productsPage->findChild<QTableWidget*>("productsTable");
    if (!table||table->selectedItems().isEmpty()) { QMessageBox::warning(this,"Aucune sélection","Veuillez sélectionner un produit à supprimer."); return; }
    int row=table->currentRow();
    QString idProd=table->item(row,0)?table->item(row,0)->text():"";
    QString nomProd=table->item(row,1)?table->item(row,1)->text():"Produit sans nom";
    if (QMessageBox::question(this,"Confirmer la suppression",QString("Supprimer \"%1\" (ID: %2) ?\n\nCette action est irréversible.").arg(nomProd,idProd),QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
    { table->removeRow(row); QMessageBox::information(this,"Succès","Produit supprimé avec succès!"); }
}

QFrame* MainWindow::createSeparator()
{
    QFrame *sep = new QFrame(); sep->setFrameShape(QFrame::HLine); sep->setFrameShadow(QFrame::Sunken); return sep;
}
