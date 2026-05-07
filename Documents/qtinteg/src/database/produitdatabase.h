#ifndef PRODUITDATABASE_H
#define PRODUITDATABASE_H

#include "../models/produit.h"   // ✅ CORRECT
#include <QList>
#include <QString>
#include <QSqlQuery>

class ProduitDatabase
{
public:
    static ProduitDatabase& instance();

    bool addProduit(const Produit& p);
    bool updateProduit(const Produit& p);
    bool deleteProduit(const QString& id);
    Produit getProduit(const QString& id) const;
    QList<Produit> getAllProduits() const;

private:
    ProduitDatabase() = default;
    Produit rowToProduit(const QSqlQuery& q) const;
};

#endif

