#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSerialPort>
#include <QTimer>

class Connection : public QObject
{
    Q_OBJECT

public:
    static constexpr const char* CONN_NAME = "oracle_conn";

    static Connection& createInstance()
    {
        static Connection instance;
        return instance;
    }

    bool createconnect();
    bool startBridge(const QString& portName = "COM4", int baud = 9600);
    void stopBridge();

    Connection(const Connection&)            = delete;
    Connection& operator=(const Connection&) = delete;

private slots:
    void tick();   // appelé par QTimer toutes les 100ms

private:
    explicit Connection() : QObject(nullptr) {}

    QString searchByPin(const QString& pin);
    void    serialSend(const QString& msg);

    QSerialPort* m_serial = nullptr;
    QTimer*      m_timer  = nullptr;
    QByteArray   m_rxBuf;
};

#endif // CONNECTION_H
