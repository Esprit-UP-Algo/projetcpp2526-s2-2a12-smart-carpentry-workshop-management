#include "stockmaterial.h"

StockMaterial::StockMaterial()
    : m_id(0)
    , m_quantite(0.0)
    , m_prixUnitaire(0.0)
    , m_seuilAlerte(0.0)
    , m_consoMensuelle(0.0)
    , m_unite("")
    , m_idProduit(0)
    , m_locale("")
    , m_emplacement("")
{
}

StockMaterial::StockMaterial(const QString& nom, const QString& type,
                             double quantite, double prixUnitaire,
                             const QString& fournisseur, double seuilAlerte,
                             const QDate& lastOrder, double consoMensuelle,
                             const QString& unite, int idProduit,
                             const QString& locale, const QString& emplacement)
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
    , m_idProduit(idProduit)
    , m_locale(locale)
    , m_emplacement(emplacement)
{
}

bool StockMaterial::isValid() const
{
    return !m_nomMat.isEmpty();
}
