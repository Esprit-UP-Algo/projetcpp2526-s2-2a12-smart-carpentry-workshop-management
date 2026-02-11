#include "employeedatabase.h"
#include <QDateTime>
#include <algorithm>

EmployeeDatabase& EmployeeDatabase::instance()
{
    static EmployeeDatabase instance;
    return instance;
}

EmployeeDatabase::EmployeeDatabase()
{
    // Initialize with sample data
    loadSampleData();
}

bool EmployeeDatabase::addEmployee(const Employee& employee)
{
    if (!employee.isValid() || m_employees.contains(employee.getId())) {
        return false;
    }
    
    m_employees.insert(employee.getId(), employee);
    return true;
}

bool EmployeeDatabase::updateEmployee(const Employee& employee)
{
    if (!employee.isValid() || !m_employees.contains(employee.getId())) {
        return false;
    }
    
    m_employees[employee.getId()] = employee;
    return true;
}

bool EmployeeDatabase::deleteEmployee(const QString& id)
{
    if (!m_employees.contains(id)) {
        return false;
    }
    
    m_employees.remove(id);
    return true;
}

Employee EmployeeDatabase::getEmployee(const QString& id) const
{
    return m_employees.value(id, Employee());
}

QList<Employee> EmployeeDatabase::getAllEmployees() const
{
    return m_employees.values();
}

QList<Employee> EmployeeDatabase::searchByCin(const QString& cin) const
{
    QList<Employee> results;
    for (const Employee& emp : m_employees) {
        if (emp.getCin().contains(cin, Qt::CaseInsensitive)) {
            results.append(emp);
        }
    }
    return results;
}

QList<Employee> EmployeeDatabase::searchByPoste(const QString& poste) const
{
    QList<Employee> results;
    for (const Employee& emp : m_employees) {
        if (emp.getPoste().contains(poste, Qt::CaseInsensitive)) {
            results.append(emp);
        }
    }
    return results;
}

QList<Employee> EmployeeDatabase::searchByName(const QString& name) const
{
    QList<Employee> results;
    for (const Employee& emp : m_employees) {
        QString fullName = emp.getFullName();
        if (fullName.contains(name, Qt::CaseInsensitive) ||
            emp.getNom().contains(name, Qt::CaseInsensitive) ||
            emp.getPrenom().contains(name, Qt::CaseInsensitive)) {
            results.append(emp);
        }
    }
    return results;
}

QList<Employee> EmployeeDatabase::sortBySalaire(bool ascending) const
{
    QList<Employee> sorted = m_employees.values();
    std::sort(sorted.begin(), sorted.end(), [ascending](const Employee& a, const Employee& b) {
        return ascending ? a.getSalaire() < b.getSalaire() : a.getSalaire() > b.getSalaire();
    });
    return sorted;
}

QList<Employee> EmployeeDatabase::sortByDateEmbauche(bool ascending) const
{
    QList<Employee> sorted = m_employees.values();
    std::sort(sorted.begin(), sorted.end(), [ascending](const Employee& a, const Employee& b) {
        return ascending ? a.getDateEmbauche() < b.getDateEmbauche() 
                        : a.getDateEmbauche() > b.getDateEmbauche();
    });
    return sorted;
}

QList<Employee> EmployeeDatabase::sortByPerformance(bool ascending) const
{
    QList<Employee> sorted = m_employees.values();
    std::sort(sorted.begin(), sorted.end(), [ascending](const Employee& a, const Employee& b) {
        return ascending ? a.getPerformance() < b.getPerformance() 
                        : a.getPerformance() > b.getPerformance();
    });
    return sorted;
}

double EmployeeDatabase::getAverageSalary() const
{
    if (m_employees.isEmpty()) return 0.0;
    
    double total = 0.0;
    for (const Employee& emp : m_employees) {
        total += emp.getSalaire();
    }
    return total / m_employees.size();
}

double EmployeeDatabase::getAveragePerformance() const
{
    if (m_employees.isEmpty()) return 0.0;
    
    double total = 0.0;
    for (const Employee& emp : m_employees) {
        total += emp.getPerformance();
    }
    return total / m_employees.size();
}

QMap<QString, int> EmployeeDatabase::getEmployeeCountByPoste() const
{
    QMap<QString, int> counts;
    for (const Employee& emp : m_employees) {
        counts[emp.getPoste()]++;
    }
    return counts;
}

QString EmployeeDatabase::generateNextId() const
{
    int maxId = 0;
    for (const Employee& emp : m_employees) {
        QString id = emp.getId();
        if (id.startsWith("EMP")) {
            bool ok;
            int num = id.mid(3).toInt(&ok);
            if (ok && num > maxId) {
                maxId = num;
            }
        }
    }
    return QString("EMP%1").arg(maxId + 1, 4, 10, QChar('0'));
}

void EmployeeDatabase::loadSampleData()
{
    // Sample employees based on specifications
    Employee emp1("EMP0001", "12345678", "Ben Ahmed", "Karim", "Menuisier");
    emp1.setEmail("karim.benahmed@woodflow.tn");
    emp1.setTelephone("+216 98 123 456");
    emp1.setDateEmbauche(QDateTime::fromString("2020-03-15", "yyyy-MM-dd"));
    emp1.setSalaire(1800.0);
    emp1.setCompetences(QStringList() << "Ébénisterie" << "Pose" << "Finition");
    emp1.setDisponibilite("Disponible");
    emp1.setPerformance(8.5);
    emp1.setNbJoursConges(5);
    emp1.setNbJoursAbsence(2);
    emp1.setHeuresTravail(160.0);
    addEmployee(emp1);
    
    Employee emp2("EMP0002", "87654321", "Trabelsi", "Mohamed", "Chef d'équipe");
    emp2.setEmail("mohamed.trabelsi@woodflow.tn");
    emp2.setTelephone("+216 22 234 567");
    emp2.setDateEmbauche(QDateTime::fromString("2018-06-01", "yyyy-MM-dd"));
    emp2.setSalaire(2500.0);
    emp2.setCompetences(QStringList() << "Gestion d'équipe" << "Menuiserie générale" << "Installation");
    emp2.setDisponibilite("Disponible");
    emp2.setPerformance(9.2);
    emp2.setNbJoursConges(10);
    emp2.setNbJoursAbsence(1);
    emp2.setHeuresTravail(168.0);
    addEmployee(emp2);
    
    Employee emp3("EMP0003", "11223344", "Hamdi", "Salma", "Apprenti");
    emp3.setEmail("salma.hamdi@woodflow.tn");
    emp3.setTelephone("+216 55 345 678");
    emp3.setDateEmbauche(QDateTime::fromString("2023-01-10", "yyyy-MM-dd"));
    emp3.setSalaire(1200.0);
    emp3.setCompetences(QStringList() << "Assemblage" << "Ponçage");
    emp3.setDisponibilite("En formation");
    emp3.setPerformance(7.0);
    emp3.setNbJoursConges(0);
    emp3.setNbJoursAbsence(3);
    emp3.setHeuresTravail(140.0);
    addEmployee(emp3);
    
    Employee emp4("EMP0004", "99887766", "Gharbi", "Youssef", "Menuisier");
    emp4.setEmail("youssef.gharbi@woodflow.tn");
    emp4.setTelephone("+216 25 456 789");
    emp4.setDateEmbauche(QDateTime::fromString("2021-09-20", "yyyy-MM-dd"));
    emp4.setSalaire(1900.0);
    emp4.setCompetences(QStringList() << "Menuiserie sur mesure" << "Finition" << "Restauration");
    emp4.setDisponibilite("En congé");
    emp4.setPerformance(8.8);
    emp4.setNbJoursConges(15);
    emp4.setNbJoursAbsence(0);
    emp4.setHeuresTravail(155.0);
    addEmployee(emp4);
    
    Employee emp5("EMP0005", "55667788", "Jlassi", "Amira", "Menuisier");
    emp5.setEmail("amira.jlassi@woodflow.tn");
    emp5.setTelephone("+216 98 567 890");
    emp5.setDateEmbauche(QDateTime::fromString("2019-11-05", "yyyy-MM-dd"));
    emp5.setSalaire(2100.0);
    emp5.setCompetences(QStringList() << "Design" << "Ébénisterie" << "Agencement");
    emp5.setDisponibilite("Disponible");
    emp5.setPerformance(9.0);
    emp5.setNbJoursConges(7);
    emp5.setNbJoursAbsence(1);
    emp5.setHeuresTravail(162.0);
    addEmployee(emp5);
}

void EmployeeDatabase::clearAll()
{
    m_employees.clear();
}
