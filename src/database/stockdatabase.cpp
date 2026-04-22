#include "stockdatabase.h"
#include "connection.h"

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
// Helper : convertit une ligne SQL en StockMaterial
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

    QVariant idProd = q.value("ID_PRODUIT");
    m.setIdProduit(idProd.isNull() ? 0 : idProd.toInt());

    m.setLocale(q.value("LOCALE").toString());
    m.setEmplacement(q.value("EMPLACEMENT").toString());

    return m;
}

// ---------------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------------
bool StockDatabase::addMaterial(const StockMaterial& mat)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    QString sql =
        "INSERT INTO MATERIAU "
        "(NOM_MAT, TYPE_MAT, QUANTITE_MAT, PRIX_UNITAIRE, "
        " FOURNISSEUR, SEUIL_ALERTE, LAST_ORDER, CONSO_MENSUELLE, UNITE, ID_PRODUIT, "
        " LOCALE, EMPLACEMENT) "
        "VALUES "
        "(:nom, :type, :qte, :prix, :fourn, :seuil, "
        " TO_DATE(:last_order, 'YYYY-MM-DD'), :conso, :unite, ";

    if (mat.getIdProduit() == 0)
        sql += "NULL";
    else
        sql += ":id_produit";

    sql += ", :locale, :emplacement)";

    q.prepare(sql);

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
    q.bindValue(":locale",     mat.getLocale().isEmpty()      ? QVariant() : QVariant(mat.getLocale()));
    q.bindValue(":emplacement",mat.getEmplacement().isEmpty() ? QVariant() : QVariant(mat.getEmplacement()));

    if (mat.getIdProduit() != 0)
        q.bindValue(":id_produit", mat.getIdProduit());

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

    QString sql =
        "UPDATE MATERIAU SET "
        "  NOM_MAT=:nom, TYPE_MAT=:type, QUANTITE_MAT=:qte, "
        "  PRIX_UNITAIRE=:prix, FOURNISSEUR=:fourn, "
        "  SEUIL_ALERTE=:seuil, LAST_ORDER=TO_DATE(:last_order,'YYYY-MM-DD'), "
        "  CONSO_MENSUELLE=:conso, UNITE=:unite, "
        "  LOCALE=:locale, EMPLACEMENT=:emplacement, ";

    if (mat.getIdProduit() == 0)
        sql += "ID_PRODUIT = NULL ";
    else
        sql += "ID_PRODUIT = :id_produit ";

    sql += "WHERE ID_MAT=:id";

    q.prepare(sql);

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
    q.bindValue(":locale",     mat.getLocale().isEmpty()      ? QVariant() : QVariant(mat.getLocale()));
    q.bindValue(":emplacement",mat.getEmplacement().isEmpty() ? QVariant() : QVariant(mat.getEmplacement()));
    q.bindValue(":id",         mat.getId());

    if (mat.getIdProduit() != 0)
        q.bindValue(":id_produit", mat.getIdProduit());

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
// Recherche par produit associé (FK)
// ---------------------------------------------------------------------------
QList<StockMaterial> StockDatabase::getMaterialsByProduit(int idProduit) const
{
    QList<StockMaterial> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM MATERIAU WHERE ID_PRODUIT = :id ORDER BY ID_MAT");
    q.bindValue(":id", idProduit);

    if (q.exec())
        while (q.next()) list.append(rowToMaterial(q));
    else
        qWarning() << "[StockDatabase] getMaterialsByProduit error:" << q.lastError().text();

    return list;
}

// ---------------------------------------------------------------------------
// Recherche par locale (pour la carte)
// ---------------------------------------------------------------------------
QList<StockMaterial> StockDatabase::getMaterialsByLocale(const QString& locale) const
{
    QList<StockMaterial> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM MATERIAU WHERE LOCALE = :locale ORDER BY EMPLACEMENT, NOM_MAT");
    q.bindValue(":locale", locale);

    if (q.exec())
        while (q.next()) list.append(rowToMaterial(q));
    else
        qWarning() << "[StockDatabase] getMaterialsByLocale error:" << q.lastError().text();

    return list;
}

// ---------------------------------------------------------------------------
// Compte de matériaux par locale (optimisé pour la carte — pas de chargement complet)
// ---------------------------------------------------------------------------
QMap<QString, int> StockDatabase::getCountByLocale() const
{
    QMap<QString, int> result;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    if (q.exec("SELECT LOCALE, COUNT(*) as CNT FROM MATERIAU WHERE LOCALE IS NOT NULL GROUP BY LOCALE")) {
        while (q.next())
            result[q.value("LOCALE").toString()] = q.value("CNT").toInt();
    } else {
        qWarning() << "[StockDatabase] getCountByLocale error:" << q.lastError().text();
    }
    return result;
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
// Sort (en mémoire)
// ---------------------------------------------------------------------------
QList<StockMaterial> StockDatabase::sortByNom(bool ascending) const
{
    QList<StockMaterial> list = getAllMaterials();
    std::sort(list.begin(), list.end(), [ascending](const StockMaterial& a, const StockMaterial& b) {
        return ascending ? a.getNom() < b.getNom() : a.getNom() > b.getNom();
    });
    return list;
}

QList<StockMaterial> StockDatabase::sortByConsoMensuelle(bool ascending) const
{
    QList<StockMaterial> list = getAllMaterials();
    std::sort(list.begin(), list.end(), [ascending](const StockMaterial& a, const StockMaterial& b) {
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
