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
#include "toggleswitch.h"

// Forward declaration (IMPORTANT : pas d'include .cpp ici)
class StockPage;

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
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onSearchTextChanged(const QString &text);
    void showProfileMenu();
    void toggleDarkMode();
    void onScaleUp();
    void onScaleDown();
    void onScaleReset();


    // Auth slots
    void onLoginSuccess(const Employee& employee);
    void onLogout();
    void onAddProductClicked();
    void onEditProductClicked();
    void onDeleteProductClicked();

private:
    Ui::MainWindow *ui;
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



};

#endif // MAINWINDOW_H
