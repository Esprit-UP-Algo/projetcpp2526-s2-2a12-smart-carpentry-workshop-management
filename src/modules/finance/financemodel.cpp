#include "financemodel.h"
#include "src/database/connection.h"
#include <QDebug>
#include <QDate>

FinanceModel::FinanceModel(QObject *parent) : QObject(parent) {}

QSqlDatabase FinanceModel::getDatabase()
{
    return QSqlDatabase::database(Connection::CONN_NAME);
}

QList<FinanceModel::Transaction> FinanceModel::loadTransactions()
{
    QList<Transaction> transactions;
    QSqlDatabase db = getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database not connected!";
        return transactions;
    }

    QSqlQuery query(db);
    query.prepare("SELECT ID_TRAN, TYPE_TRAN, MODE_PAIEMENT, STATUT_TRAN, "
                  "CATEGORIE_TRAN, MONTANT_TRAN, "
                  "TO_CHAR(DATE_TRAN, 'DD/MM/YYYY') as DATE_TRAN "
                  "FROM TRANSACTIONS "
                  "ORDER BY DATE_TRAN DESC");

    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text();
        emit errorOccurred(query.lastError().text());
        return transactions;
    }

    while (query.next()) {
        Transaction t;
        t.id = query.value(0).toString();
        t.type = query.value(1).toString();
        t.modePaiement = query.value(2).toString();
        t.statut = query.value(3).toString();
        t.categorie = query.value(4).toString();
        t.montant = query.value(5).toDouble();
        t.date = query.value(6).toString();
        transactions.append(t);
    }

    return transactions;
}

bool FinanceModel::insertTransaction(const QMap<QString, QString> &data)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("INSERT INTO TRANSACTIONS "
                  "(ID_TRAN, TYPE_TRAN, MODE_PAIEMENT, STATUT_TRAN, "
                  "CATEGORIE_TRAN, MONTANT_TRAN, DATE_TRAN) "
                  "VALUES (SEQ_TRAN.NEXTVAL, :type, :mode, :statut, "
                  ":categorie, :montant, TO_DATE(:date_str, 'DD/MM/YYYY'))");

    query.bindValue(":type",      data["TYPE_TRAN"]);
    query.bindValue(":mode",      data["MODE_PAIEMENT"]);
    query.bindValue(":statut",    data["STATUT_TRAN"]);
    query.bindValue(":categorie", data["CATEGORIE_TRAN"]);
    query.bindValue(":montant",   data["MONTANT_TRAN"].toDouble());
    query.bindValue(":date_str",  data["DATE_TRAN"]);

    if (!query.exec()) {
        qDebug() << "Insert failed:" << query.lastError().text();
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

bool FinanceModel::updateTransaction(const QString &id, const QMap<QString, QString> &data)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("UPDATE TRANSACTIONS SET "
                  "TYPE_TRAN = :type, "
                  "MODE_PAIEMENT = :mode, "
                  "STATUT_TRAN = :statut, "
                  "CATEGORIE_TRAN = :categorie, "
                  "MONTANT_TRAN = :montant, "
                  "DATE_TRAN = TO_DATE(:date_str, 'DD/MM/YYYY') "
                  "WHERE ID_TRAN = :id");

    query.bindValue(":type",      data["TYPE_TRAN"]);
    query.bindValue(":mode",      data["MODE_PAIEMENT"]);
    query.bindValue(":statut",    data["STATUT_TRAN"]);
    query.bindValue(":categorie", data["CATEGORIE_TRAN"]);
    query.bindValue(":montant",   data["MONTANT_TRAN"].toDouble());
    query.bindValue(":date_str",  data["DATE_TRAN"]);
    query.bindValue(":id",        id);

    if (!query.exec()) {
        qDebug() << "Update failed:" << query.lastError().text();
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

bool FinanceModel::deleteTransaction(const QString &id)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    query.prepare("DELETE FROM TRANSACTIONS WHERE ID_TRAN = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

FinanceModel::FinanceStats FinanceModel::getStatistics()
{
    FinanceStats stats;
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) return stats;

    QSqlQuery query(db);
    query.prepare("SELECT STATUT_TRAN, MODE_PAIEMENT, CATEGORIE_TRAN, MONTANT_TRAN, "
                  "TO_CHAR(DATE_TRAN,'MM/YYYY') as MOIS "
                  "FROM TRANSACTIONS");

    if (!query.exec()) return stats;

    while (query.next()) {
        QString statut    = query.value(0).toString();
        QString mode      = query.value(1).toString();
        QString categorie = query.value(2).toString();
        double  montant   = query.value(3).toDouble();
        QString mois      = query.value(4).toString();

        stats.statsByStatus[statut] += montant;
        stats.statsByMode[mode] += montant;
        stats.statsByCategorie[categorie] += montant;
        stats.statsByMonth[mois] += montant;

        if (categorie == "Recette") {
            stats.totalRecettes += montant;
        } else if (categorie == "Depense") {
            stats.totalDepenses += montant;
        }

        stats.totalGeneral += montant;
        stats.totalTransactions++;
    }

    return stats;
}
