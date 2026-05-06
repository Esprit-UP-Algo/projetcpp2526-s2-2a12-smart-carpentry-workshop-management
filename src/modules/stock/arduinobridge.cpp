#include "arduinobridge.h"
#include <QDebug>

ArduinoBridge::ArduinoBridge(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead,
            this,     &ArduinoBridge::onReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred,
            this,     &ArduinoBridge::onSerialError);
}

ArduinoBridge::~ArduinoBridge()
{
    close();
}

bool ArduinoBridge::open(const QString& portName, qint32 baudRate)
{
    if (m_serial->isOpen())
        m_serial->close();

    m_serial->setPortName(portName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred(
            QString("Impossible d'ouvrir %1 : %2")
                .arg(portName, m_serial->errorString()));
        emit connectionStatusChanged(false);
        return false;
    }

    m_buffer.clear();
    emit connectionStatusChanged(true);
    qDebug() << "[ArduinoBridge] Connecté sur" << portName;
    return true;
}

void ArduinoBridge::close()
{
    if (m_serial && m_serial->isOpen()) {
        m_serial->close();
        emit connectionStatusChanged(false);
    }
}

bool ArduinoBridge::isOpen() const
{
    return m_serial && m_serial->isOpen();
}

void ArduinoBridge::tare()
{
    if (isOpen())
        m_serial->write("t");
}

QStringList ArduinoBridge::availablePorts()
{
    QStringList ports;
    for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts())
        ports << info.portName();
    return ports;
}

void ArduinoBridge::onReadyRead()
{
    m_buffer += QString::fromLatin1(m_serial->readAll());

    while (m_buffer.contains('\n')) {
        int idx      = m_buffer.indexOf('\n');
        QString line = m_buffer.left(idx).trimmed();
        m_buffer     = m_buffer.mid(idx + 1);

        if (line.isEmpty()) continue;

        double grams = parseWeight(line);
        if (grams >= 0.0)
            emit weightChanged(grams);
        else
            qDebug() << "[ArduinoBridge]" << line;
    }
}

void ArduinoBridge::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) return;
    emit errorOccurred(m_serial->errorString());
    emit connectionStatusChanged(false);
}

// "WEIGHT:397.50" → 397.50 grammes  |  autre ligne → -1
double ArduinoBridge::parseWeight(const QString& line) const
{
    if (!line.startsWith("WEIGHT:")) return -1.0;
    bool ok = false;
    double grams = line.mid(7).toDouble(&ok);
    return ok ? grams : -1.0;
}