#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QTableWidget>
#include <QMenu>
#include "toggleswitch.h"
#include "src/models/employee.h"
#include "src/modules/chat/chatbotwidget.h"

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
    void keyPressEvent(QKeyEvent *event) override;  // ← AJOUTEZ CETTE LIGNE

private slots:
    void onLoginSuccess(const Employee& employee);
    void onLogout();
    void onSidebarButtonClicked(int index);
    void toggleDarkMode();
    void onScaleUp();
    void onScaleDown();
    void onScaleReset();
    void onAddProductClicked();
    void onEditProductClicked();
    void onDeleteProductClicked();
    void showProfileMenu();

private:
    void setupUI();
    void setupAuth();
    void setupChatbot();
    void showMainApp();
    void createSidebar();
    void createNavbar();
    void createContainer();
    void createPages();
    QWidget* createEmployeesPage();
    QWidget* createFinancePage();
    QWidget* createProductsPage();
    QWidget* createChatbotPage();
    QLabel* createRoundedAvatar(const QString& imagePath, int size);
    void applyScale(qreal scale);
    void loadStyleSheet();
    QFrame* createSeparator();
    void addEmployee();
    void editEmployee();
    void deleteEmployee();

    // UI Components
    Ui::MainWindow *ui;
    QWidget *centralWidget;
    QWidget *authPage;
    QWidget *mainAppPage;
    QFrame *sidebar;
    QFrame *contentArea;
    QFrame *navbar;
    QFrame *container;
    QHBoxLayout *mainLayout;
    QVBoxLayout *sidebarLayout;
    QHBoxLayout *navbarLayout;
    QVBoxLayout *contentLayout;
    QStackedWidget *authStack;
    QStackedWidget *stackedWidget;
    QLabel *pageTitle;
    ToggleSwitch *darkModeToggle;
    QPushButton *profileBtn;
    QLabel *profileName;
    QList<QPushButton*> sidebarButtons;
    QTableWidget *currentTable;

    // State
    bool isDarkMode;
    qreal currentScale;
    Employee currentEmployee;

    // Chatbot
    ChatbotWidget* m_chatbotWidget;
};

#endif // MAINWINDOW_H
