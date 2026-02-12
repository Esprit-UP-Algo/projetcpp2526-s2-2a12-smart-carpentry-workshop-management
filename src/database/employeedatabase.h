#ifndef EMPLOYEEDATABASE_H
#define EMPLOYEEDATABASE_H

#include "../models/employee.h"
#include <QList>
#include <QMap>
#include <QString>

class EmployeeDatabase {
public:
    static EmployeeDatabase& instance();
    
    // CRUD operations
    bool addEmployee(const Employee& employee);
    bool updateEmployee(const Employee& employee);
    bool deleteEmployee(const QString& id);
    Employee getEmployee(const QString& id) const;
    QList<Employee> getAllEmployees() const;
    
    // Search and filter
    QList<Employee> searchByCin(const QString& cin) const;
    QList<Employee> searchByPoste(const QString& poste) const;
    QList<Employee> searchByName(const QString& name) const;
    
    // Sorting
    QList<Employee> sortBySalaire(bool ascending = true) const;
    QList<Employee> sortByDateEmbauche(bool ascending = true) const;
    QList<Employee> sortByPerformance(bool ascending = false) const;
    
    // Statistics
    double getAverageSalary() const;
    double getAveragePerformance() const;
    int getTotalEmployees() const { return m_employees.size(); }
    QMap<QString, int> getEmployeeCountByPoste() const;
    
    // Utility
    QString generateNextId() const;
    void loadSampleData();
    void clearAll();
    
private:
    EmployeeDatabase();
    ~EmployeeDatabase() = default;
    
    // Prevent copying
    EmployeeDatabase(const EmployeeDatabase&) = delete;
    EmployeeDatabase& operator=(const EmployeeDatabase&) = delete;
    
    QMap<QString, Employee> m_employees;
};

#endif // EMPLOYEEDATABASE_H
