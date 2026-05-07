#include "connection.h"
#include <QDebug>
#include <QSerialPortInfo>
#include <QThread>

static constexpr char DB_USER[]     = "CPP_PROJECT";
static constexpr char DB_PASS[]     = "Eoseos69";
static constexpr char DB_HOST[]     = "localhost";
static constexpr int  DB_PORT       = 1522;
static constexpr char DB_SID[]      = "XE";
static constexpr char DB_ODBC_DSN[] = "CPP_PROJECT_WS";

// ═════════════════════════════════════════════════════════════════════════════
bool Connection::createconnect()
{
    qDebug() << "[DB] Drivers Qt disponibles :" << QSqlDatabase::drivers();

    if (QSqlDatabase::isDriverAvailable("QOCI")) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QOCI", CONN_NAME);
        db.setHostName(DB_HOST); db.setPort(DB_PORT);
        db.setDatabaseName(DB_SID);
        db.setUserName(DB_USER); db.setPassword(DB_PASS);
        if (db.open()) { qDebug() << "[DB] Connecte via QOCI"; return true; }
        qDebug() << "[DB] QOCI echoue :" << db.lastError().text();
        QSqlDatabase::removeDatabase(CONN_NAME);
    }

    if (QSqlDatabase::isDriverAvailable("QODBC")) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", CONN_NAME);
        db.setDatabaseName(DB_ODBC_DSN);
        db.setUserName(DB_USER); db.setPassword(DB_PASS);
        if (db.open()) { qDebug() << "[DB] Connecte via QODBC"; return true; }
        qDebug() << "[DB] QODBC echoue :" << db.lastError().text();
        QSqlDatabase::removeDatabase(CONN_NAME);
    }

    qDebug() << "[DB] Aucun driver n'a fonctionne.";
    return false;
}

// ═════════════════════════════════════════════════════════════════════════════
bool Connection::startBridge(const QString& portName, int baud)
{
    qDebug() << "[Serial] Ports disponibles :";
    for (const QSerialPortInfo& p : QSerialPortInfo::availablePorts())
        qDebug() << "  " << p.portName() << "-" << p.description();

    m_serial = new QSerialPort(this);
    m_serial->setPortName(portName);
    m_serial->setBaudRate(baud);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        qWarning() << "[Serial] Impossible d'ouvrir" << portName
                   << ":" << m_serial->errorString();
        delete m_serial;
        m_serial = nullptr;
        return false;
    }

    qDebug() << "[Serial] Port ouvert :" << portName;

    // Attendre 2s que l'Arduino démarre (il reset à l'ouverture du port)
    QThread::msleep(2000);
    m_serial->clear();
    qDebug() << "[Serial] Arduino pret.";

    // QTimer : appelle tick() toutes les 100ms
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Connection::tick);
    m_timer->start(100);

    qDebug() << "[Serial] Bridge demarre (polling 100ms).";
    return true;
}

// ═════════════════════════════════════════════════════════════════════════════
//  tick() — appelé toutes les 100ms — lit le port et traite les lignes
// ═════════════════════════════════════════════════════════════════════════════
void Connection::tick()
{
    if (!m_serial || !m_serial->isOpen()) return;

    // Lire tout ce qui est disponible
    if (m_serial->bytesAvailable() > 0)
        m_rxBuf += m_serial->readAll();

    // Traiter chaque ligne complète
    while (true) {
        int idx = m_rxBuf.indexOf('\n');
        if (idx < 0) break;

        QString line = QString::fromUtf8(m_rxBuf.left(idx)).trimmed();
        m_rxBuf.remove(0, idx + 1);

        if (line.isEmpty()) continue;

        qDebug() << "[Serial] Arduino -> PC :" << line;

        if (line.startsWith("SC", Qt::CaseInsensitive)) {
            QString pin      = line.mid(2).trimmed();
            QString response = searchByPin(pin);
            qDebug() << "[Serial] PC -> Arduino :" << response;
            serialSend(response);
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
QString Connection::searchByPin(const QString& pin)
{
    if (pin.length() != 6) {
        qWarning() << "[DB] PIN invalide :" << pin;
        return "ERR:PIN invalide";
    }

    QSqlDatabase db = QSqlDatabase::database(CONN_NAME);
    if (!db.isOpen()) {
        qWarning() << "[DB] Connexion fermee !";
        return "ERR:DB deconnectee";
    }

    QSqlQuery q(db);
    q.prepare(
        "SELECT PRENOM_EMP, NOM_EMP, POST_EMP "
        "FROM   EMPLOYE "
        "WHERE  TRIM(PIN_CODE) = :pin "
        "  AND  PIN_EXPIRES   >= SYSDATE "
        "  AND  ROWNUM         = 1"
        );
    q.bindValue(":pin", pin);

    if (!q.exec()) {
        qWarning() << "[DB] Requete echouee :" << q.lastError().text();
        return "ERR:Erreur DB";
    }

    if (!q.next()) {
        // PIN existe mais expiré ?
        QSqlQuery q2(db);
        q2.prepare("SELECT TO_CHAR(PIN_EXPIRES,'YYYY-MM-DD') "
                   "FROM EMPLOYE WHERE TRIM(PIN_CODE) = :pin");
        q2.bindValue(":pin", pin);
        if (q2.exec() && q2.next())
            qWarning() << "[DB] PIN expire le :" << q2.value(0).toString();
        else
            qWarning() << "[DB] Aucun employe pour PIN :" << pin;
        return "ERR:PIN inconnu";
    }

    QString name  = (q.value("PRENOM_EMP").toString().trimmed()
                    + " " + q.value("NOM_EMP").toString().trimmed()).left(14);
    QString poste = q.value("POST_EMP").toString().trimmed().left(10);

    qDebug() << "[DB] Employe trouve :" << name << "|" << poste;
    return QString("OK:%1|%2").arg(name, poste);
}

// ═════════════════════════════════════════════════════════════════════════════
void Connection::serialSend(const QString& msg)
{
    if (!m_serial || !m_serial->isOpen()) return;
    QByteArray data = (msg.left(32) + "\n").toUtf8();
    m_serial->write(data);
    m_serial->flush();
}

// ═════════════════════════════════════════════════════════════════════════════
void Connection::stopBridge()
{
    if (m_timer)  { m_timer->stop();  delete m_timer;  m_timer  = nullptr; }
    if (m_serial) { m_serial->close(); delete m_serial; m_serial = nullptr; }
    qDebug() << "[Serial] Bridge arrete.";
}
