#ifndef STOCKPAGE_H
#define STOCKPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

#include "src/models/stockmaterial.h"

class StockPage : public QWidget
{
    Q_OBJECT

public:
    explicit StockPage(QWidget *parent = nullptr);

private slots:
    // Actions sur la table
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onViewTriggered(int row, int column);
    void onExportAlertPdfClicked();
    void onShowAlertsClicked();

    // Filtres et tri
    void onSearchTextChanged(const QString &text);
    void onSortChanged(int index);

private:
    void setupUI();
    void refreshTable(const QList<StockMaterial>& materials);
    void refreshStats();
    StockMaterial materialFromCurrentRow() const;

    QTableWidget *stockTable;
    QLineEdit    *searchEdit;
    QComboBox    *sortCombo;

    // Stat labels
    QLabel *totalValueLabel;
    QLabel *totalCountLabel;
    QLabel *alertCountLabel;
    QLabel *renewalRateLabel;
};

#endif // STOCKPAGE_H
