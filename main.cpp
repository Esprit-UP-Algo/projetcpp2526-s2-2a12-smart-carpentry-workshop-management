#include "mainwindow.h"
#include "src/database/connection.h"

#include <QApplication>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Debug: show available SQL drivers
    qDebug() << "[main] Available SQL drivers:" << QSqlDatabase::drivers();

    if (!Connection::createInstance().createconnect()) {
        QMessageBox::critical(nullptr,
            "Échec de connexion",
            "Impossible de se connecter à la base de données Oracle.\n\n"
            "Vérifiez que :\n"
            "• Le conteneur Docker Oracle est démarré\n"
            "• La source ODBC 'CPP_PROJECT_WS' est configurée\n"
            "• Les pilotes Qt SQL sont installés");
        return 1;
    }

    MainWindow w;
    w.show();
    return a.exec();
}
