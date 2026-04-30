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
#include <QSystemTrayIcon>
#include <QComboBox>

#include "src/models/stockmaterial.h"
#include "stockalertbell.h"
#include "stockalertpanel.h"

// Forward declarations
class StockMapView;

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
    void onShowMapClicked();
    void onBackFromMapClicked();

    // Alertes
    void onBellClicked();
    void onAlertSelected(int materialId);
    void onNewAlertsDetected(int count);

private:
    void setupUI();
    void setupStatsPage();
    void setupMapPage();
    void setupTrayIcon();
    void loadProduitsMap();
    void refreshTable(const QList<StockMaterial>& materials);
    StockMaterial materialFromCurrentRow() const;

    // Stacked widget (0 = table, 1 = stats, 2 = map)
    QStackedWidget *mainStack;

    // --- Page table ---
    QWidget          *tablePage;
    QTableWidget     *stockTable;
    QLineEdit        *searchEdit;
    QComboBox        *sortCombo;
    QComboBox        *produitFilterCombo;

    // --- Cloche + panneau de notifications ---
    StockAlertBell   *m_bell        = nullptr;
    StockAlertPanel  *m_alertPanel  = nullptr;
    QSystemTrayIcon  *m_trayIcon    = nullptr;

    // --- Page stats ---
    QWidget *statsPage;

    // --- Page carte ---
    QWidget      *mapPage      = nullptr;
    StockMapView *m_mapView    = nullptr;

    // Map id_produit → nom_produit
    QMap<int, QString> m_produitsMap;
};

#endif // STOCKPAGE_H
