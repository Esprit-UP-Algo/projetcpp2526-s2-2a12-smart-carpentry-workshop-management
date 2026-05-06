#ifndef PROJECTSTATSPAGE_H
#define PROJECTSTATSPAGE_H


#include <QDialog>

class ProjectStatsPage : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectStatsPage(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // PROJECTSTATSPAGE_H
