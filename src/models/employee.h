#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QString>
#include <QStringList>
#include <QDateTime>

class Employee {
public:
    // Constructor
    Employee();
    Employee(const QString& id, const QString& cin, const QString& nom, 
             const QString& prenom, const QString& poste);
    
    // Getters
    QString getId() const { return m_id; }
    QString getCin() const { return m_cin; }
    QString getNom() const { return m_nom; }
    QString getPrenom() const { return m_prenom; }
    QString getPoste() const { return m_poste; }
    QString getEmail() const { return m_email; }
    QString getTelephone() const { return m_telephone; }
    QDateTime getDateEmbauche() const { return m_dateEmbauche; }
    double getSalaire() const { return m_salaire; }
    QStringList getCompetences() const { return m_competences; }
    QString getDisponibilite() const { return m_disponibilite; }
    double getPerformance() const { return m_performance; }
    int getNbJoursConges() const { return m_nbJoursConges; }
    int getNbJoursAbsence() const { return m_nbJoursAbsence; }
    double getHeuresTravail() const { return m_heuresTravail; }
    
    // Setters
    void setId(const QString& id) { m_id = id; }
    void setCin(const QString& cin) { m_cin = cin; }
    void setNom(const QString& nom) { m_nom = nom; }
    void setPrenom(const QString& prenom) { m_prenom = prenom; }
    void setPoste(const QString& poste) { m_poste = poste; }
    void setEmail(const QString& email) { m_email = email; }
    void setTelephone(const QString& telephone) { m_telephone = telephone; }
    void setDateEmbauche(const QDateTime& date) { m_dateEmbauche = date; }
    void setSalaire(double salaire) { m_salaire = salaire; }
    void setCompetences(const QStringList& competences) { m_competences = competences; }
    void setDisponibilite(const QString& disponibilite) { m_disponibilite = disponibilite; }
    void setPerformance(double performance) { m_performance = performance; }
    void setNbJoursConges(int nb) { m_nbJoursConges = nb; }
    void setNbJoursAbsence(int nb) { m_nbJoursAbsence = nb; }
    void setHeuresTravail(double heures) { m_heuresTravail = heures; }
    
    // Utility methods
    QString getFullName() const { return m_prenom + " " + m_nom; }
    bool isValid() const;
    QString getCompetencesString() const { return m_competences.join(", "); }
    
    // Comparison operator (needed for QList operations)
    bool operator==(const Employee& other) const {
        return m_id == other.m_id;
    }
    
private:
    QString m_id;
    QString m_cin;
    QString m_nom;
    QString m_prenom;
    QString m_poste;
    QString m_email;
    QString m_telephone;
    QDateTime m_dateEmbauche;
    double m_salaire;
    QStringList m_competences;
    QString m_disponibilite;
    double m_performance;
    int m_nbJoursConges;
    int m_nbJoursAbsence;
    double m_heuresTravail;
};

#endif // EMPLOYEE_H
