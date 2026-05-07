#include "arduinobridge.h"
#include "stocklocales.h"
#include "stockmapview.h"
#include "stockpage.h"
#include "stockalertbell.h"
#include "stockalertpanel.h"
#include "src/database/stockdatabase.h"
#include "src/database/connection.h"
#include "src/common/stockvalidators.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QFrame>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QScrollArea>
#include <QApplication>
#include <QSystemTrayIcon>
#include <cmath>
#include <algorithm>

// ============================================================================
//  Custom chart widgets — QPainter only, no QtCharts dependency
// ============================================================================

// ── Radial gauge (stock health) ─────────────────────────────────────────────
class RadialGaugeWidget : public QWidget {
public:
    double value = 0.0;   // 0..100
    QString title;
    RadialGaugeWidget(QWidget* p = nullptr) : QWidget(p) {
        setMinimumSize(200, 200);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        int w = width(), h = height();
        int r = qMin(w, h) * 3 / 8;
        int cx = w/2, cy = h/2;

        bool isDark = qApp->styleSheet().contains("0f0f0f");
        QColor bgCircle = isDark ? QColor("#2d2d2d") : QColor("#f0f2f5");
        QColor textColor = isDark ? QColor("#e0e0e0") : QColor("#1f2937");

        // Background circle
        p.setPen(Qt::NoPen);
        p.setBrush(bgCircle);
        p.drawEllipse(cx - r, cy - r, 2*r, 2*r);

        // Arc for value
        int angle = (int)(value * 360.0 / 100.0);
        QColor arcColor = (value < 30) ? QColor("#c0392b") : (value < 70) ? QColor("#e67e22") : QColor("#27ae60");
        p.setBrush(Qt::NoBrush);
        QPen pen(arcColor, 10);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.drawArc(cx - r, cy - r, 2*r, 2*r, 90*16, -angle*16);

        // Inner circle
        int innerR = r - 14;
        p.setBrush(bgCircle);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx - innerR, cy - innerR, 2*innerR, 2*innerR);

        // Value text
        QFont f; f.setPixelSize(28); f.setBold(true);
        p.setFont(f);
        p.setPen(textColor);
        p.drawText(cx - 40, cy - 15, 80, 30, Qt::AlignCenter, QString::number(value, 'f', 0) + "%");

        // Title
        QFont tf; tf.setPixelSize(12);
        p.setFont(tf);
        p.drawText(cx - 80, cy + 20, 160, 20, Qt::AlignCenter, title);
    }
};

// ── Donut chart for renewal categories ──────────────────────────────────────
class DonutChartWidget : public QWidget {
public:
    struct Slice { QString label; double value; QColor color; };
    QList<Slice> slices;
    QString title;

    DonutChartWidget(QWidget* p = nullptr) : QWidget(p) {
        setMinimumSize(250, 250);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAttribute(Qt::WA_TranslucentBackground);
    }

    void setSlices(const QList<Slice>& s) { slices = s; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (slices.isEmpty()) return;

        int w = width(), h = height();
        int tH = title.isEmpty() ? 0 : 30;
        int r = qMin(w, h - tH) * 2 / 5;
        int cx = w/2, cy = (h + tH)/2;

        double total = 0;
        for (auto& s : slices) total += s.value;
        if (total == 0) return;

        // Draw slices
        double angle = 0.0;
        for (auto& s : slices) {
            double span = 360.0 * s.value / total;
            p.setBrush(s.color);
            p.setPen(Qt::NoPen);
            p.drawPie(cx - r, cy - r, 2*r, 2*r, (int)(angle * 16), -(int)(span * 16));
            angle += span;
        }

        // Inner hole (donut)
        int hole = r * 6 / 10;
        bool isDark = qApp->styleSheet().contains("0f0f0f");
        p.setBrush(isDark ? QColor("#1e1e1e") : QColor("#ffffff"));
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx - hole, cy - hole, 2*hole, 2*hole);

        // Title
        if (!title.isEmpty()) {
            QFont tf; tf.setPixelSize(13); tf.setBold(true);
            p.setFont(tf);
            p.setPen(isDark ? QColor("#f0f0f0") : QColor("#374151"));
            p.drawText(0, 0, w, tH, Qt::AlignCenter, title);
        }

        // Legend
        int lx = 8, ly = tH + r + 15;
        QFont lf; lf.setPixelSize(10);
        p.setFont(lf);
        for (int i = 0; i < slices.size(); i++) {
            int col = i % 2, row = i / 2;
            int ox = col * (w/2) + lx;
            int oy = ly + row * 22;
            p.setBrush(slices[i].color);
            p.setPen(Qt::NoPen);
            p.drawEllipse(ox, oy + 4, 10, 10);
            p.setPen(isDark ? QColor("#9ca3af") : QColor("#4b5563"));
            p.drawText(ox + 14, oy, w/2 - 20, 18, Qt::AlignVCenter,
                       QString("%1 (%2%)").arg(slices[i].label).arg(slices[i].value * 100.0 / total, 0, 'f', 1));
        }
    }
};

// ── Horizontal bar chart for top renewal ────────────────────────────────────
class HorizontalBarWidget : public QWidget {
public:
    struct Bar { QString label; double value; };
    QList<Bar> bars;
    QString title;
    HorizontalBarWidget(QWidget* p = nullptr) : QWidget(p) {
        setMinimumSize(300, 200);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (bars.isEmpty()) return;

        int w = width(), h = height();
        int tH = 30, barH = 28, margin = 15;
        int totalH = tH + bars.size() * (barH + 6) + margin;

        bool isDark = qApp->styleSheet().contains("0f0f0f");
        QColor textColor = isDark ? QColor("#e0e0e0") : QColor("#1f2937");
        QColor barColor = QColor("#2980b9");

        // Title
        QFont tf; tf.setPixelSize(13); tf.setBold(true);
        p.setFont(tf);
        p.setPen(textColor);
        p.drawText(0, 0, w, tH, Qt::AlignCenter, title);

        double maxV = 0;
        for (auto& b : bars) maxV = qMax(maxV, b.value);
        if (maxV == 0) return;

        int startY = tH + margin;
        QFont lf; lf.setPixelSize(10);
        p.setFont(lf);
        for (int i = 0; i < bars.size(); i++) {
            int y = startY + i * (barH + 6);
            int barW = (int)((bars[i].value / maxV) * (w - 120));
            // Label
            p.setPen(textColor);
            p.drawText(10, y, 70, barH, Qt::AlignRight | Qt::AlignVCenter, bars[i].label);
            // Bar
            p.fillRect(90, y, barW, barH, barColor);
            // Value
            p.setPen(textColor);
            p.drawText(90 + barW + 5, y, 60, barH, Qt::AlignLeft | Qt::AlignVCenter,
                       QString::number(bars[i].value, 'f', 1) + " u/mois");
        }
    }
};

// ── Area chart (simulated weekly consumption) ───────────────────────────────
class AreaChartWidget : public QWidget {
public:
    QList<double> data; // weekly consumption values
    QString title;
    AreaChartWidget(QWidget* p = nullptr) : QWidget(p) {
        setMinimumSize(350, 250);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (data.isEmpty()) return;

        int w = width(), h = height();
        int tH = 30, bottomH = 45, leftMargin = 50, rightMargin = 25;
        int chartH = h - tH - bottomH;

        bool isDark = qApp->styleSheet().contains("0f0f0f");
        QColor textColor = isDark ? QColor("#e0e0e0") : QColor("#1f2937");
        QColor gridColor = isDark ? QColor("#2e2e2e") : QColor("#e5e7eb");
        QColor areaColor = QColor("#2980b9");

        // Title
        QFont tf; tf.setPixelSize(13); tf.setBold(true);
        p.setFont(tf);
        p.setPen(textColor);
        p.drawText(0, 0, w, tH, Qt::AlignCenter, title);

        double maxV = *std::max_element(data.begin(), data.end());
        if (maxV == 0) return;

        int n = data.size();
        double step = (double)(w - leftMargin - rightMargin) / (n - 1);

        QPainterPath path;
        path.moveTo(leftMargin, tH + chartH);
        for (int i = 0; i < n; i++) {
            double x = leftMargin + i * step;
            double y = tH + chartH - (data[i] / maxV) * chartH;
            if (i == 0) path.moveTo(x, y);
            else path.lineTo(x, y);
        }
        path.lineTo(leftMargin + (n-1)*step, tH + chartH);
        path.closeSubpath();

        p.fillPath(path, QColor(41, 128, 185, 100));

        // Line on top
        p.setPen(QPen(areaColor, 2));
        for (int i = 1; i < n; i++) {
            double x1 = leftMargin + (i-1)*step;
            double y1 = tH + chartH - (data[i-1] / maxV) * chartH;
            double x2 = leftMargin + i*step;
            double y2 = tH + chartH - (data[i] / maxV) * chartH;
            p.drawLine(x1, y1, x2, y2);
        }

        // Grid lines and labels
        p.setPen(QPen(gridColor, 1, Qt::DashLine));
        for (int i = 1; i <= 4; i++) {
            int y = tH + chartH - (int)(chartH * i / 4.0);
            p.drawLine(leftMargin, y, w - rightMargin, y);
            p.setPen(textColor);
            p.drawText(0, y - 5, leftMargin-5, 10, Qt::AlignRight,
                       QString::number(maxV * i / 4, 'f', 0));
        }

        // X axis labels (weeks)
        QStringList weeks;
        for (int i = 0; i < n; i++) weeks << QString::number(i+1);
        p.setPen(textColor);
        for (int i = 0; i < n; i++) {
            double x = leftMargin + i*step;
            p.drawText(x - 15, tH + chartH + 10, 30, 20, Qt::AlignHCenter, weeks[i]);
        }
        p.drawText(leftMargin, tH + chartH + 30, w - leftMargin - rightMargin, 15,
                   Qt::AlignHCenter, "Semaines");
    }
};

// ── Bar chart (for autonomy) ────────────────────────────────────────────────
class StockBarChartWidget : public QWidget {
public:
    struct Bar { QString label; double value; QColor color; };
    QString title;
    QString unit;

    StockBarChartWidget(QWidget* p = nullptr) : QWidget(p) {
        setMinimumSize(300, 300);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAttribute(Qt::WA_TranslucentBackground);
    }
    void setBars(const QList<Bar>& b) { bars = b; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (bars.isEmpty()) return;

        int w = width(), h = height();
        int tH = 35, botH = 70, topPad = 30;
        int chartH = h - tH - botH - topPad;

        bool isDark = qApp->styleSheet().contains("0f0f0f");
        QColor gridCol  = isDark ? QColor("#2e2e2e") : QColor("#e5e7eb");
        QColor labelCol = isDark ? QColor("#6b7280") : QColor("#9ca3af");
        QColor titleCol = isDark ? QColor("#f0f0f0") : QColor("#374151");

        QFont tf; tf.setPixelSize(13); tf.setBold(true); p.setFont(tf);
        p.setPen(titleCol);
        p.drawText(0, 0, w, tH, Qt::AlignCenter, title);

        double maxV = 0;
        for (auto& b : bars) maxV = qMax(maxV, b.value);
        if (maxV == 0) return;

        QFont gf; gf.setPixelSize(9);
        for (int i = 1; i <= 4; i++) {
            int y = tH + topPad + chartH - (int)(chartH * i / 4.0);
            p.setPen(QPen(gridCol, 1, Qt::DashLine));
            p.drawLine(45, y, w - 8, y);
            p.setFont(gf); p.setPen(labelCol);
            p.drawText(0, y - 8, 40, 16, Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(maxV * i / 4, 'f', 0));
        }

        int n = bars.size();
        int totalW = w - 55;
        int barW = qMax(12, (totalW / n) - 10);
        int gap  = (totalW - barW * n) / (n + 1);

        QFont vf; vf.setPixelSize(10); vf.setBold(true);
        QFont lf; lf.setPixelSize(10); lf.setBold(false);

        for (int i = 0; i < n; i++) {
            int bh = (int)(bars[i].value / maxV * chartH);
            int bx = 50 + gap + (barW + gap) * i;
            int by = tH + topPad + chartH - bh;

            // Shadow
            p.fillRect(bx + 2, by + 2, barW, bh, QColor(0, 0, 0, 18));

            // Rounded bar
            QPainterPath path;
            int rad = qMin(6, barW / 3);
            path.moveTo(bx, by + bh); path.lineTo(bx, by + rad);
            path.quadTo(bx, by, bx + rad, by);
            path.lineTo(bx + barW - rad, by);
            path.quadTo(bx + barW, by, bx + barW, by + rad);
            path.lineTo(bx + barW, by + bh);
            path.closeSubpath();

            QLinearGradient grad(bx, by, bx, by + bh);
            grad.setColorAt(0, bars[i].color.lighter(115));
            grad.setColorAt(1, bars[i].color);
            p.fillPath(path, grad);

            // Value above bar
            p.setPen(titleCol); p.setFont(vf);
            p.drawText(bx - 4, by - topPad + 2, barW + 8, topPad - 2, Qt::AlignCenter,
                       QString::number(bars[i].value, 'f', 1) + unit);

            // Label below bar
            p.setFont(lf); p.setPen(labelCol);
            p.drawText(bx - 4, tH + topPad + chartH + 5, barW + 8, 60,
                       Qt::AlignHCenter | Qt::TextWordWrap, bars[i].label);
        }
    }
private:
    QList<Bar> bars;
};

// ============================================================================
//  StockPage
// ============================================================================

StockPage::StockPage(QWidget *parent)
    : QWidget(parent)
{
    loadProduitsMap();
    setupUI();
    setupTrayIcon();
    refreshTable(StockDatabase::instance().getAllMaterials());
}

void StockPage::loadProduitsMap()
{
    m_produitsMap.clear();
    m_produitsMap[0] = "— Aucun produit —";

    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    if (q.exec("SELECT ID_PROD, NOM_PROD FROM PRODUIT ORDER BY NOM_PROD")) {
        while (q.next())
            m_produitsMap[q.value("ID_PROD").toInt()] = q.value("NOM_PROD").toString();
    } else {
        qWarning() << "[StockPage] loadProduitsMap error:" << q.lastError().text();
    }
}

void StockPage::setupUI()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    mainStack = new QStackedWidget(this);
    rootLayout->addWidget(mainStack);

    // ── PAGE 0 : tableau ─────────────────────────────────────────────────────
    tablePage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(tablePage);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    mainLayout->setSpacing(18);

    QHBoxLayout *searchSortLayout = new QHBoxLayout();
    searchSortLayout->setSpacing(12);

    searchEdit = new QLineEdit(tablePage);
    searchEdit->setPlaceholderText("Rechercher par nom ou type...");
    searchEdit->setFixedHeight(35);
    connect(searchEdit, &QLineEdit::textChanged, this, &StockPage::onSearchTextChanged);

    sortCombo = new QComboBox(tablePage);
    sortCombo->addItems({"Tri par défaut","Nom (A → Z)","Nom (Z → A)","Conso. mensuelle ↑","Conso. mensuelle ↓"});
    sortCombo->setFixedHeight(35);
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StockPage::onSortChanged);

    produitFilterCombo = new QComboBox(tablePage);
    produitFilterCombo->addItem("Tous les produits", 0);
    for (auto it = m_produitsMap.constBegin(); it != m_produitsMap.constEnd(); ++it)
        if (it.key() != 0) produitFilterCombo->addItem(it.value(), it.key());
    produitFilterCombo->setFixedHeight(35);
    produitFilterCombo->setMinimumWidth(180);
    connect(produitFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StockPage::onFilterByProduitChanged);

    searchSortLayout->addWidget(searchEdit, 3);
    searchSortLayout->addWidget(sortCombo, 1);
    searchSortLayout->addWidget(produitFilterCombo, 1);
    searchSortLayout->addStretch();
    mainLayout->addLayout(searchSortLayout);

    QHBoxLayout *actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(12);

    QPushButton *addBtn       = new QPushButton("+ Ajouter un matériau", tablePage);
    QPushButton *editBtn      = new QPushButton("Modifier",               tablePage);
    QPushButton *deleteBtn    = new QPushButton("Supprimer",              tablePage);
    QPushButton *exportPdfBtn = new QPushButton("Exporter alertes PDF",   tablePage);
    QPushButton *statsBtn     = new QPushButton("Statistiques",           tablePage);

    for (auto btn : {addBtn, editBtn, deleteBtn, exportPdfBtn, statsBtn}) {
        btn->setObjectName("actionButton");
        btn->setCursor(Qt::PointingHandCursor);
    }

    // ── Cloche de notifications ──────────────────────────────────────────────
    m_bell = new StockAlertBell(tablePage);
    m_alertPanel = new StockAlertPanel(nullptr); // parent nullptr = fenêtre flottante

    connect(m_bell,        &StockAlertBell::clicked,            this, &StockPage::onBellClicked);
    connect(m_bell,        &StockAlertBell::newAlertsDetected,  this, &StockPage::onNewAlertsDetected);
    connect(m_alertPanel,  &StockAlertPanel::alertSelected,     this, &StockPage::onAlertSelected);

    connect(addBtn,       &QPushButton::clicked, this, &StockPage::onAddButtonClicked);
    connect(editBtn,      &QPushButton::clicked, this, &StockPage::onEditButtonClicked);
    connect(deleteBtn,    &QPushButton::clicked, this, &StockPage::onDeleteButtonClicked);
    connect(exportPdfBtn, &QPushButton::clicked, this, &StockPage::onExportAlertPdfClicked);
    connect(statsBtn,     &QPushButton::clicked, this, &StockPage::onShowStatsClicked);

    actionsLayout->addWidget(addBtn);
    actionsLayout->addWidget(editBtn);
    actionsLayout->addWidget(deleteBtn);
    actionsLayout->addWidget(exportPdfBtn);

    QPushButton *mapBtn = new QPushButton("Carte", tablePage);
    mapBtn->setObjectName("actionButton");
    mapBtn->setCursor(Qt::PointingHandCursor);
    connect(mapBtn, &QPushButton::clicked, this, &StockPage::onShowMapClicked);
    actionsLayout->addWidget(statsBtn);
    actionsLayout->addWidget(mapBtn);
    actionsLayout->addStretch();
    actionsLayout->addWidget(m_bell);   // cloche à la place du bouton alertes
    mainLayout->addLayout(actionsLayout);

    stockTable = new QTableWidget(tablePage);
    stockTable->setObjectName("dataTable");
    stockTable->setColumnCount(13);
    stockTable->setHorizontalHeaderLabels({
                                           "ID","Nom","Type","Quantité","Prix unit. (DT)",
                                           "Fournisseur","Seuil alerte","Dernière commande",
                                           "Conso. mensuelle","Unité","Produit associé","Locale","Emplacement"
    });
    stockTable->horizontalHeader()->setStretchLastSection(true);
    stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    stockTable->verticalHeader()->setVisible(false);
    stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    stockTable->setSelectionMode(QAbstractItemView::SingleSelection);
    stockTable->setAlternatingRowColors(true);
    stockTable->setShowGrid(false);
    stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    stockTable->setColumnHidden(0, true);

    stockTable->setColumnHidden(11, true);  // Locale
    stockTable->setColumnHidden(12, true);  // Emplacement
    connect(stockTable, &QTableWidget::cellDoubleClicked, this, &StockPage::onViewTriggered);
    mainLayout->addWidget(stockTable);

    mainStack->addWidget(tablePage);

    setupStatsPage();
    mainStack->addWidget(statsPage);
    setupMapPage();
    mainStack->addWidget(mapPage);
    setupArduinoBar(); // ← insérer ici

    mainStack->setCurrentIndex(0);
}

void StockPage::setupStatsPage()
{
    statsPage = new QWidget();
    QVBoxLayout *pageLayout = new QVBoxLayout(statsPage);
    pageLayout->setContentsMargins(25, 25, 25, 25);
    pageLayout->setSpacing(20);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backBtn = new QPushButton("← Retour au stock", statsPage);
    backBtn->setObjectName("backButton");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &StockPage::onBackFromStatsClicked);
    QLabel *pageTitle = new QLabel("Statistiques du stock", statsPage);
    pageTitle->setObjectName("dialogTitle");
    headerLayout->addWidget(backBtn);
    headerLayout->addStretch();
    headerLayout->addWidget(pageTitle);
    headerLayout->addStretch();
    pageLayout->addLayout(headerLayout);

    QScrollArea *scroll = new QScrollArea(statsPage);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *inner = new QWidget();
    inner->setMinimumWidth(820);
    QVBoxLayout *lay = new QVBoxLayout(inner);
    lay->setSpacing(22);
    lay->setContentsMargins(0, 0, 0, 50);

    QList<StockMaterial> all = StockDatabase::instance().getAllMaterials();
    int total      = all.size();
    int alertCount = 0;
    for (const auto& m : all) if (m.isBelowAlert()) alertCount++;
    int okCount = total - alertCount;

    // ── Row 1 : Vue d'ensemble (pretty painted card) ──────────────────────────
    {
        // Custom painted overview widget — three colored bands + progress bar
        class OverviewWidget : public QWidget {
        public:
            int total, ok, alert;
            OverviewWidget(int t, int o, int a, QWidget* p = nullptr)
                : QWidget(p), total(t), ok(o), alert(a)
            {
                setMinimumHeight(180);
                setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
                setAttribute(Qt::WA_TranslucentBackground);
            }
        protected:
            void paintEvent(QPaintEvent*) override {
                QPainter p(this);
                p.setRenderHint(QPainter::Antialiasing);
                int w = width(), h = height();
                bool isDark = qApp->styleSheet().contains("0f0f0f");
                QColor textPrimary   = isDark ? QColor("#f0f0f0") : QColor("#1f2937");
                QColor textSecondary = isDark ? QColor("#9ca3af") : QColor("#6b7280");

                // Three stat tiles side by side
                struct Tile { QString label; QString value; QColor accent; };
                QList<Tile> tiles = {
                                     {"Total", QString::number(total),  QColor("#2980b9")},
                                     {"En stock", QString::number(ok),  QColor("#27ae60")},
                                     {"En alerte", QString::number(alert), QColor("#c0392b")},
                                     };
                int tileW  = (w - 40) / 3;
                int tileH  = h - 60; // leave room for progress bar below
                int tileY  = 0;
                for (int i = 0; i < tiles.size(); i++) {
                    int tx = i * (tileW + 20);
                    // Tile background
                    QPainterPath bg;
                    bg.addRoundedRect(tx, tileY, tileW, tileH, 12, 12);
                    QColor fill = tiles[i].accent;
                    fill.setAlpha(isDark ? 35 : 22);
                    p.fillPath(bg, fill);
                    // Left accent bar
                    p.fillRect(tx, tileY + 14, 4, tileH - 28, tiles[i].accent);
                    // Big number
                    QFont nf; nf.setPixelSize(46); nf.setBold(true); p.setFont(nf);
                    p.setPen(tiles[i].accent);
                    p.drawText(tx + 18, tileY + 8, tileW - 22, tileH - 30,
                               Qt::AlignLeft | Qt::AlignVCenter, tiles[i].value);
                    // Label
                    QFont lf; lf.setPixelSize(11); p.setFont(lf);
                    p.setPen(textSecondary);
                    p.drawText(tx + 18, tileY + tileH - 28, tileW - 22, 24,
                               Qt::AlignLeft | Qt::AlignVCenter, tiles[i].label);
                }

                // Progress bar: ok vs alert ratio
                int barY = tileH + 16;
                int barH2 = 14;
                int barW  = w;
                p.setPen(Qt::NoPen);
                // Background track
                QPainterPath track; track.addRoundedRect(0, barY, barW, barH2, 7, 7);
                p.fillPath(track, isDark ? QColor("#2d2d2d") : QColor("#e5e7eb"));
                // OK fill
                if (total > 0) {
                    double ratio = (double)ok / total;
                    int fillW = (int)(barW * ratio);
                    if (fillW > 0) {
                        QPainterPath fill2; fill2.addRoundedRect(0, barY, fillW, barH2, 7, 7);
                        p.fillPath(fill2, QColor("#27ae60"));
                    }
                    // Alert fill
                    int alertW = barW - (int)(barW * ratio);
                    if (alertW > 0) {
                        int ax = barW - alertW;
                        QPainterPath fill3; fill3.addRoundedRect(ax, barY, alertW, barH2, 7, 7);
                        p.fillPath(fill3, QColor("#c0392b"));
                    }
                }
                // Legend text beside bar
                QFont bf; bf.setPixelSize(9); p.setFont(bf);
                p.setPen(textSecondary);
                QString pct = total > 0
                                  ? QString("%1% en stock, %2% en alerte")
                                        .arg(ok * 100 / total).arg(alert * 100 / total)
                                  : "Aucun materiau";
                p.drawText(0, barY + barH2 + 4, w, 16, Qt::AlignCenter, pct);
            }
        };

        QFrame *card = new QFrame(inner);
        card->setObjectName("statCard");
        card->setMinimumHeight(260);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(20, 18, 20, 18);
        cl->setSpacing(10);

        QLabel *titleLbl = new QLabel("Vue d'ensemble du stock", card);
        { QFont f = titleLbl->font(); f.setPixelSize(14); f.setBold(true); titleLbl->setFont(f); }
        titleLbl->setAlignment(Qt::AlignCenter);
        cl->addWidget(titleLbl);

        OverviewWidget *ow = new OverviewWidget(total, okCount, alertCount, card);
        cl->addWidget(ow, 1);
        lay->addWidget(card);
    }

    // ── Row 2 : Rotation — Donut + detail list side by side ──────────────────
    {
        // Categorise materials
        struct RotEntry { QString nom; double conso; };
        QList<RotEntry> fastList, medList, slowList;
        for (const auto& m : all) {
            double c = m.getConsoMensuelle();
            RotEntry e{ m.getNom(), c };
            if      (c >= 20) fastList  << e;
            else if (c >=  5) medList   << e;
            else              slowList  << e;
        }

        QFrame *card = new QFrame(inner);
        card->setObjectName("statCard");
        card->setMinimumHeight(360);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(18, 16, 18, 16);
        cl->setSpacing(8);

        QLabel *titleLbl = new QLabel("Vitesse de renouvellement des matériaux", card);
        { QFont f = titleLbl->font(); f.setPixelSize(14); f.setBold(true); titleLbl->setFont(f); }
        titleLbl->setAlignment(Qt::AlignCenter);
        cl->addWidget(titleLbl);

        QLabel *subLbl = new QLabel(
            "Chaque materiau est classe selon sa consommation mensuelle : "
            "Rapide (> 20 u/mois), Moyen (5-20 u/mois), Lent (< 5 u/mois). "
            "Un materiau rapide doit etre reapprovisionne frequemment.", card);
        subLbl->setWordWrap(true);
        subLbl->setAlignment(Qt::AlignCenter);
        { QFont f = subLbl->font(); f.setPixelSize(10); subLbl->setFont(f); }
        cl->addWidget(subLbl);

        // Donut on the left, detail columns on the right
        QHBoxLayout *body = new QHBoxLayout();
        body->setSpacing(16);

        DonutChartWidget *donut = new DonutChartWidget(card);
        donut->title = "";
        QList<DonutChartWidget::Slice> slices;
        if (!fastList.isEmpty()) slices.append({"Rapide",  (double)fastList.size(), QColor("#27ae60")});
        if (!medList.isEmpty())  slices.append({"Moyen",   (double)medList.size(),  QColor("#f39c12")});
        if (!slowList.isEmpty()) slices.append({"Lent",    (double)slowList.size(), QColor("#e67e22")});
        donut->setSlices(slices);
        donut->setMinimumSize(220, 260);
        donut->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        body->addWidget(donut);

        // Detail columns — one per category
        QHBoxLayout *cols = new QHBoxLayout();
        cols->setSpacing(12);

        struct ColDef { QString label; QColor color; QList<RotEntry> entries; };
        QList<ColDef> colDefs = {
                                 {"Rapide  > 20 u/mois",  QColor("#27ae60"), fastList},
                                 {"Moyen   5-20 u/mois",  QColor("#f39c12"), medList},
                                 {"Lent    < 5 u/mois",   QColor("#e67e22"), slowList},
                                 };
        for (const auto& col : colDefs) {
            QFrame *colFrame = new QFrame(card);
            colFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
            QVBoxLayout *vl = new QVBoxLayout(colFrame);
            vl->setContentsMargins(8, 8, 8, 8);
            vl->setSpacing(4);

            // Column header
            QLabel *hdr = new QLabel(col.label, colFrame);
            { QFont f = hdr->font(); f.setPixelSize(10); f.setBold(true); hdr->setFont(f); }
            hdr->setStyleSheet(QString("color: %1;").arg(col.color.name()));
            hdr->setAlignment(Qt::AlignCenter);
            vl->addWidget(hdr);

            QLabel *countLbl = new QLabel(
                QString("%1 materiau%2").arg(col.entries.size()).arg(col.entries.size() != 1 ? "x" : ""),
                colFrame);
            { QFont f = countLbl->font(); f.setPixelSize(9); countLbl->setFont(f); }
            countLbl->setAlignment(Qt::AlignCenter);
            countLbl->setStyleSheet("color: #9ca3af;");
            vl->addWidget(countLbl);

            // Divider
            QFrame *div = new QFrame(colFrame);
            div->setFrameShape(QFrame::HLine);
            div->setStyleSheet(QString("color: %1;").arg(col.color.name()));
            vl->addWidget(div);

            // Material rows
            if (col.entries.isEmpty()) {
                QLabel *empty = new QLabel("Aucun", colFrame);
                empty->setAlignment(Qt::AlignCenter);
                { QFont f = empty->font(); f.setPixelSize(10); empty->setFont(f); }
                empty->setStyleSheet("color: #9ca3af; font-style: italic;");
                vl->addWidget(empty);
            } else {
                for (const auto& e : col.entries) {
                    QHBoxLayout *row = new QHBoxLayout();
                    row->setSpacing(4);
                    QString nomStr = e.nom.length() > 18 ? e.nom.left(16) + ".." : e.nom;
                    QLabel *nom = new QLabel(nomStr, colFrame);
                    { QFont f = nom->font(); f.setPixelSize(10); nom->setFont(f); }
                    nom->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                    QLabel *val = new QLabel(QString::number(e.conso, 'f', 1) + " u/m", colFrame);
                    { QFont f = val->font(); f.setPixelSize(10); f.setBold(true); val->setFont(f); }
                    val->setStyleSheet(QString("color: %1;").arg(col.color.name()));
                    val->setAlignment(Qt::AlignRight);
                    row->addWidget(nom);
                    row->addWidget(val);
                    vl->addLayout(row);
                }
            }
            vl->addStretch();
            cols->addWidget(colFrame, 1);
        }
        body->addLayout(cols, 1);
        cl->addLayout(body, 1);
        lay->addWidget(card);
    }

    // ── Row 3 : Top 5 consommation ────────────────────────────────────────────
    {
        QFrame *card = new QFrame(inner);
        card->setObjectName("statCard");
        card->setMinimumHeight(320);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(18, 16, 18, 16);
        cl->setSpacing(8);

        QLabel *titleLbl = new QLabel("Top 5 Matériaux les plus consommés", card);
        { QFont f = titleLbl->font(); f.setPixelSize(14); f.setBold(true); titleLbl->setFont(f); }
        titleLbl->setAlignment(Qt::AlignCenter);
        cl->addWidget(titleLbl);

        QLabel *subLbl = new QLabel(
            "Les 5 matériaux avec la consommation mensuelle la plus elevee: "
            "ceux qu'il faut surveiller et reapprovisionner en priorite.", card);
        subLbl->setWordWrap(true);
        subLbl->setAlignment(Qt::AlignCenter);
        { QFont f = subLbl->font(); f.setPixelSize(10); subLbl->setFont(f); }
        cl->addWidget(subLbl);

        HorizontalBarWidget *hbar = new HorizontalBarWidget(card);
        hbar->title = "";

        QList<StockMaterial> sorted = all;
        std::sort(sorted.begin(), sorted.end(),
                  [](const StockMaterial& a, const StockMaterial& b) {
                      return a.getConsoMensuelle() > b.getConsoMensuelle();
                  });
        QList<HorizontalBarWidget::Bar> bars;
        for (int i = 0; i < qMin(5, sorted.size()); i++)
            bars.append({sorted[i].getNom(), sorted[i].getConsoMensuelle()});
        hbar->bars = bars;
        hbar->setMinimumHeight(220);
        cl->addWidget(hbar, 1);
        lay->addWidget(card);
    }

    // ── Row 4 : Autonomie — compact horizontal list ───────────────────────────
    // Each material gets one slim row: name | mini bar | day count.
    // No vertical bar chart — much more compact, shows all materials.
    {
        QList<StockMaterial> sorted = all;
        std::sort(sorted.begin(), sorted.end(),
                  [](const StockMaterial& a, const StockMaterial& b) {
                      double dA = a.getConsoMensuelle() > 0
                                      ? a.getQuantite() / a.getConsoMensuelle() * 30.0 : 9999.0;
                      double dB = b.getConsoMensuelle() > 0
                                      ? b.getQuantite() / b.getConsoMensuelle() * 30.0 : 9999.0;
                      return dA < dB;
                  });

        // Build list (only materials with consumption > 0)
        struct AutoEntry { QString nom; double days; QColor col; };
        QList<AutoEntry> entries;
        double maxDays = 0;
        for (const auto& m : sorted) {
            if (m.getConsoMensuelle() <= 0) continue;
            double d = m.getQuantite() / m.getConsoMensuelle() * 30.0;
            QColor c = d < 15 ? QColor("#c0392b") : d < 30 ? QColor("#e67e22") : QColor("#27ae60");
            entries.append({m.getNom(), d, c});
            maxDays = qMax(maxDays, d);
        }

        // Card height: header ~80px + 28px per row + 20px padding
        int rowH    = 28;
        int cardMinH = 80 + entries.size() * rowH + 20;

        QFrame *card = new QFrame(inner);
        card->setObjectName("statCard");
        card->setMinimumHeight(cardMinH);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(18, 16, 18, 12);
        cl->setSpacing(8);

        QLabel *titleLbl = new QLabel("Autonomie estimee par materiau (jours de stock restants)", card);
        { QFont f = titleLbl->font(); f.setPixelSize(14); f.setBold(true); titleLbl->setFont(f); }
        titleLbl->setAlignment(Qt::AlignCenter);
        cl->addWidget(titleLbl);

        QLabel *subLbl = new QLabel(
            "Rouge : moins de 15 jours (commander en urgence)   "
            "Orange : 15-30 jours (a surveiller)   "
            "Vert : plus de 30 jours (confortable)", card);
        subLbl->setWordWrap(true);
        subLbl->setAlignment(Qt::AlignCenter);
        { QFont f = subLbl->font(); f.setPixelSize(10); subLbl->setFont(f); }
        cl->addWidget(subLbl);

        if (entries.isEmpty()) {
            QLabel *empty = new QLabel("Aucun materiau avec consommation enregistree.", card);
            empty->setAlignment(Qt::AlignCenter);
            cl->addWidget(empty);
        } else {
            // Custom painted list widget — slim and compact
            class AutoListWidget : public QWidget {
            public:
                QList<AutoEntry> entries;
                double maxDays;
                int rowH;
                AutoListWidget(const QList<AutoEntry>& e, double mx, int rH, QWidget* p = nullptr)
                    : QWidget(p), entries(e), maxDays(mx), rowH(rH)
                {
                    setMinimumHeight(entries.size() * rowH);
                    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
                    setAttribute(Qt::WA_TranslucentBackground);
                }
            protected:
                void paintEvent(QPaintEvent*) override {
                    QPainter p(this);
                    p.setRenderHint(QPainter::Antialiasing);
                    int w = width();
                    bool isDark = qApp->styleSheet().contains("0f0f0f");
                    QColor textPri = isDark ? QColor("#e0e0e0") : QColor("#1f2937");
                    QColor trackCol = isDark ? QColor("#2d2d2d") : QColor("#e5e7eb");
                    QColor altRow   = isDark ? QColor("#1a1a1a") : QColor("#f9fafb");

                    int nameW = 160;
                    int dayW  = 60;
                    int barX  = nameW + 8;
                    int barMaxW = w - barX - dayW - 8;

                    QFont namef; namef.setPixelSize(11);
                    QFont valf;  valf.setPixelSize(11); valf.setBold(true);

                    for (int i = 0; i < entries.size(); i++) {
                        int y = i * rowH;
                        // Alternating row bg
                        if (i % 2 == 0) p.fillRect(0, y, w, rowH, altRow);

                        const auto& e = entries[i];
                        // Name
                        p.setFont(namef); p.setPen(textPri);
                        QString nom = e.nom.length() > 22 ? e.nom.left(20) + ".." : e.nom;
                        p.drawText(0, y, nameW, rowH, Qt::AlignLeft | Qt::AlignVCenter, nom);

                        // Track
                        int barY2 = y + rowH / 2 - 5;
                        p.fillRect(barX, barY2, barMaxW, 10, trackCol);
                        // Fill
                        if (maxDays > 0) {
                            double ratio = qMin(1.0, e.days / maxDays);
                            int fillW = (int)(barMaxW * ratio);
                            if (fillW > 0) {
                                QPainterPath bp;
                                bp.addRoundedRect(barX, barY2, fillW, 10, 5, 5);
                                p.fillPath(bp, e.col);
                            }
                        }

                        // Day count
                        p.setFont(valf); p.setPen(e.col);
                        p.drawText(w - dayW, y, dayW, rowH,
                                   Qt::AlignRight | Qt::AlignVCenter,
                                   QString::number(e.days, 'f', 0) + " j");
                    }
                }
            };

            AutoListWidget *alw = new AutoListWidget(entries, maxDays, rowH, card);
            cl->addWidget(alw, 1);
        }
        lay->addWidget(card);
    }

    lay->addStretch();
    scroll->setWidget(inner);
    pageLayout->addWidget(scroll);
}
void StockPage::onShowStatsClicked()
{
    int idx = mainStack->indexOf(statsPage);
    if (idx >= 0) { mainStack->removeWidget(statsPage); delete statsPage; statsPage = nullptr; }
    setupStatsPage();
    mainStack->addWidget(statsPage);
    mainStack->setCurrentWidget(statsPage);
}

void StockPage::onBackFromStatsClicked()
{
    mainStack->setCurrentWidget(tablePage);
}

void StockPage::refreshTable(const QList<StockMaterial>& materials)
{
    stockTable->setRowCount(materials.size());
    for (int i = 0; i < materials.size(); ++i) {
        const StockMaterial& m = materials[i];
        auto item = [](const QString& text) { return new QTableWidgetItem(text); };
        QString nomProduit = m_produitsMap.value(m.getIdProduit(), "—");
        if (m.getIdProduit() == 0) nomProduit = "—";
        stockTable->setItem(i, 0,  item(QString::number(m.getId())));
        stockTable->setItem(i, 1,  item(m.getNom()));
        stockTable->setItem(i, 2,  item(m.getType()));
        stockTable->setItem(i, 3,  item(QString::number(m.getQuantite(),       'f', 2)));
        stockTable->setItem(i, 4,  item(QString::number(m.getPrixUnitaire(),   'f', 2)));
        stockTable->setItem(i, 5,  item(m.getFournisseur()));
        stockTable->setItem(i, 6,  item(QString::number(m.getSeuilAlerte(),    'f', 2)));
        stockTable->setItem(i, 7,  item(m.getLastOrder().toString("dd/MM/yyyy")));
        stockTable->setItem(i, 8,  item(QString::number(m.getConsoMensuelle(), 'f', 2)));
        stockTable->setItem(i, 9,  item(m.getUnite()));
        stockTable->setItem(i, 10, item(nomProduit));
        stockTable->setItem(i, 11, item(m.getLocale()));
        stockTable->setItem(i, 12, item(m.getEmplacement()));
        stockTable->setRowHeight(i, 48);
        if (m.isBelowAlert())
            for (int c = 0; c < 13; ++c)
                if (stockTable->item(i, c))
                    stockTable->item(i, c)->setBackground(QColor(255, 220, 220));
    }
}

StockMaterial StockPage::materialFromCurrentRow() const
{
    int row = stockTable->currentRow();
    if (row < 0) return StockMaterial();
    StockMaterial m;
    m.setId(stockTable->item(row, 0)->text().toInt());
    m.setNom(stockTable->item(row, 1)->text());
    m.setType(stockTable->item(row, 2)->text());
    m.setQuantite(stockTable->item(row, 3)->text().toDouble());
    m.setPrixUnitaire(stockTable->item(row, 4)->text().toDouble());
    m.setFournisseur(stockTable->item(row, 5)->text());
    m.setSeuilAlerte(stockTable->item(row, 6)->text().toDouble());
    m.setLastOrder(QDate::fromString(stockTable->item(row, 7)->text(), "dd/MM/yyyy"));
    m.setConsoMensuelle(stockTable->item(row, 8)->text().toDouble());
    m.setUnite(stockTable->item(row, 9)->text());
    {
        QString nomProd = stockTable->item(row, 10)->text();
        int idFound = 0;
        for (auto it = m_produitsMap.constBegin(); it != m_produitsMap.constEnd(); ++it)
            if (it.value() == nomProd && it.key() != 0) { idFound = it.key(); break; }
        m.setIdProduit(idFound);
    }
    m.setLocale(stockTable->item(row, 11) ? stockTable->item(row, 11)->text() : "");
    m.setEmplacement(stockTable->item(row, 12) ? stockTable->item(row, 12)->text() : "");
    return m;
}

static QDialog* buildMaterialDialog(QWidget* parent,
                                    const QString& windowTitle,
                                    const StockMaterial* initialData,
                                    int excludeId,
                                    bool isEdit,
                                    const QMap<int, QString>& produitsMap,
                                    QLineEdit*& nomEdit,
                                    QComboBox*& typeCombo,
                                    QDoubleSpinBox*& qteEdit,
                                    QDoubleSpinBox*& prixEdit,
                                    QLineEdit*& fournEdit,
                                    QDoubleSpinBox*& seuilEdit,
                                    QDateEdit*& dateEdit,
                                    QDoubleSpinBox*& consoEdit,
                                    QComboBox*& uniteCombo,
                                    QComboBox*& produitCombo,
                                    QComboBox*& localeCombo,
                                    QComboBox*& emplacementCombo)
{
    QDialog* dialog = new QDialog(parent);
    dialog->setObjectName("stockDialog");
    dialog->setWindowTitle(windowTitle);
    dialog->setMinimumWidth(540);
    dialog->setModal(true);

    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    QLabel* title = new QLabel(windowTitle, dialog);
    title->setObjectName("dialogTitle");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QFormLayout* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setSpacing(14);

    // Create widgets
    nomEdit   = new QLineEdit(dialog);
    typeCombo = new QComboBox(dialog);
    typeCombo->addItems({"Bois", "Métal", "Plastique", "Verre", "Peinture", "Quincaillerie", "Textile", "Autre"});

    qteEdit   = new QDoubleSpinBox(dialog); qteEdit->setRange(0, 9999999); qteEdit->setDecimals(2);
    prixEdit  = new QDoubleSpinBox(dialog); prixEdit->setRange(0, 9999999); prixEdit->setDecimals(2); prixEdit->setSuffix(" DT");
    fournEdit = new QLineEdit(dialog);
    seuilEdit = new QDoubleSpinBox(dialog); seuilEdit->setRange(0, 9999999); seuilEdit->setDecimals(2);
    dateEdit  = new QDateEdit(dialog); dateEdit->setDate(QDate::currentDate()); dateEdit->setCalendarPopup(true); dateEdit->setDisplayFormat("dd/MM/yyyy");
    consoEdit = new QDoubleSpinBox(dialog); consoEdit->setRange(0, 9999999); consoEdit->setDecimals(2);
    uniteCombo = new QComboBox(dialog);
    uniteCombo->addItems({"m", "m²", "m³", "kg", "g", "L", "mL", "pièce", "paquet", "rouleau", "barre", "feuille", "Autre"});

    produitCombo = new QComboBox(dialog);
    for (auto it = produitsMap.constBegin(); it != produitsMap.constEnd(); ++it) {
        if (it.key() == 0) produitCombo->insertItem(0, it.value(), 0);
        else               produitCombo->addItem(it.value(), it.key());
    }

    localeCombo = new QComboBox(dialog);
    localeCombo->addItem("— Aucune locale —", "");
    for (const QString& n : StockLocales::localeNames())
        localeCombo->addItem(n, n);

    emplacementCombo = new QComboBox(dialog);
    emplacementCombo->addItem("— Aucun emplacement —", "");
    for (const QString& e : StockLocales::emplacements())
        emplacementCombo->addItem(e, e);

    // Populate initial data if provided
    if (initialData) {
        nomEdit->setText(initialData->getNom());
        typeCombo->setCurrentText(initialData->getType());
        qteEdit->setValue(initialData->getQuantite());
        prixEdit->setValue(initialData->getPrixUnitaire());
        fournEdit->setText(initialData->getFournisseur());
        seuilEdit->setValue(initialData->getSeuilAlerte());
        if (!initialData->getLastOrder().isNull())
            dateEdit->setDate(initialData->getLastOrder());
        consoEdit->setValue(initialData->getConsoMensuelle());
        uniteCombo->setCurrentText(initialData->getUnite());
        int idx = produitCombo->findData(initialData->getIdProduit());
        if (idx >= 0) produitCombo->setCurrentIndex(idx);
        else produitCombo->setCurrentIndex(0);

        int locIdx = localeCombo->findData(initialData->getLocale());
        if (locIdx >= 0) localeCombo->setCurrentIndex(locIdx);

        int emplIdx = emplacementCombo->findData(initialData->getEmplacement());
        if (emplIdx >= 0) emplacementCombo->setCurrentIndex(emplIdx);
    }

    // Error labels
    QLabel* errorName   = new QLabel(dialog);
    QLabel* errorType   = new QLabel(dialog);
    QLabel* errorQty    = new QLabel(dialog);
    QLabel* errorPrice  = new QLabel(dialog);
    QLabel* errorSupp   = new QLabel(dialog);
    QLabel* errorThresh = new QLabel(dialog);
    QLabel* errorDate   = new QLabel(dialog);
    QLabel* errorConso  = new QLabel(dialog);
    QLabel* errorUnit   = new QLabel(dialog);

    for (auto lbl : {errorName, errorType, errorQty, errorPrice, errorSupp, errorThresh, errorDate, errorConso, errorUnit}) {
        lbl->setStyleSheet("color: #ef4444; font-size: 11px; padding-top: 2px;");
        lbl->hide();
    }

    auto addRow = [&](const QString& label, QWidget* field, QLabel* errorLabel) {
        QVBoxLayout* vbox = new QVBoxLayout();
        vbox->setSpacing(0);
        vbox->setContentsMargins(0, 0, 0, 0);
        vbox->addWidget(field);
        vbox->addWidget(errorLabel);
        form->addRow(label, vbox);
    };

    addRow("Nom *",                     nomEdit,     errorName);
    addRow("Type *",                    typeCombo,   errorType);
    addRow("Quantité en stock",         qteEdit,     errorQty);
    addRow("Prix unitaire (DT) *",      prixEdit,    errorPrice);
    addRow("Fournisseur *",             fournEdit,   errorSupp);
    addRow("Seuil d'alerte *",          seuilEdit,   errorThresh);
    addRow("Date dernière commande *",  dateEdit,    errorDate);
    addRow("Consommation mensuelle *",  consoEdit,   errorConso);
    addRow("Unité *",                   uniteCombo,  errorUnit);
    form->addRow("Produit associé",     produitCombo);
    form->addRow("Locale (entrepôt)",   localeCombo);
    form->addRow("Emplacement",         emplacementCombo);

    mainLayout->addLayout(form);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(isEdit ? "Modifier" : "Ajouter");
    buttons->button(QDialogButtonBox::Ok)->setObjectName("saveButton");
    buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");
    buttons->button(QDialogButtonBox::Cancel)->setObjectName("cancelButton");
    mainLayout->addWidget(buttons, 0, Qt::AlignCenter);

    // Validate and accept on OK button
    QPushButton* okButton = buttons->button(QDialogButtonBox::Ok);
    QObject::connect(okButton, &QPushButton::clicked, dialog, [=]() {
        bool ok = true;

        // Clear previous errors
        for (auto lbl : {errorName, errorType, errorQty, errorPrice, errorSupp, errorThresh, errorDate, errorConso, errorUnit}) {
            lbl->hide();
            lbl->clear();
        }

        QString err;
        // Name
        if (!StockValidators::validateName(nomEdit->text().trimmed(), err, excludeId)) {
            errorName->setText(err); errorName->show(); ok = false;
        }
        // Type
        if (!StockValidators::validateType(typeCombo->currentText(), err)) {
            errorType->setText(err); errorType->show(); ok = false;
        }
        // Quantity
        if (!StockValidators::validateQuantity(qteEdit->value(), err)) {
            errorQty->setText(err); errorQty->show(); ok = false;
        }
        // Price
        if (!StockValidators::validatePrice(prixEdit->value(), err)) {
            errorPrice->setText(err); errorPrice->show(); ok = false;
        }
        // Supplier
        if (!StockValidators::validateSupplier(fournEdit->text().trimmed(), err)) {
            errorSupp->setText(err); errorSupp->show(); ok = false;
        }
        // Threshold
        if (!StockValidators::validateThreshold(seuilEdit->value(), err)) {
            errorThresh->setText(err); errorThresh->show(); ok = false;
        }
        // Date
        if (!StockValidators::validateDate(dateEdit->date(), err)) {
            errorDate->setText(err); errorDate->show(); ok = false;
        }
        // Consumption
        if (!StockValidators::validateMonthlyConsumption(consoEdit->value(), err)) {
            errorConso->setText(err); errorConso->show(); ok = false;
        }
        // Unit
        if (!StockValidators::validateUnit(uniteCombo->currentText(), err)) {
            errorUnit->setText(err); errorUnit->show(); ok = false;
        }

        if (ok) {
            dialog->accept();
        }
    });

    QObject::connect(buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked, dialog, &QDialog::reject);

    return dialog;
}

void StockPage::onAddButtonClicked()
{
    QLineEdit* nomEdit;
    QComboBox* typeCombo;
    QDoubleSpinBox* qteEdit;
    QDoubleSpinBox* prixEdit;
    QLineEdit* fournEdit;
    QDoubleSpinBox* seuilEdit;
    QDateEdit* dateEdit;
    QDoubleSpinBox* consoEdit;
    QComboBox* uniteCombo;
    QComboBox* produitCombo;
    QComboBox* localeCombo;
    QComboBox* emplacementCombo;

    QDialog* dialog = buildMaterialDialog(this,
                                          "Ajouter un matériau",
                                          nullptr,
                                          -1,
                                          false,
                                          m_produitsMap,
                                          nomEdit,
                                          typeCombo,
                                          qteEdit,
                                          prixEdit,
                                          fournEdit,
                                          seuilEdit,
                                          dateEdit,
                                          consoEdit,
                                          uniteCombo,
                                          produitCombo,
                                          localeCombo,
                                          emplacementCombo);

    if (dialog->exec() == QDialog::Accepted) {
        StockMaterial mat;
        mat.setNom(nomEdit->text().trimmed());
        mat.setType(typeCombo->currentText());
        mat.setQuantite(qteEdit->value());
        mat.setPrixUnitaire(prixEdit->value());
        mat.setFournisseur(fournEdit->text().trimmed());
        mat.setSeuilAlerte(seuilEdit->value());
        mat.setLastOrder(dateEdit->date());
        mat.setConsoMensuelle(consoEdit->value());
        mat.setUnite(uniteCombo->currentText());
        mat.setIdProduit(produitCombo->currentData().toInt());
        mat.setLocale(localeCombo->currentData().toString());
        mat.setEmplacement(emplacementCombo->currentData().toString());

        if (StockDatabase::instance().addMaterial(mat)) {
            QMessageBox::information(this, "Succès", "Matériau ajouté avec succès.");
            refreshTable(StockDatabase::instance().getAllMaterials());
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible d'ajouter le matériau.");
        }
    }
    delete dialog;
}

void StockPage::onEditButtonClicked()
{
    if (stockTable->currentRow() < 0) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un matériau à modifier.");
        return;
    }
    StockMaterial current = materialFromCurrentRow();

    QLineEdit* nomEdit;
    QComboBox* typeCombo;
    QDoubleSpinBox* qteEdit;
    QDoubleSpinBox* prixEdit;
    QLineEdit* fournEdit;
    QDoubleSpinBox* seuilEdit;
    QDateEdit* dateEdit;
    QDoubleSpinBox* consoEdit;
    QComboBox* uniteCombo;
    QComboBox* produitCombo;

    QComboBox* localeCombo;
    QComboBox* emplacementCombo;

    QDialog* dialog = buildMaterialDialog(this,
                                          "Modifier un matériau",
                                          &current,
                                          current.getId(),
                                          true,
                                          m_produitsMap,
                                          nomEdit,
                                          typeCombo,
                                          qteEdit,
                                          prixEdit,
                                          fournEdit,
                                          seuilEdit,
                                          dateEdit,
                                          consoEdit,
                                          uniteCombo,
                                          produitCombo,
                                          localeCombo,
                                          emplacementCombo);

    if (dialog->exec() == QDialog::Accepted) {
        current.setNom(nomEdit->text().trimmed());
        current.setType(typeCombo->currentText());
        current.setQuantite(qteEdit->value());
        current.setPrixUnitaire(prixEdit->value());
        current.setFournisseur(fournEdit->text().trimmed());
        current.setSeuilAlerte(seuilEdit->value());
        current.setLastOrder(dateEdit->date());
        current.setConsoMensuelle(consoEdit->value());
        current.setUnite(uniteCombo->currentText());
        current.setIdProduit(produitCombo->currentData().toInt());
        current.setLocale(localeCombo->currentData().toString());
        current.setEmplacement(emplacementCombo->currentData().toString());

        if (StockDatabase::instance().updateMaterial(current)) {
            QMessageBox::information(this, "Succès", "Matériau mis à jour avec succès.");
            refreshTable(StockDatabase::instance().getAllMaterials());
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de mettre à jour le matériau.");
        }
    }
    delete dialog;
}
void StockPage::onDeleteButtonClicked()
{
    if (stockTable->currentRow() < 0) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un matériau à supprimer.");
        return;
    }
    StockMaterial current = materialFromCurrentRow();
    if (QMessageBox::question(this, "Confirmer la suppression",
                              QString("Êtes-vous sûr de vouloir supprimer « %1 » ?").arg(current.getNom()),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (StockDatabase::instance().deleteMaterial(current.getId())) {
            QMessageBox::information(this, "Succès", "Matériau supprimé avec succès.");
            refreshTable(StockDatabase::instance().getAllMaterials());
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer le matériau.");
        }
    }
}

void StockPage::onViewTriggered(int row, int)
{
    if (row < 0) return;
    QStringList labels = {"ID","Nom","Type","Quantité en stock","Prix unitaire (DT)",
                          "Fournisseur","Seuil alerte","Dernière commande",
                          "Conso. mensuelle","Unité","Produit associé",
                          "Locale","Emplacement"};
    QDialog dialog(this);
    dialog.setObjectName("stockViewDialog");
    dialog.setWindowTitle("Détails du matériau");
    dialog.setMinimumWidth(480);
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);
    QLabel *title = new QLabel("Détails du matériau", &dialog);
    title->setObjectName("dialogTitle"); title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);
    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight); form->setSpacing(12);
    for (int i = 1; i < stockTable->columnCount(); ++i) {
        QString valText = stockTable->item(row, i) ? stockTable->item(row, i)->text() : "—";
        QLabel *val = new QLabel(valText, &dialog); val->setObjectName("viewValue");
        form->addRow(labels[i] + " :", val);
    }
    mainLayout->addLayout(form);
    QPushButton *closeBtn = new QPushButton("Fermer", &dialog);
    closeBtn->setObjectName("backButton"); closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignCenter);
    dialog.exec();
}

// ---------------------------------------------------------------------------
//  setupTrayIcon : initialise la notification système
// ---------------------------------------------------------------------------
void StockPage::setupTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) return;

    m_trayIcon = new QSystemTrayIcon(this);
    // Utilise l'icône d'application, ou une icône générique si non définie
    QIcon icon = QApplication::windowIcon();
    if (icon.isNull())
        icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    m_trayIcon->setIcon(icon);
    m_trayIcon->setToolTip("WoodFlow — Stock");
    m_trayIcon->show();
}

// ---------------------------------------------------------------------------
//  onBellClicked : ouvre / ferme le panneau de notifications
// ---------------------------------------------------------------------------
void StockPage::onBellClicked()
{
    if (m_alertPanel->isVisible()) {
        m_alertPanel->hide();
        return;
    }

    // Recharger les matériaux en alerte
    QList<StockMaterial> all = StockDatabase::instance().getAllMaterials();
    QList<StockMaterial> alerts;
    for (const auto& m : all)
        if (m.isBelowAlert()) alerts.append(m);

    m_alertPanel->populate(alerts);
    m_alertPanel->showUnder(m_bell);
}

// ---------------------------------------------------------------------------
//  onAlertSelected : l'utilisateur a cliqué sur une carte → sélectionner la ligne
// ---------------------------------------------------------------------------
void StockPage::onAlertSelected(int materialId)
{
    // S'assurer qu'on est sur la page table
    mainStack->setCurrentIndex(0);

    // Chercher la ligne correspondant à l'ID dans le QTableWidget
    for (int row = 0; row < stockTable->rowCount(); ++row) {
        QTableWidgetItem *idItem = stockTable->item(row, 0); // colonne ID (cachée)
        if (idItem && idItem->text().toInt() == materialId) {
            stockTable->selectRow(row);
            stockTable->scrollToItem(idItem);
            break;
        }
    }
}

// ---------------------------------------------------------------------------
//  onNewAlertsDetected : nouvelles alertes → notification système
// ---------------------------------------------------------------------------
void StockPage::onNewAlertsDetected(int count)
{
    if (!m_trayIcon || count == 0) return;

    QString msg = count == 1
                      ? "1 matériau est passé sous son seuil d'alerte."
                      : QString("%1 matériaux sont passés sous leur seuil d'alerte.").arg(count);

    m_trayIcon->showMessage(
        "Alerte Stock WoodFlow",
        msg,
        QSystemTrayIcon::Warning,
        5000 // durée en ms
        );
}

void StockPage::onShowAlertsClicked()
{
    QList<StockMaterial> all = StockDatabase::instance().getAllMaterials();
    QStringList alertLines;
    for (const auto& m : all)
        if (m.isBelowAlert())
            alertLines << QString("• %1 — stock: %2 (seuil: %3)")
                              .arg(m.getNom()).arg(m.getQuantite()).arg(m.getSeuilAlerte());
    if (alertLines.isEmpty())
        QMessageBox::information(this, "Alertes Stock", "Aucun matériau en dessous du seuil d'alerte.");
    else
        QMessageBox::warning(this, "Alertes Stock",
                             QString("%1 matériau(x) en alerte :\n\n").arg(alertLines.size()) + alertLines.join("\n"));
}

void StockPage::onExportAlertPdfClicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter les alertes en PDF", "alertes_stock.pdf", "Fichiers PDF (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Landscape);

    QList<StockMaterial> all = StockDatabase::instance().getAllMaterials();
    QString html = "<html><body style='font-family:Arial;'>";
    html += "<h2 style='color:#c0392b;'>Rapport d'alertes Stock — WoodFlow</h2>";
    html += "<p>Date : " + QDate::currentDate().toString("dd/MM/yyyy") + "</p>";
    html += "<table border='1' cellspacing='0' cellpadding='5' width='100%'>";
    html += "<tr style='background:#c0392b;color:white;'><th>Nom</th><th>Type</th><th>Quantité</th>"
            "<th>Seuil</th><th>Fournisseur</th><th>Conso. mensuelle</th><th>Produit associé</th>tr";
    bool hasAlerts = false;
    for (const auto& m : all) {
        if (m.isBelowAlert()) {
            hasAlerts = true;
            QString nomProd = m_produitsMap.value(m.getIdProduit(), "—");
            if (m.getIdProduit() == 0) nomProd = "—";
            html += QString("<tr style='background:#ffd5d5;'><td>%1</td><td>%2</td><td>%3</td>"
                            "<td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
                        .arg(m.getNom()).arg(m.getType()).arg(m.getQuantite()).arg(m.getSeuilAlerte())
                        .arg(m.getFournisseur()).arg(m.getConsoMensuelle()).arg(nomProd);
        }
    }
    if (!hasAlerts) html += "<tr><td colspan='7'>Aucun article en alerte.</td></tr>";
    html += "</table></body></html>";
    QTextDocument doc; doc.setHtml(html); doc.print(&printer);
    QMessageBox::information(this, "Export PDF", "Rapport d'alertes exporté avec succès :\n" + fileName);
}

void StockPage::onSearchTextChanged(const QString& text)
{
    if (text.trimmed().isEmpty()) { onSortChanged(sortCombo->currentIndex()); return; }
    refreshTable(StockDatabase::instance().searchByNomOrType(text.trimmed()));
}

void StockPage::onSortChanged(int index)
{
    QString searchText = searchEdit->text().trimmed();
    QList<StockMaterial> base = searchText.isEmpty()
                                    ? StockDatabase::instance().getAllMaterials()
                                    : StockDatabase::instance().searchByNomOrType(searchText);
    switch (index) {
    case 1: std::sort(base.begin(), base.end(), [](const StockMaterial& a, const StockMaterial& b){ return a.getNom() < b.getNom(); }); break;
    case 2: std::sort(base.begin(), base.end(), [](const StockMaterial& a, const StockMaterial& b){ return a.getNom() > b.getNom(); }); break;
    case 3: std::sort(base.begin(), base.end(), [](const StockMaterial& a, const StockMaterial& b){ return a.getConsoMensuelle() < b.getConsoMensuelle(); }); break;
    case 4: std::sort(base.begin(), base.end(), [](const StockMaterial& a, const StockMaterial& b){ return a.getConsoMensuelle() > b.getConsoMensuelle(); }); break;
    default: break;
    }
    refreshTable(base);
}

void StockPage::onFilterByProduitChanged(int)
{
    int idProduit = produitFilterCombo->currentData().toInt();
    QList<StockMaterial> list = (idProduit == 0)
                                    ? StockDatabase::instance().getAllMaterials()
                                    : StockDatabase::instance().getMaterialsByProduit(idProduit);
    searchEdit->blockSignals(true); searchEdit->clear(); searchEdit->blockSignals(false);
    refreshTable(list);
}
void StockPage::setupMapPage()
{
    mapPage = new QWidget();
    QVBoxLayout *pageLayout = new QVBoxLayout(mapPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // ── Back button header ───────────────────────────────────────────────────
    QFrame *header = new QFrame(mapPage);
    header->setObjectName("mapHeader");

    bool isDark = qApp->styleSheet().contains("0f0f0f");
    header->setStyleSheet(isDark
                              ? "QFrame#mapHeader { background:#1a1a1a; border-bottom:1px solid #2a2a2a; }"
                              : "QFrame#mapHeader { background:#ffffff; border-bottom:1px solid #e5e7eb; }");

    QHBoxLayout *hLay = new QHBoxLayout(header);
    hLay->setContentsMargins(16, 10, 16, 10);

    QPushButton *backBtn = new QPushButton("<- Retour au stock", mapPage);
    backBtn->setObjectName("backButton");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setFixedHeight(34);
    connect(backBtn, &QPushButton::clicked, this, &StockPage::onBackFromMapClicked);

    QLabel *pageTitle = new QLabel("Carte des entrepots — Tunisie", mapPage);
    pageTitle->setObjectName("dialogTitle");
    {
        QFont f = pageTitle->font(); f.setPixelSize(15); f.setBold(true);
        pageTitle->setFont(f);
    }

    hLay->addWidget(backBtn);
    hLay->addStretch();
    hLay->addWidget(pageTitle);
    hLay->addStretch();
    pageLayout->addWidget(header);

    // ── Body: StockMapView (with built-in toolbar) + side panel ─────────────
    QHBoxLayout *body = new QHBoxLayout();
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);

    m_mapView = new StockMapView(mapPage);
    m_mapView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    WarehouseDetailPanel *detailPanel = new WarehouseDetailPanel(mapPage);

    connect(m_mapView,   &StockMapView::localeClicked,
            detailPanel, &WarehouseDetailPanel::showLocale);

    connect(detailPanel, &WarehouseDetailPanel::materialSelected,
            this,        &StockPage::onAlertSelected);

    body->addWidget(m_mapView, 1);
    body->addWidget(detailPanel);

    pageLayout->addLayout(body, 1);
}
void StockPage::onShowMapClicked()
{
    if (m_mapView)
        m_mapView->refresh();
    mainStack->setCurrentWidget(mapPage);
}

void StockPage::onBackFromMapClicked()
{
    mainStack->setCurrentWidget(tablePage);
}
// ============================================================================
//  Arduino — barre de connexion et gestion du poids
// ============================================================================

void StockPage::setupArduinoBar()
{
    m_arduino = new ArduinoBridge(this);

    connect(m_arduino, &ArduinoBridge::weightChanged,
            this,      &StockPage::onWeightChanged);
    connect(m_arduino, &ArduinoBridge::connectionStatusChanged,
            this,      &StockPage::onArduinoConnectionChanged);
    connect(m_arduino, &ArduinoBridge::errorOccurred,
            this,      &StockPage::onArduinoError);

    // ── Barre visuelle en bas de la page table ───────────────────────────
    QFrame *bar = new QFrame(tablePage);
    bar->setFixedHeight(44);
    bar->setStyleSheet(
        "QFrame { background: palette(base);"
        "         border-top: 1px solid rgba(0,0,0,0.09); }");

    QHBoxLayout *lay = new QHBoxLayout(bar);
    lay->setContentsMargins(14, 0, 14, 0);
    lay->setSpacing(10);

    // Pastille de statut (grise par défaut)
    m_arduinoStatus = new QLabel(bar);
    m_arduinoStatus->setFixedSize(10, 10);
    m_arduinoStatus->setToolTip("Arduino déconnecté");
    m_arduinoStatus->setStyleSheet(
        "background: #9ca3af; border-radius: 5px;");

    QLabel *lbl = new QLabel("Arduino :", bar);
    { QFont f = lbl->font(); f.setPixelSize(12); lbl->setFont(f); }

    // ComboBox ports série
    m_portCombo = new QComboBox(bar);
    m_portCombo->setFixedWidth(100);
    m_portCombo->setFixedHeight(28);
    for (const QString& p : ArduinoBridge::availablePorts())
        m_portCombo->addItem(p);
    // Pré-sélectionner COM12 si disponible
    int idx = m_portCombo->findText("COM12");
    if (idx >= 0) m_portCombo->setCurrentIndex(idx);

    // Boutons
    m_connectBtn    = new QPushButton("Connecter",   bar);
    m_disconnectBtn = new QPushButton("Déconnecter", bar);
    m_tareBtn       = new QPushButton("Tare",        bar);

    for (auto btn : {m_connectBtn, m_disconnectBtn, m_tareBtn}) {
        btn->setFixedHeight(28);
        btn->setCursor(Qt::PointingHandCursor);
    }
    m_disconnectBtn->setEnabled(false);
    m_tareBtn->setEnabled(false);

    // Label poids temps réel
    m_arduinoWeightLabel = new QLabel("— g", bar);
    { QFont f = m_arduinoWeightLabel->font();
        f.setPixelSize(13); f.setBold(true);
        m_arduinoWeightLabel->setFont(f); }

    lay->addWidget(m_arduinoStatus);
    lay->addWidget(lbl);
    lay->addWidget(m_portCombo);
    lay->addWidget(m_connectBtn);
    lay->addWidget(m_disconnectBtn);
    lay->addWidget(m_tareBtn);
    lay->addStretch();
    lay->addWidget(m_arduinoWeightLabel);

    // Ajouter la barre au layout de tablePage
    QVBoxLayout *tl = qobject_cast<QVBoxLayout*>(tablePage->layout());
    if (tl) tl->addWidget(bar);

    // ── Connexions boutons ───────────────────────────────────────────────
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        QString port = m_portCombo->currentText();
        if (port.isEmpty()) return;
        if (m_arduino->open(port)) {
            m_connectBtn->setEnabled(false);
            m_disconnectBtn->setEnabled(true);
            m_tareBtn->setEnabled(true);
            m_portCombo->setEnabled(false);
        }
    });

    connect(m_disconnectBtn, &QPushButton::clicked, this, [this]() {
        m_arduino->close();
        m_connectBtn->setEnabled(true);
        m_disconnectBtn->setEnabled(false);
        m_tareBtn->setEnabled(false);
        m_portCombo->setEnabled(true);
        m_arduinoWeightLabel->setText("— g");
    });

    connect(m_tareBtn, &QPushButton::clicked,
            m_arduino, &ArduinoBridge::tare);
}

void StockPage::onWeightChanged(double grams)
{
    // 1. Mettre à jour le label en temps réel
    m_arduinoWeightLabel->setText(
        QString("%1 g").arg(grams, 0, 'f', 1));

    // 2. Mettre à jour QUANTITE_MAT dans Oracle
    bool ok = StockDatabase::instance()
                  .updateQuantiteByNom(m_arduinoMaterialName, grams);

    if (ok) {
        // 3. Rafraîchir la table Qt
        refreshTable(StockDatabase::instance().getAllMaterials());
        // La cloche se met à jour via son propre QTimer (déjà en place)
    } else {
        qWarning() << "[StockPage] Échec mise à jour BD pour"
                   << m_arduinoMaterialName;
    }
}

void StockPage::onArduinoConnectionChanged(bool connected)
{
    if (!m_arduinoStatus) return;
    if (connected) {
        m_arduinoStatus->setStyleSheet(
            "background: #22c55e; border-radius: 5px;");
        m_arduinoStatus->setToolTip("Arduino connecté");
    } else {
        m_arduinoStatus->setStyleSheet(
            "background: #9ca3af; border-radius: 5px;");
        m_arduinoStatus->setToolTip("Arduino déconnecté");
    }
}

void StockPage::onArduinoError(const QString& msg)
{
    qWarning() << "[Arduino]" << msg;
    QMessageBox::warning(this, "Erreur Arduino", msg);
}