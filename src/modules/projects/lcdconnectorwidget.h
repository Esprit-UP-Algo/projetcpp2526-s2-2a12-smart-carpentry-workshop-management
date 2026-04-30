#ifndef LCDCONNECTORWIDGET_H
#define LCDCONNECTORWIDGET_H

/**
 * LcdConnectorWidget
 * ─────────────────────────────────────────────────────────────
 * A compact status bar widget that lets the user pick a serial
 * port, connect/disconnect, and shows the LCD connection state.
 * Embed it inside ProjectManagementPage's top toolbar area.
 * ─────────────────────────────────────────────────────────────
 */

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class LcdConnectorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LcdConnectorWidget(QWidget* parent = nullptr);

    /** Refreshes the port list (called on show or user request). */
    void refreshPorts();

private slots:
    void onToggleConnection();
    void onConnectionChanged(bool connected);
    void onError(const QString& message);

private:
    void setupUI();

    QComboBox*   m_portCombo   = nullptr;
    QPushButton* m_connectBtn  = nullptr;
    QLabel*      m_statusLabel = nullptr;
};

#endif // LCDCONNECTORWIDGET_H
