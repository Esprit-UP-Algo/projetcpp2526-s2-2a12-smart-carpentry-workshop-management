#include "produitdatabase.h"
#include "connection.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

ProduitDatabase& ProduitDatabase::instance()
{
    static ProduitDatabase inst;
    return inst;
}

// 🔁 Conversion SQL → Objet
Produit ProduitDatabase::rowToProduit(const QSqlQuery& q) const
{
    Produit p;
    p.setId(QString::number(q.value("ID_PROD").toInt())); // ✅ NUMBER → QString
    p.setNom(q.value("NOM_PROD").toString());
    p.setCategorie(q.value("CATEGORIE_PROD").toString());
    p.setPrix(q.value("PRIX_PROD").toDouble());
    p.setDimensions(q.value("DIMENSIONS").toString());
    p.setMat(q.value("MAT_UTILISE").toString());

    QVariant dateVal = q.value("DATE_CREATION");
    if (!dateVal.isNull())
        p.setDate(dateVal.toDate());

    p.setImage(q.value("IMAGE").toString());

    return p;
}

// ➕ AJOUT
bool ProduitDatabase::addProduit(const Produit& p)
{
    QSqlDatabase db = QSqlDatabase::database("oracle_conn");

    if(!db.isOpen())
    {
        qDebug() << "❌ DB NON OUVERTE (ADD)";
        return false;
    }

    QSqlQuery q(QSqlDatabase::database(Connection::CONN_NAME));

    q.prepare(
        "INSERT INTO PRODUIT "
        "(ID_PROD, NOM_PROD, CATEGORIE_PROD, PRIX_PROD, DIMENSIONS, MAT_UTILISE, DATE_CREATION, IMAGE) "
        "VALUES "
        "(:id, :nom, :cat, :prix, :dim, :mat, TO_DATE(:date,'YYYY-MM-DD'), :img)"
        );

    q.bindValue(":id", p.getId().toInt()); // ✅ IMPORTANT
    q.bindValue(":nom", p.getNom());
    q.bindValue(":cat", p.getCategorie());
    q.bindValue(":prix", p.getPrix());
    q.bindValue(":dim", p.getDimensions());
    q.bindValue(":mat", p.getMat());

    q.bindValue(":date", p.getDate().isNull()
                             ? QDate::currentDate().toString("yyyy-MM-dd")
                             : p.getDate().toString("yyyy-MM-dd"));

    q.bindValue(":img", p.getImage());

    if (!q.exec()) {
        qDebug() << "ERREUR SQL ADD :" << q.lastError().text();
        return false;
    }
    return true;
}

// ✏️ MODIFIER
bool ProduitDatabase::updateProduit(const Produit& p)
{
    QSqlDatabase db = QSqlDatabase::database("oracle_conn");

    if(!db.isOpen())
    {
        qDebug() << "❌ DB NON OUVERTE (UPDATE)";
        return false;
    }

    QSqlQuery q(QSqlDatabase::database(Connection::CONN_NAME));

    q.prepare(
        "UPDATE PRODUIT SET "
        "NOM_PROD=:nom, "
        "CATEGORIE_PROD=:cat, "
        "PRIX_PROD=:prix, "
        "DIMENSIONS=:dim, "
        "MAT_UTILISE=:mat, "
        "DATE_CREATION=TO_DATE(:date,'YYYY-MM-DD'), "
        "IMAGE=:img "
        "WHERE ID_PROD=:id"
        );

    q.bindValue(":id", p.getId().toInt()); // ✅ IMPORTANT
    q.bindValue(":nom", p.getNom());
    q.bindValue(":cat", p.getCategorie());
    q.bindValue(":prix", p.getPrix());
    q.bindValue(":dim", p.getDimensions());
    q.bindValue(":mat", p.getMat());
    q.bindValue(":date", p.getDate().toString("yyyy-MM-dd"));
    q.bindValue(":img", p.getImage());

    if (!q.exec()) {
        qDebug() << "Erreur modification:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

// ❌ SUPPRIMER
bool ProduitDatabase::deleteProduit(const QString& id)
{
    QSqlDatabase db = QSqlDatabase::database("oracle_conn");

    if(!db.isOpen())
    {
        qDebug() << "❌ DB NON OUVERTE (DELETE)";
        return false;
    }

    QSqlQuery q(QSqlDatabase::database(Connection::CONN_NAME));

    q.prepare("DELETE FROM PRODUIT WHERE ID_PROD=:id");
    q.bindValue(":id", id.toInt()); // ✅ IMPORTANT

    if (!q.exec()) {
        qDebug() << "Erreur suppression:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

// 📄 GET ONE
Produit ProduitDatabase::getProduit(const QString& id) const
{
    QSqlQuery q(QSqlDatabase::database(Connection::CONN_NAME));

    q.prepare("SELECT * FROM PRODUIT WHERE ID_PROD=:id");
    q.bindValue(":id", id.toInt()); // ✅ IMPORTANT

    if (q.exec() && q.next())
        return rowToProduit(q);

    return Produit();
}

// 📄 GET ALL
QList<Produit> ProduitDatabase::getAllProduits() const
{
    QList<Produit> list;
    QSqlQuery q(QSqlDatabase::database(Connection::CONN_NAME));

    if (q.exec("SELECT * FROM PRODUIT")) {
        while (q.next())
            list.append(rowToProduit(q));
    }

    return list;
}
