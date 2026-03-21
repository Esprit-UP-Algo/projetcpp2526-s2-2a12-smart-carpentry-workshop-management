#ifndef EMPLOYEEDATABASE_H
#define EMPLOYEEDATABASE_H

#include "../models/employee.h"
#include <QList>
#include <QMap>
#include <QString>

class EmployeeDatabase {
public:
    static EmployeeDatabase& instance();

    // CRUD
    bool addEmployee(const Employee& employee);
    bool updateEmployee(const Employee& employee);
    bool deleteEmployee(const QString& id);
    Employee getEmployee(const QString& id) const;
    QList<Employee> getAllEmployees() const;

    // Auth
    Employee authenticate(const QString& cin, const QString& plainPassword) const;

    // 2FA secret management
    bool saveTotpSecret(const QString& employeeId, const QString& secret);
    bool clearTotpSecret(const QString& employeeId);

    // Search
    QList<Employee> searchByName(const QString& name) const;
    QList<Employee> searchByCin(const QString& cin) const;
    QList<Employee> searchByPoste(const QString& poste) const;

    // Sort
    QList<Employee> sortBySalaire(bool ascending = true) const;
    QList<Employee> sortByDateEmbauche(bool ascending = true) const;
    QList<Employee> sortByPerformance(bool ascending = false) const;

    // Stats
    double getAverageSalary() const;
    double getAveragePerformance() const;
    int    getTotalEmployees() const;
    QMap<QString, int> getEmployeeCountByPoste() const;

    QString generateNextId() const;

private:
    EmployeeDatabase() = default;
    ~EmployeeDatabase() = default;
    EmployeeDatabase(const EmployeeDatabase&) = delete;
    EmployeeDatabase& operator=(const EmployeeDatabase&) = delete;

    Employee rowToEmployee(const class QSqlQuery& q) const;
};

#endif // EMPLOYEEDATABASE_H
