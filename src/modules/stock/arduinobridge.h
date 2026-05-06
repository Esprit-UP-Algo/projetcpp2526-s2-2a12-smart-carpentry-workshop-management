#ifndef ARDUINOBRIDGE_H
#define ARDUINOBRIDGE_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class ArduinoBridge : public QObject
{
    Q_OBJECT

public:
    explicit ArduinoBridge(QObject *parent = nullptr);
    ~ArduinoBridge();

    bool open(const QString& portName, qint32 baudRate = 9600);
    void close();
    bool isOpen() const;
    void tare();

    static QStringList availablePorts();

signals:
    void weightChanged(double grams);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString& message);

private slots:
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serial = nullptr;
    QString      m_buffer;

    double parseWeight(const QString& line) const;
};

#endif // ARDUINOBRIDGE_H