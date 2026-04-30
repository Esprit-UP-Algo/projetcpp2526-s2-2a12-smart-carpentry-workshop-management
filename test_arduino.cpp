#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "src/common/arduino.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "Testing Arduino connection...";

    ArduinoHandler arduino;

    // Connect signals
    QObject::connect(&arduino, &ArduinoHandler::rfidCardRead, [](const QString &uid) {
        qDebug() << "RFID Card Read:" << uid;
        QCoreApplication::quit();
    });

    QObject::connect(&arduino, &ArduinoHandler::errorOccurred, [](const QString &error) {
        qDebug() << "Arduino Error:" << error;
        QCoreApplication::quit();
    });

    // Connect to Arduino
    if (arduino.connectToArduino()) {
        qDebug() << "Connected to Arduino successfully";

        // Request RFID read
        arduino.requestRFIDRead();

        // Wait for response or timeout
        QTimer::singleShot(15000, &a, &QCoreApplication::quit); // 15 second timeout

        return a.exec();
    } else {
        qDebug() << "Failed to connect to Arduino";
        return 1;
    }
}