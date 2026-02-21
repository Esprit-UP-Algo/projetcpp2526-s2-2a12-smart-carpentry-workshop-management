#ifndef EMPLOYEEMANAGEMENTPAGE_H
#define EMPLOYEEMANAGEMENTPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
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
    QPushButton *m_refreshButton;
    
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
    
    // Utility methods
    void updateButtonStates();
    QString getPosteBadgeColor(const QString& poste) const;
    QString getDisponibiliteBadgeColor(const QString& disponibilite) const;
};

#endif // EMPLOYEEMANAGEMENTPAGE_H
