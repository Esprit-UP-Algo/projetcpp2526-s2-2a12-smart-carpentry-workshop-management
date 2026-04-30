#include "lcdserialbridge.h"
#include <QDebug>

// ─────────────────────────────────────────────────────────────────────────────
//  Singleton
// ─────────────────────────────────────────────────────────────────────────────
LcdSerialBridge& LcdSerialBridge::instance()
{
    static LcdSerialBridge inst;
    return inst;
}

LcdSerialBridge::LcdSerialBridge(QObject* parent)
    : QObject(parent)
{
    m_serial = new QSerialPort(this);

    // Log any serial errors to qWarning
    connect(m_serial, &QSerialPort::errorOccurred,
            this, [this](QSerialPort::SerialPortError err) {
                if (err != QSerialPort::NoError) {
                    QString msg = m_serial->errorString();
                    qWarning() << "[LcdSerialBridge] Serial error:" << msg;
                    emit errorOccurred(msg);
                }
            });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Connection management
// ─────────────────────────────────────────────────────────────────────────────
bool LcdSerialBridge::connectPort(const QString& portName, qint32 baudRate)
{
    if (m_serial->isOpen())
        m_serial->close();

    m_serial->setPortName(portName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::WriteOnly)) {
        qWarning() << "[LcdSerialBridge] Cannot open" << portName
                   << ":" << m_serial->errorString();
        emit errorOccurred("Impossible d'ouvrir " + portName
                           + ": " + m_serial->errorString());
        return false;
    }

    qDebug() << "[LcdSerialBridge] Connected to" << portName
             << "at" << baudRate << "baud";
    emit connectionChanged(true);

    // Tell the LCD we're alive: show a welcome message
    sendLines("WoodFlow", "Connecte...");
    return true;
}

void LcdSerialBridge::disconnectPort()
{
    if (m_serial->isOpen()) {
        clearLcd();
        m_serial->close();
        qDebug() << "[LcdSerialBridge] Disconnected";
        emit connectionChanged(false);
    }
}

bool LcdSerialBridge::isConnected() const
{
    return m_serial && m_serial->isOpen();
}

QString LcdSerialBridge::currentPort() const
{
    return m_serial ? m_serial->portName() : QString();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Public send API
// ─────────────────────────────────────────────────────────────────────────────
void LcdSerialBridge::sendToLcd(const QString& projectName, const QString& status)
{
    sendLines(projectName, status);
}

void LcdSerialBridge::sendLines(const QString& line1, const QString& line2)
{
    if (!isConnected()) return;

    // Protocol: "LINE1:<16 chars>\nLINE2:<16 chars>\n"
    QString msg = QString("LINE1:%1\nLINE2:%2\n")
                      .arg(formatLine(line1))
                      .arg(formatLine(line2));

    QByteArray data = msg.toUtf8();
    qint64 written  = m_serial->write(data);

    if (written != data.size())
        qWarning() << "[LcdSerialBridge] Partial write:"
                   << written << "/" << data.size();
    else
        qDebug() << "[LcdSerialBridge] Sent:" << msg.trimmed();
}

void LcdSerialBridge::clearLcd()
{
    sendLines("                ", "                ");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────
QString LcdSerialBridge::formatLine(const QString& text)
{
    // LCD 1602 has exactly 16 columns
    constexpr int LCD_COLS = 16;

    // Truncate if too long, then pad with spaces to fill the row
    QString result = text.left(LCD_COLS);
    while (result.length() < LCD_COLS)
        result += ' ';
    return result;
}
