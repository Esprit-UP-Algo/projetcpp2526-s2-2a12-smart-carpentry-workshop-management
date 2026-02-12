#ifndef STOCKPAGE_H
#define STOCKPAGE_H

#include <QWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QComboBox>

class StockPage : public QWidget
{
    Q_OBJECT

public:
    explicit StockPage(QWidget *parent = nullptr);

private slots:
    // List page actions
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onViewTriggered();            // double-click on table row
    void onBackToList();              // common back button for all detail pages

    // Add page actions
    void onSaveAddButtonClicked();

    // Edit page actions
    void onSaveEditButtonClicked();

private:
    void setupUI();
    QWidget* createStockListPage();   // page 0
    QWidget* createAddPage();         // page 1
    QWidget* createEditPage();        // page 2
    QWidget* createViewPage();        // page 3

    void populateEditForm(int row);   // fill edit fields with row data
    void populateViewForm(int row);   // fill view labels with row data
    void clearAddForm();             // clear add form fields

    QStackedWidget *stackedWidget;
    QTableWidget *stockTable;

    // Current selected row (for edit/view/delete)
    int currentRow;

    // ---------- Add Page widgets ----------
    QLineEdit   *addId, *addNom, *addType, *addFournisseur;
    QSpinBox    *addQuantite;
    QDoubleSpinBox *addPrixUnitaire;
    QSpinBox    *addSeuilAlerte;
    QDateEdit   *addDateCommande;
    QLineEdit   *addConsoMensuelle;

    // ---------- Edit Page widgets ----------
    QLineEdit   *editId, *editNom, *editType, *editFournisseur;
    QSpinBox    *editQuantite;
    QDoubleSpinBox *editPrixUnitaire;
    QSpinBox    *editSeuilAlerte;
    QDateEdit   *editDateCommande;
    QLineEdit   *editConsoMensuelle;

    // ---------- View Page widgets ----------
    QLabel *viewId, *viewNom, *viewType, *viewFournisseur;
    QLabel *viewQuantite, *viewPrixUnitaire, *viewSeuilAlerte;
    QLabel *viewDateCommande, *viewConsoMensuelle;
};

#endif // STOCKPAGE_H
