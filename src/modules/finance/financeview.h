#ifndef FINANCEVIEW_H
#define FINANCEVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMap>

#include "financemodel.h"

class FinanceView : public QWidget
{
    Q_OBJECT
public:
    explicit FinanceView(QWidget *parent = nullptr);
    ~FinanceView();

    void setModel(FinanceModel *model);
    void refreshTable();

signals:
    void addRequested();
    void editRequested(int row);
    void deleteRequested(const QString &id);
    void exportRequested();
    void statsRequested();

private slots:
    void onAddClicked();
    void onEditClicked(int row);
    void onDeleteClicked();
    void onExportClicked();
    void onStatsClicked();
    void onCellDoubleClicked(int row, int column);
    void onFilterChanged();
    void onResetFilters();
    void onModelDataChanged();
    void onModelError(const QString &error);
    
    void onArchiveClicked();
private:
    void setupUI();
    void setupConnections();
    void applyFilters();
    void loadTransactions();
    QPushButton *m_archiveButton;


    void exportToExcelNative(const QString &fileName,
                             const FinanceModel::FinanceStats &stats,
                             const QList<FinanceModel::Transaction> &transactions);

    FinanceModel *m_model;

    QTableWidget *m_table;
    QLineEdit *m_searchEdit;
    QComboBox *m_typeCombo;
    QComboBox *m_categoryCombo;
    QPushButton *m_addButton;
    QPushButton *m_deleteButton;
    QPushButton *m_exportButton;
    QPushButton *m_statsButton;
    QPushButton *m_filterButton;
    QPushButton *m_resetButton;

    QList<FinanceModel::Transaction> m_currentTransactions;
};

#endif // FINANCEVIEW_H
