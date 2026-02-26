#ifndef PROJET_H
#define PROJET_H

#include <QString>
#include <QDate>
#include <QStringList>

class Projet
{
public:
    Projet();
    Projet(const QString& id, const QString& nom, const QString& client,
           const QString& adresse, const QString& type, const QDate& dateDebut,
           const QDate& dateFin, const QString& statut, double budget);

    // Getters
    QString getId() const { return m_id; }
    QString getNom() const { return m_nom; }
    QString getClient() const { return m_client; }
    QString getAdresse() const { return m_adresse; }
    QString getType() const { return m_type; }
    QDate getDateDebut() const { return m_dateDebut; }
    QDate getDateFin() const { return m_dateFin; }
    QString getStatut() const { return m_statut; }
    double getBudget() const { return m_budget; }
    QString getDescription() const { return m_description; }
    QStringList getMateriaux() const { return m_materiaux; }
    double getAvancement() const { return m_avancement; }

    // Setters
    void setId(const QString& v) { m_id = v; }
    void setNom(const QString& v) { m_nom = v; }
    void setClient(const QString& v) { m_client = v; }
    void setAdresse(const QString& v) { m_adresse = v; }
    void setType(const QString& v) { m_type = v; }
    void setDateDebut(const QDate& v) { m_dateDebut = v; }
    void setDateFin(const QDate& v) { m_dateFin = v; }
    void setStatut(const QString& v) { m_statut = v; }
    void setBudget(double v) { m_budget = v; }
    void setDescription(const QString& v) { m_description = v; }
    void setMateriaux(const QStringList& v) { m_materiaux = v; }
    void setAvancement(double v) { m_avancement = v; }

    // Utility
    QString getFullInfo() const { return m_nom + " - " + m_client; }
    bool isValid() const;
    QColor getStatutColor() const;
    QString getAvancementText() const;

    bool operator==(const Projet& o) const { return m_id == o.m_id; }

private:
    QString m_id;
    QString m_nom;
    QString m_client;
    QString m_adresse;
    QString m_type;
    QDate m_dateDebut;
    QDate m_dateFin;
    QString m_statut;
    double m_budget = 0.0;
    QString m_description;
    QStringList m_materiaux;
    double m_avancement = 0.0; // Pourcentage d'avancement
};

#endif // PROJET_H
