#ifndef PROJECTDATABASE_H
#define PROJECTDATABASE_H

#include "src/models/projet.h"
#include <QList>
#include <QString>
#include <QMap>

/**
 * ProjectDatabase — thin Oracle DB wrapper (singleton).
 * Gère la table PROJET, avec FK CHEF_PROJET → EMPLOYE.ID_EMP.
 */
class ProjectDatabase {
public:
    static ProjectDatabase& instance();

    // CRUD
    bool addProjet(const Projet& p);
    bool updateProjet(const Projet& p);
    bool deleteProjet(int id);
    Projet        getProjet(int id)    const;
    QList<Projet> getAllProjets()      const;

    // Search
    QList<Projet> searchByNom(const QString& nom)        const;
    QList<Projet> searchByClient(const QString& client)  const;
    QList<Projet> searchByNomOrClient(const QString& t)  const;

    // Sort
    QList<Projet> sortByDeadline(bool ascending = true)  const;
    QList<Projet> sortByBudget(bool ascending = true)    const;

    // Stats
    double getTotalBudget()           const;
    int    getTotalCount()            const;
    int    getCountByStatus(const QString& status) const;
    double getAverageBudget()         const;
    QMap<QString, int> getCountPerStatus() const;
    QMap<QString, int> getCountPerType()   const;
    int    getOverdueCount()          const;

    // FK helper: get employee name from id
    QString getEmployeeName(int idEmp) const;
    // Returns map id -> name for all employees
    QMap<int, QString> getAllEmployees() const;

private:
    ProjectDatabase() = default;
    ~ProjectDatabase() = default;
    ProjectDatabase(const ProjectDatabase&)            = delete;
    ProjectDatabase& operator=(const ProjectDatabase&) = delete;

    Projet rowToProjet(const class QSqlQuery& q) const;
};

#endif // PROJECTDATABASE_H
