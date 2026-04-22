#ifndef EMPLOYEEMANAGEMENTPAGE_H
#define EMPLOYEEMANAGEMENTPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QFrame>
#include <QLabel>
#include <QEvent>
#include <QTabWidget>
#include "../../models/employee.h"

class EmployeeManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeManagementPage(QWidget *parent = nullptr);
    ~EmployeeManagementPage() = default;

private slots:
    void onAddEmployee();
    void onEditEmployee();
    void onDeleteEmployee();
    void onSearchTextChanged(const QString& text);
    void onFilterChanged(const QString& filter);
    void onSortChanged(int index);
    void onExportPDF();
    void onExportCertificate();
    void onRefreshTable();
    void onTableSelectionChanged();

private:
    // UI Components
    QTableWidget *m_table;
    QLineEdit *m_searchInput;
    QComboBox *m_filterCombo;
    QComboBox *m_sortCombo;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_exportButton;
    QPushButton *m_certButton;
    QPushButton *m_refreshButton;

    // Stats tab widgets
    QHBoxLayout  *m_kpiRow     = nullptr;
    QFrame       *m_kpiTotal   = nullptr;
    QFrame       *m_kpiPerf    = nullptr;
    QFrame       *m_kpiAvail   = nullptr;
    QFrame       *m_kpiPay     = nullptr;
    QTableWidget *m_statsTable = nullptr;
    QWidget      *m_listContainer = nullptr;
    QFrame       *m_toolbar    = nullptr;
    QLabel       *m_absLabel   = nullptr;
    QWidget      *m_statsInner  = nullptr;
    QTabWidget   *m_tabs        = nullptr;
    // Chart widgets (forward-declared via void* to avoid header pollution)
    class PieChartWidget *m_posteDonut = nullptr;
    class PieChartWidget *m_dispoPie   = nullptr;
    class GaugeWidget    *m_perfGauge  = nullptr;
    class BarChartWidget *m_salaryBar  = nullptr;

    // Layout methods
    void setupUI();
    void createToolbar();
    void createTable();
    void setupConnections();

    // Data methods
    void loadEmployees();
    void loadEmployees(const QList<Employee>& employees);
    void addEmployeeToTable(const Employee& employee, int row);
    Employee getSelectedEmployee() const;
    void populateFilterCombo();   // <-- NEW: fetches postes from DB

    QWidget* createStatsTab();
    void refreshStats();
    void applyTabTheme();
    void changeEvent(QEvent *event) override;

    // Utility methods
    void updateButtonStates();
    QString getPosteBadgeColor(const QString& poste) const;
    QString getDisponibiliteBadgeColor(const QString& disponibilite) const;
};

#endif // EMPLOYEEMANAGEMENTPAGE_H
