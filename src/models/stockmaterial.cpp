#include "stockmaterial.h"

StockMaterial::StockMaterial()
    : m_id(0)
    , m_quantite(0.0)
    , m_prixUnitaire(0.0)
    , m_seuilAlerte(0.0)
    , m_consoMensuelle(0.0)
    , m_unite("")
{
}

StockMaterial::StockMaterial(const QString& nom, const QString& type,
                             double quantite, double prixUnitaire,
                             const QString& fournisseur, double seuilAlerte,
                             const QDate& lastOrder, double consoMensuelle,
                             const QString& unite)
    : m_id(0)
    , m_nomMat(nom)
    , m_typeMat(type)
    , m_quantite(quantite)
    , m_prixUnitaire(prixUnitaire)
    , m_fournisseur(fournisseur)
    , m_seuilAlerte(seuilAlerte)
    , m_lastOrder(lastOrder)
    , m_consoMensuelle(consoMensuelle)
    , m_unite(unite)
{
}

bool StockMaterial::isValid() const
{
    return !m_nomMat.isEmpty();
}
