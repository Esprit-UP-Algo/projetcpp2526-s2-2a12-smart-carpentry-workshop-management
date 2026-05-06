#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QAtomicInt>

// ── Forward declaration ───────────────────────────────────────────────────────
class BridgeThread;

// ── Connection (singleton) ────────────────────────────────────────────────────
class Connection
{
public:
    static constexpr const char* CONN_NAME = "oracle_conn";

    static Connection& createInstance()
    {
        static Connection instance;
        return instance;
    }

    /** Connect to the Oracle database. Call once at startup. */
    bool createconnect();

    /**
     * Start the serial bridge on a background QThread.
     * Uses the existing DB connection — no second Oracle login.
     * Call once after createconnect() succeeds.
     *
     * @param portName  Serial device, e.g. "/dev/ttyACM0"
     * @param baud      Baud rate (default 9600)
     * @return true if the thread started successfully.
     */
    bool startBridge(const QString& portName = "/dev/ttyACM0", int baud = 9600);

    /** Send a command string to the bridge (as if typed on stdin). */
    void sendBridgeCommand(const QString& cmd);

    /** Stop the bridge thread gracefully. */
    void stopBridge();

    Connection(const Connection&)            = delete;
    Connection& operator=(const Connection&) = delete;

private:
    Connection() = default;

    BridgeThread* m_bridge = nullptr;
};

#endif // CONNECTION_H
