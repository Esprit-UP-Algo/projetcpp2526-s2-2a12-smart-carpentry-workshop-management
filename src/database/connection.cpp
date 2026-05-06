#include "connection.h"
#include <QDebug>
#include <QMutex>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QThread>

#include <libserialport.h>

// ═══════════════════════════════════════════════════════════════════════════════
//  DB credentials (shared with the main app's connection)
// ═══════════════════════════════════════════════════════════════════════════════
static constexpr char DB_USER[]     = "CPP_PROJECT";
static constexpr char DB_PASS[]     = "Eoseos69";
static constexpr char DB_HOST[]     = "localhost";
static constexpr int  DB_PORT       = 1522;
static constexpr char DB_SID[]      = "XE";
static constexpr char DB_ODBC_DSN[] = "CPP_PROJECT_WS";

// ═══════════════════════════════════════════════════════════════════════════════
//  DB connection helpers (main app)
// ═══════════════════════════════════════════════════════════════════════════════
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

bool Connection::createconnect()
{
    qDebug() << "[Connection] Available Qt SQL drivers:" << QSqlDatabase::drivers();
    if (tryOCI())  return true;
    if (tryODBC()) return true;
    qDebug() << "[Connection] All drivers failed.";
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  BridgeThread — all serial + DB logic lives here
// ═══════════════════════════════════════════════════════════════════════════════
static constexpr char BRIDGE_CONN[] = "bridge_conn";   // separate Qt SQL connection for thread

class BridgeThread : public QThread
{
public:
    explicit BridgeThread(const QString& port, int baud, QObject* parent = nullptr)
        : QThread(parent), m_portName(port), m_baud(baud) {}

    ~BridgeThread() override { stopAndWait(); }

    void stopAndWait()
    {
        m_running.storeRelease(0);
        wait(3000);
    }

    /** Queue a command to be handled on the next loop tick. */
    void postCommand(const QString& cmd)
    {
        QMutexLocker lk(&m_mutex);
        m_cmdQueue.append(cmd);
    }

protected:
    void run() override
    {
        // ── Open a dedicated DB connection for this thread ──────────────────
        if (!openBridgeDb()) {
            qCritical() << "[Bridge] Cannot connect to DB — thread exiting.";
            return;
        }

        // ── Open serial port ────────────────────────────────────────────────
        sp_port* port = openSerial();
        if (!port) {
            qWarning() << "[Bridge] Serial port unavailable — running in DB-only mode.";
            // Still run; commands can arrive via postCommand()
        }

        m_running.storeRelease(1);
        qDebug() << "[Bridge] Thread started. Port:" << m_portName;

        static QString serialBuf;

        while (m_running.loadAcquire()) {
            // ── Commands queued from the main thread ────────────────────────
            QStringList pending;
            {
                QMutexLocker lk(&m_mutex);
                pending.swap(m_cmdQueue);
            }
            for (const QString& cmd : pending)
                handleCommand(cmd, port);

            // ── Bytes arriving from the Arduino ────────────────────────────
            if (port) {
                char c = 0;
                while (sp_nonblocking_read(port, &c, 1) == 1) {
                    if (c == '\n') {
                        QString line = serialBuf.trimmed();
                        serialBuf.clear();
                        if (!line.isEmpty()) {
                            qDebug() << "[Bridge] Arduino says:" << line;
                            handleCommand(line, port);
                        }
                    } else if (c != '\r') {
                        serialBuf += c;
                    }
                }
            }

            QThread::msleep(5);
        }

        // ── Cleanup ─────────────────────────────────────────────────────────
        if (port) {
            sp_close(port);
            sp_free_port(port);
        }
        QSqlDatabase::removeDatabase(BRIDGE_CONN);
        qDebug() << "[Bridge] Thread stopped.";
    }

private:
    // ── DB ──────────────────────────────────────────────────────────────────

    bool openBridgeDb()
    {
        // Try QOCI first, fall back to QODBC — mirrors the main app logic
        if (QSqlDatabase::isDriverAvailable("QOCI")) {
            QSqlDatabase db = QSqlDatabase::addDatabase("QOCI", BRIDGE_CONN);
            db.setHostName(DB_HOST);
            db.setPort(DB_PORT);
            db.setDatabaseName(DB_SID);
            db.setUserName(DB_USER);
            db.setPassword(DB_PASS);
            if (db.open()) { qDebug() << "[Bridge] DB connected via QOCI."; return true; }
            QSqlDatabase::removeDatabase(BRIDGE_CONN);
        }
        if (QSqlDatabase::isDriverAvailable("QODBC")) {
            QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", BRIDGE_CONN);
            db.setDatabaseName(DB_ODBC_DSN);
            db.setUserName(DB_USER);
            db.setPassword(DB_PASS);
            if (db.open()) { qDebug() << "[Bridge] DB connected via QODBC."; return true; }
            QSqlDatabase::removeDatabase(BRIDGE_CONN);
        }
        return false;
    }

    // ── Serial ──────────────────────────────────────────────────────────────

    sp_port* openSerial()
    {
        sp_port* p = nullptr;
        if (sp_get_port_by_name(m_portName.toUtf8().constData(), &p) != SP_OK) {
            qWarning() << "[Bridge] Port not found:" << m_portName;
            return nullptr;
        }
        if (sp_open(p, SP_MODE_READ_WRITE) != SP_OK) {
            qWarning() << "[Bridge] Cannot open port:" << m_portName;
            sp_free_port(p);
            return nullptr;
        }
        sp_set_baudrate(p, m_baud);
        sp_set_bits(p, 8);
        sp_set_parity(p, SP_PARITY_NONE);
        sp_set_stopbits(p, 1);
        sp_set_flowcontrol(p, SP_FLOWCONTROL_NONE);
        qDebug() << "[Bridge] Serial port opened:" << m_portName;
        return p;
    }

    static void serialWriteLine(sp_port* p, const QString& s)
    {
        if (!p) return;
        QByteArray data = (s + "\n").toUtf8();
        sp_blocking_write(p, data.constData(), data.size(), 2000);
        qDebug() << "[Bridge] Sent:" << s;
    }

    // ── Command handlers ────────────────────────────────────────────────────

    static QStringList splitFields(const QString& s)
    {
        return s.split('/', Qt::SkipEmptyParts);
    }

    static QString cmdInit()
    {
        QSqlDatabase db = QSqlDatabase::database(BRIDGE_CONN);
        QSqlQuery check(db);
        check.prepare("SELECT COUNT(*) FROM user_tables WHERE table_name = 'ACCESS_CODES'");
        if (!check.exec() || !check.next())
            return "ERR:INIT check failed — " + check.lastError().text();
        if (check.value(0).toInt() > 0)
            return "OK:Table already exists";

        QSqlQuery create(db);
        create.prepare(
            "CREATE TABLE ACCESS_CODES ("
            "  CIN      VARCHAR2(20),"
            "  PIN_CODE CHAR(6),"
            "  EXPIRES  DATE,"
            "  PRIMARY KEY (CIN, PIN_CODE)"
            ")"
        );
        if (!create.exec())
            return "ERR:Create failed — " + create.lastError().text();
        return "OK:Table created";
    }

    static QString cmdCheck()
    {
        QSqlDatabase db = QSqlDatabase::database(BRIDGE_CONN);
        QSqlQuery q(db);
        q.prepare(
            "SELECT AC.CIN, AC.PIN_CODE, "
            "       TO_CHAR(AC.EXPIRES,'YYYY-MM-DD') AS EXPIRES, "
            "       CASE WHEN AC.EXPIRES >= SYSDATE THEN 'VALID' ELSE 'EXPIRED' END AS STATUS, "
            "       E.PRENOM_EMP || ' ' || E.NOM_EMP AS FULLNAME "
            "FROM ACCESS_CODES AC "
            "LEFT JOIN EMPLOYE E ON E.CIN = AC.CIN "
            "ORDER BY AC.CIN, AC.EXPIRES"
        );
        if (!q.exec())
            return "ERR:Query failed — " + q.lastError().text();

        int count = 0;
        QString result;
        while (q.next()) {
            result += QString("[%1] %2 | PIN:%3 | exp:%4 | %5\n")
                          .arg(++count)
                          .arg(q.value("FULLNAME").toString().trimmed())
                          .arg(q.value("PIN_CODE").toString().trimmed())
                          .arg(q.value("EXPIRES").toString().trimmed())
                          .arg(q.value("STATUS").toString().trimmed());
        }
        return result.isEmpty() ? "CHEK:Table is empty" : "CHEK:" + result.trimmed();
    }

    static QString cmdAddCode(const QString& cin, const QString& pin, int days)
    {
        if (cin.isEmpty() || pin.isEmpty()) return "ERR:Missing CIN or PIN";
        if (pin.length() != 6)             return "ERR:PIN must be exactly 6 digits";
        if (days <= 0 || days > 365)       return "ERR:Days must be between 1 and 365";

        QSqlDatabase db = QSqlDatabase::database(BRIDGE_CONN);

        QSqlQuery emp(db);
        emp.prepare("SELECT COUNT(*) FROM EMPLOYE WHERE CIN = :cin");
        emp.bindValue(":cin", cin);
        if (!emp.exec() || !emp.next() || emp.value(0).toInt() == 0)
            return "ERR:Employee not found";

        QSqlQuery dup(db);
        dup.prepare("SELECT COUNT(*) FROM ACCESS_CODES WHERE CIN = :cin AND PIN_CODE = :pin");
        dup.bindValue(":cin", cin);
        dup.bindValue(":pin", pin);
        if (!dup.exec() || !dup.next()) return "ERR:Duplicate check failed";
        if (dup.value(0).toInt() > 0)   return "ERR:PIN already exists for this CIN";

        QSqlQuery q(db);
        q.prepare(QString("INSERT INTO ACCESS_CODES (CIN, PIN_CODE, EXPIRES) "
                          "VALUES (:cin, :pin, SYSDATE + %1)").arg(days));
        q.bindValue(":cin", cin);
        q.bindValue(":pin", pin);
        if (!q.exec()) return "ERR:Insert failed — " + q.lastError().text();

        QSqlQuery ex(db);
        ex.prepare("SELECT TO_CHAR(EXPIRES,'YYYY-MM-DD') FROM ACCESS_CODES "
                   "WHERE CIN = :cin AND PIN_CODE = :pin");
        ex.bindValue(":cin", cin);
        ex.bindValue(":pin", pin);
        ex.exec(); ex.next();
        return QString("OK:Code added, expires %1").arg(ex.value(0).toString());
    }

    static QString cmdRemoveCodes(const QString& cin)
    {
        if (cin.isEmpty()) return "ERR:Missing CIN";
        QSqlDatabase db = QSqlDatabase::database(BRIDGE_CONN);
        QSqlQuery q(db);
        q.prepare("DELETE FROM ACCESS_CODES WHERE CIN = :cin");
        q.bindValue(":cin", cin);
        if (!q.exec()) return "ERR:Delete failed — " + q.lastError().text();
        int rows = q.numRowsAffected();
        return rows == 0 ? "WARN:No codes found for CIN"
                         : QString("OK:Removed %1 code(s)").arg(rows);
    }

    static QString cmdExtendCode(const QString& cin, const QString& pin, int days)
    {
        if (cin.isEmpty() || pin.isEmpty()) return "ERR:Missing CIN or PIN";
        if (days <= 0 || days > 365)        return "ERR:Days must be between 1 and 365";

        QSqlDatabase db = QSqlDatabase::database(BRIDGE_CONN);
        QSqlQuery q(db);
        q.prepare(QString("UPDATE ACCESS_CODES SET EXPIRES = SYSDATE + %1 "
                          "WHERE CIN = :cin AND PIN_CODE = :pin").arg(days));
        q.bindValue(":cin", cin);
        q.bindValue(":pin", pin);
        if (!q.exec()) return "ERR:Update failed — " + q.lastError().text();
        if (q.numRowsAffected() == 0) return "ERR:Code not found for this CIN/PIN";

        QSqlQuery ex(db);
        ex.prepare("SELECT TO_CHAR(EXPIRES,'YYYY-MM-DD') FROM ACCESS_CODES "
                   "WHERE CIN = :cin AND PIN_CODE = :pin");
        ex.bindValue(":cin", cin);
        ex.bindValue(":pin", pin);
        ex.exec(); ex.next();
        return QString("OK:Extended %1 day(s), new expiry %2").arg(days).arg(ex.value(0).toString());
    }

    static QString cmdAuthenticate(const QString& cin, const QString& pin)
    {
        QSqlDatabase db = QSqlDatabase::database(BRIDGE_CONN);

        if (pin.isEmpty()) {
            // Plain employee lookup
            QSqlQuery q(db);
            q.prepare("SELECT PRENOM_EMP, NOM_EMP, POST_EMP FROM EMPLOYE WHERE CIN = :cin");
            q.bindValue(":cin", cin);
            if (!q.exec()) return "ERR:Lookup query failed";
            if (!q.next()) return "ERR:Employee not found";
            QString name  = q.value("PRENOM_EMP").toString().trimmed()
                          + " " + q.value("NOM_EMP").toString().trimmed();
            QString poste = q.value("POST_EMP").toString().trimmed();
            return QString("OK:%1|%2").arg(name, poste);
        }

        // Full CIN + PIN auth
        QSqlQuery q(db);
        q.prepare(
            "SELECT E.PRENOM_EMP, E.NOM_EMP, E.POST_EMP, AC.EXPIRES "
            "FROM ACCESS_CODES AC "
            "JOIN EMPLOYE E ON E.CIN = AC.CIN "
            "WHERE AC.CIN = :cin AND AC.PIN_CODE = :pin AND ROWNUM = 1"
        );
        q.bindValue(":cin", cin);
        q.bindValue(":pin", pin);
        if (!q.exec()) return "ERR:Auth query failed";
        if (!q.next()) return "ERR:Invalid PIN";

        QString name  = q.value("PRENOM_EMP").toString().trimmed()
                      + " " + q.value("NOM_EMP").toString().trimmed();
        QString poste = q.value("POST_EMP").toString().trimmed();
        QDateTime expires = q.value("EXPIRES").toDateTime();

        if (QDateTime::currentDateTime() > expires)
            return QString("ERR:PIN expired on %1").arg(expires.toString("yyyy-MM-dd"));

        return QString("OK:%1|%2").arg(name, poste);
    }

    static QString cmdSearchByPin(const QString& pin)
    {
        if (pin.isEmpty())       return "ERR:Missing PIN";
        if (pin.length() != 6)  return "ERR:PIN must be exactly 6 digits";

        QSqlDatabase db = QSqlDatabase::database(BRIDGE_CONN);
        QSqlQuery q(db);
        q.prepare(
            "SELECT AC.CIN, E.PRENOM_EMP, E.NOM_EMP, E.POST_EMP "
            "FROM ACCESS_CODES AC "
            "JOIN EMPLOYE E ON E.CIN = AC.CIN "
            "WHERE AC.PIN_CODE = :pin AND AC.EXPIRES >= SYSDATE AND ROWNUM = 1"
        );
        q.bindValue(":pin", pin);
        if (!q.exec()) return "ERR:Auth query failed";
        if (!q.next()) return "ERR:Invalid PIN";

        QString name  = q.value("PRENOM_EMP").toString().trimmed()
                      + " " + q.value("NOM_EMP").toString().trimmed();
        QString poste = q.value("POST_EMP").toString().trimmed();
        return QString("OK:%1|%2").arg(name, poste);
    }

    // ── Dispatcher ──────────────────────────────────────────────────────────

    void handleCommand(const QString& rawLine, sp_port* port)
    {
        QString line = rawLine.trimmed().toUpper();
        QString response;

        if (line == "INIT") {
            response = cmdInit();
        } else if (line == "CHEK") {
            response = cmdCheck();
        } else if (line.startsWith("ADD")) {
            QStringList f = splitFields(line.mid(3));
            response = (f.size() < 2) ? "ERR:ADD format: ADD<cin>/<pin>[/<days>]"
                                       : cmdAddCode(f[0].trimmed(), f[1].trimmed(),
                                                    f.size() >= 3 ? f[2].toInt() : 7);
        } else if (line.startsWith("RM")) {
            response = cmdRemoveCodes(line.mid(2).trimmed());
        } else if (line.startsWith("EXT")) {
            QStringList f = splitFields(line.mid(3));
            response = (f.size() < 2) ? "ERR:EXT format: EXT<cin>/<pin>[/<days>]"
                                       : cmdExtendCode(f[0].trimmed(), f[1].trimmed(),
                                                       f.size() >= 3 ? f[2].toInt() : 7);
        } else if (line == "EXIT") {
            qInfo() << "[Bridge] EXIT received. Stopping thread.";
            if (port) serialWriteLine(port, "OK:Shutting down");
            m_running.storeRelease(0);
            return;
        } else if (line.startsWith("SC")) {
            response = cmdSearchByPin(line.mid(2).trimmed());
        } else if (line.startsWith("SE")) {
            QStringList f = splitFields(line.mid(2));
            response = f.isEmpty() ? "ERR:SE format: SE<cin>[/<pin>]"
                                   : cmdAuthenticate(f[0].trimmed(),
                                                     f.size() >= 2 ? f[1].trimmed() : "");
        } else {
            response = "ERR:Unknown command";
        }

        qInfo() << "[Bridge] Response:" << response;
        if (port) serialWriteLine(port, response);
    }

    // ── Members ─────────────────────────────────────────────────────────────
    QString      m_portName;
    int          m_baud;
    QAtomicInt   m_running{0};
    QMutex       m_mutex;
    QStringList  m_cmdQueue;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  Connection public API — bridge management
// ═══════════════════════════════════════════════════════════════════════════════

bool Connection::startBridge(const QString& portName, int baud)
{
    if (m_bridge && m_bridge->isRunning()) {
        qDebug() << "[Connection] Bridge already running.";
        return true;
    }
    delete m_bridge;
    m_bridge = new BridgeThread(portName, baud);
    m_bridge->start();
    // Give the thread a moment to open the DB and serial port
    QThread::msleep(500);
    qDebug() << "[Connection] Bridge thread launched.";
    return true;
}

void Connection::sendBridgeCommand(const QString& cmd)
{
    if (m_bridge && m_bridge->isRunning())
        m_bridge->postCommand(cmd);
    else
        qWarning() << "[Connection] sendBridgeCommand: bridge not running.";
}

void Connection::stopBridge()
{
    if (m_bridge) {
        m_bridge->stopAndWait();
        delete m_bridge;
        m_bridge = nullptr;
    }
}
