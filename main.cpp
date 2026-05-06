#include "mainwindow.h"
#include "src/database/connection.h"

#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Basic");

    QApplication a(argc, argv);

    qDebug() << "[main] Available SQL drivers:" << QSqlDatabase::drivers();

    Connection& conn = Connection::createInstance();

    // ── 1. Connect to the database ────────────────────────────────────────────
    if (!conn.createconnect()) {
        QMessageBox::critical(nullptr,
                              "Échec de connexion",
                              "Impossible de se connecter à la base de données Oracle.\n\n"
                              "Vérifiez que :\n"
                              "• Le conteneur Docker Oracle est démarré\n"
                              "• La source ODBC 'CPP_PROJECT_WS' est configurée\n"
                              "• Les pilotes Qt SQL sont installés");
        return 1;
    }

    // ── 2. Start the serial bridge on a background thread ────────────────────
    //
    // The bridge opens its own DB connection internally (Qt SQL connections
    // are per-thread). If the serial port is absent the thread still runs
    // and handles commands sent via conn.sendBridgeCommand().
    //
    // Change "/dev/ttyACM0" to the actual port if needed, or make it
    // configurable via QSettings / environment variable.
    conn.startBridge("/dev/ttyACM0", 9600);

    // ── 3. Open the main window ───────────────────────────────────────────────
    MainWindow w;
    w.show();
    int ret = a.exec();

    // ── 4. Clean up the bridge thread on exit ─────────────────────────────────
    conn.stopBridge();
    return ret;
}
