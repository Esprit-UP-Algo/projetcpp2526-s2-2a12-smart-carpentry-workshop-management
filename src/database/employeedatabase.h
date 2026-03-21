#ifndef EMPLOYEEDATABASE_H
#define EMPLOYEEDATABASE_H

#include "../models/employee.h"
#include <QList>
#include <QMap>
#include <QString>

/**
 * EmployeeDatabase — thin Oracle DB wrapper (singleton).
 * All operations use the named connection "oracle_conn" (Connection::CONN_NAME).
 */
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

    // Search
    QList<Employee> searchByName(const QString& name) const;
    QList<Employee> searchByCin(const QString& cin) const;
    QList<Employee> searchByPoste(const QString& poste) const;

    // Sort (fetched + sorted in memory — small dataset)
    QList<Employee> sortBySalaire(bool ascending = true) const;
    QList<Employee> sortByDateEmbauche(bool ascending = true) const;
    QList<Employee> sortByPerformance(bool ascending = false) const;

    // Stats
    double getAverageSalary() const;
    double getAveragePerformance() const;
    int    getTotalEmployees() const;
    QMap<QString, int> getEmployeeCountByPoste() const;

    // Utility
    QString generateNextId() const;   // returns "EMP0007" style string (unused for Oracle auto-ID)

private:
    EmployeeDatabase() = default;
    ~EmployeeDatabase() = default;
    EmployeeDatabase(const EmployeeDatabase&) = delete;
    EmployeeDatabase& operator=(const EmployeeDatabase&) = delete;

    // Helpers
    Employee rowToEmployee(const class QSqlQuery& q) const;
};

#endif // EMPLOYEEDATABASE_H
