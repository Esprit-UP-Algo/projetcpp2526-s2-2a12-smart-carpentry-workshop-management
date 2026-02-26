#include "stockdatabase.h"
#include "connection.h"   // same connection header as EmployeeDatabase

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <algorithm>

StockDatabase& StockDatabase::instance()
{
    static StockDatabase inst;
    return inst;
}

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------
StockMaterial StockDatabase::rowToMaterial(const QSqlQuery& q) const
{
    StockMaterial m;
    m.setId(q.value("ID_MAT").toInt());
    m.setNom(q.value("NOM_MAT").toString());
    m.setType(q.value("TYPE_MAT").toString());
    m.setQuantite(q.value("QUANTITE_MAT").toDouble());
    m.setPrixUnitaire(q.value("PRIX_UNITAIRE").toDouble());
    m.setFournisseur(q.value("FOURNISSEUR").toString());
    m.setSeuilAlerte(q.value("SEUIL_ALERTE").toDouble());

    QVariant dateVal = q.value("LAST_ORDER");
    if (!dateVal.isNull())
        m.setLastOrder(dateVal.toDate());

    m.setConsoMensuelle(q.value("CONSO_MENSUELLE").toDouble());
    m.setUnite(q.value("UNITE").toString());
    return m;
}

// ---------------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------------
bool StockDatabase::addMaterial(const StockMaterial& mat)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    q.prepare(
        "INSERT INTO MATERIAU "
        "(NOM_MAT, TYPE_MAT, QUANTITE_MAT, PRIX_UNITAIRE, "
        " FOURNISSEUR, SEUIL_ALERTE, LAST_ORDER, CONSO_MENSUELLE, UNITE) "
        "VALUES "
        "(:nom, :type, :qte, :prix, :fourn, :seuil, "
        " TO_DATE(:last_order, 'YYYY-MM-DD'), :conso, :unite)"
        );

    q.bindValue(":nom",        mat.getNom());
    q.bindValue(":type",       mat.getType().isEmpty()        ? QVariant() : QVariant(mat.getType()));
    q.bindValue(":qte",        mat.getQuantite());
    q.bindValue(":prix",       mat.getPrixUnitaire());
    q.bindValue(":fourn",      mat.getFournisseur().isEmpty() ? QVariant() : QVariant(mat.getFournisseur()));
    q.bindValue(":seuil",      mat.getSeuilAlerte());
    q.bindValue(":last_order", mat.getLastOrder().isNull()
                                   ? QDate::currentDate().toString("yyyy-MM-dd")
                                   : mat.getLastOrder().toString("yyyy-MM-dd"));
    q.bindValue(":conso",      mat.getConsoMensuelle());
    q.bindValue(":unite",      mat.getUnite().isEmpty()       ? QVariant() : QVariant(mat.getUnite()));

    if (!q.exec()) {
        qWarning() << "[StockDatabase] addMaterial error:" << q.lastError().text();
        return false;
    }
    return true;
}

bool StockDatabase::updateMaterial(const StockMaterial& mat)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    q.prepare(
        "UPDATE MATERIAU SET "
        "  NOM_MAT=:nom, TYPE_MAT=:type, QUANTITE_MAT=:qte, "
        "  PRIX_UNITAIRE=:prix, FOURNISSEUR=:fourn, "
        "  SEUIL_ALERTE=:seuil, LAST_ORDER=TO_DATE(:last_order,'YYYY-MM-DD'), "
        "  CONSO_MENSUELLE=:conso, UNITE=:unite "   // ← ajout
        "WHERE ID_MAT=:id"
        );

    q.bindValue(":nom",        mat.getNom());
    q.bindValue(":type",       mat.getType());
    q.bindValue(":qte",        mat.getQuantite());
    q.bindValue(":prix",       mat.getPrixUnitaire());
    q.bindValue(":fourn",      mat.getFournisseur());
    q.bindValue(":seuil",      mat.getSeuilAlerte());
    q.bindValue(":last_order", mat.getLastOrder().isNull()
                                   ? QDate::currentDate().toString("yyyy-MM-dd")
                                   : mat.getLastOrder().toString("yyyy-MM-dd"));
    q.bindValue(":conso",      mat.getConsoMensuelle());
    q.bindValue(":id",         mat.getId());  q.bindValue(":unite", mat.getUnite().isEmpty() ? QVariant() : QVariant(mat.getUnite()));

    if (!q.exec()) {
        qWarning() << "[StockDatabase] updateMaterial error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool StockDatabase::deleteMaterial(int id)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("DELETE FROM MATERIAU WHERE ID_MAT=:id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        qWarning() << "[StockDatabase] deleteMaterial error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

StockMaterial StockDatabase::getMaterial(int id) const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM MATERIAU WHERE ID_MAT=:id");
    q.bindValue(":id", id);

    if (q.exec() && q.next())
        return rowToMaterial(q);

    qWarning() << "[StockDatabase] getMaterial(" << id << ") failed:" << q.lastError().text();
    return StockMaterial();
}

QList<StockMaterial> StockDatabase::getAllMaterials() const
{
    QList<StockMaterial> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    if (!q.exec("SELECT * FROM MATERIAU ORDER BY ID_MAT")) {
        qWarning() << "[StockDatabase] getAllMaterials error:" << q.lastError().text();
        return list;
    }

    while (q.next())
        list.append(rowToMaterial(q));

    return list;
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------
QList<StockMaterial> StockDatabase::searchByNom(const QString& nom) const
{
    QList<StockMaterial> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM MATERIAU WHERE UPPER(NOM_MAT) LIKE :p ORDER BY ID_MAT");
    q.bindValue(":p", "%" + nom.toUpper() + "%");
    if (q.exec())
        while (q.next()) list.append(rowToMaterial(q));
    return list;
}

QList<StockMaterial> StockDatabase::searchByType(const QString& type) const
{
    QList<StockMaterial> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM MATERIAU WHERE UPPER(TYPE_MAT) LIKE :p ORDER BY ID_MAT");
    q.bindValue(":p", "%" + type.toUpper() + "%");
    if (q.exec())
        while (q.next()) list.append(rowToMaterial(q));
    return list;
}

QList<StockMaterial> StockDatabase::searchByNomOrType(const QString& text) const
{
    QList<StockMaterial> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    QString p = "%" + text.toUpper() + "%";
    q.prepare(
        "SELECT * FROM MATERIAU "
        "WHERE UPPER(NOM_MAT) LIKE :p OR UPPER(TYPE_MAT) LIKE :p "
        "ORDER BY ID_MAT"
    );
    q.bindValue(":p", p);
    if (q.exec())
        while (q.next()) list.append(rowToMaterial(q));
    return list;
}

// ---------------------------------------------------------------------------
// Sort (in-memory, consistent with EmployeeDatabase approach)
// ---------------------------------------------------------------------------
QList<StockMaterial> StockDatabase::sortByNom(bool ascending) const
{
    QList<StockMaterial> list = getAllMaterials();
    std::sort(list.begin(), list.end(), [ascending](const StockMaterial& a, const StockMaterial& b){
        return ascending ? a.getNom() < b.getNom() : a.getNom() > b.getNom();
    });
    return list;
}

QList<StockMaterial> StockDatabase::sortByConsoMensuelle(bool ascending) const
{
    QList<StockMaterial> list = getAllMaterials();
    std::sort(list.begin(), list.end(), [ascending](const StockMaterial& a, const StockMaterial& b){
        return ascending ? a.getConsoMensuelle() < b.getConsoMensuelle()
                         : a.getConsoMensuelle() > b.getConsoMensuelle();
    });
    return list;
}

// ---------------------------------------------------------------------------
// Stats
// ---------------------------------------------------------------------------
double StockDatabase::getTotalValue() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT SUM(QUANTITE_MAT * PRIX_UNITAIRE) FROM MATERIAU") && q.next())
        return q.value(0).toDouble();
    return 0.0;
}

int StockDatabase::getTotalCount() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT COUNT(*) FROM MATERIAU") && q.next())
        return q.value(0).toInt();
    return 0;
}

int StockDatabase::getAlertCount() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT COUNT(*) FROM MATERIAU WHERE QUANTITE_MAT < SEUIL_ALERTE") && q.next())
        return q.value(0).toInt();
    return 0;
}

double StockDatabase::getAverageRenewalRate() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT AVG(CONSO_MENSUELLE) FROM MATERIAU") && q.next())
        return q.value(0).toDouble();
    return 0.0;
}
