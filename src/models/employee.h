#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QString>
#include <QStringList>
#include <QDateTime>

class Employee {
public:
    Employee();
    Employee(const QString& id, const QString& cin, const QString& nom,
             const QString& prenom, const QString& poste);

    // Getters
    QString getId()           const { return m_id; }
    QString getCin()          const { return m_cin; }
    QString getNom()          const { return m_nom; }
    QString getPrenom()       const { return m_prenom; }
    QString getPoste()        const { return m_poste; }
    QString getEmail()        const { return m_email; }
    QString getTelephone()    const { return m_telephone; }
    QDateTime getDateEmbauche() const { return m_dateEmbauche; }
    double getSalaire()       const { return m_salaire; }
    QStringList getCompetences() const { return m_competences; }
    QString getDisponibilite() const { return m_disponibilite; }
    double getPerformance()   const { return m_performance; }
    int getNbJoursConges()    const { return m_nbJoursConges; }
    int getNbJoursAbsence()   const { return m_nbJoursAbsence; }
    double getHeuresTravail() const { return m_heuresTravail; }
    // NEW: password (stored as SHA-256 hex hash)
    QString getMotDePasse()   const { return m_motDePasse; }

    // Setters
    void setId(const QString& v)            { m_id = v; }
    void setCin(const QString& v)           { m_cin = v; }
    void setNom(const QString& v)           { m_nom = v; }
    void setPrenom(const QString& v)        { m_prenom = v; }
    void setPoste(const QString& v)         { m_poste = v; }
    void setEmail(const QString& v)         { m_email = v; }
    void setTelephone(const QString& v)     { m_telephone = v; }
    void setDateEmbauche(const QDateTime& v){ m_dateEmbauche = v; }
    void setSalaire(double v)               { m_salaire = v; }
    void setCompetences(const QStringList& v){ m_competences = v; }
    void setDisponibilite(const QString& v) { m_disponibilite = v; }
    void setPerformance(double v)           { m_performance = v; }
    void setNbJoursConges(int v)            { m_nbJoursConges = v; }
    void setNbJoursAbsence(int v)           { m_nbJoursAbsence = v; }
    void setHeuresTravail(double v)         { m_heuresTravail = v; }
    void setMotDePasse(const QString& v)    { m_motDePasse = v; }

    // Utility
    QString getFullName() const { return m_prenom + " " + m_nom; }
    QString getCompetencesString() const { return m_competences.join(", "); }
    bool isValid() const;

    bool operator==(const Employee& o) const { return m_id == o.m_id; }

private:
    QString     m_id;
    QString     m_cin;
    QString     m_nom;
    QString     m_prenom;
    QString     m_poste;
    QString     m_email;
    QString     m_telephone;
    QDateTime   m_dateEmbauche;
    double      m_salaire       = 0.0;
    QStringList m_competences;
    QString     m_disponibilite;
    double      m_performance   = 0.0;
    int         m_nbJoursConges = 0;
    int         m_nbJoursAbsence= 0;
    double      m_heuresTravail = 0.0;
    QString     m_motDePasse;   // SHA-256 hex
};

#endif // EMPLOYEE_H
