#include "connection.h"
#include <QDebug>

static constexpr char DB_USER[]     = "CPP_PROJECT";
static constexpr char DB_PASS[]     = "Eoseos69";
static constexpr char DB_HOST[]     = "localhost";
static constexpr int  DB_PORT       = 1521;
static constexpr char DB_SID[]      = "XE";
static constexpr char DB_ODBC_DSN[] = "CPP_PROJECT_WS";

static bool tryOCI()
{
    if (!QSqlDatabase::isDriverAvailable("QOCI")) {
        qDebug() << "[Connection] QOCI not available.";
        return false;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QOCI", Connection::CONN_NAME);
    db.setHostName(DB_HOST);
    db.setPort(DB_PORT);
    db.setDatabaseName(DB_SID);
    db.setUserName(DB_USER);
    db.setPassword(DB_PASS);
    if (db.open()) {
        qDebug() << "[Connection] Connected via QOCI.";
        return true;
    }
    qDebug() << "[Connection] QOCI failed:" << db.lastError().text();
    QSqlDatabase::removeDatabase(Connection::CONN_NAME);
    return false;
}

static bool tryODBC()
{
    if (!QSqlDatabase::isDriverAvailable("QODBC")) {
        qDebug() << "[Connection] QODBC not available.";
        return false;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", Connection::CONN_NAME);
    db.setDatabaseName(DB_ODBC_DSN);
    db.setUserName(DB_USER);
    db.setPassword(DB_PASS);
    if (db.open()) {
        qDebug() << "[Connection] Connected via QODBC.";
        return true;
    }
    qDebug() << "[Connection] QODBC failed:" << db.lastError().text();
    QSqlDatabase::removeDatabase(Connection::CONN_NAME);
    return false;
}

static bool tryODBC_ELA()
{
    if (!QSqlDatabase::isDriverAvailable("QODBC")) {
        qDebug() << "[Connection] QODBC not available.";
        return false;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", Connection::CONN_NAME);
    db.setDatabaseName(
        "Driver={Oracle in XE};"
        "DBQ=localhost:1522/XE;"
        "Uid=CPP_PROJECT;"
        "Pwd=Eoseos69;"
        );

    if (db.open()) {
        qDebug() << "[Connection] Connected via QODBC.";
        return true;
    }
    qDebug() << "[Connection] QODBC failed:" << db.lastError().text();
    QSqlDatabase::removeDatabase(Connection::CONN_NAME);
    return false;
}

bool Connection::createconnect()
{
    qDebug() << "[Connection] Available Qt SQL drivers:" << QSqlDatabase::drivers();
    if (tryOCI())  return true;
    if (tryODBC()) return true;
    if (tryODBC_ELA()) return true;
    qDebug() << "[Connection] All drivers failed.";
    return false;
}
