#include "produit.h"
#include <QPdfWriter>

Produit::Produit() {}

Produit::Produit(const QString& id, const QString& nom, const QString& categorie,
                 double prix, const QString& dimensions,
                 const QString& mat, const QDate& date,
                 const QString& image)
    : m_id(id),
    m_nom(nom),
    m_categorie(categorie),
    m_prix(prix),
    m_dimensions(dimensions),
    m_mat(mat),
    m_date(date),
    m_image(image)
{
}

bool Produit::isValid() const
{
    return !m_id.isEmpty() && !m_nom.isEmpty();
}
