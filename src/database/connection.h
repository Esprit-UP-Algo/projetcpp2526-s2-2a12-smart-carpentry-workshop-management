#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class Connection
{
public:
    static constexpr const char* CONN_NAME = "oracle_conn";

    static Connection& createInstance()
    {
        static Connection instance;
        return instance;
    }

    bool createconnect();

    Connection(const Connection&)            = delete;
    Connection& operator=(const Connection&) = delete;

private:
    Connection() = default;
};

#endif // CONNECTION_H
