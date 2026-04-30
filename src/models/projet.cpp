#include "projet.h"
#include <QDate>

Projet::Projet()
    : m_id(0)
    , m_budget(0.0)
    , m_chefProjet(0)
{}

Projet::Projet(const QString& nom, const QDate& dateProjet, const QDate& deadline,
               const QString& status, double budget, const QString& adresse,
               const QString& client, const QString& type, int chefProjet)
    : m_id(0)
    , m_nom(nom)
    , m_dateProjet(dateProjet)
    , m_deadline(deadline)
    , m_status(status)
    , m_budget(budget)
    , m_adresse(adresse)
    , m_client(client)
    , m_type(type)
    , m_chefProjet(chefProjet)
{}

bool Projet::isOverdue() const
{
    return m_deadline.isValid()
    && QDate::currentDate() > m_deadline
                    && m_status != "Terminé"
        && m_status != "Annulé";
}

int Projet::daysRemaining() const
{
    if (!m_deadline.isValid()) return 0;
    return QDate::currentDate().daysTo(m_deadline);
}
