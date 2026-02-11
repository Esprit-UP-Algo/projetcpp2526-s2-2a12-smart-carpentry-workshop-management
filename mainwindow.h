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
<<<<<<< HEAD
#include <QKeyEvent>
#include <QList>

#include "src/models/employee.h"
#include "toggleswitch.h"

// Forward declaration (IMPORTANT : pas d'include .cpp ici)
class StockPage;

=======
#include "src/models/employee.h"
#include "src/modules/stock/stockpage.h"
#include "toggleswitch.h"

>>>>>>> 99c9fa649b408161021bdb48c65a32620ad00b4a
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
<<<<<<< HEAD
    explicit MainWindow(QWidget *parent = nullptr);
=======
    MainWindow(QWidget *parent = nullptr);
>>>>>>> 99c9fa649b408161021bdb48c65a32620ad00b4a
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
<<<<<<< HEAD


=======
    
>>>>>>> 99c9fa649b408161021bdb48c65a32620ad00b4a
    // Auth slots
    void onLoginSuccess(const Employee& employee);
    void onLogout();

private:
    Ui::MainWindow *ui;
<<<<<<< HEAD
    QFrame* createSeparator();
    QTableWidget *financeTable;
    // Layout components
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;

=======
    
    // Layout components
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    
>>>>>>> 99c9fa649b408161021bdb48c65a32620ad00b4a
    // Auth stack
    QStackedWidget *authStack;
    QWidget *authPage;
    QWidget *mainAppPage;
<<<<<<< HEAD

=======
    
>>>>>>> 99c9fa649b408161021bdb48c65a32620ad00b4a
    // Sidebar
    QFrame *sidebar;
    QVBoxLayout *sidebarLayout;
    QList<QPushButton*> sidebarButtons;
    QLabel *logoLabel;
<<<<<<< HEAD

    // Content area
    QFrame *contentArea;
    QVBoxLayout *contentLayout;

=======
    
    // Content area
    QFrame *contentArea;
    QVBoxLayout *contentLayout;
    
>>>>>>> 99c9fa649b408161021bdb48c65a32620ad00b4a
    // Navigation bar
    QFrame *navbar;
    QHBoxLayout *navbarLayout;
    QLabel *pageTitle;
    QPushButton *profileBtn;
    QLabel *profileName;
    ToggleSwitch *darkModeToggle;
<<<<<<< HEAD

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
=======
    
    // Container
    QFrame *container;
    QStackedWidget *stackedWidget;
    
    // State
    bool isDarkMode;
    qreal currentScale;
    Employee currentEmployee;
    
    // Current page reference
    QTableWidget *currentTable;
    
>>>>>>> 99c9fa649b408161021bdb48c65a32620ad00b4a
    void setupUI();
    void setupAuth();
    void loadStyleSheet();
    void createSidebar();
    void createNavbar();
    void createContainer();
    void createPages();
    void applyScale(qreal scale);
    void showMainApp();
<<<<<<< HEAD

    QLabel* createRoundedAvatar(const QString& imagePath, int size);

=======
    QLabel* createRoundedAvatar(const QString& imagePath, int size);
    
>>>>>>> 99c9fa649b408161021bdb48c65a32620ad00b4a
    // Page creation methods
    QWidget* createProjectsPage();
    QWidget* createEmployeesPage();
    QWidget* createFinancePage();
    QWidget* createDesignsPage();
<<<<<<< HEAD

=======
    
>>>>>>> 99c9fa649b408161021bdb48c65a32620ad00b4a
    // CRUD operations for Employees
    void addEmployee();
    void editEmployee();
    void deleteEmployee();
};

#endif // MAINWINDOW_H
