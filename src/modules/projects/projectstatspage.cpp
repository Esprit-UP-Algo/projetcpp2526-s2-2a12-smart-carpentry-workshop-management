#include "projectstatspage.h"
#include "src/database/projectdatabase.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QMap>
#include <QFont>
#include <QScrollArea>
#include <cmath>

// ============================================================
//  Small painted widget: vertical bar chart
// ============================================================
class BarChartWidget : public QWidget {

public:
    struct Bar { QString label; double value; QColor color; };

    explicit BarChartWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(200);
    }
    void setBars(const QList<Bar>& bars) { m_bars = bars; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        if (m_bars.isEmpty()) return;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const int marginL = 40, marginB = 40, marginT = 16, marginR = 16;
        QRect area(marginL, marginT, width() - marginL - marginR, height() - marginT - marginB);

        double maxVal = 1;
        for (auto& b : m_bars) maxVal = qMax(maxVal, b.value);

        int n = m_bars.size();
        double barW = (double)area.width() / n * 0.55;
        double gap  = (double)area.width() / n;

        // Grid lines
        p.setPen(QPen(QColor(220,220,220), 1, Qt::DashLine));
        for (int i = 0; i <= 4; ++i) {
            int y = area.bottom() - (int)(i / 4.0 * area.height());
            p.drawLine(area.left(), y, area.right(), y);
            p.setPen(QColor(150,150,150));
            p.drawText(0, y + 5, marginL - 4, 14, Qt::AlignRight,
                       QString::number((int)(maxVal * i / 4)));
            p.setPen(QPen(QColor(220,220,220), 1, Qt::DashLine));
        }

        for (int i = 0; i < n; ++i) {
            const Bar& b = m_bars[i];
            double ratio = (maxVal > 0) ? b.value / maxVal : 0;
            int barH = (int)(ratio * area.height());
            int x = area.left() + (int)(i * gap + (gap - barW) / 2);
            int y = area.bottom() - barH;

            // Shadow
            p.fillRect(x + 3, y + 3, (int)barW, barH, QColor(0,0,0,18));

            // Bar with gradient
            QLinearGradient grad(x, y, x, area.bottom());
            grad.setColorAt(0, b.color.lighter(115));
            grad.setColorAt(1, b.color);
            p.setPen(Qt::NoPen);
            QPainterPath path;
            path.addRoundedRect(x, y, (int)barW, barH, 5, 5);
            p.fillPath(path, grad);

            // Value label
            p.setPen(QColor(50,50,50));
            p.setFont(QFont("Arial", 9, QFont::Bold));
            p.drawText(x, y - 18, (int)barW, 18, Qt::AlignCenter,
                       QString::number((int)b.value));

            // X label
            p.setFont(QFont("Arial", 8));
            p.setPen(QColor(80,80,80));
            p.drawText(x - 10, area.bottom() + 4, (int)barW + 20, 30,
                       Qt::AlignHCenter | Qt::AlignTop, b.label);
        }

        // Axis
        p.setPen(QPen(QColor(180,180,180), 1));
        p.drawLine(area.left(), area.top(), area.left(), area.bottom());
        p.drawLine(area.left(), area.bottom(), area.right(), area.bottom());
    }

private:
    QList<Bar> m_bars;
};

// ============================================================
//  Donut chart widget (types de projets)
// ============================================================
class DonutChartWidget : public QWidget {
public:
    struct Slice { QString label; double value; QColor color; };

    explicit DonutChartWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(220);
    }
    void setSlices(const QList<Slice>& slices) { m_slices = slices; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        if (m_slices.isEmpty()) return;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        double total = 0;
        for (auto& s : m_slices) total += s.value;
        if (total == 0) return;

        int side = qMin(width() / 2, height()) - 20;
        QRect rect((width() / 2 - side) / 2, (height() - side) / 2, side, side);

        double angle = -90.0;
        for (auto& s : m_slices) {
            double sweep = s.value / total * 360.0;
            p.setPen(QPen(Qt::white, 2));
            p.setBrush(s.color);
            p.drawPie(rect, (int)(angle * 16), (int)(sweep * 16));
            angle += sweep;
        }

        // Donut hole
        p.setPen(Qt::NoPen);
        p.setBrush(palette().window());
        int hole = side / 3;
        p.drawEllipse(rect.center(), hole, hole);

        // Legend
        int lx = width() / 2 + 10, ly = 20;
        p.setFont(QFont("Arial", 9));
        for (auto& s : m_slices) {
            p.setPen(Qt::NoPen);
            p.setBrush(s.color);
            p.drawRoundedRect(lx, ly, 14, 14, 3, 3);
            p.setPen(QColor(60,60,60));
            p.drawText(lx + 20, ly, 200, 16, Qt::AlignVCenter | Qt::AlignLeft,
                       QString("%1 (%2)").arg(s.label).arg((int)s.value));
            ly += 22;
        }
    }

private:
    QList<Slice> m_slices;
};

// ============================================================
//  ProjectStatsPage
// ============================================================
ProjectStatsPage::ProjectStatsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    refresh();
}

void ProjectStatsPage::setupUI()
{
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *content = new QWidget();
    scroll->setWidget(content);

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->addWidget(scroll);

    QVBoxLayout *vl = new QVBoxLayout(content);
    vl->setContentsMargins(24, 24, 24, 24);
    vl->setSpacing(20);

    // ── Page title ──────────────────────────────────────────
    QLabel *title = new QLabel("📊  Statistiques d'Avancement des Projets", content);
    title->setStyleSheet("font-size:20px;font-weight:bold;color:#2c3e50;");
    vl->addWidget(title);

    // ── Refresh button ──────────────────────────────────────
    QPushButton *refreshBtn = new QPushButton("🔄  Actualiser", content);
    refreshBtn->setFixedHeight(34);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(
        "QPushButton{background:#8A9A5B;color:white;border:none;border-radius:6px;"
        "padding:6px 18px;font-weight:bold;}"
        "QPushButton:hover{background:#9aaa6b;}");
    connect(refreshBtn, &QPushButton::clicked, this, &ProjectStatsPage::refresh);
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addWidget(refreshBtn);
    btnRow->addStretch();
    vl->addLayout(btnRow);

    // ── KPI cards ───────────────────────────────────────────
    auto makeCard = [&](const QString& lbl, QLabel*& valLbl,
                        const QString& bg, const QString& icon) -> QFrame* {
        QFrame *f = new QFrame(content);
        f->setStyleSheet(QString(
                             "QFrame{background:%1;border-radius:12px;}"
                             "QLabel{color:white;}").arg(bg));
        f->setFixedHeight(90);
        QVBoxLayout *cl = new QVBoxLayout(f);
        cl->setContentsMargins(16,10,16,10);
        QLabel *icoLbl = new QLabel(icon + "  " + lbl, f);
        icoLbl->setStyleSheet("font-size:10pt;opacity:0.9;");
        valLbl = new QLabel("—", f);
        valLbl->setStyleSheet("font-size:22pt;font-weight:bold;");
        cl->addWidget(icoLbl);
        cl->addWidget(valLbl);
        return f;
    };

    QGridLayout *kpi = new QGridLayout();
    kpi->setSpacing(14);
    kpi->addWidget(makeCard("Total Projets",   m_lblTotal,      "#5b6f8a", "📁"), 0, 0);
    kpi->addWidget(makeCard("En cours",        m_lblEnCours,    "#27ae60", "🔄"), 0, 1);
    kpi->addWidget(makeCard("En attente",      m_lblEnAttente,  "#f39c12", "⏳"), 0, 2);
    kpi->addWidget(makeCard("Terminés",        m_lblTermines,   "#2980b9", "✅"), 0, 3);
    kpi->addWidget(makeCard("Annulés",         m_lblAnnules,    "#e74c3c", "❌"), 1, 0);
    kpi->addWidget(makeCard("En retard",       m_lblEnRetard,   "#8e44ad", "⚠️"), 1, 1);
    kpi->addWidget(makeCard("Budget Total DT", m_lblBudgetTotal,"#8A9A5B", "💰"), 1, 2);
    kpi->addWidget(makeCard("Budget Moyen DT", m_lblBudgetMoyen,"#16a085", "📈"), 1, 3);
    vl->addLayout(kpi);

    // ── Chart section ────────────────────────────────────────
    auto makeSection = [&](const QString& header) -> QFrame* {
        QFrame *f = new QFrame(content);
        f->setStyleSheet(
            "QFrame{background:white;border-radius:10px;border:1px solid #e5e7eb;}"
            "QLabel{color:#2c3e50;}");
        QVBoxLayout *fl = new QVBoxLayout(f);
        fl->setContentsMargins(18,14,18,18);
        fl->setSpacing(10);
        QLabel *h = new QLabel(header, f);
        h->setStyleSheet("font-size:13px;font-weight:bold;color:#8A9A5B;"
                         "border-bottom:2px solid #8A9A5B;padding-bottom:4px;");
        fl->addWidget(h);
        return f;
    };

    QHBoxLayout *chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(14);

    // Bar chart — avancement par statut
    QFrame *barSec = makeSection("Répartition par Statut");
    m_barChart = new BarChartWidget(barSec);
    m_barChart->setMinimumHeight(220);
    dynamic_cast<QVBoxLayout*>(barSec->layout())->addWidget(m_barChart);

    // Donut — types
    QFrame *pieSec = makeSection("Répartition par Type de Projet");
    m_pieChart = new DonutChartWidget(pieSec);
    m_pieChart->setMinimumHeight(220);
    dynamic_cast<QVBoxLayout*>(pieSec->layout())->addWidget(m_pieChart);

    chartsRow->addWidget(barSec, 1);
    chartsRow->addWidget(pieSec, 1);
    vl->addLayout(chartsRow);

    // Budget bar chart
    QFrame *budSec = makeSection("Budget par Type de Projet (DT)");
    m_budgetBar = new BarChartWidget(budSec);
    m_budgetBar->setMinimumHeight(220);
    dynamic_cast<QVBoxLayout*>(budSec->layout())->addWidget(m_budgetBar);
    vl->addWidget(budSec);
}

void ProjectStatsPage::refresh()
{
    ProjectDatabase& db = ProjectDatabase::instance();

    // KPI values
    int total      = db.getTotalCount();
    int enCours    = db.getCountByStatus("En cours");
    int enAttente  = db.getCountByStatus("En attente");
    int termines   = db.getCountByStatus("Terminé");
    int annules    = db.getCountByStatus("Annulé");
    int enRetard   = db.getOverdueCount();
    double budgetT = db.getTotalBudget();
    double budgetM = db.getAverageBudget();

    m_lblTotal->setText(QString::number(total));
    m_lblEnCours->setText(QString::number(enCours));
    m_lblEnAttente->setText(QString::number(enAttente));
    m_lblTermines->setText(QString::number(termines));
    m_lblAnnules->setText(QString::number(annules));
    m_lblEnRetard->setText(QString::number(enRetard));
    m_lblBudgetTotal->setText(QString::number(budgetT, 'f', 0));
    m_lblBudgetMoyen->setText(QString::number(budgetM, 'f', 0));

    // ── Bar chart: statuts ───────────────────────────────────
    QList<BarChartWidget::Bar> bars;
    bars << BarChartWidget::Bar{"En cours",   (double)enCours,   QColor("#27ae60")}
         << BarChartWidget::Bar{"En attente", (double)enAttente, QColor("#f39c12")}
         << BarChartWidget::Bar{"Terminé",    (double)termines,  QColor("#2980b9")}
         << BarChartWidget::Bar{"Annulé",     (double)annules,   QColor("#e74c3c")};
    dynamic_cast<BarChartWidget*>(m_barChart)->setBars(bars);

    // ── Donut: types ─────────────────────────────────────────
    QMap<QString, int> typesMap = db.getCountPerType();
    QList<QColor> palette = {
        QColor("#8A9A5B"), QColor("#2980b9"), QColor("#f39c12"),
        QColor("#27ae60"), QColor("#e74c3c"), QColor("#8e44ad"),
        QColor("#16a085"), QColor("#d35400")
    };
    QList<DonutChartWidget::Slice> slices;
    int ci = 0;
    for (auto it = typesMap.cbegin(); it != typesMap.cend(); ++it, ++ci)
        slices << DonutChartWidget::Slice{it.key(), (double)it.value(),
                                          palette[ci % palette.size()]};
    dynamic_cast<DonutChartWidget*>(m_pieChart)->setSlices(slices);

    // ── Budget bar chart per type ────────────────────────────
    // Build budget per type from all projects
    QMap<QString, double> budgetPerType;
    for (const Projet& p : db.getAllProjets())
        budgetPerType[p.getType()] += p.getBudget();

    QList<BarChartWidget::Bar> budgetBars;
    ci = 0;
    for (auto it = budgetPerType.cbegin(); it != budgetPerType.cend(); ++it, ++ci)
        budgetBars << BarChartWidget::Bar{it.key(), it.value(),
                                          palette[ci % palette.size()]};
    dynamic_cast<BarChartWidget*>(m_budgetBar)->setBars(budgetBars);
}
