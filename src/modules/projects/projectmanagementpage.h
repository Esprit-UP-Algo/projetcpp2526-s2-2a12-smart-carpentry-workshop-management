#ifndef PROJECTMANAGEMENTPAGE_H
#define PROJECTMANAGEMENTPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "src/models/projet.h"
#include "lcdconnectorwidget.h"

class ProjectManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectManagementPage(QWidget *parent = nullptr);

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onExportPdfClicked();
    void onSearchTextChanged(const QString &text);
    void onRowDoubleClicked(int row, int column);
    void onStatsClicked();
    void onCalendarClicked();
    void onEmailAlertClicked();
    void onQrScannerClicked();

private:
    void setupUI();
    void populateTable(const QList<Projet>& projets);
    void refreshTable();
    void colorizeStatusItem(QTableWidgetItem* item, const QString& status);

    // Dialog helpers (shared between add/edit)
    struct ProjectFormData {
        int     id = -1;          // <-- add this
        QString nom;
        QDate   dateProjet;
        QDate   deadline;
        QString status;
        double  budget;
        QString adresse;
        QString client;
        QString type;
        int     chefProjetId;
    };
    bool showProjectDialog(const QString& title,
                           ProjectFormData& data,
                           bool isEdit = false);

    // Maps col index → field role (for reading row data)
    // Columns: 0=ID(hidden), 1=NOM, 2=CLIENT, 3=TYPE, 4=DATE DEBUT,
    //          5=DEADLINE, 6=STATUT, 7=BUDGET, 8=ADRESSE, 9=CHEF

    QTableWidget *m_table;
    QLineEdit    *m_searchEdit;
    QPushButton  *m_addBtn;
    QPushButton  *m_editBtn;
    QPushButton  *m_deleteBtn;
    QPushButton  *m_exportPdfBtn;
    QPushButton *m_statsBtn;
    QPushButton *m_calendarBtn;
    QPushButton *m_emailAlertBtn;
    LcdConnectorWidget* m_lcdWidget = nullptr;
};

#endif // PROJECTMANAGEMENTPAGE_H
