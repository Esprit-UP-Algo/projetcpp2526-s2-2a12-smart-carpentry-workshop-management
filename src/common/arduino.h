#ifndef ARDUINOHANDLER_H
#define ARDUINOHANDLER_H

#include <QObject>
#include <QTimer>
#include <QDebug>

#ifdef QT_SERIALPORT_LIB
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

class ArduinoHandler : public QObject
{
    Q_OBJECT

public:
    explicit ArduinoHandler(QObject *parent = nullptr);
    ~ArduinoHandler();

    bool connectToArduino();
    void disconnectArduino();
    bool isConnected() const;

    // Send command to read RFID card
    void requestRFIDRead();

signals:
    void rfidCardRead(const QString &uid);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void onTimeout();

private:
#ifdef QT_SERIALPORT_LIB
    QSerialPort *m_serialPort;
#endif
    QTimer *m_timeoutTimer;
    QString m_buffer;
    bool m_waitingForResponse;
};

#endif // ARDUINOHANDLER_H