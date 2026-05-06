#ifndef STOCKDATABASE_H
#define STOCKDATABASE_H

#include "src/models/stockmaterial.h"
#include <QList>
#include <QString>
#include <QMap>

/**
 * StockDatabase — thin Oracle DB wrapper (singleton).
 * Gère la table MATERIAU, avec FK ID_PRODUIT → PRODUIT.ID_PROD.
 */
class StockDatabase {
public:
    static StockDatabase& instance();

    // CRUD


    //arduino
    bool updateQuantiteByNom(const QString& nom, double quantiteGrams);

    bool addMaterial(const StockMaterial& mat);
    bool updateMaterial(const StockMaterial& mat);
    bool deleteMaterial(int id);
    StockMaterial        getMaterial(int id)    const;
    QList<StockMaterial> getAllMaterials()       const;

    // Recherche par produit associé (FK)
    QList<StockMaterial> getMaterialsByProduit(int idProduit) const;

    // Recherche par locale (pour la carte)
    QList<StockMaterial> getMaterialsByLocale(const QString& locale) const;

    // Nombre de matériaux par locale (pour la carte — évite de charger tout)
    QMap<QString, int> getCountByLocale() const;

    // Search
    QList<StockMaterial> searchByNom(const QString& nom)             const;
    QList<StockMaterial> searchByType(const QString& type)           const;
    QList<StockMaterial> searchByNomOrType(const QString& text)      const;

    // Sort
    QList<StockMaterial> sortByNom(bool ascending = true)            const;
    QList<StockMaterial> sortByConsoMensuelle(bool ascending = true) const;

    // Stats
    double getTotalValue()         const;
    int    getTotalCount()         const;
    int    getAlertCount()         const;
    double getAverageRenewalRate() const;

private:
    StockDatabase() = default;
    ~StockDatabase() = default;
    StockDatabase(const StockDatabase&)            = delete;
    StockDatabase& operator=(const StockDatabase&) = delete;

    StockMaterial rowToMaterial(const class QSqlQuery& q) const;
};

#endif // STOCKDATABASE_H
