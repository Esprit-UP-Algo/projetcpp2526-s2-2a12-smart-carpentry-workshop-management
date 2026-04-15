#include "stockalertbell.h"
#include "src/database/stockdatabase.h"

#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QApplication>

StockAlertBell::StockAlertBell(QWidget *parent)
    : QPushButton(parent)
{
    setFixedSize(44, 44);
    setFlat(true);
    setCursor(Qt::PointingHandCursor);
    setToolTip("Alertes stock");

    // Timer : vérification toutes les 60 secondes
    m_timer = new QTimer(this);
    m_timer->setInterval(10000);
    connect(m_timer, &QTimer::timeout, this, &StockAlertBell::refresh);
    m_timer->start();

    // Première lecture immédiate
    refresh();
}

void StockAlertBell::refresh()
{
    int newCount = StockDatabase::instance().getAlertCount();

    if (newCount != m_lastCount) {
        if (m_lastCount >= 0) // pas au premier démarrage
            emit newAlertsDetected(newCount);
        m_lastCount = newCount;
    }

    m_count = newCount;
    update(); // redessiner
}

void StockAlertBell::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    bool isDark = qApp->styleSheet().contains("0f0f0f");
    QColor bellColor  = isDark ? QColor("#c8c8c8") : QColor("#374151");
    QColor hoverColor = isDark ? QColor("#ffffff") : QColor("#111827");

    // Effet hover
    bool hovered = underMouse();
    QColor currentBell = hovered ? hoverColor : bellColor;

    int cx = width()  / 2;
    int cy = height() / 2 + 1;

    // ── Dessin de la cloche ──────────────────────────────────────────────────
    p.setPen(Qt::NoPen);
    p.setBrush(currentBell);

    // Corps de la cloche (demi-cercle + rectangle)
    int bellW = 18, bellH = 14, bellX = cx - bellW / 2, bellY = cy - 10;

    QPainterPath bell;
    // Arc supérieur
    bell.moveTo(bellX, bellY + bellH);
    bell.lineTo(bellX, bellY + 6);
    bell.arcTo(bellX, bellY, bellW, bellW, 180, -180);
    bell.lineTo(bellX + bellW, bellY + bellH);
    bell.closeSubpath();
    p.fillPath(bell, currentBell);

    // Base plate
    p.fillRect(bellX - 2, bellY + bellH, bellW + 4, 3, currentBell);

    // Tige du battant (clapper)
    QPainterPath clapper;
    clapper.addEllipse(cx - 3, bellY + bellH + 2, 6, 6);
    p.fillPath(clapper, currentBell);

    // Tige de suspension
    p.fillRect(cx - 1, bellY - 3, 2, 5, currentBell);

    // ── Badge rouge ─────────────────────────────────────────────────────────
    if (m_count > 0) {
        QString countStr = m_count > 99 ? "99+" : QString::number(m_count);
        int badgeW = countStr.length() > 1 ? 18 : 14;
        int badgeH = 14;
        int badgeX = cx + 4;
        int badgeY = cy - 18;

        // Fond rouge
        QPainterPath badge;
        badge.addRoundedRect(badgeX, badgeY, badgeW, badgeH, badgeH / 2, badgeH / 2);
        p.fillPath(badge, QColor("#e53e3e"));

        // Contour blanc pour lisibilité
        p.setPen(QPen(isDark ? QColor("#1a1a1a") : QColor("#ffffff"), 1.5));
        p.drawPath(badge);

        // Texte du badge
        QFont f;
        f.setPixelSize(8);
        f.setBold(true);
        p.setFont(f);
        p.setPen(QColor("#ffffff"));
        p.drawText(badgeX, badgeY, badgeW, badgeH, Qt::AlignCenter, countStr);
    }
}
