#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class QFrame;
class QVBoxLayout;
class QHBoxLayout;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUi();
    void loadStyleSheet();
    QFrame* createBox(const QString& text, const QString& objName, const QString& className = "card");

    // New helper methods for dashboard sections
    void setupHeader(QVBoxLayout* mainLayout);
    void setupSummarySection(QVBoxLayout* layout);
    void setupChartsSection(QVBoxLayout* layout);
    void setupFormSection(QVBoxLayout* layout);
    void setupTableSection(QVBoxLayout* layout);
};

#endif // MAINWINDOW_H
