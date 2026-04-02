#ifndef PRODUITDATABASE_H
#define PRODUITDATABASE_H

#include "src/models/produit.h"
#include <QList>

class ProduitDatabase
{
public:
    static ProduitDatabase& instance();

    // CRUD
    bool addProduit(const Produit& p);
    bool updateProduit(const Produit& p);
    bool deleteProduit(const QString& id);
    Produit getProduit(const QString& id) const;
    QList<Produit> getAllProduits() const;

private:
    ProduitDatabase() = default;
    ~ProduitDatabase() = default;

    ProduitDatabase(const ProduitDatabase&) = delete;
    ProduitDatabase& operator=(const ProduitDatabase&) = delete;

    Produit rowToProduit(const class QSqlQuery& q) const;
};

#endif
