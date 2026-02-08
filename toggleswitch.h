#ifndef TOGGLESWITCH_H
#define TOGGLESWITCH_H

#include <QWidget>
#include <QPropertyAnimation>

class ToggleSwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int offset READ offset WRITE setOffset)

public:
    explicit ToggleSwitch(QWidget *parent = nullptr);
    
    QSize sizeHint() const override;
    bool isChecked() const { return m_checked; }
    void setChecked(bool checked);

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int offset() const { return m_offset; }
    void setOffset(int offset);

    bool m_checked;
    int m_offset;
    bool m_hovered;
    QPropertyAnimation *m_animation;
    
    static constexpr int SWITCH_WIDTH = 44;
    static constexpr int SWITCH_HEIGHT = 22;
    static constexpr int THUMB_SIZE = 18;
};

#endif // TOGGLESWITCH_H
