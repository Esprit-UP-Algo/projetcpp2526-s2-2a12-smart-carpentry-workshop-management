#include "toggleswitch.h"
#include <QPainter>
#include <QMouseEvent>

ToggleSwitch::ToggleSwitch(QWidget *parent)
    : QWidget(parent)
    , m_checked(false)
    , m_offset(2)
    , m_hovered(false)
{
    setFixedSize(SWITCH_WIDTH, SWITCH_HEIGHT);
    setCursor(Qt::PointingHandCursor);
    
    m_animation = new QPropertyAnimation(this, "offset", this);
    m_animation->setDuration(120);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);
}

QSize ToggleSwitch::sizeHint() const
{
    return QSize(SWITCH_WIDTH, SWITCH_HEIGHT);
}

void ToggleSwitch::setChecked(bool checked)
{
    if (m_checked != checked) {
        m_checked = checked;
        
        int endValue = m_checked ? (SWITCH_WIDTH - THUMB_SIZE - 2) : 2;
        m_animation->setStartValue(m_offset);
        m_animation->setEndValue(endValue);
        m_animation->start();
        
        emit toggled(m_checked);
    }
}

void ToggleSwitch::setOffset(int offset)
{
    m_offset = offset;
    update();
}

void ToggleSwitch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw track
    QColor trackColor;
    if (m_checked) {
        trackColor = QColor("#8A9A5B");
        if (m_hovered) {
            trackColor = trackColor.lighter(110);
        }
    } else {
        trackColor = QColor("#cbd5e0");
        if (m_hovered) {
            trackColor = trackColor.darker(110);
        }
    }
    
    painter.setBrush(trackColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(0, 0, SWITCH_WIDTH, SWITCH_HEIGHT, SWITCH_HEIGHT / 2, SWITCH_HEIGHT / 2);
    
    // Draw thumb
    QColor thumbColor = QColor("#ffffff");
    painter.setBrush(thumbColor);
    
    // Add subtle shadow
    painter.setPen(QPen(QColor(0, 0, 0, 20), 1));
    painter.drawEllipse(m_offset, 2, THUMB_SIZE, THUMB_SIZE);
}

void ToggleSwitch::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setChecked(!m_checked);
    }
    QWidget::mouseReleaseEvent(event);
}

void ToggleSwitch::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void ToggleSwitch::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}
