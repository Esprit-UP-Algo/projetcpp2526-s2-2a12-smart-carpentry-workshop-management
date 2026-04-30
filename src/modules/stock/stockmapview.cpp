#include "stockmapview.h"
#include "stocklocales.h"
#include "src/database/stockdatabase.h"

#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariantMap>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QScrollArea>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QMouseEvent>
#include <QToolTip>
#include <QDebug>
#include <QTimer>

// ============================================================================
//  Helpers
// ============================================================================

static bool isDarkMode()
{
    return qApp->styleSheet().contains("0f0f0f");
}

// ============================================================================
//  IsometricWarehouseWidget
// ============================================================================

IsometricWarehouseWidget::IsometricWarehouseWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(220);
    setAttribute(Qt::WA_TranslucentBackground);
}

void IsometricWarehouseWidget::setMaterials(const QList<StockMaterial>& materials)
{
    // Build lookup maps
    QMap<QString, int>     matId;
    QMap<QString, QString> matName;
    QMap<QString, double>  matQty;
    QMap<QString, double>  matThresh;
    QMap<QString, bool>    matAlert;

    for (const StockMaterial& m : materials) {
        if (m.getEmplacement().isEmpty()) continue;
        const QString& emp = m.getEmplacement();
        matId[emp]     = m.getId();
        matName[emp]   = m.getNom();
        matQty[emp]    = m.getQuantite();
        matThresh[emp] = m.getSeuilAlerte();
        matAlert[emp]  = m.isBelowAlert();
    }

    m_slots.clear();
    m_hoveredIndex = -1;

    const QStringList& codes = StockLocales::emplacements(); // A1…D5
    for (const QString& code : codes) {
        SlotInfo s;
        s.code         = code;
        s.occupied     = matId.contains(code);
        s.materialId   = matId.value(code, 0);
        s.materialName = matName.value(code, "");
        s.quantity     = matQty.value(code, 0.0);
        s.threshold    = matThresh.value(code, 0.0);
        s.alert        = matAlert.value(code, false);
        m_slots.append(s);
    }

    rebuildGeometry();
    update();
}

void IsometricWarehouseWidget::clear()
{
    m_slots.clear();
    m_hoveredIndex = -1;
    update();
}

// ---------------------------------------------------------------------------
//  isoProject : convert grid (x,y,z) → screen (px,py)
// ---------------------------------------------------------------------------
QPointF IsometricWarehouseWidget::isoProject(double x, double y, double z,
                                             double originX, double originY,
                                             double tileW,  double tileH) const
{
    double screenX = originX + (x - y) * tileW / 2.0;
    double screenY = originY + (x + y) * tileH / 2.0 - z * tileH;
    return QPointF(screenX, screenY);
}

// ---------------------------------------------------------------------------
//  rebuildGeometry : compute the three faces of each isometric cube
// ---------------------------------------------------------------------------
void IsometricWarehouseWidget::rebuildGeometry()
{
    if (m_slots.isEmpty()) return;

    int   w      = width()  > 0 ? width()  : 320;
    int   h      = height() > 0 ? height() : 220;

    // Grid is 5 columns (x) × 4 rows (y)
    const int COLS = 5;
    const int ROWS = 4;

    // Tile dimensions (isometric projected)
    double tileW = (double)(w - 60) / (COLS + ROWS) * 1.4;
    double tileH = tileW * 0.55;
    double cubeH = tileH * 1.1; // height of the vertical faces

    // Origin: center-top of the widget, pushed down a bit
    double originX = w / 2.0;
    double originY = 28.0 + ROWS * tileH / 2.0;

    for (int i = 0; i < m_slots.size(); i++) {
        // code is "A1" … "D5" — row = letter (A=0), col = digit-1
        int row = m_slots[i].code[0].toLatin1() - 'A'; // 0..3
        int col = m_slots[i].code[1].digitValue() - 1; // 0..4

        // The four corners of the top face (z = cubeH/tileH level)
        double cx = col, cy = row;
        double z  = 1.0; // cube height in grid units

        QPointF p00 = isoProject(cx,     cy,     z, originX, originY, tileW, tileH);
        QPointF p10 = isoProject(cx + 1, cy,     z, originX, originY, tileW, tileH);
        QPointF p11 = isoProject(cx + 1, cy + 1, z, originX, originY, tileW, tileH);
        QPointF p01 = isoProject(cx,     cy + 1, z, originX, originY, tileW, tileH);

        // Bottom corners (z = 0)
        QPointF b00 = isoProject(cx,     cy,     0, originX, originY, tileW, tileH);
        QPointF b10 = isoProject(cx + 1, cy,     0, originX, originY, tileW, tileH);
        QPointF b11 = isoProject(cx + 1, cy + 1, 0, originX, originY, tileW, tileH);
        QPointF b01 = isoProject(cx,     cy + 1, 0, originX, originY, tileW, tileH);

        // Top face
        m_slots[i].topFace.clear();
        m_slots[i].topFace << p00 << p10 << p11 << p01;

        // Left face (x=col, y=col..col+1, z=0..1) — visible left side
        m_slots[i].leftFace.clear();
        m_slots[i].leftFace << p01 << p11 << b11 << b01;

        // Right face (y=col+1, x=col..col+1, z=0..1) — visible right side
        m_slots[i].rightFace.clear();
        m_slots[i].rightFace << p10 << p11 << b11 << b10;
    }
}

// ---------------------------------------------------------------------------
//  paintEvent
// ---------------------------------------------------------------------------
void IsometricWarehouseWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    bool dark = isDarkMode();

    if (m_slots.isEmpty()) {
        p.setPen(dark ? QColor("#6b7280") : QColor("#9ca3af"));
        QFont f; f.setPixelSize(12); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "Aucun emplacement disponible");
        return;
    }

    // Row/col label colors
    QColor labelColor = dark ? QColor("#6b7280") : QColor("#9ca3af");

    // Draw slots back-to-front (painter's algorithm — row 0 col 0 first,
    // last row last col last so closer tiles paint over farther ones)
    for (int i = 0; i < m_slots.size(); i++) {
        const SlotInfo& s  = m_slots[i];
        bool  hovered      = (i == m_hoveredIndex);

        // Choose face colors based on state
        QColor topColor, leftColor, rightColor;

        if (!s.occupied) {
            // Empty slot — muted
            topColor   = dark ? QColor("#2a2a2a") : QColor("#f3f4f6");
            leftColor  = dark ? QColor("#1f1f1f") : QColor("#e5e7eb");
            rightColor = dark ? QColor("#252525") : QColor("#e9eaec");
        } else if (s.alert) {
            // Alert — red palette
            topColor   = hovered ? QColor("#fca5a5") : QColor("#ef4444");
            leftColor  = QColor("#b91c1c");
            rightColor = QColor("#dc2626");
        } else {
            // OK — sage green palette (matches app theme)
            topColor   = hovered ? QColor("#bbf7d0") : QColor("#8A9A5B");
            leftColor  = QColor("#5a6a3b");
            rightColor = QColor("#6a7a4b");
        }

        // Highlight hover with a slight brightness boost
        if (hovered && s.occupied) {
            topColor   = topColor.lighter(130);
            leftColor  = leftColor.lighter(115);
            rightColor = rightColor.lighter(115);
        }

        // Draw the three faces
        p.setPen(Qt::NoPen);

        // Right face (drawn first — farthest in isometric view)
        p.setBrush(rightColor);
        p.drawPolygon(s.rightFace);

        // Left face
        p.setBrush(leftColor);
        p.drawPolygon(s.leftFace);

        // Top face
        p.setBrush(topColor);
        p.drawPolygon(s.topFace);

        // Outline
        QPen outline(dark ? QColor(255,255,255,25) : QColor(0,0,0,30), 0.8);
        p.setPen(outline);
        p.setBrush(Qt::NoBrush);
        p.drawPolygon(s.topFace);
        p.drawPolygon(s.leftFace);
        p.drawPolygon(s.rightFace);

        // Slot code label on top face
        if (!s.topFace.isEmpty()) {
            QRectF tf = s.topFace.boundingRect();
            QFont  cf; cf.setPixelSize(8); cf.setBold(true);
            p.setFont(cf);
            QColor textCol;
            if (!s.occupied) {
                textCol = labelColor;
            } else if (s.alert) {
                textCol = hovered ? QColor("#7f1d1d") : QColor("#ffffff");
            } else {
                textCol = hovered ? QColor("#14532d") : QColor("#ffffff");
            }
            p.setPen(textCol);
            p.drawText(tf, Qt::AlignCenter, s.code);
        }
    }

    // ── Draw row labels (A-D) and column labels (1-5) ──────────────────────
    QFont lf; lf.setPixelSize(9);
    p.setFont(lf);
    p.setPen(labelColor);

    // We use the first slot of each row/col for reference positions
    // Row labels: place to the left of column 0 for each row
    for (int row = 0; row < 4; ++row) {
        // Find slot index for this row, col 0
        int idx = row * 5; // slots are ordered A1,A2,...,A5,B1,...
        if (idx < m_slots.size() && !m_slots[idx].leftFace.isEmpty()) {
            // Left-most point of the left face
            QPointF ref = m_slots[idx].leftFace[0]; // top-left of left face
            QChar rowChar('A' + row);
            p.drawText(QRectF(ref.x() - 22, ref.y() - 8, 18, 16),
                       Qt::AlignRight | Qt::AlignVCenter,
                       QString(rowChar));
        }
    }
    // Col labels: place above row 0 for each col
    for (int col = 0; col < 5; ++col) {
        int idx = col; // row 0, col N
        if (idx < m_slots.size() && !m_slots[idx].topFace.isEmpty()) {
            QRectF tf = m_slots[idx].topFace.boundingRect();
            p.drawText(QRectF(tf.center().x() - 8, tf.top() - 16, 16, 14),
                       Qt::AlignCenter,
                       QString::number(col + 1));
        }
    }
}

// ---------------------------------------------------------------------------
//  mouse events
// ---------------------------------------------------------------------------
void IsometricWarehouseWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pos = event->position();
    int prev    = m_hoveredIndex;
    m_hoveredIndex = -1;

    // Test top-face of each slot (back-to-front order — last match wins for
    // the visually topmost slot)
    for (int i = 0; i < m_slots.size(); i++) {
        if (m_slots[i].topFace.containsPoint(pos, Qt::OddEvenFill) ||
            m_slots[i].leftFace.containsPoint(pos, Qt::OddEvenFill) ||
            m_slots[i].rightFace.containsPoint(pos, Qt::OddEvenFill)) {
            m_hoveredIndex = i;
        }
    }

    if (m_hoveredIndex != prev) {
        update();
        if (m_hoveredIndex >= 0) {
            const SlotInfo& s = m_slots[m_hoveredIndex];
            QString tip;
            if (!s.occupied) {
                tip = QString("Emplacement %1 — Libre").arg(s.code);
            } else {
                tip = QString("Emplacement %1\n%2\nStock : %3  |  Seuil : %4%5")
                .arg(s.code)
                    .arg(s.materialName)
                    .arg(s.quantity, 0, 'f', 1)
                    .arg(s.threshold, 0, 'f', 1)
                    .arg(s.alert ? "\nALERTE : stock insuffisant" : "");
            }
            QToolTip::showText(event->globalPosition().toPoint(), tip, this);
        } else {
            QToolTip::hideText();
        }
    }
}

void IsometricWarehouseWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;
    if (m_hoveredIndex >= 0 && m_slots[m_hoveredIndex].occupied) {
        const SlotInfo& s = m_slots[m_hoveredIndex];
        emit slotClicked(s.code, s.materialId);
    }
}

void IsometricWarehouseWidget::leaveEvent(QEvent *)
{
    m_hoveredIndex = -1;
    QToolTip::hideText();
    update();
}

// ============================================================================
//  WarehouseDetailPanel
// ============================================================================

WarehouseDetailPanel::WarehouseDetailPanel(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("warehouseDetailPanel");
    setMinimumWidth(300);
    setMaximumWidth(360);

    bool dark = isDarkMode();

    setStyleSheet(dark
                      ? "QFrame#warehouseDetailPanel { background: #141414; border-left: 1px solid #2a2a2a; }"
                      : "QFrame#warehouseDetailPanel { background: #f9fafb; border-left: 1px solid #e5e7eb; }");

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Header ─────────────────────────────────────────────────────────────
    QFrame *header = new QFrame(this);
    header->setStyleSheet(dark
                              ? "background: #1a1a1a; border-bottom: 1px solid #2a2a2a; padding: 0px;"
                              : "background: #ffffff; border-bottom: 1px solid #e5e7eb; padding: 0px;");
    QVBoxLayout *hLay = new QVBoxLayout(header);
    hLay->setContentsMargins(16, 14, 16, 14);
    hLay->setSpacing(4);

    m_titleLabel = new QLabel("Selectionnez un entrepot", header);
    {
        QFont f = m_titleLabel->font();
        f.setPixelSize(15); f.setBold(true);
        m_titleLabel->setFont(f);
    }
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setStyleSheet(dark ? "color: #f9fafb;" : "color: #111827;");
    hLay->addWidget(m_titleLabel);

    m_statsLabel = new QLabel("", header);
    {
        QFont f = m_statsLabel->font(); f.setPixelSize(11);
        m_statsLabel->setFont(f);
    }
    m_statsLabel->setStyleSheet("color: #6b7280;");
    hLay->addWidget(m_statsLabel);

    root->addWidget(header);

    // ── Isometric warehouse widget ──────────────────────────────────────────
    QFrame *isoFrame = new QFrame(this);
    isoFrame->setStyleSheet(dark
                                ? "background: #111111; border-bottom: 1px solid #2a2a2a;"
                                : "background: #f3f4f6; border-bottom: 1px solid #e5e7eb;");
    QVBoxLayout *isoLay = new QVBoxLayout(isoFrame);
    isoLay->setContentsMargins(8, 8, 8, 8);
    isoLay->setSpacing(4);

    // Legend row above the 3D grid
    QHBoxLayout *legendRow = new QHBoxLayout();
    legendRow->setSpacing(12);
    auto makeLegendDot = [&](const QString& color, const QString& label) {
        QWidget *w = new QWidget(isoFrame);
        QHBoxLayout *l = new QHBoxLayout(w);
        l->setContentsMargins(0,0,0,0); l->setSpacing(4);
        QLabel *dot = new QLabel(w);
        dot->setFixedSize(9, 9);
        dot->setStyleSheet(QString("background:%1; border-radius:4px;").arg(color));
        QLabel *txt = new QLabel(label, w);
        QFont f = txt->font(); f.setPixelSize(9); txt->setFont(f);
        txt->setStyleSheet("color: #6b7280;");
        l->addWidget(dot); l->addWidget(txt);
        return w;
    };
    legendRow->addWidget(makeLegendDot("#8A9A5B", "Stock OK"));
    legendRow->addWidget(makeLegendDot("#ef4444", "Alerte"));
    legendRow->addWidget(makeLegendDot(dark ? "#2a2a2a" : "#e5e7eb", "Libre"));
    legendRow->addStretch();
    isoLay->addLayout(legendRow);

    m_isoWidget = new IsometricWarehouseWidget(isoFrame);
    isoLay->addWidget(m_isoWidget);

    // Connect slot click → materialSelected
    connect(m_isoWidget, &IsometricWarehouseWidget::slotClicked,
            [this](const QString&, int matId) {
                if (matId > 0) emit materialSelected(matId);
            });

    root->addWidget(isoFrame);

    // ── Material list ───────────────────────────────────────────────────────
    // Section title
    QFrame *listHeader = new QFrame(this);
    listHeader->setStyleSheet(dark
                                  ? "background: #1a1a1a; border-bottom: 1px solid #2a2a2a; padding: 0px;"
                                  : "background: #ffffff; border-bottom: 1px solid #f3f4f6; padding: 0px;");
    QHBoxLayout *lhLay = new QHBoxLayout(listHeader);
    lhLay->setContentsMargins(16, 8, 16, 8);
    QLabel *listTitle = new QLabel("Materiaux", listHeader);
    {
        QFont f = listTitle->font(); f.setPixelSize(11); f.setBold(true);
        listTitle->setFont(f);
    }
    listTitle->setStyleSheet("color: #6b7280; text-transform: uppercase; letter-spacing: 1px;");
    lhLay->addWidget(listTitle);
    root->addWidget(listHeader);

    m_listScroll = new QScrollArea(this);
    m_listScroll->setWidgetResizable(true);
    m_listScroll->setFrameShape(QFrame::NoFrame);
    m_listScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listScroll->setStyleSheet("background: transparent;");

    m_listContainer = new QWidget();
    m_listContainer->setStyleSheet("background: transparent;");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(10, 8, 10, 8);
    m_listLayout->setSpacing(6);
    m_listLayout->addStretch();

    m_listScroll->setWidget(m_listContainer);
    root->addWidget(m_listScroll, 1);
}

void WarehouseDetailPanel::clear()
{
    m_titleLabel->setText("Selectionnez un entrepot");
    m_statsLabel->clear();
    m_isoWidget->clear();

    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_listLayout->addStretch();
}

void WarehouseDetailPanel::showLocale(const QString& localeName)
{
    clear();

    const QList<StockMaterial> materials =
        StockDatabase::instance().getMaterialsByLocale(localeName);

    int alertCount = 0;
    for (const StockMaterial& m : materials)
        if (m.isBelowAlert()) ++alertCount;

    m_titleLabel->setText(localeName);
    if (materials.isEmpty()) {
        m_statsLabel->setText("Aucun materiau enregistre");
    } else {
        m_statsLabel->setText(
            QString("%1 materiau%2%3")
                .arg(materials.size())
                .arg(materials.size() > 1 ? "x" : "")
                .arg(alertCount > 0
                         ? QString("  —  %1 alerte%2").arg(alertCount).arg(alertCount > 1 ? "s" : "")
                         : ""));
    }

    m_isoWidget->setMaterials(materials);
    buildList(materials);
}

void WarehouseDetailPanel::buildList(const QList<StockMaterial>& materials)
{
    bool dark = isDarkMode();

    // Remove old stretch
    QLayoutItem *last = m_listLayout->itemAt(m_listLayout->count() - 1);
    if (last && last->spacerItem()) {
        m_listLayout->removeItem(last);
        delete last;
    }

    if (materials.isEmpty()) {
        QLabel *empty = new QLabel("Aucun materiau dans cet entrepot.", m_listContainer);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color: #6b7280; padding: 20px;");
        m_listLayout->addWidget(empty);
        m_listLayout->addStretch();
        return;
    }

    for (const StockMaterial& mat : materials) {
        QFrame *card = new QFrame(m_listContainer);
        card->setObjectName("matCard");
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(dark
                                ? "QFrame#matCard { background:#1e1e1e; border-radius:8px; border:1px solid #2a2a2a; }"
                                  "QFrame#matCard:hover { border:1px solid #8A9A5B; background:#222222; }"
                                : "QFrame#matCard { background:#ffffff; border-radius:8px; border:1px solid #e5e7eb; }"
                                  "QFrame#matCard:hover { border:1px solid #8A9A5B; background:#fafff7; }");

        QVBoxLayout *vl = new QVBoxLayout(card);
        vl->setContentsMargins(12, 9, 12, 9);
        vl->setSpacing(5);

        // Row 1: name + slot badge + alert indicator
        QHBoxLayout *row1 = new QHBoxLayout();
        row1->setSpacing(6);

        QLabel *nomLbl = new QLabel(mat.getNom(), card);
        {
            QFont f = nomLbl->font(); f.setPixelSize(12); f.setBold(true);
            nomLbl->setFont(f);
        }
        nomLbl->setStyleSheet(dark ? "color:#f0f0f0;" : "color:#1f2937;");
        row1->addWidget(nomLbl, 1);

        // Slot badge
        if (!mat.getEmplacement().isEmpty()) {
            QLabel *slotBadge = new QLabel(mat.getEmplacement(), card);
            {
                QFont f = slotBadge->font(); f.setPixelSize(9); f.setBold(true);
                slotBadge->setFont(f);
            }
            slotBadge->setStyleSheet(
                "color: #8A9A5B; background: rgba(138,154,91,0.12);"
                "border: 1px solid rgba(138,154,91,0.3); border-radius: 4px; padding: 1px 5px;");
            row1->addWidget(slotBadge);
        }

        // Alert indicator
        if (mat.isBelowAlert()) {
            QLabel *alertDot = new QLabel(card);
            alertDot->setFixedSize(8, 8);
            alertDot->setStyleSheet("background: #ef4444; border-radius: 4px;");
            row1->addWidget(alertDot);
        }
        vl->addLayout(row1);

        // Row 2: stock progress bar
        double ratio = mat.getSeuilAlerte() > 0
                           ? mat.getQuantite() / mat.getSeuilAlerte() : 1.0;
        QProgressBar *bar = new QProgressBar(card);
        bar->setFixedHeight(4);
        bar->setRange(0, 100);
        bar->setValue(qBound(0, (int)(ratio * 100), 100));
        bar->setTextVisible(false);
        QString barClr = mat.isBelowAlert() ? "#ef4444" : "#8A9A5B";
        bar->setStyleSheet(QString(
                               "QProgressBar { background:%1; border-radius:2px; }"
                               "QProgressBar::chunk { background:%2; border-radius:2px; }"
                               ).arg(dark ? "#2a2a2a" : "#e5e7eb", barClr));
        vl->addWidget(bar);

        // Row 3: detail text
        QLabel *detail = new QLabel(
            QString("%1 %2  —  seuil %3 %4")
                .arg(mat.getQuantite(), 0, 'f', 1).arg(mat.getUnite())
                .arg(mat.getSeuilAlerte(), 0, 'f', 1).arg(mat.getUnite()),
            card);
        {
            QFont f = detail->font(); f.setPixelSize(10);
            detail->setFont(f);
        }
        detail->setStyleSheet(dark ? "color:#6b7280;" : "color:#9ca3af;");
        vl->addWidget(detail);

        m_listLayout->addWidget(card);
        card->installEventFilter(this);
        card->setProperty("materialId", mat.getId());
    }
    m_listLayout->addStretch();
}

bool WarehouseDetailPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QFrame *card = qobject_cast<QFrame*>(watched);
        if (card) {
            int id = card->property("materialId").toInt();
            emit materialSelected(id);
            return true;
        }
    }
    return QFrame::eventFilter(watched, event);
}

// ============================================================================
//  StockMapView
// ============================================================================

StockMapView::StockMapView(QWidget *parent)
    : QWidget(parent)
{
    bool dark = isDarkMode();

    QVBoxLayout *rootLay = new QVBoxLayout(this);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);

    // ── Top toolbar: search + filter pills + locale jump ─────────────────────
    QFrame *toolbar = new QFrame(this);
    toolbar->setObjectName("mapToolbar");
    toolbar->setStyleSheet(dark
                               ? "QFrame#mapToolbar { background: #1a1a1a; border-bottom: 1px solid #2a2a2a; }"
                               : "QFrame#mapToolbar { background: #ffffff; border-bottom: 1px solid #e5e7eb; }");
    toolbar->setFixedHeight(54);

    QHBoxLayout *tbLay = new QHBoxLayout(toolbar);
    tbLay->setContentsMargins(16, 8, 16, 8);
    tbLay->setSpacing(10);

    // Search icon label
    QLabel *searchIcon = new QLabel("  Rechercher", toolbar);
    {
        QFont f = searchIcon->font(); f.setPixelSize(12);
        searchIcon->setFont(f);
    }
    searchIcon->setStyleSheet("color: #6b7280;");
    tbLay->addWidget(searchIcon);

    // Search input
    m_searchEdit = new QLineEdit(toolbar);
    m_searchEdit->setPlaceholderText("Nom de materiau ou entrepot...");
    m_searchEdit->setFixedHeight(34);
    m_searchEdit->setMinimumWidth(220);
    m_searchEdit->setStyleSheet(dark
                                    ? "QLineEdit { background:#252525; border:1.5px solid #3a3a3a; border-radius:8px;"
                                      "  padding:0 10px; font-size:12px; color:#f9fafb; }"
                                      "QLineEdit:focus { border-color:#8A9A5B; }"
                                    : "QLineEdit { background:#f9fafb; border:1.5px solid #e5e7eb; border-radius:8px;"
                                      "  padding:0 10px; font-size:12px; color:#111827; }"
                                      "QLineEdit:focus { border-color:#8A9A5B; background:#fff; }");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &StockMapView::onSearchChanged);
    tbLay->addWidget(m_searchEdit);

    // Separator
    QFrame *sep = new QFrame(toolbar);
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet(dark ? "color:#3a3a3a;" : "color:#e5e7eb;");
    tbLay->addWidget(sep);

    // Filter pills
    auto makePill = [&](const QString& label) -> QPushButton* {
        QPushButton *btn = new QPushButton(label, toolbar);
        btn->setFixedHeight(30);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(false);
        return btn;
    };

    QString pillBase = dark
                           ? "QPushButton { background:#252525; color:#9ca3af; border:1.5px solid #3a3a3a;"
                             "  border-radius:15px; padding:0 14px; font-size:11px; font-weight:600; }"
                             "QPushButton:hover { border-color:#8A9A5B; color:#f9fafb; }"
                           : "QPushButton { background:#f3f4f6; color:#6b7280; border:1.5px solid #e5e7eb;"
                             "  border-radius:15px; padding:0 14px; font-size:11px; font-weight:600; }"
                             "QPushButton:hover { border-color:#8A9A5B; color:#374151; }";

    QString pillActive = dark
                             ? "QPushButton { background:rgba(138,154,91,0.15); color:#8A9A5B; border:1.5px solid #8A9A5B;"
                               "  border-radius:15px; padding:0 14px; font-size:11px; font-weight:600; }"
                             : "QPushButton { background:rgba(138,154,91,0.12); color:#5a6a3b; border:1.5px solid #8A9A5B;"
                               "  border-radius:15px; padding:0 14px; font-size:11px; font-weight:600; }";

    m_filterAll   = makePill("Tous");
    m_filterAlert = makePill("En alerte");
    m_filterOk    = makePill("Stock OK");

    // Store style strings as properties so we can toggle them
    m_filterAll->setProperty("pillBase",   pillBase);
    m_filterAll->setProperty("pillActive", pillActive);
    m_filterAlert->setProperty("pillBase",   pillBase);
    m_filterAlert->setProperty("pillActive", pillActive);
    m_filterOk->setProperty("pillBase",   pillBase);
    m_filterOk->setProperty("pillActive", pillActive);

    // Default: "Tous" is active
    m_filterAll->setStyleSheet(pillActive);
    m_filterAlert->setStyleSheet(pillBase);
    m_filterOk->setStyleSheet(pillBase);

    connect(m_filterAll,   &QPushButton::clicked, this, &StockMapView::onFilterAll);
    connect(m_filterAlert, &QPushButton::clicked, this, &StockMapView::onFilterAlert);
    connect(m_filterOk,    &QPushButton::clicked, this, &StockMapView::onFilterOk);

    tbLay->addWidget(m_filterAll);
    tbLay->addWidget(m_filterAlert);
    tbLay->addWidget(m_filterOk);
    tbLay->addStretch();

    // Jump-to locale combo
    QLabel *jumpLbl = new QLabel("Aller a :", toolbar);
    {
        QFont f = jumpLbl->font(); f.setPixelSize(11);
        jumpLbl->setFont(f);
    }
    jumpLbl->setStyleSheet("color: #6b7280;");
    tbLay->addWidget(jumpLbl);

    m_localeCombo = new QComboBox(toolbar);
    m_localeCombo->setFixedHeight(32);
    m_localeCombo->setMinimumWidth(150);
    m_localeCombo->addItem("— Selectionner —", "");
    for (const LocaleInfo& info : StockLocales::locales())
        m_localeCombo->addItem(info.nom, info.nom);
    m_localeCombo->setStyleSheet(dark
                                     ? "QComboBox { background:#252525; border:1.5px solid #3a3a3a; border-radius:8px;"
                                       "  padding:0 8px; font-size:11px; color:#f9fafb; }"
                                       "QComboBox:focus { border-color:#8A9A5B; }"
                                       "QComboBox::drop-down { border:none; width:22px; }"
                                       "QComboBox::down-arrow { border-left:4px solid transparent; border-right:4px solid transparent;"
                                       "  border-top:5px solid #9ca3af; margin-right:6px; }"
                                       "QComboBox QAbstractItemView { background:#1a1a1a; border:1px solid #2a2a2a;"
                                       "  selection-background-color:rgba(138,154,91,0.15); color:#f9fafb; }"
                                     : "QComboBox { background:#f9fafb; border:1.5px solid #e5e7eb; border-radius:8px;"
                                       "  padding:0 8px; font-size:11px; color:#111827; }"
                                       "QComboBox:focus { border-color:#8A9A5B; background:#fff; }"
                                       "QComboBox::drop-down { border:none; width:22px; }"
                                       "QComboBox::down-arrow { border-left:4px solid transparent; border-right:4px solid transparent;"
                                       "  border-top:5px solid #6b7280; margin-right:6px; }"
                                       "QComboBox QAbstractItemView { background:#fff; border:1px solid #e5e7eb;"
                                       "  selection-background-color:rgba(138,154,91,0.1); color:#111827; }");

    // When a locale is chosen from combo → emit localeClicked to fly map there
    connect(m_localeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int idx) {
                QString name = m_localeCombo->itemData(idx).toString();
                if (!name.isEmpty()) {
                    emit localeClicked(name);
                    // Also highlight it in QML
                    if (QQuickItem *root = m_quickWidget->rootObject())
                        root->setProperty("highlightedLocale", name);
                    // Reset combo to placeholder after a moment
                    QTimer::singleShot(300, [this]{ m_localeCombo->setCurrentIndex(0); });
                }
            });
    tbLay->addWidget(m_localeCombo);

    rootLay->addWidget(toolbar);

    // ── Stats mini-bar ────────────────────────────────────────────────────────
    QFrame *statsBar = new QFrame(this);
    statsBar->setObjectName("mapStatsBar");
    statsBar->setFixedHeight(36);
    statsBar->setStyleSheet(dark
                                ? "QFrame#mapStatsBar { background:#111111; border-bottom:1px solid #2a2a2a; }"
                                : "QFrame#mapStatsBar { background:#f9fafb; border-bottom:1px solid #f0f0f0; }");

    QHBoxLayout *sbLay = new QHBoxLayout(statsBar);
    sbLay->setContentsMargins(20, 0, 20, 0);
    sbLay->setSpacing(24);

    auto makeStatLabel = [&](const QString& prefix) -> QLabel* {
        QLabel *lbl = new QLabel(prefix + " —", statsBar);
        QFont f = lbl->font(); f.setPixelSize(11); lbl->setFont(f);
        lbl->setStyleSheet("color: #6b7280;");
        return lbl;
    };

    m_statEntrepots = makeStatLabel("Entrepots");
    m_statMaterials = makeStatLabel("Materiaux");
    m_statAlerts    = makeStatLabel("Alertes");

    sbLay->addWidget(m_statEntrepots);
    sbLay->addWidget(m_statMaterials);
    sbLay->addWidget(m_statAlerts);
    sbLay->addStretch();
    rootLay->addWidget(statsBar);

    // ── Body: map + side panel ────────────────────────────────────────────────
    QHBoxLayout *body = new QHBoxLayout();
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop);

    connect(m_quickWidget, &QQuickWidget::statusChanged,
            this, &StockMapView::onQmlStatusChanged);

    m_quickWidget->setSource(QUrl("qrc:/WoodFlowApp/src/modules/stock/qml/MapView.qml"));

    body->addWidget(m_quickWidget, 1);
    rootLay->addLayout(body, 1);
}

// ---------------------------------------------------------------------------
void StockMapView::onQmlStatusChanged(QQuickWidget::Status status)
{
    if (status == QQuickWidget::Ready) {
        connectQmlSignals();
        refresh();
    } else if (status == QQuickWidget::Error) {
        qWarning() << "[StockMapView] QML errors:";
        for (const QQmlError& e : m_quickWidget->errors())
            qWarning() << "  " << e.toString();
    }
}

// ---------------------------------------------------------------------------
void StockMapView::connectQmlSignals()
{
    QQuickItem *root = m_quickWidget->rootObject();
    if (!root) {
        qWarning() << "[StockMapView] rootObject() is null after Ready";
        return;
    }
    connect(root, SIGNAL(markerClicked(QString)),
            this, SLOT(onMarkerClicked(QString)));
}

// ---------------------------------------------------------------------------
void StockMapView::refresh()
{
    m_allMarkersData = buildMarkersData();
    applyFilter();
}

// ---------------------------------------------------------------------------
void StockMapView::onMarkerClicked(const QString& localeName)
{
    emit localeClicked(localeName);
}

// ---------------------------------------------------------------------------
void StockMapView::onSearchChanged(const QString& text)
{
    applyFilter();

    // Highlight in QML
    if (QQuickItem *root = m_quickWidget->rootObject())
        root->setProperty("highlightedLocale", text.trimmed());
}

// ---------------------------------------------------------------------------
void StockMapView::onFilterAll()
{
    m_activeFilter = "all";
    setFilterButtonActive(m_filterAll);
    applyFilter();
}

void StockMapView::onFilterAlert()
{
    m_activeFilter = "alert";
    setFilterButtonActive(m_filterAlert);
    applyFilter();
}

void StockMapView::onFilterOk()
{
    m_activeFilter = "ok";
    setFilterButtonActive(m_filterOk);
    applyFilter();
}

// ---------------------------------------------------------------------------
void StockMapView::setFilterButtonActive(QPushButton *btn)
{
    QString active = btn->property("pillActive").toString();
    QString base   = btn->property("pillBase").toString();

    for (QPushButton *b : {m_filterAll, m_filterAlert, m_filterOk}) {
        b->setStyleSheet(b == btn ? active : base);
    }
}

// ---------------------------------------------------------------------------
void StockMapView::applyFilter()
{
    QString search = m_searchEdit ? m_searchEdit->text().trimmed().toLower() : "";

    QVariantList filtered;
    for (const QVariant& v : m_allMarkersData) {
        QVariantMap entry = v.toMap();
        QString name      = entry["name"].toString();
        int alertCount    = entry["alertCount"].toInt();
        int total         = entry["total"].toInt();

        // Text search filter (locale name)
        if (!search.isEmpty() && !name.toLower().contains(search))
            continue;

        // Status filter
        if (m_activeFilter == "alert" && alertCount == 0)
            continue;
        if (m_activeFilter == "ok" && alertCount > 0)
            continue;

        filtered.append(v);
    }

    // If search is non-empty, also try to match materials by name and include their locale
    if (!search.isEmpty()) {
        const QList<StockMaterial> all = StockDatabase::instance().getAllMaterials();
        QSet<QString> matchedLocales;
        for (const StockMaterial& m : all) {
            if (m.getNom().toLower().contains(search) && !m.getLocale().isEmpty())
                matchedLocales.insert(m.getLocale());
        }
        // Add any locales found via material name that aren't already in filtered
        QSet<QString> alreadyIn;
        for (const QVariant& v : filtered)
            alreadyIn.insert(v.toMap()["name"].toString());

        for (const QVariant& v : m_allMarkersData) {
            QVariantMap entry = v.toMap();
            QString name = entry["name"].toString();
            if (matchedLocales.contains(name) && !alreadyIn.contains(name))
                filtered.append(v);
        }
    }

    pushMarkersToQml(filtered);
    updateStatBar(filtered);
}

// ---------------------------------------------------------------------------
void StockMapView::pushMarkersToQml(const QVariantList& data)
{
    m_markersData = data;
    QQuickItem *root = m_quickWidget->rootObject();
    if (root)
        root->setProperty("markersData", m_markersData);
    emit markersDataChanged();
}

// ---------------------------------------------------------------------------
void StockMapView::updateStatBar(const QVariantList& data)
{
    int totalEntrepots = data.size();
    int totalMaterials = 0;
    int totalAlerts    = 0;
    for (const QVariant& v : data) {
        QVariantMap m = v.toMap();
        totalMaterials += m["total"].toInt();
        totalAlerts    += m["alertCount"].toInt();
    }

    auto fmt = [](const QString& label, int val, const QString& color) -> QString {
        return QString("<span style='color:%3;'>%1</span>"
                       "<span style='color:#374151; font-weight:600;'> %2</span>")
            .arg(label).arg(val).arg(color);
    };

    if (m_statEntrepots)
        m_statEntrepots->setText(fmt("Entrepots actifs :", totalEntrepots, "#6b7280"));
    if (m_statMaterials)
        m_statMaterials->setText(fmt("Materiaux :", totalMaterials, "#6b7280"));
    if (m_statAlerts) {
        QString alertColor = totalAlerts > 0 ? "#ef4444" : "#8A9A5B";
        m_statAlerts->setText(fmt("Alertes :", totalAlerts, alertColor));
    }

    // Make labels rich-text
    if (m_statEntrepots) m_statEntrepots->setTextFormat(Qt::RichText);
    if (m_statMaterials) m_statMaterials->setTextFormat(Qt::RichText);
    if (m_statAlerts)    m_statAlerts->setTextFormat(Qt::RichText);
}

// ---------------------------------------------------------------------------
QVariantList StockMapView::buildMarkersData() const
{
    QMap<QString, int> totals = StockDatabase::instance().getCountByLocale();

    QMap<QString, int> alerts;
    const QList<StockMaterial> all = StockDatabase::instance().getAllMaterials();
    for (const StockMaterial& m : all) {
        if (m.isBelowAlert() && !m.getLocale().isEmpty())
            alerts[m.getLocale()]++;
    }

    QVariantList result;
    for (const LocaleInfo& info : StockLocales::locales()) {
        int total      = totals.value(info.nom, 0);
        int alertCount = alerts.value(info.nom, 0);
        if (total == 0) continue;

        QVariantMap entry;
        entry["name"]       = info.nom;
        entry["lat"]        = info.latitude;
        entry["lon"]        = info.longitude;
        entry["total"]      = total;
        entry["alertCount"] = alertCount;
        result.append(entry);
    }
    return result;
}