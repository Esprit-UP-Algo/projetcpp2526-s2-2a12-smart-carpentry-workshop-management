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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
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
    void showNotificationsMenu();
    void toggleDarkMode();
    void onScaleUp();
    void onScaleDown();
    void onScaleReset();

private:
    Ui::MainWindow *ui;
    
    // Layout components
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    
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
    QPushButton *notifBtn;
    
    // Container
    QFrame *container;
    QStackedWidget *stackedWidget;
    
    // State
    bool isDarkMode;
    qreal currentScale;
    
    // Current page reference
    QTableWidget *currentTable;
    
    void setupUI();
    void loadStyleSheet();
    void createSidebar();
    void createNavbar();
    void createContainer();
    void createPages();
    void applyScale(qreal scale);
    QLabel* createRoundedAvatar(const QString& imagePath, int size);
    
    // Page creation methods
    QWidget* createProjectsPage();
    QWidget* createEmployeesPage();
    QWidget* createStocksPage();
    QWidget* createFinancePage();
    QWidget* createDesignsPage();
    
    // CRUD operations for Employees
    void addEmployee();
    void editEmployee();
    void deleteEmployee();
};

#endif // MAINWINDOW_H
