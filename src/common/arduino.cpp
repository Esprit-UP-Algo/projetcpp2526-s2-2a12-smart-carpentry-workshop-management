#include "arduino.h"

ArduinoHandler::ArduinoHandler(QObject *parent)
    : QObject(parent)
#ifdef QT_SERIALPORT_LIB
    , m_serialPort(nullptr)
#endif
    , m_timeoutTimer(nullptr)
    , m_waitingForResponse(false)
{
#ifdef QT_SERIALPORT_LIB
    qDebug() << "ArduinoHandler: QT_SERIALPORT_LIB is defined";
#else
    qDebug() << "ArduinoHandler: QT_SERIALPORT_LIB is NOT defined";
#endif
}

ArduinoHandler::~ArduinoHandler()
{
    disconnectArduino();
}

bool ArduinoHandler::connectToArduino()
{
#ifdef QT_SERIALPORT_LIB
    if (m_serialPort && m_serialPort->isOpen()) {
        return true;
    }

    // Find Arduino port
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    qDebug() << "Available serial ports:" << ports.size();
    for (const QSerialPortInfo &port : ports) {
        qDebug() << "Port:" << port.portName()
                 << "Description:" << port.description()
                 << "Manufacturer:" << port.manufacturer()
                 << "Serial:" << port.serialNumber();
    }

    QList<QSerialPortInfo> arduinoPorts;
    QList<QSerialPortInfo> otherPorts;

    for (const QSerialPortInfo &port : ports) {
        const QString desc = port.description();
        const QString manu = port.manufacturer();
        const QString serial = port.serialNumber();

        if (desc.contains("Arduino", Qt::CaseInsensitive) ||
            manu.contains("Arduino", Qt::CaseInsensitive) ||
            serial.contains("HNDZ", Qt::CaseInsensitive) ||
            port.portName().contains("COM9", Qt::CaseInsensitive)) {
            arduinoPorts.append(port);
        } else if (port.portName().startsWith("COM", Qt::CaseInsensitive)) {
            otherPorts.append(port);
        }
    }

    auto tryPorts = [&](const QList<QSerialPortInfo> &list) {
        for (const QSerialPortInfo &port : list) {
            qDebug() << "Trying to connect to port:" << port.portName()
                     << "Description:" << port.description()
                     << "Manufacturer:" << port.manufacturer();
            m_serialPort = new QSerialPort(port.portName(), this);
            m_serialPort->setBaudRate(QSerialPort::Baud9600);
            m_serialPort->setDataBits(QSerialPort::Data8);
            m_serialPort->setParity(QSerialPort::NoParity);
            m_serialPort->setStopBits(QSerialPort::OneStop);
            m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

            if (m_serialPort->open(QIODevice::ReadWrite)) {
                qDebug() << "Successfully opened port:" << port.portName();
                m_serialPort->clear(QSerialPort::Input);
                m_serialPort->clear(QSerialPort::Output);
                connect(m_serialPort, &QSerialPort::readyRead, this, &ArduinoHandler::onReadyRead);
                connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
                        this, &ArduinoHandler::onErrorOccurred);

                m_timeoutTimer = new QTimer(this);
                m_timeoutTimer->setSingleShot(true);
                connect(m_timeoutTimer, &QTimer::timeout, this, &ArduinoHandler::onTimeout);

                emit connectionStatusChanged(true);
                qDebug() << "Connected to Arduino on port:" << port.portName();
                return true;
            } else {
                qDebug() << "Failed to open port:" << port.portName() << "Error:" << m_serialPort->errorString();
                delete m_serialPort;
                m_serialPort = nullptr;
            }
        }
        return false;
    };

    if (tryPorts(arduinoPorts)) {
        return true;
    }

    if (!arduinoPorts.isEmpty()) {
        qDebug() << "Arduino-specific ports were found but none opened successfully.";
    }

    if (tryPorts(otherPorts)) {
        return true;
    }

    emit errorOccurred("Arduino not found or connection failed");
    return false;
#else
    emit errorOccurred("Qt SerialPort not available");
    return false;
#endif
}

void ArduinoHandler::disconnectArduino()
{
#ifdef QT_SERIALPORT_LIB
    if (m_serialPort) {
        m_serialPort->close();
        delete m_serialPort;
        m_serialPort = nullptr;
    }
#endif

    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
        delete m_timeoutTimer;
        m_timeoutTimer = nullptr;
    }

    m_waitingForResponse = false;
    emit connectionStatusChanged(false);
}

bool ArduinoHandler::isConnected() const
{
#ifdef QT_SERIALPORT_LIB
    return m_serialPort && m_serialPort->isOpen();
#else
    return false;
#endif
}

void ArduinoHandler::requestRFIDRead()
{
    if (!isConnected()) {
        qDebug() << "Arduino not connected, attempting to connect...";
        if (!connectToArduino()) {
            emit errorOccurred("Cannot connect to Arduino");
            return;
        }
    }

    m_buffer.clear();
    m_waitingForResponse = true;

    // Send command to Arduino to read RFID
#ifdef QT_SERIALPORT_LIB
    QByteArray command = "READ_RFID\n";
    m_serialPort->write(command);
    m_serialPort->flush();
    qDebug() << "Sent READ_RFID command to Arduino";
#endif

    // Start timeout timer (10 seconds)
    if (m_timeoutTimer) {
        m_timeoutTimer->start(10000);
    }

    qDebug() << "Requested RFID read from Arduino";
}

void ArduinoHandler::onReadyRead()
{
#ifdef QT_SERIALPORT_LIB
    if (!m_waitingForResponse) return;

    QByteArray data = m_serialPort->readAll();
    data.replace('\0', ' ');
    m_buffer += QString::fromUtf8(data);

    if (!m_buffer.contains('\n')) {
        return;
    }

    bool endsWithNewline = m_buffer.endsWith('\n');
    QStringList parts = m_buffer.split('\n');
    if (!endsWithNewline) {
        m_buffer = parts.takeLast();
    } else {
        m_buffer.clear();
    }

    for (const QString &rawLine : parts) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) {
            continue;
        }

        qDebug() << "Arduino serial line:" << line;

        if (line.startsWith("UID:", Qt::CaseInsensitive)) {
            QString uid = line.mid(4).trimmed();
            m_waitingForResponse = false;
            if (m_timeoutTimer) {
                m_timeoutTimer->stop();
            }
            emit rfidCardRead(uid);
            qDebug() << "RFID card read:" << uid;
            return;
        }

        if (line.compare("NO_CARD", Qt::CaseInsensitive) == 0) {
            m_waitingForResponse = false;
            if (m_timeoutTimer) {
                m_timeoutTimer->stop();
            }
            emit errorOccurred("No RFID card detected");
            return;
        }

        if (line.startsWith("ERROR:", Qt::CaseInsensitive)) {
            m_waitingForResponse = false;
            if (m_timeoutTimer) {
                m_timeoutTimer->stop();
            }
            emit errorOccurred(line);
            return;
        }

        // Ignore startup/welcome text and other non-response lines.
    }
#endif
}

void ArduinoHandler::onErrorOccurred(QSerialPort::SerialPortError error)
{
#ifdef QT_SERIALPORT_LIB
    if (error != QSerialPort::NoError) {
        QString errorMsg = "Serial port error: " + m_serialPort->errorString();
        emit errorOccurred(errorMsg);
        qDebug() << errorMsg;
        disconnectArduino();
    }
#endif
}

void ArduinoHandler::onTimeout()
{
    m_waitingForResponse = false;
    m_buffer.clear();
    emit errorOccurred("Délai d'attente dépassé pour la carte RFID");
    qDebug() << "Timeout waiting for RFID response";
}