#include "produit.h"

Produit::Produit()
    : m_prix(0.0)
{
}

Produit::Produit(const QString& id, const QString& nom, const QString& categorie,
                 double prix, const QString& dimensions, const QString& matUtilise,
                 const QDate& dateCreation, const QString& projetId)
    : m_id(id), m_nom(nom), m_categorie(categorie), m_prix(prix),
    m_dimensions(dimensions), m_matUtilise(matUtilise),
    m_dateCreation(dateCreation), m_projetId(projetId)
{
}

bool Produit::isValid() const
{
    return !m_nom.isEmpty() && m_prix >= 0.0 && !m_projetId.isEmpty();
}
