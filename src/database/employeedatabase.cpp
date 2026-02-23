#include "employeedatabase.h"
#include "connection.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDebug>
#include <algorithm>

EmployeeDatabase& EmployeeDatabase::instance()
{
    static EmployeeDatabase inst;
    return inst;
}

Employee EmployeeDatabase::rowToEmployee(const QSqlQuery& q) const
{
    Employee e;
    e.setId(q.value("ID_EMP").toString());
    e.setCin(q.value("CIN").toString());
    e.setNom(q.value("NOM_EMP").toString());
    e.setPrenom(q.value("PRENOM_EMP").toString());
    e.setPoste(q.value("POST_EMP").toString());
    e.setEmail(q.value("EMAIL_EMP").toString());
    e.setTelephone(q.value("NUM_TEL").toString());

    QVariant dateVal = q.value("DATE_EMBAUCHE");
    if (!dateVal.isNull())
        e.setDateEmbauche(dateVal.toDateTime());

    e.setSalaire(q.value("SALAIRE").toDouble());

    QString comp = q.value("COMPETENCES").toString();
    if (!comp.isEmpty()) {
        QStringList list = comp.split(',', Qt::SkipEmptyParts);
        for (QString& s : list) s = s.trimmed();
        e.setCompetences(list);
    }

    e.setDisponibilite(q.value("DISPO_EMP").toString());
    e.setPerformance(q.value("PERFORMANCE").toDouble());
    e.setNbJoursConges(q.value("NJC").toInt());
    e.setNbJoursAbsence(q.value("NJA").toInt());
    e.setHeuresTravail(q.value("HDT").toDouble());
    e.setMotDePasse(q.value("MOT_DE_PASSE").toString());
    return e;
}

// ---------------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------------
bool EmployeeDatabase::addEmployee(const Employee& employee)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    q.prepare(
        "INSERT INTO EMPLOYE "
        "(ID_EMP, CIN, NOM_EMP, PRENOM_EMP, POST_EMP, EMAIL_EMP, NUM_TEL, "
        " DATE_EMBAUCHE, SALAIRE, COMPETENCES, DISPO_EMP, PERFORMANCE, NJC, NJA, HDT, MOT_DE_PASSE) "
        "VALUES "
        "((SELECT NVL(MAX(ID_EMP),0)+1 FROM EMPLOYE), "
        " :cin, :nom, :prenom, :poste, :email, :tel, "
        " TO_DATE(:date_emb,'YYYY-MM-DD'), :salaire, :competences, "
        " :dispo, :perf, :njc, :nja, :hdt, :pwd)"
    );

    q.bindValue(":cin",        employee.getCin());
    q.bindValue(":nom",        employee.getNom());
    q.bindValue(":prenom",     employee.getPrenom());
    q.bindValue(":poste",      employee.getPoste().isEmpty()           ? QVariant() : QVariant(employee.getPoste()));
    q.bindValue(":email",      employee.getEmail().isEmpty()           ? QVariant() : QVariant(employee.getEmail()));
    q.bindValue(":tel",        employee.getTelephone().isEmpty()       ? QVariant() : QVariant(employee.getTelephone()));
    q.bindValue(":date_emb",   employee.getDateEmbauche().isNull()
                                   ? QDate::currentDate().toString("yyyy-MM-dd")
                                   : employee.getDateEmbauche().toString("yyyy-MM-dd"));
    q.bindValue(":salaire",    employee.getSalaire());
    q.bindValue(":competences",employee.getCompetencesString().isEmpty() ? QVariant() : QVariant(employee.getCompetencesString()));
    q.bindValue(":dispo",      employee.getDisponibilite().isEmpty()   ? QVariant() : QVariant(employee.getDisponibilite()));
    q.bindValue(":perf",       employee.getPerformance());
    q.bindValue(":njc",        employee.getNbJoursConges());
    q.bindValue(":nja",        employee.getNbJoursAbsence());
    q.bindValue(":hdt",        employee.getHeuresTravail());
    q.bindValue(":pwd",        employee.getMotDePasse().isEmpty() ? "test123" : employee.getMotDePasse());

    if (!q.exec()) {
        qWarning() << "[EmployeeDatabase] addEmployee error:" << q.lastError().text();
        return false;
    }
    return true;
}

bool EmployeeDatabase::updateEmployee(const Employee& employee)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    q.prepare(
        "UPDATE EMPLOYE SET "
        "  CIN=:cin, NOM_EMP=:nom, PRENOM_EMP=:prenom, POST_EMP=:poste, "
        "  EMAIL_EMP=:email, NUM_TEL=:tel, "
        "  DATE_EMBAUCHE=TO_DATE(:date_emb,'YYYY-MM-DD'), SALAIRE=:salaire, "
        "  COMPETENCES=:competences, DISPO_EMP=:dispo, PERFORMANCE=:perf, "
        "  NJC=:njc, NJA=:nja, HDT=:hdt, MOT_DE_PASSE=:pwd "
        "WHERE ID_EMP=:id"
    );

    q.bindValue(":cin",        employee.getCin());
    q.bindValue(":nom",        employee.getNom());
    q.bindValue(":prenom",     employee.getPrenom());
    q.bindValue(":poste",      employee.getPoste());
    q.bindValue(":email",      employee.getEmail());
    q.bindValue(":tel",        employee.getTelephone());
    q.bindValue(":date_emb",   employee.getDateEmbauche().isNull()
                                   ? QDate::currentDate().toString("yyyy-MM-dd")
                                   : employee.getDateEmbauche().toString("yyyy-MM-dd"));
    q.bindValue(":salaire",    employee.getSalaire());
    q.bindValue(":competences",employee.getCompetencesString());
    q.bindValue(":dispo",      employee.getDisponibilite());
    q.bindValue(":perf",       employee.getPerformance());
    q.bindValue(":njc",        employee.getNbJoursConges());
    q.bindValue(":nja",        employee.getNbJoursAbsence());
    q.bindValue(":hdt",        employee.getHeuresTravail());
    q.bindValue(":pwd",        employee.getMotDePasse().isEmpty() ? "test123" : employee.getMotDePasse());
    q.bindValue(":id",         employee.getId());

    if (!q.exec()) {
        qWarning() << "[EmployeeDatabase] updateEmployee error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool EmployeeDatabase::deleteEmployee(const QString& id)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("DELETE FROM EMPLOYE WHERE ID_EMP=:id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        qWarning() << "[EmployeeDatabase] deleteEmployee error:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

Employee EmployeeDatabase::getEmployee(const QString& id) const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM EMPLOYE WHERE ID_EMP=:id");
    q.bindValue(":id", id);

    if (q.exec() && q.next())
        return rowToEmployee(q);

    qWarning() << "[EmployeeDatabase] getEmployee(" << id << ") failed:" << q.lastError().text();
    return Employee();
}

QList<Employee> EmployeeDatabase::getAllEmployees() const
{
    QList<Employee> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);

    if (!q.exec("SELECT * FROM EMPLOYE ORDER BY ID_EMP")) {
        qWarning() << "[EmployeeDatabase] getAllEmployees error:" << q.lastError().text();
        return list;
    }

    while (q.next())
        list.append(rowToEmployee(q));

    return list;
}

// ---------------------------------------------------------------------------
// Authentication — plain text comparison
// ---------------------------------------------------------------------------
Employee EmployeeDatabase::authenticate(const QString& cin, const QString& password) const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM EMPLOYE WHERE CIN=:cin AND MOT_DE_PASSE=:pwd");
    q.bindValue(":cin", cin.trimmed());
    q.bindValue(":pwd", password);   // plain text, no hashing

    if (q.exec() && q.next())
        return rowToEmployee(q);

    return Employee();
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------
QList<Employee> EmployeeDatabase::searchByName(const QString& name) const
{
    QList<Employee> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    QString p = "%" + name.toUpper() + "%";
    q.prepare(
        "SELECT * FROM EMPLOYE "
        "WHERE UPPER(NOM_EMP) LIKE :p OR UPPER(PRENOM_EMP) LIKE :p "
        "   OR UPPER(NOM_EMP||' '||PRENOM_EMP) LIKE :p "
        "   OR UPPER(PRENOM_EMP||' '||NOM_EMP) LIKE :p"
    );
    q.bindValue(":p", p);
    if (q.exec())
        while (q.next()) list.append(rowToEmployee(q));
    return list;
}

QList<Employee> EmployeeDatabase::searchByCin(const QString& cin) const
{
    QList<Employee> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM EMPLOYE WHERE CIN LIKE :p");
    q.bindValue(":p", "%" + cin + "%");
    if (q.exec())
        while (q.next()) list.append(rowToEmployee(q));
    return list;
}

QList<Employee> EmployeeDatabase::searchByPoste(const QString& poste) const
{
    QList<Employee> list;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM EMPLOYE WHERE UPPER(POST_EMP) LIKE :p");
    q.bindValue(":p", "%" + poste.toUpper() + "%");
    if (q.exec())
        while (q.next()) list.append(rowToEmployee(q));
    return list;
}

// ---------------------------------------------------------------------------
// Sort
// ---------------------------------------------------------------------------
QList<Employee> EmployeeDatabase::sortBySalaire(bool ascending) const
{
    QList<Employee> list = getAllEmployees();
    std::sort(list.begin(), list.end(), [ascending](const Employee& a, const Employee& b){
        return ascending ? a.getSalaire() < b.getSalaire() : a.getSalaire() > b.getSalaire();
    });
    return list;
}

QList<Employee> EmployeeDatabase::sortByDateEmbauche(bool ascending) const
{
    QList<Employee> list = getAllEmployees();
    std::sort(list.begin(), list.end(), [ascending](const Employee& a, const Employee& b){
        return ascending ? a.getDateEmbauche() < b.getDateEmbauche()
                         : a.getDateEmbauche() > b.getDateEmbauche();
    });
    return list;
}

QList<Employee> EmployeeDatabase::sortByPerformance(bool ascending) const
{
    QList<Employee> list = getAllEmployees();
    std::sort(list.begin(), list.end(), [ascending](const Employee& a, const Employee& b){
        return ascending ? a.getPerformance() < b.getPerformance()
                         : a.getPerformance() > b.getPerformance();
    });
    return list;
}

// ---------------------------------------------------------------------------
// Statistics
// ---------------------------------------------------------------------------
double EmployeeDatabase::getAverageSalary() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT AVG(SALAIRE) FROM EMPLOYE") && q.next())
        return q.value(0).toDouble();
    return 0.0;
}

double EmployeeDatabase::getAveragePerformance() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT AVG(PERFORMANCE) FROM EMPLOYE") && q.next())
        return q.value(0).toDouble();
    return 0.0;
}

int EmployeeDatabase::getTotalEmployees() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT COUNT(*) FROM EMPLOYE") && q.next())
        return q.value(0).toInt();
    return 0;
}

QMap<QString, int> EmployeeDatabase::getEmployeeCountByPoste() const
{
    QMap<QString, int> map;
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT POST_EMP, COUNT(*) FROM EMPLOYE GROUP BY POST_EMP"))
        while (q.next())
            map[q.value(0).toString()] = q.value(1).toInt();
    return map;
}

QString EmployeeDatabase::generateNextId() const
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT NVL(MAX(ID_EMP),0)+1 FROM EMPLOYE") && q.next())
        return QString("EMP%1").arg(q.value(0).toInt(), 4, 10, QChar('0'));
    return "EMP0001";
}
