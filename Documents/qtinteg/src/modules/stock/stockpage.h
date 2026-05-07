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

#include "src/models/stockmaterial.h"
#include "stockalertbell.h"
#include "stockalertpanel.h"
#include "arduinobridge.h"      // ← NOUVEAU

class StockMapView;

class StockPage : public QWidget
{
    Q_OBJECT

public:
    explicit StockPage(QWidget *parent = nullptr);

private slots:
    // Table
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

    // Alertes cloche
    void onBellClicked();
    void onAlertSelected(int materialId);
    void onNewAlertsDetected(int count);

    // ── Arduino ── (NOUVEAU)
    void onWeightChanged(double grams);
    void onArduinoConnectionChanged(bool connected);
    void onArduinoError(const QString& msg);

private:
    void setupUI();
    void setupStatsPage();
    void setupMapPage();
    void setupArduinoBar();     // ← NOUVEAU
    void setupTrayIcon();
    void loadProduitsMap();
    void refreshTable(const QList<StockMaterial>& materials);
    StockMaterial materialFromCurrentRow() const;

    // Stack (0=table, 1=stats, 2=map)
    QStackedWidget   *mainStack;

    // Page table
    QWidget          *tablePage;
    QTableWidget     *stockTable;
    QLineEdit        *searchEdit;
    QComboBox        *sortCombo;
    QComboBox        *produitFilterCombo;

    // Cloche
    StockAlertBell   *m_bell       = nullptr;
    StockAlertPanel  *m_alertPanel = nullptr;
    QSystemTrayIcon  *m_trayIcon   = nullptr;

    // Stats
    QWidget          *statsPage;

    // Carte
    QWidget          *mapPage   = nullptr;
    StockMapView     *m_mapView = nullptr;

    // Produits
    QMap<int, QString> m_produitsMap;
    // ── Arduino (NOUVEAU) ────────────────────────────────────────────────
    ArduinoBridge *m_arduino             = nullptr;
    QLabel        *m_arduinoStatus       = nullptr;
    QLabel        *m_arduinoWeightLabel  = nullptr;
    QPushButton   *m_connectBtn          = nullptr;
    QPushButton   *m_disconnectBtn       = nullptr;
    QPushButton   *m_tareBtn             = nullptr;
    QComboBox     *m_portCombo           = nullptr;
    const QString  m_arduinoMaterialName = "Bois Arduino";
};

#endif // STOCKPAGE_H