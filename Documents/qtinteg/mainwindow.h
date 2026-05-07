#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QFrame>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QMenu>
#include <QKeyEvent>
#include <QList>

#include "src/models/employee.h"
#include "src/database/produitdatabase.h"
#include "src/modules/chat/chatbotwidget.h"
#include "toggleswitch.h"
#include "produit3ddialog.h"
#include "produitbarcodedialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSidebarButtonClicked(int index);
    void showProfileMenu();
    void toggleDarkMode();
    void onScaleUp();
    void onScaleDown();
    void onScaleReset();
    void onShowProductStats();

    // Auth
    void onLoginSuccess(const Employee& employee);
    void onLogout();

    // Produit — CRUD
    void onAddProductClicked();
    void onEditProductClicked();
    void onDeleteProductClicked();

    // Recherche + tri combinés
    void onProductSearchOrSort(const QString& text, const QString& sortKey);

    // Export & fonctionnalités avancées
    void onExportProductsPDF();
    void onShow3DProduct();
    void onShowBarcode();
    void onProductRowSelected(int row);

private:
    Ui::MainWindow *ui;

    // ── Layout principal ──────────────────────────────────────────────────────
    QWidget       *centralWidget;
    QHBoxLayout   *mainLayout;

    // ── Pile auth ─────────────────────────────────────────────────────────────
    QStackedWidget *authStack;
    QWidget        *authPage;
    QWidget        *mainAppPage;

    // ── Sidebar ───────────────────────────────────────────────────────────────
    QFrame               *sidebar;
    QVBoxLayout          *sidebarLayout;
    QList<QPushButton*>   sidebarButtons;

    // ── Zone de contenu ───────────────────────────────────────────────────────
    QFrame      *contentArea;
    QVBoxLayout *contentLayout;

    // ── Navbar ────────────────────────────────────────────────────────────────
    QFrame      *navbar;
    QHBoxLayout *navbarLayout;
    QLabel      *pageTitle;
    QPushButton *profileBtn;
    QLabel      *profileName;
    ToggleSwitch *darkModeToggle;

    // ── Container + pages ─────────────────────────────────────────────────────
    QFrame         *container;
    QStackedWidget *stackedWidget;

    // ── État ──────────────────────────────────────────────────────────────────
    bool      isDarkMode   = false;
    qreal     currentScale = 1.0;
    Employee  currentEmployee;
    QTableWidget *currentTable;

    // ── Cache produits ────────────────────────────────────────────────────────
    QList<Produit>  m_allProduits;
    QFrame         *m_detailPanel  = nullptr;
    QLabel         *m_detailImg    = nullptr;
    QLabel         *m_detailName   = nullptr;
    QLabel         *m_detailRef    = nullptr;
    QLabel         *m_detailPrice  = nullptr;
    QLabel         *m_detailCat    = nullptr;
    QLabel         *m_detailMat    = nullptr;
    QLabel         *m_detailDim    = nullptr;
    QLabel         *m_detailDate   = nullptr;
    QPushButton    *m_btn3D        = nullptr;
    QPushButton    *m_btnBarcode   = nullptr;
    Produit         m_selectedProduit;

    // ── Chatbot ───────────────────────────────────────────────────────────────
    ChatbotWidget *m_chatbotWidget;

    // ── Base de données produits ──────────────────────────────────────────────
    ProduitDatabase& produitDB;

    // ── Setup UI ──────────────────────────────────────────────────────────────
    void setupUI();
    void setupAuth();
    void setupChatbot();
    void loadStyleSheet();
    void showMainApp();
    void createSidebar();
    void createNavbar();
    void createContainer();
    void createPages();
    void applyScale(qreal scale);

    QLabel* createRoundedAvatar(const QString& imagePath, int size);
    QFrame* createSeparator();

    // ── Création des pages ────────────────────────────────────────────────────
    QWidget* createEmployeesPage();
    QWidget* createFinancePage();
    QWidget* createProductsPage();
    QWidget* createChatbotPage();

    // ── Panneau de détail produit ─────────────────────────────────────────────
    void buildDetailPanel(QWidget *parent);
    void refreshDetailPanel(const Produit& p);
    void clearDetailPanel();

    // ── Helpers produits ──────────────────────────────────────────────────────
    void loadProduits();
    void populateProductTable(QTableWidget *table, const QList<Produit>& list);
    void updateProductStatCards(QWidget *productsPage, const QList<Produit>& list);

    // ── CRUD employés (stubs) ─────────────────────────────────────────────────
    void addEmployee();
    void editEmployee();
    void deleteEmployee();
};

#endif // MAINWINDOW_H
