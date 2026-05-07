#include "produitdatabase.h"
#include "connection.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QVariant>
#include <QDebug>

// ─── Helper interne : retourne la connexion ouverte "oracle_conn" ─────────────
static QSqlDatabase getDB()
{
    QSqlDatabase db = QSqlDatabase::database("oracle_conn");
    if (!db.isOpen())
        qWarning() << "[ProduitDatabase] Connexion 'oracle_conn' non ouverte !";
    return db;
}

// Singleton
ProduitDatabase& ProduitDatabase::instance()
{
    static ProduitDatabase inst;
    return inst;
}

// Convertir ligne → Produit
Produit ProduitDatabase::rowToProduit(const QSqlQuery& q) const
{
    Produit p;
    p.setId(q.value("ID_PROD").toString());
    p.setNom(q.value("NOM_PROD").toString());
    p.setCategorie(q.value("CATEGORIE_PROD").toString());
    p.setPrix(q.value("PRIX_PROD").toDouble());
    p.setDimensions(q.value("DIMENSIONS").toString());
    p.setMatUtilise(q.value("MAT_UTILISE").toString());
    p.setProjetId(q.value("PRODUIT_PROJET").toString());

    if (!q.value("DATE_CREATION").isNull())
        p.setDateCreation(q.value("DATE_CREATION").toDate());

    if (!q.value("IMAGE").isNull())
        p.setImage(q.value("IMAGE").toString().toUtf8());

    return p;
}

// ADD
bool ProduitDatabase::addProduit(const Produit& p)
{
    QSqlQuery q(getDB());   // ← connexion nommée
    q.prepare("INSERT INTO PRODUIT (NOM_PROD, CATEGORIE_PROD, PRIX_PROD, DIMENSIONS, MAT_UTILISE, DATE_CREATION, IMAGE, PRODUIT_PROJET) "
              "VALUES (:nom, :cat, :prix, :dim, :mat, :date, :img, :proj)");

    q.bindValue(":nom",  p.getNom());
    q.bindValue(":cat",  p.getCategorie());
    q.bindValue(":prix", p.getPrix());
    q.bindValue(":dim",  p.getDimensions());
    q.bindValue(":mat",  p.getMatUtilise());
    q.bindValue(":date", p.getDateCreation());
    q.bindValue(":img",  QString::fromUtf8(p.getImage()));
    q.bindValue(":proj", p.getProjetId().toInt());  // PRODUIT_PROJET est NUMBER

    if (!q.exec()) {
        qDebug() << "[addProduit] ERREUR :" << q.lastError().text();
        return false;
    }
    return true;
}

// UPDATE
bool ProduitDatabase::updateProduit(const Produit& p)
{
    QSqlQuery q(getDB());   // ← connexion nommée
    q.prepare("UPDATE PRODUIT SET NOM_PROD=:nom, CATEGORIE_PROD=:cat, PRIX_PROD=:prix, "
              "DIMENSIONS=:dim, MAT_UTILISE=:mat, DATE_CREATION=:date, IMAGE=:img, PRODUIT_PROJET=:proj "
              "WHERE ID_PROD=:id");

    q.bindValue(":id",   p.getId().toInt());        // ID_PROD est NUMBER
    q.bindValue(":nom",  p.getNom());
    q.bindValue(":cat",  p.getCategorie());
    q.bindValue(":prix", p.getPrix());
    q.bindValue(":dim",  p.getDimensions());
    q.bindValue(":mat",  p.getMatUtilise());
    q.bindValue(":date", p.getDateCreation());
    q.bindValue(":img",  QString::fromUtf8(p.getImage()));
    q.bindValue(":proj", p.getProjetId().toInt());  // PRODUIT_PROJET est NUMBER

    if (!q.exec()) {
        qDebug() << "[updateProduit] ERREUR :" << q.lastError().text();
        return false;
    }
    return true;
}

// DELETE
bool ProduitDatabase::deleteProduit(const QString& id)
{
    QSqlQuery q(getDB());   // ← connexion nommée
    q.prepare("DELETE FROM PRODUIT WHERE ID_PROD=:id");
    q.bindValue(":id", id.toInt());                 // ID_PROD est NUMBER

    if (!q.exec()) {
        qDebug() << "[deleteProduit] ERREUR :" << q.lastError().text();
        return false;
    }
    return true;
}

// GET ONE
Produit ProduitDatabase::getProduit(const QString& id) const
{
    QSqlQuery q(getDB());   // ← connexion nommée
    q.prepare("SELECT * FROM PRODUIT WHERE ID_PROD=:id");
    q.bindValue(":id", id.toInt());

    if (q.exec() && q.next())
        return rowToProduit(q);

    return Produit();
}

// GET ALL
QList<Produit> ProduitDatabase::getAllProduits() const
{
    QList<Produit> list;
    QSqlQuery q(getDB());   // ← connexion nommée
    q.exec("SELECT * FROM PRODUIT");

    while (q.next())
        list.append(rowToProduit(q));

    return list;
}
