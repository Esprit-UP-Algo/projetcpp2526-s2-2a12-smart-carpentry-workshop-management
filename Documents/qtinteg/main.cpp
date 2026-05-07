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

    Connection& conn = Connection::createInstance();

    if (!conn.createconnect()) {
        QMessageBox::critical(nullptr, "Erreur DB",
                              "Impossible de se connecter a Oracle.\n"
                              "Verifiez que Docker est demarre et que\n"
                              "le DSN 'CPP_PROJECT_WS' est configure.");
        return 1;
    }

    conn.startBridge("COM4", 9600);

    MainWindow w;
    w.show();
    int ret = a.exec();

    conn.stopBridge();
    return ret;
}
