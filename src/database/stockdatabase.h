#ifndef STOCKDATABASE_H
#define STOCKDATABASE_H

#include "src/models/stockmaterial.h"
#include <QList>
#include <QMap>
#include <QString>

/**
 * StockDatabase — thin Oracle DB wrapper (singleton).
 * Mirrors EmployeeDatabase for the MATERIAU table.
 */
class StockDatabase {
public:
    static StockDatabase& instance();

    // CRUD
    bool addMaterial(const StockMaterial& mat);
    bool updateMaterial(const StockMaterial& mat);
    bool deleteMaterial(int id);
    StockMaterial getMaterial(int id) const;
    QList<StockMaterial> getAllMaterials() const;

    // Search
    QList<StockMaterial> searchByNom(const QString& nom) const;
    QList<StockMaterial> searchByType(const QString& type) const;
    QList<StockMaterial> searchByNomOrType(const QString& text) const;

    // Sort
    QList<StockMaterial> sortByNom(bool ascending = true) const;
    QList<StockMaterial> sortByConsoMensuelle(bool ascending = true) const;

    // Stats
    double getTotalValue() const;          // SUM(QUANTITE_MAT * PRIX_UNITAIRE)
    int    getTotalCount() const;
    int    getAlertCount() const;          // WHERE QUANTITE_MAT < SEUIL_ALERTE
    double getAverageRenewalRate() const;  // AVG(CONSO_MENSUELLE)

private:
    StockDatabase() = default;
    ~StockDatabase() = default;
    StockDatabase(const StockDatabase&) = delete;
    StockDatabase& operator=(const StockDatabase&) = delete;

    StockMaterial rowToMaterial(const class QSqlQuery& q) const;
};

#endif // STOCKDATABASE_H
