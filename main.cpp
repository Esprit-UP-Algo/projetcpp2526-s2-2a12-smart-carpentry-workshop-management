#include <QApplication>
#include <QFile>
#include <QDebug>  // Pour le debug
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 1. Charger le CSS depuis les ressources
    QFile styleFile(":/style.qss");  // Note le ':' pour les ressources
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
        qDebug() << "CSS chargé depuis les ressources!";
    }
    else
    {
        qDebug() << "Erreur: Impossible d'ouvrir :/style.qss";
        // Style par défaut en cas d'erreur
        app.setStyleSheet(
            "QPushButton {"
            "    background-color: #007bff;"
            "    color: white;"
            "    border-radius: 5px;"
            "}"
            );
    }

    // 2. Créer et afficher la fenêtre principale
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
