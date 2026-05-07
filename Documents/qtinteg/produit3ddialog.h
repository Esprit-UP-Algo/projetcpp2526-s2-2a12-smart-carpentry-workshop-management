#ifndef PRODUIT3DDIALOG_H
#define PRODUIT3DDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QRegularExpression>
#include <QColor>
#include <QVector>
#include <QPointF>
#include "src/models/produit.h"

class QPainter;

// ═══════════════════════════════════════════════════════════════════════════════
//  Viewer3DWidget — rendu 3D isométrique par catégorie
// ═══════════════════════════════════════════════════════════════════════════════
class Viewer3DWidget : public QWidget
{
    Q_OBJECT
public:
    explicit Viewer3DWidget(QWidget *parent = nullptr);

    void setProduit(const Produit& p);
    void setMatColor(const QColor& face, const QColor& side, const QColor& top);

    // Exposition publique pour les lambdas de contrôle (reset, zoom)
    double m_rotX =  0.42;
    double m_rotY =  0.55;
    double m_zoom =  1.0;

public slots:
    void autoRotateStep();

protected:
    void paintEvent      (QPaintEvent *)   override;
    void mousePressEvent (QMouseEvent *e)  override;
    void mouseMoveEvent  (QMouseEvent *e)  override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent      (QWheelEvent *e)  override;

public:
    // Méthodes de dessin appelables depuis le helper global drawBox
    QPointF project (double x, double y, double z) const;
    void    drawFace(QPainter& p, const QVector<QPointF>& pts,
                  const QVector<int>& idx, const QColor& color);
    QColor  lighten (const QColor& c, int amount) const;

private:
    // Dessin spécialisé par catégorie
    void drawFenetre   (QPainter& p);
    void drawPorte     (QPainter& p);
    void drawEscalier  (QPainter& p);
    void drawMeuble    (QPainter& p);
    void drawMenuiserie(QPainter& p);
    void drawDecoration(QPainter& p);
    void drawGeneric   (QPainter& p);

    Produit m_produit;
    double  m_L = 120, m_l = 60, m_H = 80;
    bool    m_dragging = false;
    QPoint  m_lastPos;

    QColor m_colorFace { "#8B6914" };
    QColor m_colorSide { "#6B4E10" };
    QColor m_colorTop  { "#A07828" };
};

// ═══════════════════════════════════════════════════════════════════════════════
//  Produit3DDialog — dialogue complet avec contrôles
// ═══════════════════════════════════════════════════════════════════════════════
class Produit3DDialog : public QDialog
{
    Q_OBJECT
public:
    explicit Produit3DDialog(const Produit& produit, QWidget *parent = nullptr);

private:
    void setupUI();

    Produit         m_produit;
    Viewer3DWidget *m_viewer = nullptr;
    QTimer         *m_timer  = nullptr;

    struct MatPreset { QString name; QColor face, side, top; };
    QVector<MatPreset> m_presets;
};

#endif // PRODUIT3DDIALOG_H
