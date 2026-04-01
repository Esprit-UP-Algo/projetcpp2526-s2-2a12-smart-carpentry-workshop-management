#include "projectdatabase.h"
#include "src/database/connection.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <algorithm>

ProjectDatabase& ProjectDatabase::instance()
{
    static ProjectDatabase inst;
    return inst;
}

// ---------------------------------------------------------------------------
// Helper : convertit une ligne SQL en Projet
// ---------------------------------------------------------------------------
Projet ProjectDatabase::rowToProjet(const QSqlQuery& q) const
{
    Projet p;
    p.setId(q.value("ID_PROJET").toInt());
    p.setNom(q.value("NOM_PROJET").toString());
    p.setDateProjet(q.value("DATE_PROJET").toDate());
    p.setDeadline(q.value("DEADLINE").toDate());
    p.setStatus(q.value("STATUS_PROJET").toString());
    p.setBudget(q.value("BUDGET_PROJET").toDouble());
    p.setAdresse(q.value("ADRESSE_CHANTIER").toString());
    p.setClient(q.value("CLIENT").toString());
    p.setType(q.value("TYPE").toString());

    QVariant chef = q.value("CHEF_PROJET");
    p.setChefProjet(chef.isNull() ? 0 : chef.toInt());

    return p;
}

// ---------------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------------
bool ProjectDatabase::addProjet(const Projet& p)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    QString sql =
        "INSERT INTO PROJET "
        "(NOM_PROJET, DATE_PROJET, DEADLINE, STATUS_PROJET, BUDGET_PROJET, "
        " ADRESSE_CHANTIER, CLIENT, TYPE, CHEF_PROJET) "
        "VALUES "
        "(:nom, TO_DATE(:date_proj,'YYYY-MM-DD'), TO_DATE(:deadline,'YYYY-MM-DD'), "
        " :status, :budget, :adresse, :client, :type, ";

    if (p.getChefProjet() == 0)
        sql += "NULL)";
    else
        sql += ":chef)";

    q.prepare(sql);
    q.bindValue(":nom",       p.getNom());
    q.bindValue(":date_proj", p.getDateProjet().toString("yyyy-MM-dd"));
    q.bindValue(":deadline",  p.getDeadline().toString("yyyy-MM-dd"));
    q.bindValue(":status",    p.getStatus());
    q.bindValue(":budget",    p.getBudget());
    q.bindValue(":adresse",   p.getAdresse().isEmpty() ? QVariant() : QVariant(p.getAdresse()));
    q.bindValue(":client",    p.getClient());
    q.bindValue(":type",      p.getType());
    if (p.getChefProjet() != 0)
        q.bindValue(":chef", p.getChefProjet());

    if (!q.exec()) {
        qWarning() << "[ProjectDatabase] addProjet error:" << q.lastError().text();
        return false;
    }
    return true;
}

bool ProjectDatabase::updateProjet(const Projet& p)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    QString sql =
        "UPDATE PROJET SET "
        "  NOM_PROJET=:nom, DATE_PROJET=TO_DATE(:date_proj,'YYYY-MM-DD'), "
        "  DEADLINE=TO_DATE(:deadline,'YYYY-MM-DD'), STATUS_PROJET=:status, "
        "  BUDGET_PROJET=:budget, ADRESSE_CHANTIER=:adresse, CLIENT=:client, TYPE=:type, ";

    if (p.getChefProjet() == 0)
        sql += "CHEF_PROJET=NULL ";
    else
        sql += "CHEF_PROJET=:chef ";

    sql += "WHERE ID_PROJET=:id";

    q.prepare(sql);
    q.bindValue(":nom",       p.getNom());
    q.bindValue(":date_proj", p.getDateProjet().toString("yyyy-MM-dd"));
    q.bindValue(":deadline",  p.getDeadline().toString("yyyy-MM-dd"));
    q.bindValue(":status",    p.getStatus());
    q.bindValue(":budget",    p.getBudget());
    q.bindValue(":adresse",   p.getAdresse().isEmpty() ? QVariant() : QVariant(p.getAdresse()));
    q.bindValue(":client",    p.getClient());
    q.bindValue(":type",      p.getType());
    q.bindValue(":id",        p.getId());
    if (p.getChefProjet() != 0)
        q.bindValue(":chef", p.getChefProjet());

    if (!q.exec()) {
        qWarning() << "[ProjectDatabase] updateProjet error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool ProjectDatabase::deleteProjet(int id)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("DELETE FROM PROJET WHERE ID_PROJET=:id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        qWarning() << "[ProjectDatabase] deleteProjet error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

Projet ProjectDatabase::getProjet(int id) const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM PROJET WHERE ID_PROJET=:id");
    q.bindValue(":id", id);

    if (q.exec() && q.next())
        return rowToProjet(q);

    qWarning() << "[ProjectDatabase] getProjet(" << id << ") failed:" << q.lastError().text();
    return Projet();
}

QList<Projet> ProjectDatabase::getAllProjets() const
{
    QList<Projet> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    if (!q.exec("SELECT * FROM PROJET ORDER BY ID_PROJET")) {
        qWarning() << "[ProjectDatabase] getAllProjets error:" << q.lastError().text();
        return list;
    }
    while (q.next())
        list.append(rowToProjet(q));
    return list;
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------
QList<Projet> ProjectDatabase::searchByNom(const QString& nom) const
{
    QList<Projet> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM PROJET WHERE UPPER(NOM_PROJET) LIKE :p ORDER BY ID_PROJET");
    q.bindValue(":p", "%" + nom.toUpper() + "%");
    if (q.exec())
        while (q.next()) list.append(rowToProjet(q));
    return list;
}

QList<Projet> ProjectDatabase::searchByClient(const QString& client) const
{
    QList<Projet> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM PROJET WHERE UPPER(CLIENT) LIKE :p ORDER BY ID_PROJET");
    q.bindValue(":p", "%" + client.toUpper() + "%");
    if (q.exec())
        while (q.next()) list.append(rowToProjet(q));
    return list;
}

QList<Projet> ProjectDatabase::searchByNomOrClient(const QString& text) const
{
    QList<Projet> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    QString p = "%" + text.toUpper() + "%";
    q.prepare(
        "SELECT * FROM PROJET "
        "WHERE UPPER(NOM_PROJET) LIKE :p OR UPPER(CLIENT) LIKE :p "
        "ORDER BY ID_PROJET"
    );
    q.bindValue(":p", p);
    if (q.exec())
        while (q.next()) list.append(rowToProjet(q));
    return list;
}

// ---------------------------------------------------------------------------
// Sort (en mémoire)
// ---------------------------------------------------------------------------
QList<Projet> ProjectDatabase::sortByDeadline(bool ascending) const
{
    QList<Projet> list = getAllProjets();
    std::sort(list.begin(), list.end(), [ascending](const Projet& a, const Projet& b) {
        return ascending ? a.getDeadline() < b.getDeadline()
                         : a.getDeadline() > b.getDeadline();
    });
    return list;
}

QList<Projet> ProjectDatabase::sortByBudget(bool ascending) const
{
    QList<Projet> list = getAllProjets();
    std::sort(list.begin(), list.end(), [ascending](const Projet& a, const Projet& b) {
        return ascending ? a.getBudget() < b.getBudget()
                         : a.getBudget() > b.getBudget();
    });
    return list;
}

// ---------------------------------------------------------------------------
// Stats
// ---------------------------------------------------------------------------
double ProjectDatabase::getTotalBudget() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT SUM(BUDGET_PROJET) FROM PROJET") && q.next())
        return q.value(0).toDouble();
    return 0.0;
}

int ProjectDatabase::getTotalCount() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT COUNT(*) FROM PROJET") && q.next())
        return q.value(0).toInt();
    return 0;
}

int ProjectDatabase::getCountByStatus(const QString& status) const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM PROJET WHERE STATUS_PROJET=:s");
    q.bindValue(":s", status);
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return 0;
}

double ProjectDatabase::getAverageBudget() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT AVG(BUDGET_PROJET) FROM PROJET") && q.next())
        return q.value(0).toDouble();
    return 0.0;
}

QMap<QString, int> ProjectDatabase::getCountPerStatus() const
{
    QMap<QString, int> map;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT STATUS_PROJET, COUNT(*) FROM PROJET GROUP BY STATUS_PROJET"))
        while (q.next())
            map[q.value(0).toString()] = q.value(1).toInt();
    return map;
}

QMap<QString, int> ProjectDatabase::getCountPerType() const
{
    QMap<QString, int> map;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT TYPE, COUNT(*) FROM PROJET GROUP BY TYPE"))
        while (q.next())
            map[q.value(0).toString()] = q.value(1).toInt();
    return map;
}

int ProjectDatabase::getOverdueCount() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT COUNT(*) FROM PROJET "
               "WHERE DEADLINE < SYSDATE "
               "AND STATUS_PROJET NOT IN ('Terminé','Annulé')") && q.next())
        return q.value(0).toInt();
    return 0;
}

// ---------------------------------------------------------------------------
// FK helpers
// ---------------------------------------------------------------------------
QString ProjectDatabase::getEmployeeName(int idEmp) const
{
    if (idEmp == 0) return QString();
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT NOM_EMP || ' ' || PRENOM_EMP FROM EMPLOYE WHERE ID_EMP=:id");
    q.bindValue(":id", idEmp);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return QString();
}

QMap<int, QString> ProjectDatabase::getAllEmployees() const
{
    QMap<int, QString> map;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT ID_EMP, NOM_EMP || ' ' || PRENOM_EMP FROM EMPLOYE ORDER BY NOM_EMP"))
        while (q.next())
            map[q.value(0).toInt()] = q.value(1).toString();
    return map;
}
