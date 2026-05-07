#include "projectcalendarpage.h"
#include "src/database/projectdatabase.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextCharFormat>
#include <QListWidgetItem>
#include <QFrame>
#include <QFont>

// ─────────────────────────────────────────────
//  Colour per status
// ─────────────────────────────────────────────
static QColor statusColor(const QString& s)
{
    if (s == "En cours")    return QColor("#2ecc71");
    if (s == "En attente")  return QColor("#f39c12");
    if (s == "Terminé")     return QColor("#3498db");
    if (s == "Annulé")      return QColor("#e74c3c");
    return QColor("#aaaaaa");
}

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
ProjectCalendarPage::ProjectCalendarPage(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Calendrier des Deadlines");
    setMinimumSize(780, 540);
    setModal(true);
    setStyleSheet("QDialog{background:#f1f5f9;}");
    setupUI();
}

// ─────────────────────────────────────────────
//  Build deadline map from DB
// ─────────────────────────────────────────────
void ProjectCalendarPage::paintDeadlines()
{
    m_deadlineMap.clear();

    // Reset all custom formats first
    m_cal->setDateTextFormat(QDate(), QTextCharFormat());

    QList<Projet> all = ProjectDatabase::instance().getAllProjets();
    for (const Projet& p : all) {
        if (!p.getDeadline().isValid()) continue;
        m_deadlineMap[p.getDeadline()].append(p);
    }

    // Apply a coloured dot background for every deadline date.
    // If multiple projects share a date we pick the "worst" status colour:
    // overdue > en attente > en cours > terminé > annulé
    auto priority = [](const QString& s) {
        if (s == "En attente") return 4;
        if (s == "En cours")   return 3;
        if (s == "Annulé")     return 2;
        if (s == "Terminé")    return 1;
        return 0;
    };

    for (auto it = m_deadlineMap.begin(); it != m_deadlineMap.end(); ++it) {
        const QDate& d = it.key();
        const QList<Projet>& projs = it.value();

        QString dominant = projs.first().getStatus();
        // Overdue check trumps everything
        bool anyOverdue = false;
        for (const Projet& p : projs) {
            if (p.isOverdue()) { anyOverdue = true; break; }
            if (priority(p.getStatus()) > priority(dominant))
                dominant = p.getStatus();
        }

        QTextCharFormat fmt;
        QColor bg = anyOverdue ? QColor("#e74c3c") : statusColor(dominant);
        bg.setAlpha(180);
        fmt.setBackground(bg);
        fmt.setForeground(QColor("#ffffff"));
        QFont f = fmt.font();
        f.setBold(true);
        fmt.setFont(f);
        m_cal->setDateTextFormat(d, fmt);
    }

    // Highlight today
    QTextCharFormat todayFmt = m_cal->dateTextFormat(QDate::currentDate());
    todayFmt.setFontUnderline(true);
    m_cal->setDateTextFormat(QDate::currentDate(), todayFmt);
}

// ─────────────────────────────────────────────
//  UI
// ─────────────────────────────────────────────
void ProjectCalendarPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    // Title
    QLabel* title = new QLabel("Calendrier des Deadlines Projets", this);
    title->setStyleSheet("font-size:18px;font-weight:800;color:#1e293b;");
    mainLayout->addWidget(title);

    // ── Legend ──────────────────────────────────────────────────
    QHBoxLayout* legendRow = new QHBoxLayout();
    legendRow->setSpacing(16);
    struct LegItem { QString label; QString color; };
    QList<LegItem> legend = {
                             { "En cours",   "#2ecc71" },
                             { "En attente", "#f39c12" },
                             { "Terminé",    "#3498db" },
                             { "Annulé / En retard", "#e74c3c" },
                             };
    for (const LegItem& li : legend) {
        QLabel* dot = new QLabel("●", this);
        dot->setStyleSheet(QString("color:%1;font-size:18px;").arg(li.color));
        QLabel* lbl = new QLabel(li.label, this);
        lbl->setStyleSheet("font-size:12px;color:#4b5563;");
        legendRow->addWidget(dot);
        legendRow->addWidget(lbl);
    }
    legendRow->addStretch();
    mainLayout->addLayout(legendRow);

    // ── Horizontal split: calendar + detail panel ────────────────
    QHBoxLayout* split = new QHBoxLayout();
    split->setSpacing(16);

    // Calendar
    m_cal = new QCalendarWidget(this);
    m_cal->setGridVisible(true);
    m_cal->setMinimumWidth(420);
    m_cal->setStyleSheet(
        "QCalendarWidget QAbstractItemView{"
        "  selection-background-color:#8A9A5B;"
        "  selection-color:white;"
        "  font-size:12px;"
        "}"
        "QCalendarWidget QToolButton{"
        "  color:#374151; font-weight:700;"
        "  background:transparent; border:none;"
        "}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar{"
        "  background:#e8f0e0; border-radius:6px;"
        "}");

    paintDeadlines();
    split->addWidget(m_cal, 1);

    // Detail panel (right side)
    QFrame* panel = new QFrame(this);
    panel->setStyleSheet(
        "QFrame{background:white;border-radius:10px;border:1px solid #e2e8f0;}");
    panel->setMinimumWidth(260);
    QVBoxLayout* pl = new QVBoxLayout(panel);
    pl->setContentsMargins(16, 14, 16, 14);
    pl->setSpacing(8);

    m_dateLabel = new QLabel("Sélectionnez une date", panel);
    m_dateLabel->setStyleSheet(
        "font-size:13px;font-weight:700;color:#374151;border:none;");
    pl->addWidget(m_dateLabel);

    QFrame* sep = new QFrame(panel);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#e2e8f0;border:none;background:#e2e8f0;max-height:1px;");
    pl->addWidget(sep);

    m_projectList = new QListWidget(panel);
    m_projectList->setStyleSheet(
        "QListWidget{border:none;background:transparent;font-size:12px;}"
        "QListWidget::item{padding:8px 4px;border-bottom:1px solid #f1f5f9;}"
        "QListWidget::item:hover{background:#f8fafc;}");
    m_projectList->setWordWrap(true);
    pl->addWidget(m_projectList, 1);

    split->addWidget(panel);
    mainLayout->addLayout(split, 1);

    // ── Close ────────────────────────────────────────────────────
    QPushButton* closeBtn = new QPushButton("Fermer", this);
    closeBtn->setFixedHeight(38);
    closeBtn->setStyleSheet(
        "QPushButton{background:#8A9A5B;color:white;border:none;border-radius:8px;"
        "font-size:13px;font-weight:700;padding:0 28px;}"
        "QPushButton:hover{background:#9aaa6b;}");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    // Signal: date clicked
    connect(m_cal, &QCalendarWidget::clicked,
            this, &ProjectCalendarPage::onDateSelected);

    // Show today's projects immediately if any
    onDateSelected(QDate::currentDate());
}

// ─────────────────────────────────────────────
//  Slot: date selected
// ─────────────────────────────────────────────
void ProjectCalendarPage::onDateSelected(const QDate& date)
{
    m_dateLabel->setText(date.toString("dddd d MMMM yyyy"));
    m_projectList->clear();

    if (!m_deadlineMap.contains(date)) {
        QListWidgetItem* item = new QListWidgetItem("Aucun projet dû ce jour.");
        item->setForeground(QColor("#9ca3af"));
        m_projectList->addItem(item);
        return;
    }

    for (const Projet& p : m_deadlineMap[date]) {
        QString label = QString("%1\n   Client : %2\n   Statut : %3")
        .arg(p.getNom())
            .arg(p.getClient())
            .arg(p.getStatus());
        if (p.isOverdue())
            label += "EN RETARD";

        QListWidgetItem* item = new QListWidgetItem(label);
        item->setForeground(statusColor(p.getStatus()));
        QFont f = item->font();
        f.setPointSize(11);
        item->setFont(f);
        m_projectList->addItem(item);
    }
}
