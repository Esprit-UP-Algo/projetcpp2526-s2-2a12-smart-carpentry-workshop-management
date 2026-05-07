#ifndef STOCKALERTBELL_H
#define STOCKALERTBELL_H

#include <QPushButton>
#include <QTimer>

/**
 * StockAlertBell — bouton cloche avec badge de notification.
 * Affiche une pastille rouge avec le nombre d'alertes stock.
 * Se met à jour automatiquement via un QTimer.
 */
class StockAlertBell : public QPushButton
{
    Q_OBJECT

public:
    explicit StockAlertBell(QWidget *parent = nullptr);

    int  alertCount() const { return m_count; }

public slots:
    void refresh(); // interroge la DB et met à jour le badge

signals:
    void newAlertsDetected(int count); // émis quand le nombre change

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override { return QSize(44, 44); }

private:
    int    m_count       = 0;
    int    m_lastCount   = -1; // pour détecter les changements
    QTimer *m_timer      = nullptr;
};

#endif // STOCKALERTBELL_H
