#include "mainwindow.h"
#include "connection.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    bool connected = Connection::createInstance().createconnect();

    if (connected)
    {
        MainWindow w;
        w.show();
        return a.exec();
    }
    else
    {
        QMessageBox::critical(nullptr,
                              "Échec de connexion",
                              "Impossible de se connecter à la base de données.\n"
                              "Vérifiez la source ODBC, le nom d'utilisateur et le mot de passe.");
        return 1;
    }
}
