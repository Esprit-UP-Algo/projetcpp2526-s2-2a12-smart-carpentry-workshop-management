#include "lcdconnectorwidget.h"
#include "lcdserialbridge.h"

#include <QHBoxLayout>
#include <QSerialPortInfo>

LcdConnectorWidget::LcdConnectorWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    refreshPorts();

    // React to bridge state changes
    connect(&LcdSerialBridge::instance(), &LcdSerialBridge::connectionChanged,
            this, &LcdConnectorWidget::onConnectionChanged);
    connect(&LcdSerialBridge::instance(), &LcdSerialBridge::errorOccurred,
            this, &LcdConnectorWidget::onError);
}

void LcdConnectorWidget::setupUI()
{
    QHBoxLayout* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);

    // LCD icon label
    QLabel* icon = new QLabel("LCD:", this);
    icon->setStyleSheet("font-size:12px;color:#4b5563;font-weight:700;");
    lay->addWidget(icon);

    // Port selector
    m_portCombo = new QComboBox(this);
    m_portCombo->setFixedHeight(32);
    m_portCombo->setMinimumWidth(100);
    m_portCombo->setStyleSheet(
        "QComboBox{border:1px solid #d1d5db;border-radius:5px;"
        "padding:2px 8px;background:white;font-size:12px;}"
        "QComboBox::drop-down{border:none;}"
        );
    lay->addWidget(m_portCombo);

    // Connect / Disconnect button
    m_connectBtn = new QPushButton("Connecter", this);
    m_connectBtn->setFixedHeight(32);
    m_connectBtn->setCursor(Qt::PointingHandCursor);
    m_connectBtn->setStyleSheet(
        "QPushButton{background:#475569;color:white;border:none;"
        "border-radius:5px;font-size:12px;font-weight:700;padding:0 12px;}"
        "QPushButton:hover{background:#334155;}"
        );
    connect(m_connectBtn, &QPushButton::clicked,
            this, &LcdConnectorWidget::onToggleConnection);
    lay->addWidget(m_connectBtn);

    // Status indicator
    m_statusLabel = new QLabel("⚫ Déconnecté", this);
    m_statusLabel->setStyleSheet("font-size:11px;color:#9ca3af;");
    lay->addWidget(m_statusLabel);
}

void LcdConnectorWidget::refreshPorts()
{
    m_portCombo->clear();
    for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts())
        m_portCombo->addItem(info.portName());

    if (m_portCombo->count() == 0)
        m_portCombo->addItem("(aucun port)");
}

void LcdConnectorWidget::onToggleConnection()
{
    LcdSerialBridge& bridge = LcdSerialBridge::instance();

    if (bridge.isConnected()) {
        bridge.disconnectPort();
    } else {
        QString port = m_portCombo->currentText();
        if (port.isEmpty() || port.startsWith("(")) return;
        bridge.connectPort(port, 9600);
    }
}

void LcdConnectorWidget::onConnectionChanged(bool connected)
{
    if (connected) {
        m_statusLabel->setText("🟢 Connecté → " + LcdSerialBridge::instance().currentPort());
        m_statusLabel->setStyleSheet("font-size:11px;color:#16a34a;font-weight:700;");
        m_connectBtn->setText("Déconnecter");
        m_connectBtn->setStyleSheet(
            "QPushButton{background:#dc2626;color:white;border:none;"
            "border-radius:5px;font-size:12px;font-weight:700;padding:0 12px;}"
            "QPushButton:hover{background:#b91c1c;}");
        m_portCombo->setEnabled(false);
    } else {
        m_statusLabel->setText("⚫ Déconnecté");
        m_statusLabel->setStyleSheet("font-size:11px;color:#9ca3af;");
        m_connectBtn->setText("Connecter");
        m_connectBtn->setStyleSheet(
            "QPushButton{background:#475569;color:white;border:none;"
            "border-radius:5px;font-size:12px;font-weight:700;padding:0 12px;}"
            "QPushButton:hover{background:#334155;}");
        m_portCombo->setEnabled(true);
        refreshPorts();
    }
}

void LcdConnectorWidget::onError(const QString& message)
{
    m_statusLabel->setText("🔴 " + message.left(40));
    m_statusLabel->setStyleSheet("font-size:11px;color:#dc2626;font-weight:700;");
}
