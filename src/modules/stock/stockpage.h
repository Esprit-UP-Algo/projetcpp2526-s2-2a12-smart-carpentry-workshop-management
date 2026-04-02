#ifndef STOCKPAGE_H
#define STOCKPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QStackedWidget>

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
    void onFilterByProduitChanged(int index);

    // Navigation
    void onShowStatsClicked();
    void onBackFromStatsClicked();

private:
    void setupUI();
    void setupStatsPage();
    void loadProduitsMap();
    void refreshTable(const QList<StockMaterial>& materials);
    StockMaterial materialFromCurrentRow() const;

    // Stacked widget (page 0 = table, page 1 = stats)
    QStackedWidget *mainStack;

    // --- Page table ---
    QWidget      *tablePage;
    QTableWidget *stockTable;
    QLineEdit    *searchEdit;
    QComboBox    *sortCombo;
    QComboBox    *produitFilterCombo;

    // --- Page stats ---
    QWidget *statsPage;

    // Map id_produit → nom_produit
    QMap<int, QString> m_produitsMap;
};

#endif // STOCKPAGE_H
