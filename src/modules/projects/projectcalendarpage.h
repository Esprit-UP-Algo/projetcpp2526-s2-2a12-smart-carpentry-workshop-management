#ifndef PROJECTCALENDARPAGE_H
#define PROJECTCALENDARPAGE_H

/*  ProjectCalendarPage
 *  ──────────────────────────────────────────────────────────────
 *  Modal dialog showing a QCalendarWidget with project deadlines
 *  highlighted by colour (one colour per status).
 *
 *  When the user clicks a date that has deadlines, a small panel
 *  below lists the projects due that day.
 */

#include <QDialog>
#include <QCalendarWidget>
#include <QListWidget>
#include <QLabel>
#include <QMap>
#include <QDate>
#include "src/models/projet.h"

class ProjectCalendarPage : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectCalendarPage(QWidget *parent = nullptr);

private slots:
    void onDateSelected(const QDate& date);

private:
    void setupUI();
    void paintDeadlines();        // apply custom cell text/colour

    QCalendarWidget* m_cal;
    QListWidget*     m_projectList;
    QLabel*          m_dateLabel;

    // date → list of projects due that day
    QMap<QDate, QList<Projet>> m_deadlineMap;
};

#endif // PROJECTCALENDARPAGE_H
