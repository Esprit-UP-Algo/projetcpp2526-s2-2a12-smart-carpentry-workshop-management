#ifndef LCDSERIALBRIDGE_H
#define LCDSERIALBRIDGE_H

/**
 * LcdSerialBridge
 * ───────────────────────────────────────────────────────────────────────────
 * Singleton that owns a QSerialPort connected to an Arduino driving an
 * LCD 1602 (4-bit mode). Call sendToLcd() whenever a project is selected;
 * the bridge formats two 16-char lines and sends them over serial.
 *
 * Protocol (plain text, Arduino side):
 *   "LINE1:<text>\nLINE2:<text>\n"
 *
 * The Arduino reads each line and writes it to the LCD.
 * ───────────────────────────────────────────────────────────────────────────
 */

#include <QObject>
#include <QSerialPort>
#include <QString>

class LcdSerialBridge : public QObject
{
    Q_OBJECT
public:
    static LcdSerialBridge& instance();

    /** Open the serial port (e.g. "COM3" or "/dev/ttyUSB0"), 9600 baud. */
    bool connectPort(const QString& portName, qint32 baudRate = 9600);

    /** Close the serial port. */
    void disconnectPort();

    bool isConnected() const;
    QString currentPort() const;

    /**
     * Send project info to the LCD.
     *   Line 1 → project name   (truncated / padded to 16 chars)
     *   Line 2 → status         (truncated / padded to 16 chars)
     */
    void sendToLcd(const QString& projectName, const QString& status);

    /** Send any two arbitrary lines. */
    void sendLines(const QString& line1, const QString& line2);

    /** Clear the LCD (sends a blank message). */
    void clearLcd();

signals:
    void connectionChanged(bool connected);
    void errorOccurred(const QString& message);

private:
    explicit LcdSerialBridge(QObject* parent = nullptr);
    ~LcdSerialBridge() override = default;
    LcdSerialBridge(const LcdSerialBridge&)            = delete;
    LcdSerialBridge& operator=(const LcdSerialBridge&) = delete;

    /** Pad / truncate to exactly 16 characters for the LCD. */
    static QString formatLine(const QString& text);

    QSerialPort* m_serial = nullptr;
};

#endif // LCDSERIALBRIDGE_H
