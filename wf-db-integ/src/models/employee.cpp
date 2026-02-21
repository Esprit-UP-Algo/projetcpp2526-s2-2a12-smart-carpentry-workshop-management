#include "employee.h"

Employee::Employee()
    : m_salaire(0.0)
    , m_performance(0.0)
    , m_nbJoursConges(0)
    , m_nbJoursAbsence(0)
    , m_heuresTravail(0.0)
{
}

Employee::Employee(const QString& id, const QString& cin, const QString& nom, 
                   const QString& prenom, const QString& poste)
    : m_id(id)
    , m_cin(cin)
    , m_nom(nom)
    , m_prenom(prenom)
    , m_poste(poste)
    , m_salaire(0.0)
    , m_performance(0.0)
    , m_nbJoursConges(0)
    , m_nbJoursAbsence(0)
    , m_heuresTravail(0.0)
{
}

bool Employee::isValid() const
{
    return !m_id.isEmpty() && 
           !m_cin.isEmpty() && 
           !m_nom.isEmpty() && 
           !m_prenom.isEmpty() && 
           !m_poste.isEmpty();
}
