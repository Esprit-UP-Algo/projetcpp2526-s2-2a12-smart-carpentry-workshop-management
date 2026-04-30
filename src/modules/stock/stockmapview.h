#ifndef STOCKMAPVIEW_H
#define STOCKMAPVIEW_H

#include <QWidget>
#include <QQuickWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVariantList>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTimer>
#include <QPropertyAnimation>
#include <QSet>

#include "src/models/stockmaterial.h"

// ============================================================================
//  IsometricWarehouseWidget
//  Draws a 5×4 warehouse grid in isometric perspective using QPainter only.
//  Each slot is a 3-faced cube (top + left + right faces).
//  Hover → tooltip.  Click → emits slotClicked(emplacement).
// ============================================================================
class IsometricWarehouseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IsometricWarehouseWidget(QWidget *parent = nullptr);

    // Load materials for one locale — rebuilds internal state & repaints
    void setMaterials(const QList<StockMaterial>& materials);
    void clear();

signals:
    void slotClicked(const QString& emplacement, int materialId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QSize sizeHint() const override { return QSize(320, 220); }
    QSize minimumSizeHint() const override { return QSize(240, 160); }

private:
    struct SlotInfo {
        QString code;        // "A1" … "D5"
        int     materialId;  // 0 if empty
        QString materialName;
        double  quantity;
        double  threshold;
        bool    alert;
        bool    occupied;
        QPolygonF topFace;
        QPolygonF leftFace;
        QPolygonF rightFace;
    };

    QList<SlotInfo> m_slots;
    int             m_hoveredIndex = -1;

    void  rebuildGeometry();
    QPointF isoProject(double x, double y, double z,
                       double originX, double originY,
                       double tileW,  double tileH) const;
};

// ============================================================================
//  WarehouseDetailPanel
//  Side panel shown on map marker click.
//  Top section: isometric warehouse widget.
//  Bottom section: scrollable material list.
// ============================================================================
class WarehouseDetailPanel : public QFrame
{
    Q_OBJECT

public:
    explicit WarehouseDetailPanel(QWidget *parent = nullptr);

    void showLocale(const QString& localeName);
    void clear();

signals:
    void materialSelected(int materialId);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QLabel                   *m_titleLabel    = nullptr;
    QLabel                   *m_statsLabel    = nullptr;
    IsometricWarehouseWidget *m_isoWidget     = nullptr;
    QVBoxLayout              *m_listLayout    = nullptr;
    QScrollArea              *m_listScroll    = nullptr;
    QWidget                  *m_listContainer = nullptr;

    void buildList(const QList<StockMaterial>& materials);
};

// ============================================================================
//  StockMapView
//  Full map page widget:
//    - Top search bar (search by material name or entrepôt)
//    - Filter pills (Tous / En alerte / Stock OK)
//    - Stats mini-bar (total entrepôts, materials, alerts)
//    - QQuickWidget with OSM map and animated markers
//    - Right-side WarehouseDetailPanel
// ============================================================================
class StockMapView : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariantList markersData READ markersData NOTIFY markersDataChanged)

public:
    explicit StockMapView(QWidget *parent = nullptr);

    // Reload stats from DB and push updated markers to QML
    void refresh();

    QVariantList markersData() const { return m_markersData; }

signals:
    void localeClicked(const QString& localeName);
    void markersDataChanged();

private slots:
    void onQmlStatusChanged(QQuickWidget::Status status);
    void onMarkerClicked(const QString& localeName);
    void onSearchChanged(const QString& text);
    void onFilterAll();
    void onFilterAlert();
    void onFilterOk();

private:
    QQuickWidget *m_quickWidget    = nullptr;
    QVariantList  m_markersData;
    QVariantList  m_allMarkersData; // full unfiltered set

    // Search / filter toolbar
    QLineEdit    *m_searchEdit     = nullptr;
    QComboBox    *m_localeCombo    = nullptr;
    QPushButton  *m_filterAll      = nullptr;
    QPushButton  *m_filterAlert    = nullptr;
    QPushButton  *m_filterOk       = nullptr;

    // Stats mini-bar labels
    QLabel       *m_statEntrepots  = nullptr;
    QLabel       *m_statMaterials  = nullptr;
    QLabel       *m_statAlerts     = nullptr;

    QString       m_activeFilter   = "all"; // "all" | "alert" | "ok"

    QVariantList buildMarkersData() const;
    void         connectQmlSignals();
    void         applyFilter();
    void         pushMarkersToQml(const QVariantList& data);
    void         updateStatBar(const QVariantList& data);
    void         setFilterButtonActive(QPushButton *btn);
};

#endif // STOCKMAPVIEW_H