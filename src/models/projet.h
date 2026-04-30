#ifndef PROJET_H
#define PROJET_H

#include <QString>
#include <QDate>

class Projet {
public:
    Projet();
    Projet(const QString& nom, const QDate& dateProjet, const QDate& deadline,
           const QString& status, double budget, const QString& adresse,
           const QString& client, const QString& type, int chefProjet = 0);

    // Getters
    int     getId()           const { return m_id; }
    QString getNom()          const { return m_nom; }
    QDate   getDateProjet()   const { return m_dateProjet; }
    QDate   getDeadline()     const { return m_deadline; }
    QString getStatus()       const { return m_status; }
    double  getBudget()       const { return m_budget; }
    QString getAdresse()      const { return m_adresse; }
    QString getClient()       const { return m_client; }
    QString getType()         const { return m_type; }
    int     getChefProjet()   const { return m_chefProjet; }

    // Setters
    void setId(int v)                   { m_id = v; }
    void setNom(const QString& v)       { m_nom = v; }
    void setDateProjet(const QDate& v)  { m_dateProjet = v; }
    void setDeadline(const QDate& v)    { m_deadline = v; }
    void setStatus(const QString& v)    { m_status = v; }
    void setBudget(double v)            { m_budget = v; }
    void setAdresse(const QString& v)   { m_adresse = v; }
    void setClient(const QString& v)    { m_client = v; }
    void setType(const QString& v)      { m_type = v; }
    void setChefProjet(int v)           { m_chefProjet = v; }  // 0 = aucun

    bool isValid() const { return !m_nom.isEmpty() && !m_client.isEmpty(); }
    bool isOverdue() const;
    int  daysRemaining() const;

    bool operator==(const Projet& o) const { return m_id == o.m_id; }

private:
    int     m_id          = 0;
    QString m_nom;
    QDate   m_dateProjet;
    QDate   m_deadline;
    QString m_status;
    double  m_budget      = 0.0;
    QString m_adresse;
    QString m_client;
    QString m_type;
    int     m_chefProjet  = 0;   // FK → EMPLOYE.ID_EMP  (0 = NULL en DB)
};

#endif // PROJET_H
