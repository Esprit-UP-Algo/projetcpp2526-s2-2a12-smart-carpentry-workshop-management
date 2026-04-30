#include "projectstatspage.h"
#include "src/database/projectdatabase.h"

// ── Qt Charts ── Qt5/Qt6 compatibility ──────────────────────────
// Qt5: classes live in QtCharts namespace → QT_CHARTS_USE_NAMESPACE pulls them in
// Qt6: classes are already in the global namespace → macro does not exist
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>

    // ─────────────────────────────────────────────
    //  Small helper: titled chart frame
    // ─────────────────────────────────────────────
    static QFrame* chartCard(const QString& title, QChartView* view, QWidget* parent)
{
    QFrame* card = new QFrame(parent);
    card->setStyleSheet(
        "QFrame { background:#ffffff; border-radius:12px; "
        "border:1px solid #e2e8f0; }");

    QVBoxLayout* vl = new QVBoxLayout(card);
    vl->setContentsMargins(16, 12, 16, 12);
    vl->setSpacing(8);

    QLabel* lbl = new QLabel(title, card);
    lbl->setStyleSheet("font-size:14px; font-weight:700; color:#374151; border:none;");
    vl->addWidget(lbl);

    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(260);
    view->setStyleSheet("border:none; background:transparent;");
    vl->addWidget(view);

    return card;
}

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
ProjectStatsPage::ProjectStatsPage(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Statistiques des Projets");
    setMinimumSize(900, 620);
    setModal(true);
    setStyleSheet("QDialog { background:#f1f5f9; }");
    setupUI();
}

// ─────────────────────────────────────────────
//  UI
// ─────────────────────────────────────────────

void ProjectStatsPage::setupUI()
{
    ProjectDatabase& db = ProjectDatabase::instance();

    // ── Fetch data ──────────────────────────────────────────────
    QMap<QString, int>    statusCount = db.getCountPerStatus();
    QMap<QString, int>    typeCount   = db.getCountPerType();
    int    total     = db.getTotalCount();
    int    overdue   = db.getOverdueCount();
    double budget    = db.getTotalBudget();
    double avgBudget = db.getAverageBudget();

    // ── Outer layout ────────────────────────────────────────────
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    // ── Title ───────────────────────────────────────────────────
    QLabel* title = new QLabel("Tableau de Bord — Projets", this);
    title->setStyleSheet("font-size:20px; font-weight:800; color:#1e293b;");
    mainLayout->addWidget(title);

    // ── KPI strip ───────────────────────────────────────────────
    QHBoxLayout* kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(12);

    struct KPI { QString label; QString value; QString bg; };
    QList<KPI> kpis = {
                       { "Total Projets",     QString::number(total),                              "#5b6f8a" },
                       { "En cours",          QString::number(statusCount.value("En cours",  0)),  "#2ecc71" },
                       { "Terminés",          QString::number(statusCount.value(
                                        [&]{ for(auto k : statusCount.keys()) if(k.contains("termin", Qt::CaseInsensitive)) return k;
         return QString("Terminé"); }(), 0)),  "#3498db" },
                       { "En retard",         QString::number(overdue),                            "#e74c3c" },
                       { "Budget Total (DT)", QString::number(budget,    'f', 0),                  "#8A9A5B" },
                       { "Budget Moyen (DT)", QString::number(avgBudget, 'f', 0),                  "#9b59b6" },
                       };

    for (const KPI& k : kpis) {
        QFrame* card = new QFrame(this);
        card->setStyleSheet(QString(
                                "QFrame{background:%1;border-radius:10px;padding:4px;}").arg(k.bg));
        card->setFixedHeight(72);
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(14, 6, 14, 6);
        QLabel* lbl = new QLabel(k.label, card);
        lbl->setStyleSheet("color:rgba(255,255,255,0.85);font-size:10px;font-weight:600;border:none;");
        QLabel* val = new QLabel(k.value, card);
        val->setStyleSheet("color:white;font-size:20px;font-weight:800;border:none;");
        cl->addWidget(lbl);
        cl->addWidget(val);
        kpiRow->addWidget(card);
    }
    mainLayout->addLayout(kpiRow);

    // ── Scroll area for charts ───────────────────────────────────
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;background:transparent;}");
    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background:transparent;");
    QVBoxLayout* chartsLayout = new QVBoxLayout(scrollContent);
    chartsLayout->setSpacing(16);
    chartsLayout->setContentsMargins(0, 0, 0, 0);

    // ── Chart 1: Donut Pie – Répartition par statut ──────────────
    {
        QPieSeries* pie = new QPieSeries();

        // Color map — fallback to grey if status not recognized
        auto colorForStatus = [](const QString& s) -> QString {
            if (s.contains("cours",   Qt::CaseInsensitive)) return "#2ecc71";
            if (s.contains("attente", Qt::CaseInsensitive)) return "#f39c12";
            if (s.contains("termin",  Qt::CaseInsensitive)) return "#3498db"; // matches "Terminé" or "Termine"
            if (s.contains("annul",   Qt::CaseInsensitive)) return "#e74c3c"; // matches "Annulé" or "Annule"
            return "#aaaaaa";
        };

        // Calculate total for percentages
        int totalStatusCount = 0;
        for (auto it = statusCount.constBegin(); it != statusCount.constEnd(); ++it)
            totalStatusCount += it.value();

        for (auto it = statusCount.constBegin(); it != statusCount.constEnd(); ++it) {
            if (it.value() <= 0) continue;

            double percent = totalStatusCount > 0
                                 ? (100.0 * it.value() / totalStatusCount)
                                 : 0.0;
            QString label = QString("%1  (%2)  %3%")
                                .arg(it.key())
                                .arg(it.value())
                                .arg(percent, 0, 'f', 1);   // 1 decimal place

            QPieSlice* slice = pie->append(label, it.value());
            slice->setColor(QColor(colorForStatus(it.key())));
            slice->setLabelVisible(true);
            slice->setLabelColor(QColor("#1e293b"));
        }
        pie->setHoleSize(0.38);   // donut

        QChart* chart = new QChart();
        chart->addSeries(pie);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);
        chart->setBackgroundBrush(Qt::transparent);
        chart->setMargins(QMargins(0, 0, 0, 0));
        chart->setDropShadowEnabled(false);

        QChartView* view = new QChartView(chart);
        chartsLayout->addWidget(
            chartCard("  Répartition par Statut", view, scrollContent));
    }

    // ── Chart 2: Bar – Projets par type ──────────────────────────
    {
        QBarSet* set = new QBarSet("Projets");
        set->setColor(QColor("#8A9A5B"));
        set->setBorderColor(QColor("#6a8a4b"));

        QStringList categories;
        for (auto it = typeCount.constBegin(); it != typeCount.constEnd(); ++it) {
            categories << it.key();
            *set << it.value();
        }

        QBarSeries* series = new QBarSeries();
        series->append(set);

        QChart* chart = new QChart();
        chart->addSeries(series);
        chart->setBackgroundBrush(Qt::transparent);
        chart->setMargins(QMargins(0, 0, 0, 0));
        chart->setDropShadowEnabled(false);

        QBarCategoryAxis* axisX = new QBarCategoryAxis();
        axisX->append(categories);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis* axisY = new QValueAxis();
        axisY->setLabelFormat("%d");

        // Find the highest bar value
        int maxCount = 0;
        for (int val : typeCount.values())
            if (val > maxCount) maxCount = val;

        // Set range always starting at zero
        axisY->setRange(0, maxCount + 1);

        // For small ranges, use integers steps to avoid duplicate labels
        if (maxCount <= 10) {
            axisY->setTickInterval(1);   // ticks at 0,1,2,...maxCount+1
        } else {
            axisY->setTickCount(6);      // about 6 evenly spaced ticks
        }
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
        chart->legend()->setVisible(false);

        QChartView* view = new QChartView(chart);
        chartsLayout->addWidget(
            chartCard("  Projets par Type", view, scrollContent));
    }

    chartsLayout->addStretch();
    scroll->setWidget(scrollContent);
    mainLayout->addWidget(scroll, 1);

    // ── Close button ────────────────────────────────────────────
    QPushButton* closeBtn = new QPushButton("Fermer", this);
    closeBtn->setFixedHeight(38);
    closeBtn->setStyleSheet(
        "QPushButton{background:#8A9A5B;color:white;border:none;border-radius:8px;"
        "font-size:13px;font-weight:700;padding:0 28px;}"
        "QPushButton:hover{background:#9aaa6b;}");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);
}
