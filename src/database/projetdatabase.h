#ifndef PROJETDATABASE_H
#define PROJETDATABASE_H

#include "../models/projet.h"
#include <QList>
#include <QMap>
#include <QString>

class ProjetDatabase
{
public:
    static ProjetDatabase& instance();

    // CRUD
    bool addProjet(const Projet& projet);
    bool updateProjet(const Projet& projet);
    bool deleteProjet(const QString& id);
    Projet getProjet(const QString& id) const;
    QList<Projet> getAllProjets() const;

    // Search
    QList<Projet> searchByNom(const QString& nom) const;
    QList<Projet> searchByClient(const QString& client) const;
    QList<Projet> searchByStatut(const QString& statut) const;
    QList<Projet> searchByDateRange(const QDate& debut, const QDate& fin) const;

    // Sort
    QList<Projet> sortByDateDebut(bool ascending = true) const;
    QList<Projet> sortByDateFin(bool ascending = true) const;
    QList<Projet> sortByBudget(bool ascending = false) const;
    QList<Projet> sortByAvancement(bool ascending = false) const;

    // Stats
    int getTotalProjets() const;
    int getProjetsActifs() const;
    int getProjetsTermines() const;
    int getProjetsEnAttente() const;
    double getBudgetMoyen() const;
    double getBudgetTotal() const;
    QMap<QString, int> getProjetsByStatut() const;
    QMap<QString, double> getAvancementMoyenByType() const;

    // Export
    QString generateContratPDF(const QString& projetId, const QString& outputPath = "");

private:
    ProjetDatabase() = default;
    ~ProjetDatabase() = default;
    ProjetDatabase(const ProjetDatabase&) = delete;
    ProjetDatabase& operator=(const ProjetDatabase&) = delete;

    Projet rowToProjet(const class QSqlQuery& q) const;
};

#endif // PROJETDATABASE_H
