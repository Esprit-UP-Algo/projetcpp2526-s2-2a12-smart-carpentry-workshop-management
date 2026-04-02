#ifndef PROJECTSTATSPAGE_H
#define PROJECTSTATSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class ProjectStatsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectStatsPage(QWidget *parent = nullptr);

public slots:
    void refresh();

private:
    void setupUI();
    void buildCharts();

    // Stat labels
    QLabel *m_lblTotal;
    QLabel *m_lblEnCours;
    QLabel *m_lblEnAttente;
    QLabel *m_lblTermines;
    QLabel *m_lblAnnules;
    QLabel *m_lblBudgetTotal;
    QLabel *m_lblBudgetMoyen;
    QLabel *m_lblEnRetard;

    // Chart containers (painted manually)
    QWidget *m_barChart;   // statuts
    QWidget *m_pieChart;   // types
    QWidget *m_budgetBar;  // budget par type
};

#endif // PROJECTSTATSPAGE_H
