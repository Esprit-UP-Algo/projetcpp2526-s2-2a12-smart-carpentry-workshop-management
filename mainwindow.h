#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
<<<<<<< Updated upstream
=======
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
#include "toggleswitch.h"
#include "src/database/produitdatabase.h"
#include "src/models/produit.h"

// Forward declaration (IMPORTANT : pas d'include .cpp ici)
class StockPage;
>>>>>>> Stashed changes

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
<<<<<<< Updated upstream
=======
    QFrame* createSeparator();
    QTableWidget *financeTable;
    // Layout components
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;

    // Auth stack
    QStackedWidget *authStack;
    QWidget *authPage;
    QWidget *mainAppPage;

    // Sidebar
    QFrame *sidebar;
    QVBoxLayout *sidebarLayout;
    QList<QPushButton*> sidebarButtons;
    QLabel *logoLabel;

    // Content area
    QFrame *contentArea;
    QVBoxLayout *contentLayout;

    // Navigation bar
    QFrame *navbar;
    QHBoxLayout *navbarLayout;
    QLabel *pageTitle;
    QPushButton *profileBtn;
    QLabel *profileName;
    ToggleSwitch *darkModeToggle;

    // Container
    QFrame *container;
    QStackedWidget *stackedWidget;

    // State
    bool isDarkMode = false;
    qreal currentScale = 1.0;
    Employee currentEmployee;

    // Current page reference
    QTableWidget *currentTable;

    // UI setup
    void setupUI();
    void setupAuth();
    void loadStyleSheet();
    void createSidebar();
    void createNavbar();
    void createContainer();
    void createPages();
    void applyScale(qreal scale);
    void showMainApp();

    QLabel* createRoundedAvatar(const QString& imagePath, int size);

    // Page creation methods
    QWidget* createProjectsPage();
    QWidget* createEmployeesPage();
    QWidget* createFinancePage();
     QWidget* createProductsPage();

    // CRUD operations for Employees
    void addEmployee();
    void editEmployee();
    void deleteEmployee();





>>>>>>> Stashed changes
};
#endif // MAINWINDOW_H
