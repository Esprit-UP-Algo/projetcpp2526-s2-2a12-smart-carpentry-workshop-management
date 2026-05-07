#include "produit3ddialog.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QTimer>
#include <QtMath>
#include <QVector3D>
#include <QVector>
#include <QFont>
#include <QDialog>
#include <QFrame>
#include <QScrollArea>

// ═══════════════════════════════════════════════════════════════════════════════
//  Viewer3DWidget — moteur de rendu 3D isométrique par catégorie
// ═══════════════════════════════════════════════════════════════════════════════

Viewer3DWidget::Viewer3DWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(420, 340);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

void Viewer3DWidget::setProduit(const Produit& p)
{
    m_produit = p;

    // Extraire les dimensions depuis la chaîne "LxlxH" ou "LxH"
    QString dim = p.getDimensions();
    QStringList parts = dim.split(QRegularExpression("[xX*×]"));
    m_L = (parts.size() > 0 && !parts[0].trimmed().isEmpty()) ? parts[0].trimmed().toDouble() : 100;
    m_l = (parts.size() > 1 && !parts[1].trimmed().isEmpty()) ? parts[1].trimmed().toDouble() : 60;
    m_H = (parts.size() > 2 && !parts[2].trimmed().isEmpty()) ? parts[2].trimmed().toDouble() : 80;

    // Normaliser pour l'affichage (max 150 unités)
    double maxDim = qMax(m_L, qMax(m_l, m_H));
    if (maxDim > 0) {
        double scale = 150.0 / maxDim;
        m_L *= scale; m_l *= scale; m_H *= scale;
    }

    update();
}

void Viewer3DWidget::setMatColor(const QColor& face, const QColor& side, const QColor& top)
{
    m_colorFace = face;
    m_colorSide = side;
    m_colorTop  = top;
    update();
}

QPointF Viewer3DWidget::project(double x, double y, double z) const
{
    double cx = width()  / 2.0;
    double cy = height() / 2.0;

    double cosX = qCos(m_rotX), sinX = qSin(m_rotX);
    double cosY = qCos(m_rotY), sinY = qSin(m_rotY);

    double x1 =  x * cosY + z * sinY;
    double y1 =  y;
    double z1 = -x * sinY + z * cosY;

    double x2 = x1;
    double y2 = y1 * cosX - z1 * sinX;

    return QPointF(cx + x2 * m_zoom, cy - y2 * m_zoom);
}

QColor Viewer3DWidget::lighten(const QColor& c, int amount) const
{
    return QColor(
        qMin(255, c.red()   + amount),
        qMin(255, c.green() + amount),
        qMin(255, c.blue()  + amount)
        );
}

void Viewer3DWidget::drawFace(QPainter& p, const QVector<QPointF>& pts,
                              const QVector<int>& idx, const QColor& color)
{
    QPainterPath path;
    path.moveTo(pts[idx[0]]);
    for (int i = 1; i < idx.size(); ++i)
        path.lineTo(pts[idx[i]]);
    path.closeSubpath();

    p.setBrush(color);
    p.setPen(QPen(color.darker(130), 1.2));
    p.drawPath(path);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers de dessin spécialisés par catégorie
// ─────────────────────────────────────────────────────────────────────────────

// Dessine un parallélépipède générique centré en (0,0,0)
static void drawBox(Viewer3DWidget* w, QPainter& p,
                    double L, double H, double D,
                    const QColor& face, const QColor& side, const QColor& top,
                    double ox=0, double oy=0, double oz=0)
{
    double l=L/2, h=H/2, d=D/2;
    QVector<QPointF> v(8);
    v[0]=w->project(ox-l, oy-h, oz-d);
    v[1]=w->project(ox+l, oy-h, oz-d);
    v[2]=w->project(ox+l, oy+h, oz-d);
    v[3]=w->project(ox-l, oy+h, oz-d);
    v[4]=w->project(ox-l, oy-h, oz+d);
    v[5]=w->project(ox+l, oy-h, oz+d);
    v[6]=w->project(ox+l, oy+h, oz+d);
    v[7]=w->project(ox-l, oy+h, oz+d);

    w->drawFace(p, v, {4,5,6,7}, top.lighter(115));   // dessus
    w->drawFace(p, v, {0,1,5,4}, face);               // avant
    w->drawFace(p, v, {1,2,6,5}, side);               // droite
}

// ─────────────────────────────────────────────────────────────────────────────
//  paintEvent — dispatche le dessin selon la catégorie
// ─────────────────────────────────────────────────────────────────────────────
void Viewer3DWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // Fond dégradé
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor("#1a202c"));
    bg.setColorAt(1, QColor("#2d3748"));
    p.fillRect(rect(), bg);

    // Grille de sol
    p.save();
    p.setPen(QPen(QColor(255,255,255,18), 1));
    int gs = 40;
    int gn = 7;
    int gcx = width()/2, gcy = height()*3/4;
    for (int i = -gn; i <= gn; ++i) {
        p.drawLine(gcx + i*gs, gcy - gn*10,  gcx + i*gs + gn*20, gcy + gn*10);
        p.drawLine(gcx - gn*gs + i*20, gcy + i*10, gcx + gn*gs + i*20, gcy + i*10);
    }
    p.restore();

    QString cat = m_produit.getCategorie();

    if      (cat == "Fenêtre")    drawFenetre(p);
    else if (cat == "Porte")      drawPorte(p);
    else if (cat == "Escalier")   drawEscalier(p);
    else if (cat == "Meuble")     drawMeuble(p);
    else if (cat == "Menuiserie") drawMenuiserie(p);
    else if (cat == "Décoration") drawDecoration(p);
    else                          drawGeneric(p);

    // Nom du produit en bas
    p.save();
    QFont fn("Arial", 11, QFont::Bold);
    p.setFont(fn);
    p.setPen(QColor(255,255,255,200));
    p.drawText(QRect(0, height()-30, width(), 24), Qt::AlignCenter, m_produit.getNom());
    p.restore();

    // Dimensions en haut à gauche
    p.save();
    QFont fd("Arial", 8);
    p.setFont(fd);
    p.setPen(QColor(160,200,255,160));
    QString dimStr = QString("L:%1  l:%2  H:%3")
                         .arg(m_produit.getDimensions().isEmpty() ? "—" : m_produit.getDimensions());
    p.drawText(QRect(10, 10, 200, 20), Qt::AlignLeft, m_produit.getDimensions());
    p.restore();
}

// ─────────────────────────────────────────────────────────────────────────────
//  FENÊTRE — cadre + vitres + croisillons
// ─────────────────────────────────────────────────────────────────────────────
void Viewer3DWidget::drawFenetre(QPainter& p)
{
    double W = m_L, H = m_H, D = m_l * 0.15;

    // Cadre extérieur
    QColor bois = m_colorFace;
    double ep = W * 0.07; // épaisseur montant

    // Panneau de fond (vitre)
    {
        QVector<QPointF> v(8);
        double hw=W/2, hh=H/2, hd=D/2;
        v[0]=project(-hw,   -hh,   -hd);
        v[1]=project( hw,   -hh,   -hd);
        v[2]=project( hw,    hh,   -hd);
        v[3]=project(-hw,    hh,   -hd);
        v[4]=project(-hw,   -hh,    hd);
        v[5]=project( hw,   -hh,    hd);
        v[6]=project( hw,    hh,    hd);
        v[7]=project(-hw,    hh,    hd);

        // Vitre bleue transparente
        QPainterPath vpath;
        vpath.moveTo(v[4]); vpath.lineTo(v[5]); vpath.lineTo(v[6]); vpath.lineTo(v[7]); vpath.closeSubpath();
        p.fillPath(vpath, QColor(120, 180, 255, 90));
        p.setPen(QPen(QColor(100,160,220,120), 1));
        p.drawPath(vpath);
    }

    // Montants du cadre (4 barres)
    auto bar = [&](double x1, double y1, double x2, double y2) {
        drawBox(this, p, qAbs(x2-x1)+ep*0.5, H, D, bois, m_colorSide, m_colorTop, (x1+x2)/2, 0, 0);
    };

    // Montants verticaux gauche/droite
    drawBox(this, p, ep, H, D, bois, m_colorSide, m_colorTop, -W/2+ep/2, 0, 0);
    drawBox(this, p, ep, H, D, bois, m_colorSide, m_colorTop,  W/2-ep/2, 0, 0);
    // Traverses haut/bas
    drawBox(this, p, W, ep, D, bois, m_colorSide, m_colorTop, 0, -H/2+ep/2, 0);
    drawBox(this, p, W, ep, D, bois, m_colorSide, m_colorTop, 0,  H/2-ep/2, 0);
    // Croisillon central
    drawBox(this, p, ep*0.7, H-ep*2, D*0.6, bois.darker(110), m_colorSide, m_colorTop, 0, 0, D*0.1);
    drawBox(this, p, W-ep*2, ep*0.7, D*0.6, bois.darker(110), m_colorSide, m_colorTop, 0, 0, D*0.1);

    // Poignée
    drawBox(this, p, ep*0.4, ep*2, ep*0.4, QColor("#C0C0C0"), QColor("#A0A0A0"), QColor("#E0E0E0"), ep, 0, D/2+ep*0.3);
}

// ─────────────────────────────────────────────────────────────────────────────
//  PORTE — panneau + chambranle + poignée + paumelles
// ─────────────────────────────────────────────────────────────────────────────
void Viewer3DWidget::drawPorte(QPainter& p)
{
    double W = m_L * 0.6, H = m_H, D = m_l * 0.12;
    QColor bois = m_colorFace;

    // Panneau principal
    drawBox(this, p, W, H, D, bois, m_colorSide, m_colorTop);

    // Cadre (légèrement en relief)
    double ep = W * 0.08;
    QColor cadre = bois.darker(115);
    drawBox(this, p, ep, H, D*0.4, cadre, m_colorSide.darker(115), m_colorTop.darker(115), -W/2+ep/2, 0, D/2);
    drawBox(this, p, ep, H, D*0.4, cadre, m_colorSide.darker(115), m_colorTop.darker(115),  W/2-ep/2, 0, D/2);
    drawBox(this, p, W, ep, D*0.4, cadre, m_colorSide.darker(115), m_colorTop.darker(115), 0, -H/2+ep/2, D/2);

    // Panneaux encastrés (2 rectangles décoratifs)
    QColor panneau = bois.lighter(110);
    drawBox(this, p, W-ep*3, H*0.35, D*0.2, panneau, panneau.darker(110), panneau.lighter(110), 0, -H*0.2, D/2+D*0.12);
    drawBox(this, p, W-ep*3, H*0.35, D*0.2, panneau, panneau.darker(110), panneau.lighter(110), 0,  H*0.2, D/2+D*0.12);

    // Poignée
    QColor metal("#B8B8B8");
    drawBox(this, p, ep*0.35, ep*2.5, ep*0.35, metal, metal.darker(115), metal.lighter(120), W*0.35, 0, D/2+ep*0.3);
    drawBox(this, p, ep*1.5,  ep*0.35, ep*0.35, metal, metal.darker(115), metal.lighter(120), W*0.35+ep*0.6, 0, D/2+ep*0.3);

    // Paumelles (3 charnières)
    QColor paumelle("#808080");
    for (int i = -1; i <= 1; ++i)
        drawBox(this, p, ep*0.6, ep*0.8, D*0.15, paumelle, paumelle, paumelle.lighter(120), -W/2+ep*0.3, i*H*0.28, D/2);
}

// ─────────────────────────────────────────────────────────────────────────────
//  ESCALIER — marches empilées décalées
// ─────────────────────────────────────────────────────────────────────────────
void Viewer3DWidget::drawEscalier(QPainter& p)
{
    int nMarches = 7;
    double largeur = m_L;
    double hauteurMarche = m_H / nMarches;
    double profondeurMarche = m_l / nMarches;
    QColor bois = m_colorFace;

    for (int i = 0; i < nMarches; ++i) {
        double ox = (i - nMarches/2.0 + 0.5) * profondeurMarche;
        double oy = -(nMarches/2.0 - i - 0.5) * hauteurMarche;
        double ep = hauteurMarche * 0.85;

        // Giron (surface horizontale)
        drawBox(this, p, largeur, ep * 0.18, profondeurMarche * (i+1),
                bois.lighter(115), m_colorSide, m_colorTop.lighter(120),
                0, oy, -profondeurMarche * (nMarches - i - 1) / 2.0 + profondeurMarche * i / 2.0);

        // Contremarche (surface verticale)
        drawBox(this, p, largeur, ep, profondeurMarche * 0.12,
                bois, m_colorSide.darker(105), m_colorTop,
                0, oy - ep*0.4, -m_l/2.0 + profondeurMarche * i);
    }

    // Main courante
    double railY = m_H/2 + hauteurMarche * 0.5;
    for (int i = 0; i < nMarches-1; ++i) {
        drawBox(this, p, largeur * 0.05, largeur * 0.05, profondeurMarche,
                QColor("#A0522D"), QColor("#8B4513"), QColor("#CD853F"),
                largeur/2 - largeur*0.03,
                -(nMarches/2.0 - i - 1) * hauteurMarche,
                -m_l/2.0 + profondeurMarche * (i + 0.5));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  MEUBLE — armoire/commode avec tiroirs et pieds
// ─────────────────────────────────────────────────────────────────────────────
void Viewer3DWidget::drawMeuble(QPainter& p)
{
    double W = m_L, H = m_H, D = m_l * 0.5;
    QColor bois = m_colorFace;
    double ep = W * 0.04;

    // Corps principal
    drawBox(this, p, W, H, D, bois, m_colorSide, m_colorTop);

    // Tiroirs (3 rangées)
    int nTiroirs = 3;
    double tirH = (H - ep*2) / nTiroirs - ep;
    QColor tiroir = bois.lighter(112);
    QColor metal("#A0A0A0");

    for (int i = 0; i < nTiroirs; ++i) {
        double ty = -H/2 + ep + (tirH + ep) * i + tirH/2;
        // Face tiroir
        drawBox(this, p, W - ep*2, tirH, D*0.06,
                tiroir, tiroir.darker(110), tiroir.lighter(115),
                0, ty, D/2 + D*0.04);
        // Poignée
        drawBox(this, p, W*0.2, ep*0.6, ep*0.5,
                metal, metal.darker(115), metal.lighter(120),
                0, ty, D/2 + D*0.1);
    }

    // Pieds (4 coins)
    QColor pied = bois.darker(130);
    double piedH = H * 0.06;
    for (int dx : {-1, 1})
        for (int dz : {-1, 1})
            drawBox(this, p, ep*1.2, piedH, ep*1.2,
                    pied, pied.darker(110), pied.lighter(110),
                    dx*(W/2 - ep*0.8), -H/2 - piedH/2, dz*(D/2 - ep*0.8));
}

// ─────────────────────────────────────────────────────────────────────────────
//  MENUISERIE — panneau de bois lamellé avec rainures
// ─────────────────────────────────────────────────────────────────────────────
void Viewer3DWidget::drawMenuiserie(QPainter& p)
{
    double W = m_L, H = m_H, D = m_l * 0.2;
    QColor bois = m_colorFace;

    // Planches horizontales avec décalage de teinte
    int nPlanches = 5;
    double planH = H / nPlanches;

    for (int i = 0; i < nPlanches; ++i) {
        int var = (i % 2 == 0) ? 0 : 8;
        QColor c = QColor(
            qMin(255, bois.red()   + var),
            qMin(255, bois.green() + var),
            qMin(255, bois.blue()  + var)
            );
        double py = -H/2 + planH * i + planH/2;
        drawBox(this, p, W, planH * 0.92, D, c, m_colorSide, m_colorTop.lighter(108), 0, py, 0);
    }

    // Rainures verticales (encoches)
    int nRainures = 4;
    QColor rainure = bois.darker(140);
    for (int i = 1; i < nRainures; ++i) {
        double rx = -W/2 + W * i / nRainures;
        drawBox(this, p, W*0.015, H, D*0.5, rainure, rainure, rainure, rx, 0, D*0.3);
    }

    // Visserie décorative
    QColor vis("#888888");
    for (int i = 1; i < nRainures; ++i) {
        double rx = -W/2 + W * i / nRainures;
        for (int j : {-1, 1})
            drawBox(this, p, W*0.025, W*0.025, D*0.15, vis, vis, vis.lighter(130), rx, j*H*0.35, D/2);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DÉCORATION — cadre miroir / tableau décoratif
// ─────────────────────────────────────────────────────────────────────────────
void Viewer3DWidget::drawDecoration(QPainter& p)
{
    double W = m_L, H = m_H, D = m_l * 0.08;
    QColor bois = m_colorFace;
    double ep = W * 0.10;

    // Cadre (4 barres)
    // Haut / Bas
    drawBox(this, p, W, ep, D, bois, m_colorSide, m_colorTop, 0, -H/2+ep/2, 0);
    drawBox(this, p, W, ep, D, bois, m_colorSide, m_colorTop, 0,  H/2-ep/2, 0);
    // Gauche / Droite
    drawBox(this, p, ep, H, D, bois, m_colorSide, m_colorTop, -W/2+ep/2, 0, 0);
    drawBox(this, p, ep, H, D, bois, m_colorSide, m_colorTop,  W/2-ep/2, 0, 0);

    // Surface intérieure (miroir / vitrage)
    double iW = W - ep*2, iH = H - ep*2;
    {
        QVector<QPointF> v(4);
        v[0]=project(-iW/2, -iH/2, D/2);
        v[1]=project( iW/2, -iH/2, D/2);
        v[2]=project( iW/2,  iH/2, D/2);
        v[3]=project(-iW/2,  iH/2, D/2);
        QPainterPath path;
        path.moveTo(v[0]); path.lineTo(v[1]); path.lineTo(v[2]); path.lineTo(v[3]); path.closeSubpath();
        // Dégradé miroir
        QLinearGradient miroir(v[0].x(), v[0].y(), v[2].x(), v[2].y());
        miroir.setColorAt(0.0, QColor(200, 220, 255, 180));
        miroir.setColorAt(0.4, QColor(240, 250, 255, 220));
        miroir.setColorAt(0.7, QColor(180, 200, 240, 160));
        miroir.setColorAt(1.0, QColor(210, 225, 255, 180));
        p.fillPath(path, miroir);
        p.setPen(QPen(QColor(180,200,255,100), 1));
        p.drawPath(path);

        // Reflet diagonal
        p.save();
        p.setClipPath(path);
        p.setPen(QPen(QColor(255,255,255,60), iW*0.08));
        p.drawLine(v[0].toPoint(), v[2].toPoint());
        p.restore();
    }

    // Coins dorés
    QColor or_("#D4A017");
    double cs = ep * 0.5;
    for (int dx : {-1, 1})
        for (int dy : {-1, 1})
            drawBox(this, p, cs, cs, D*1.2, or_, or_.darker(120), or_.lighter(130),
                    dx*(W/2-ep*0.3), dy*(H/2-ep*0.3), 0);

    // Crochet de suspension
    drawBox(this, p, ep*0.3, ep*0.8, D*0.3, QColor("#888"), QColor("#666"), QColor("#AAA"), 0, -H/2-ep*0.5, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  GÉNÉRIQUE — boîte avec texture bois
// ─────────────────────────────────────────────────────────────────────────────
void Viewer3DWidget::drawGeneric(QPainter& p)
{
    drawBox(this, p, m_L, m_H, m_l, m_colorFace, m_colorSide, m_colorTop);

    // Veinage bois sur la face avant
    double hw = m_L/2, hh = m_H/2, hd = m_l/2;
    QVector<QPointF> face = {
        project(-hw, -hh, hd), project(hw, -hh, hd),
        project(hw,  hh, hd),  project(-hw, hh, hd)
    };
    QPainterPath clip;
    clip.moveTo(face[0]); clip.lineTo(face[1]); clip.lineTo(face[2]); clip.lineTo(face[3]); clip.closeSubpath();
    p.save();
    p.setClipPath(clip);
    p.setPen(QPen(m_colorFace.darker(115), 1.5, Qt::SolidLine));
    p.setOpacity(0.35);
    for (int i = 0; i < 6; ++i) {
        QPointF a = face[0] + (face[1]-face[0]) * (i / 6.0);
        QPointF b = face[3] + (face[2]-face[3]) * (i / 6.0);
        p.drawLine(a, b);
    }
    p.restore();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Interactions souris / molette
// ─────────────────────────────────────────────────────────────────────────────
void Viewer3DWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastPos  = e->pos();
    }
}

void Viewer3DWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_dragging) return;
    QPoint delta = e->pos() - m_lastPos;
    m_rotY += delta.x() * 0.012;
    m_rotX -= delta.y() * 0.012;
    m_rotX  = qBound(-1.2, m_rotX, 1.2);
    m_lastPos = e->pos();
    update();
}

void Viewer3DWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        m_dragging = false;
}

void Viewer3DWidget::wheelEvent(QWheelEvent *e)
{
    m_zoom *= (e->angleDelta().y() > 0) ? 1.10 : 0.91;
    m_zoom  = qBound(0.3, m_zoom, 3.5);
    update();
}

void Viewer3DWidget::autoRotateStep()
{
    m_rotY += 0.018;
    update();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Produit3DDialog — interface complète
// ═══════════════════════════════════════════════════════════════════════════════

Produit3DDialog::Produit3DDialog(const Produit& produit, QWidget *parent)
    : QDialog(parent), m_produit(produit)
{
    setWindowTitle(QString("Visualisation 3D — %1").arg(produit.getNom()));
    setMinimumSize(820, 540);
    setStyleSheet(R"(
        QDialog { background: #0f172a; }
        QLabel  { color: #e2e8f0; }
        QGroupBox {
            color: #94a3b8;
            border: 1px solid #334155;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 8px;
            font-size: 11px;
            font-weight: bold;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }
        QPushButton {
            background: #1e293b;
            color: #94a3b8;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 6px 14px;
            font-size: 11px;
        }
        QPushButton:hover  { background: #334155; color: #e2e8f0; }
        QPushButton:checked { background: #0ea5e9; color: white; border-color: #0ea5e9; }
        QSlider::groove:horizontal {
            background: #1e293b; border: 1px solid #334155;
            height: 4px; border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #0ea5e9; width: 14px; height: 14px;
            margin: -5px 0; border-radius: 7px;
        }
        QSlider::sub-page:horizontal { background: #0ea5e9; border-radius: 2px; }
    )");

    setupUI();
}

void Produit3DDialog::setupUI()
{
    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(16);

    // ── Panneau gauche : viewer ───────────────────────────────────────────────
    QVBoxLayout *leftLay = new QVBoxLayout();
    leftLay->setSpacing(10);

    // En-tête produit
    QFrame *header = new QFrame();
    header->setStyleSheet("background:#1e293b; border-radius:10px; padding:4px;");
    QHBoxLayout *hdrLay = new QHBoxLayout(header);
    hdrLay->setContentsMargins(14, 10, 14, 10);

    // Badge catégorie
    static const QMap<QString,QString> catColors = {
                                                     {"Fenêtre","#14b8a6"},   {"Porte","#8b5cf6"},
                                                     {"Escalier","#ef4444"},  {"Meuble","#3b82f6"},
                                                     {"Menuiserie","#10b981"},{"Décoration","#f59e0b"},
                                                     {"Autre","#6b7280"},
                                                     };
    QString accent = catColors.value(m_produit.getCategorie(), "#6b7280");

    QLabel *badge = new QLabel(m_produit.getCategorie());
    badge->setStyleSheet(QString("background:%1; color:white; border-radius:10px;"
                                 "padding:3px 10px; font-size:10px; font-weight:bold;").arg(accent));
    badge->setFixedHeight(22);

    QLabel *nomLbl = new QLabel(m_produit.getNom());
    nomLbl->setStyleSheet("font-size:16px; font-weight:bold; color:#f1f5f9;");

    QLabel *refLbl = new QLabel(QString("Réf. #%1  ·  %2 €  ·  %3")
                                    .arg(m_produit.getId())
                                    .arg(m_produit.getPrix(), 0, 'f', 2)
                                    .arg(m_produit.getDimensions()));
    refLbl->setStyleSheet("font-size:11px; color:#64748b;");

    QVBoxLayout *hdrText = new QVBoxLayout();
    hdrText->setSpacing(3);
    hdrText->addWidget(nomLbl);
    hdrText->addWidget(refLbl);

    hdrLay->addWidget(badge);
    hdrLay->addSpacing(10);
    hdrLay->addLayout(hdrText);
    hdrLay->addStretch();

    leftLay->addWidget(header);

    // Viewer 3D
    m_viewer = new Viewer3DWidget();
    m_viewer->setProduit(m_produit);
    m_viewer->setStyleSheet("border-radius:12px;");
    leftLay->addWidget(m_viewer, 1);

    // Contrôles navigation
    QHBoxLayout *navLay = new QHBoxLayout();
    navLay->setSpacing(8);

    QLabel *hint = new QLabel("🖱  Glisser pour tourner  ·  Molette pour zoomer");
    hint->setStyleSheet("color:#475569; font-size:10px;");

    QPushButton *resetBtn = new QPushButton("↺  Réinitialiser");
    connect(resetBtn, &QPushButton::clicked, this, [this](){
        m_viewer->m_rotX = 0.42; m_viewer->m_rotY = 0.55;
        m_viewer->m_zoom = 1.0;  m_viewer->update();
    });

    QPushButton *autoBtn = new QPushButton("▶  Auto-rotation");
    autoBtn->setCheckable(true);
    autoBtn->setChecked(true);
    connect(autoBtn, &QPushButton::toggled, this, [this](bool on){
        if (on) m_timer->start(30); else m_timer->stop();
    });

    navLay->addWidget(hint);
    navLay->addStretch();
    navLay->addWidget(resetBtn);
    navLay->addWidget(autoBtn);
    leftLay->addLayout(navLay);

    root->addLayout(leftLay, 3);

    // ── Panneau droit : contrôles ─────────────────────────────────────────────
    QVBoxLayout *rightLay = new QVBoxLayout();
    rightLay->setSpacing(12);
    rightLay->setContentsMargins(0, 0, 0, 0);

    // ── Présets matériaux ──────────────────────────────────────────────────────
    // Définis selon la catégorie
    QString cat = m_produit.getCategorie();
    QString mat = m_produit.getMatUtilise().toLower();

    // Présets communs bois
    m_presets = {
                 {"Chêne naturel",  QColor("#8B6914"), QColor("#6B4E10"), QColor("#A07828")},
                 {"Pin clair",      QColor("#D4A869"), QColor("#B8893A"), QColor("#E4BB7A")},
                 {"Noyer sombre",   QColor("#4A3020"), QColor("#362418"), QColor("#5C4030")},
                 {"MDF laqué blanc",QColor("#F0EDE8"), QColor("#D8D5CE"), QColor("#FAFAF8")},
                 {"Mérisier",       QColor("#C0724A"), QColor("#A05838"), QColor("#D08658")},
                 {"Hêtre",          QColor("#C8A878"), QColor("#A88858"), QColor("#D8B888")},
                 };

    // Présets supplémentaires selon catégorie
    if (cat == "Fenêtre" || cat == "Porte") {
        m_presets.append({"Blanc laqué",   QColor("#E8E8E0"), QColor("#C8C8C0"), QColor("#F8F8F8")});
        m_presets.append({"Anthracite",    QColor("#3C3C40"), QColor("#282830"), QColor("#505055")});
    }
    if (cat == "Escalier") {
        m_presets.append({"Acier brossé",  QColor("#8C9098"), QColor("#6C7078"), QColor("#ACAAB0")});
    }

    QGroupBox *matBox = new QGroupBox("MATÉRIAU / FINITION");
    QVBoxLayout *matLay = new QVBoxLayout(matBox);
    matLay->setSpacing(6);

    // Sélectionner le preset selon le matériau du produit
    int defaultPreset = 0;
    for (int i = 0; i < m_presets.size(); ++i) {
        if (mat.contains(m_presets[i].name.split(' ')[0].toLower()))
            defaultPreset = i;
    }

    for (int i = 0; i < m_presets.size(); ++i) {
        const auto& pr = m_presets[i];
        QPushButton *btn = new QPushButton(pr.name);
        btn->setCheckable(true);
        btn->setChecked(i == defaultPreset);

        // Pastille couleur
        QPixmap dot(12, 12);
        dot.fill(pr.face);
        btn->setIcon(QIcon(dot));

        connect(btn, &QPushButton::clicked, this, [this, i, btn](){
            // Décocher les autres
            QGroupBox *gb = qobject_cast<QGroupBox*>(btn->parentWidget());
            if (gb) for (auto *b : gb->findChildren<QPushButton*>()) b->setChecked(false);
            btn->setChecked(true);
            m_viewer->setMatColor(m_presets[i].face, m_presets[i].side, m_presets[i].top);
        });
        matLay->addWidget(btn);
    }
    // Appliquer le preset par défaut
    m_viewer->setMatColor(m_presets[defaultPreset].face,
                          m_presets[defaultPreset].side,
                          m_presets[defaultPreset].top);

    rightLay->addWidget(matBox);

    // ── Zoom manuel ───────────────────────────────────────────────────────────
    QGroupBox *zoomBox = new QGroupBox("ZOOM");
    QVBoxLayout *zoomLay = new QVBoxLayout(zoomBox);
    QSlider *zoomSlider = new QSlider(Qt::Horizontal);
    zoomSlider->setRange(30, 350);
    zoomSlider->setValue(100);
    connect(zoomSlider, &QSlider::valueChanged, this, [this](int v){
        m_viewer->m_zoom = v / 100.0;
        m_viewer->update();
    });
    zoomLay->addWidget(zoomSlider);

    QHBoxLayout *zoomLabels = new QHBoxLayout();
    QLabel *z1 = new QLabel("0.3×"); z1->setStyleSheet("color:#475569;font-size:9px;");
    QLabel *z2 = new QLabel("3.5×"); z2->setStyleSheet("color:#475569;font-size:9px;");
    zoomLabels->addWidget(z1); zoomLabels->addStretch(); zoomLabels->addWidget(z2);
    zoomLay->addLayout(zoomLabels);
    rightLay->addWidget(zoomBox);

    // ── Fiche technique ───────────────────────────────────────────────────────
    QGroupBox *ficheBox = new QGroupBox("FICHE TECHNIQUE");
    QVBoxLayout *ficheLay = new QVBoxLayout(ficheBox);
    ficheLay->setSpacing(6);

    auto addRow = [&](const QString& label, const QString& val) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("color:#64748b; font-size:10px;");
        lbl->setMinimumWidth(80);
        QLabel *vlbl = new QLabel(val.isEmpty() ? "—" : val);
        vlbl->setStyleSheet("color:#cbd5e1; font-size:10px; font-weight:bold;");
        vlbl->setWordWrap(true);
        row->addWidget(lbl);
        row->addWidget(vlbl, 1);
        ficheLay->addLayout(row);
    };

    addRow("Matériau :",   m_produit.getMatUtilise());
    addRow("Dimensions :", m_produit.getDimensions());
    addRow("Prix :",       QString("%1 €").arg(m_produit.getPrix(), 0, 'f', 2));
    addRow("Projet :",     QString("#%1").arg(m_produit.getProjetId()));
    addRow("Créé le :",    m_produit.getDateCreation().isValid()
                            ? m_produit.getDateCreation().toString("dd/MM/yyyy") : "—");

    rightLay->addWidget(ficheBox);
    rightLay->addStretch();

    // ── Bouton fermer ─────────────────────────────────────────────────────────
    QPushButton *closeBtn = new QPushButton("✕   Fermer");
    closeBtn->setStyleSheet(
        "QPushButton{background:#ef4444;color:white;border:none;border-radius:8px;"
        "padding:10px;font-weight:bold;font-size:12px;}"
        "QPushButton:hover{background:#dc2626;}");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    rightLay->addWidget(closeBtn);

    root->addLayout(rightLay, 1);

    // ── Timer auto-rotation ───────────────────────────────────────────────────
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, m_viewer, &Viewer3DWidget::autoRotateStep);
    m_timer->start(30);
}
