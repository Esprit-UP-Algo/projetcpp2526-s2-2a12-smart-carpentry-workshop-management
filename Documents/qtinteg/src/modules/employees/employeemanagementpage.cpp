#include "employeemanagementpage.h"
#include "employeedialog.h"
#include "../../database/employeedatabase.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPageSize>
#include <QPageLayout>
#include <QDateTime>
#include <QFont>
#include <QLocale>
#include <QImage>
#include <QPixmap>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QFrame>
#include <QApplication>
#include <QPalette>
#include <QTabWidget>
#include <QScrollArea>
#include <QPainterPath>
#include <QMap>
#include <algorithm>
#include <cmath>
#include "pinassigndialog.h"
#include "../../database/connection.h"

EmployeeManagementPage::EmployeeManagementPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("empPage");
    setupUI();
    setupConnections();
    loadEmployees();
}

void EmployeeManagementPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    m_tabs = new QTabWidget(this);
    QTabWidget *tabs = m_tabs;
    tabs->setObjectName("empTabs");

    applyTabTheme();
    QObject::connect(tabs, &QTabWidget::currentChanged, tabs, [this](int){ applyTabTheme(); });

    // Tab 1 — employee list
    QWidget *listTab = new QWidget;
    QVBoxLayout *listLay = new QVBoxLayout(listTab);
    listLay->setContentsMargins(0, 8, 0, 0);
    listLay->setSpacing(12);
    m_listContainer = listTab;
    createToolbar();
    createTable();
    listLay->addWidget(m_toolbar);
    listLay->addWidget(m_table);
    tabs->addTab(listTab, "Liste des employes");

    // Tab 2 — statistics
    tabs->addTab(createStatsTab(), "Statistiques");

    connect(tabs, &QTabWidget::currentChanged, this, [this](int idx){
        if (idx == 1) refreshStats();
    });



    mainLayout->addWidget(tabs);
}

void EmployeeManagementPage::createToolbar()
{
    QFrame *toolbar = new QFrame(this);
    m_toolbar = toolbar;
    toolbar->setObjectName("empToolbar");

    QVBoxLayout *toolbarLayout = new QVBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(10);

    // Search row
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(10);

    QLabel *searchLabel = new QLabel("Rechercher:", toolbar);
    searchLabel->setObjectName("empSearchLabel");

    m_searchInput = new QLineEdit(toolbar);
    m_searchInput->setObjectName("empSearchInput");
    m_searchInput->setPlaceholderText("Rechercher par nom, CIN ou poste...");
    m_searchInput->setMinimumWidth(300);
    m_searchInput->setFixedHeight(38);

    m_filterCombo = new QComboBox(toolbar);
    m_filterCombo->setObjectName("empFilterCombo");
    m_filterCombo->addItem("Tous les postes", "all");  // base item only; rest populated from DB
    m_filterCombo->setMinimumWidth(180);
    m_filterCombo->setFixedHeight(38);

    m_sortCombo = new QComboBox(toolbar);
    m_sortCombo->setObjectName("empSortCombo");
    m_sortCombo->addItem("Trier par nom", "name");
    m_sortCombo->addItem("Salaire (croissant)", "salary_asc");
    m_sortCombo->addItem("Salaire (decroissant)", "salary_desc");
    m_sortCombo->addItem("Date d'embauche (recent)", "date_desc");
    m_sortCombo->addItem("Date d'embauche (ancien)", "date_asc");
    m_sortCombo->addItem("Performance (meilleur)", "performance_desc");
    m_sortCombo->setMinimumWidth(200);
    m_sortCombo->setFixedHeight(38);

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchInput);
    searchLayout->addWidget(m_filterCombo);
    searchLayout->addWidget(m_sortCombo);
    searchLayout->addStretch();

    // Buttons row
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    m_addButton = new QPushButton("+ Ajouter un employe", toolbar);
    m_addButton->setObjectName("empAddBtn");
    m_addButton->setFixedHeight(40);
    m_addButton->setCursor(Qt::PointingHandCursor);

    m_editButton = new QPushButton("Modifier", toolbar);
    m_editButton->setObjectName("empSecondaryBtn");
    m_editButton->setFixedHeight(40);
    m_editButton->setCursor(Qt::PointingHandCursor);
    m_editButton->setEnabled(false);

    m_deleteButton = new QPushButton("Supprimer", toolbar);
    m_deleteButton->setObjectName("empDangerBtn");
    m_deleteButton->setFixedHeight(40);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setEnabled(false);

    /*
    m_exportButton = new QPushButton("Exporter PDF", toolbar);
    m_exportButton->setObjectName("empSecondaryBtn");
    m_exportButton->setFixedHeight(40);
    m_exportButton->setCursor(Qt::PointingHandCursor);
    */


    m_certButton = new QPushButton("Attestation de travail", toolbar);
    m_certButton->setObjectName("empSecondaryBtn");
    m_certButton->setFixedHeight(40);
    m_certButton->setCursor(Qt::PointingHandCursor);
    m_certButton->setEnabled(false);   // only active when a row is selected

    m_refreshButton = new QPushButton("Actualiser", toolbar);
    m_refreshButton->setObjectName("empSecondaryBtn");
    m_refreshButton->setFixedHeight(40);
    m_refreshButton->setCursor(Qt::PointingHandCursor);

    m_pinButton = new QPushButton("🔑 PIN Arduino", toolbar);  // ← ADD
    m_pinButton->setObjectName("empSecondaryBtn");              // ← ADD
    m_pinButton->setFixedHeight(40);                            // ← ADD
    m_pinButton->setCursor(Qt::PointingHandCursor);             // ← ADD
    m_pinButton->setEnabled(false);

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    //buttonLayout->addWidget(m_exportButton);  // export list is broken so i commented out for now    ~lain
    buttonLayout->addWidget(m_certButton);
    buttonLayout->addWidget(m_pinButton);
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();

    toolbarLayout->addLayout(searchLayout);
    toolbarLayout->addLayout(buttonLayout);

    // Populate filter combo from DB after widgets are created
    populateFilterCombo();
}

void EmployeeManagementPage::createTable()
{
    m_table = new QTableWidget(this);
    m_table->setObjectName("empTable");
    m_table->setColumnCount(11);
    m_table->setHorizontalHeaderLabels({
        "ID", "CIN", "Nom complet", "Poste", "Email",
        "Telephone", "Salaire", "Performance", "Disponibilite",
        "Heures", "Competences"
    });

    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->setColumnWidth(0, 80);
    m_table->setColumnWidth(1, 100);
    m_table->setColumnWidth(2, 180);
    m_table->setColumnWidth(3, 130);
    m_table->setColumnWidth(4, 200);
    m_table->setColumnWidth(5, 130);
    m_table->setColumnWidth(6, 100);
    m_table->setColumnWidth(7, 100);
    m_table->setColumnWidth(8, 130);
    m_table->setColumnWidth(9, 80);

    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(false);
    m_table->setShowGrid(false);
    m_table->setColumnHidden(0, true);   // ID hidden — still used internally

}


void EmployeeManagementPage::setupConnections()
{
    connect(m_pinButton, &QPushButton::clicked, this,        &EmployeeManagementPage::onAssignPin);
    connect(m_addButton,    &QPushButton::clicked, this, &EmployeeManagementPage::onAddEmployee);
    connect(m_editButton,   &QPushButton::clicked, this, &EmployeeManagementPage::onEditEmployee);
    connect(m_deleteButton, &QPushButton::clicked, this, &EmployeeManagementPage::onDeleteEmployee);
    //connect(m_exportButton, &QPushButton::clicked, this, &EmployeeManagementPage::onExportPDF);
    connect(m_certButton,   &QPushButton::clicked, this, &EmployeeManagementPage::onExportCertificate);
    connect(m_refreshButton,&QPushButton::clicked, this, &EmployeeManagementPage::onRefreshTable);
    connect(m_searchInput,  &QLineEdit::textChanged, this, &EmployeeManagementPage::onSearchTextChanged);
    connect(m_filterCombo,  &QComboBox::currentTextChanged, this, &EmployeeManagementPage::onFilterChanged);
    connect(m_sortCombo,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EmployeeManagementPage::onSortChanged);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &EmployeeManagementPage::onTableSelectionChanged);
    connect(m_table, &QTableWidget::doubleClicked, this, &EmployeeManagementPage::onEditEmployee);
}

// ---------------------------------------------------------------------------
// populateFilterCombo — fetches distinct postes from the DB
// ---------------------------------------------------------------------------
void EmployeeManagementPage::populateFilterCombo()
{
    // Remember current selection so we can restore it
    QString current = m_filterCombo->currentData().toString();

    m_filterCombo->blockSignals(true);

    // Remove everything except the "all" entry at index 0
    while (m_filterCombo->count() > 1)
        m_filterCombo->removeItem(1);

    // Fetch distinct postes with their employee counts from DB
    QMap<QString, int> byPoste = EmployeeDatabase::instance().getEmployeeCountByPoste();
    for (auto it = byPoste.constBegin(); it != byPoste.constEnd(); ++it) {
        if (!it.key().isEmpty())
            m_filterCombo->addItem(
                QString("%1 (%2)").arg(it.key()).arg(it.value()),
                it.key()   // store raw poste as item data for filtering
            );
    }

    // Restore previous selection if it still exists, otherwise reset to "all"
    int idx = m_filterCombo->findData(current);
    m_filterCombo->setCurrentIndex(idx >= 0 ? idx : 0);

    m_filterCombo->blockSignals(false);
}

// ---------------------------------------------------------------------------
// Data loading
// ---------------------------------------------------------------------------
void EmployeeManagementPage::loadEmployees()
{
    loadEmployees(EmployeeDatabase::instance().getAllEmployees());
}

void EmployeeManagementPage::loadEmployees(const QList<Employee>& employees)
{
    m_table->setRowCount(0);
    for (int i = 0; i < employees.size(); ++i) {
        m_table->insertRow(i);
        addEmployeeToTable(employees[i], i);
    }
}

void EmployeeManagementPage::addEmployeeToTable(const Employee& employee, int row)
{
    m_table->setItem(row, 0, new QTableWidgetItem(employee.getId()));
    m_table->setItem(row, 1, new QTableWidgetItem(employee.getCin()));
    m_table->setItem(row, 2, new QTableWidgetItem(employee.getFullName()));

    QTableWidgetItem *posteItem = new QTableWidgetItem(employee.getPoste());
    posteItem->setBackground(QColor(getPosteBadgeColor(employee.getPoste())));
    m_table->setItem(row, 3, posteItem);

    m_table->setItem(row, 4, new QTableWidgetItem(employee.getEmail()));
    m_table->setItem(row, 5, new QTableWidgetItem(employee.getTelephone()));
    m_table->setItem(row, 6, new QTableWidgetItem(QString::number(employee.getSalaire(), 'f', 2) + " €"));
    m_table->setItem(row, 7, new QTableWidgetItem(QString::number(employee.getPerformance(), 'f', 1) + "/10"));

    QTableWidgetItem *dispItem = new QTableWidgetItem(employee.getDisponibilite());
    dispItem->setBackground(QColor(getDisponibiliteBadgeColor(employee.getDisponibilite())));
    m_table->setItem(row, 8, dispItem);

    m_table->setItem(row, 9,  new QTableWidgetItem(QString::number(employee.getHeuresTravail(), 'f', 0) + "h"));
    m_table->setItem(row, 10, new QTableWidgetItem(employee.getCompetencesString()));
    m_table->setRowHeight(row, 50);
}

Employee EmployeeManagementPage::getSelectedEmployee() const
{
    int row = m_table->currentRow();
    if (row < 0) return Employee();
    return EmployeeDatabase::instance().getEmployee(m_table->item(row, 0)->text());
}

void EmployeeManagementPage::updateButtonStates()
{
    bool has = m_table->currentRow() >= 0;
    m_editButton->setEnabled(has);
    m_deleteButton->setEnabled(has);
    m_certButton->setEnabled(has);
    m_pinButton->setEnabled(has);
}

QString EmployeeManagementPage::getPosteBadgeColor(const QString& poste) const
{
    if (poste == "Chef d'equipe" || poste == "Chef Equipe" || poste == "Chef de Projet") return "#E8F0E3";
    if (poste == "Menuisier" || poste == "Menuisier Senior") return "#F3EFE0";
    if (poste == "Apprenti")  return "#FFF3E0";
    if (poste == "Designer")  return "#EEF0FF";
    return "#F3EFE0";
}

QString EmployeeManagementPage::getDisponibiliteBadgeColor(const QString& disponibilite) const
{
    if (disponibilite == "Disponible")   return "#E8F0E3";
    if (disponibilite == "En conge")     return "#FFE0E0";
    if (disponibilite == "Indisponible") return "#FFE8E0";
    if (disponibilite == "En formation") return "#E0F0FF";
    return "#F3EFE0";
}

// ---------------------------------------------------------------------------
// CRUD slots — repopulate filter after any change
// ---------------------------------------------------------------------------
void EmployeeManagementPage::onAddEmployee()
{
    EmployeeDialog dialog(this);
    dialog.setWindowTitle("Ajouter un employe");
    if (dialog.exec() == QDialog::Accepted) {
        Employee e = dialog.getEmployee();
        e.setId(EmployeeDatabase::instance().generateNextId());
        if (EmployeeDatabase::instance().addEmployee(e)) {
            loadEmployees();
            populateFilterCombo();   // keep filter in sync
            QMessageBox::information(this, "Succes", "Employe ajoute!");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible d'ajouter l'employe.");
        }
    }
}

void EmployeeManagementPage::onEditEmployee()
{
    Employee e = getSelectedEmployee();
    if (!e.isValid()) { QMessageBox::warning(this, "Aucune selection", "Selectionnez un employe."); return; }
    EmployeeDialog dialog(this);
    dialog.setWindowTitle("Modifier l'employe");
    dialog.setEmployee(e);
    if (dialog.exec() == QDialog::Accepted) {
        Employee updated = dialog.getEmployee();
        updated.setId(e.getId());
        if (EmployeeDatabase::instance().updateEmployee(updated)) {
            loadEmployees();
            populateFilterCombo();   // poste may have changed
            QMessageBox::information(this, "Succes", "Employe modifie!");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible de modifier l'employe.");
        }
    }
}

void EmployeeManagementPage::onDeleteEmployee()
{
    Employee e = getSelectedEmployee();
    if (!e.isValid()) { QMessageBox::warning(this, "Aucune selection", "Selectionnez un employe."); return; }
    auto r = QMessageBox::question(this, "Confirmer",
        QString("Supprimer %1 ?").arg(e.getFullName()), QMessageBox::Yes | QMessageBox::No);
    if (r == QMessageBox::Yes) {
        if (EmployeeDatabase::instance().deleteEmployee(e.getId())) {
            loadEmployees();
            populateFilterCombo();   // a poste may have disappeared
            QMessageBox::information(this, "Succes", "Employe supprime!");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible de supprimer l'employe.");
        }
    }
}


void EmployeeManagementPage::onAssignPin()
{
    Employee e = getSelectedEmployee();
    if (!e.isValid()) {
        QMessageBox::warning(this, "Aucune sélection", "Sélectionnez un employé.");
        return;
    }
    PinAssignDialog dlg(e.getCin(), e.getFullName(), this);
    dlg.exec();
    // No need to reload the table — PIN columns are not displayed in the list
}

// ---------------------------------------------------------------------------
// Search / filter / sort
// ---------------------------------------------------------------------------
void EmployeeManagementPage::onSearchTextChanged(const QString& text)
{
    if (text.isEmpty()) { onRefreshTable(); return; }
    QList<Employee> results = EmployeeDatabase::instance().searchByName(text);
    for (const Employee& emp : EmployeeDatabase::instance().searchByCin(text))
        if (!results.contains(emp)) results.append(emp);
    loadEmployees(results);
}

void EmployeeManagementPage::onFilterChanged(const QString&)
{
    QString fd = m_filterCombo->currentData().toString();
    if (fd == "all") loadEmployees();
    else loadEmployees(EmployeeDatabase::instance().searchByPoste(fd));
}

void EmployeeManagementPage::onSortChanged(int index)
{
    QString s = m_sortCombo->itemData(index).toString();
    QList<Employee> sorted;
    if      (s == "salary_asc")       sorted = EmployeeDatabase::instance().sortBySalaire(true);
    else if (s == "salary_desc")      sorted = EmployeeDatabase::instance().sortBySalaire(false);
    else if (s == "date_asc")         sorted = EmployeeDatabase::instance().sortByDateEmbauche(true);
    else if (s == "date_desc")        sorted = EmployeeDatabase::instance().sortByDateEmbauche(false);
    else if (s == "performance_desc") sorted = EmployeeDatabase::instance().sortByPerformance(false);
    else                              sorted = EmployeeDatabase::instance().getAllEmployees();
    loadEmployees(sorted);
}

void EmployeeManagementPage::onExportPDF()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "Exporter en PDF", "liste_employees.pdf", "PDF Files (*.pdf)");
    if (filePath.isEmpty()) return;

    // ── Collect employees currently visible in the table (respects filter/sort)
    QList<Employee> employees;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        QString id = m_table->item(row, 0)->text();
        Employee e = EmployeeDatabase::instance().getEmployee(id);
        if (e.isValid()) employees.append(e);
    }

    if (employees.isEmpty()) {
        QMessageBox::warning(this, "Export PDF", "Aucun employé à exporter.");
        return;
    }

    // ── Aggregate stats ───────────────────────────────────────────────────────
    double totalSalary = 0, totalPerf = 0;
    int available = 0;
    for (const Employee& e : employees) {
        totalSalary += e.getSalaire();
        totalPerf   += e.getPerformance();
        if (e.getDisponibilite() == "Disponible") ++available;
    }
    double avgPerf = totalPerf / employees.size();

    // ── Badge style helpers ───────────────────────────────────────────────────
    auto posteBadge = [](const QString& p) -> QString {
        if (p == "Chef d'equipe" || p == "Chef Equipe" || p == "Chef de Projet")
            return "background:#d1fae5;color:#065f46";
        if (p == "Menuisier" || p == "Menuisier Senior") return "background:#fef9c3;color:#713f12";
        if (p == "Apprenti")  return "background:#ffedd5;color:#7c2d12";
        if (p == "Designer")  return "background:#ede9fe;color:#4c1d95";
        return "background:#f3f4f6;color:#374151";
    };
    auto dispoBadge = [](const QString& d) -> QString {
        if (d == "Disponible")   return "background:#d1fae5;color:#065f46";
        if (d == "En conge")     return "background:#fee2e2;color:#7f1d1d";
        if (d == "Indisponible") return "background:#ffedd5;color:#7c2d12";
        if (d == "En formation") return "background:#dbeafe;color:#1e3a5f";
        return "background:#f3f4f6;color:#374151";
    };
    auto perfColor = [](double p) -> QString {
        if (p >= 8.0) return "#16a34a";
        if (p >= 5.0) return "#374151";
        return "#dc2626";
    };

    // ── Build table rows ──────────────────────────────────────────────────────
    QString rows;
    for (int i = 0; i < employees.size(); ++i) {
        const Employee& e = employees[i];
        QString rowBg = (i % 2 == 0) ? "#ffffff" : "#f9f7f2";
        double  perf  = e.getPerformance();

        rows += QString(
            "<tr style='background-color:%1;'>"
            "  <td style='padding:8px 10px;font-size:10px;color:#6b7280;'>%2</td>"
            "  <td style='padding:8px 10px;font-size:11px;font-weight:bold;color:#111827;'>%3</td>"
            "  <td style='padding:8px 10px;font-size:10px;color:#374151;'>%4</td>"
            "  <td style='padding:8px 10px;text-align:center;'>"
            "    <span style='%5;padding:2px 8px;font-size:9px;font-weight:bold;border:1px solid #cccccc;'>%6</span>"
            "  </td>"
            "  <td style='padding:8px 10px;font-size:10px;color:#374151;'>%7</td>"
            "  <td style='padding:8px 10px;font-size:10px;color:#374151;text-align:center;'>%8</td>"
            "  <td style='padding:8px 10px;font-size:11px;font-weight:bold;color:#111827;text-align:right;'>%9 €</td>"
            "  <td style='padding:8px 10px;text-align:center;font-size:11px;font-weight:bold;color:%10;'>%11</td>"
            "  <td style='padding:8px 10px;text-align:center;'>"
            "    <span style='%12;padding:2px 8px;font-size:9px;font-weight:bold;border:1px solid #cccccc;'>%13</span>"
            "  </td>"
            "</tr>")
            .arg(rowBg)                                              // %1
            .arg(e.getCin())                                         // %2
            .arg(e.getFullName().toHtmlEscaped())                    // %3
            .arg(e.getEmail().toHtmlEscaped())                       // %4
            .arg(posteBadge(e.getPoste()))                           // %5
            .arg(e.getPoste().toHtmlEscaped())                       // %6
            .arg(e.getTelephone().toHtmlEscaped())                   // %7
            .arg(e.getDateEmbauche().toString("dd/MM/yyyy"))         // %8
            .arg(QString::number(e.getSalaire(), 'f', 2))            // %9
            .arg(perfColor(perf))                                    // %10
            .arg(QString::number(perf, 'f', 1) + "/10")              // %11
            .arg(dispoBadge(e.getDisponibilite()))                   // %12
            .arg(e.getDisponibilite().toHtmlEscaped());              // %13
    }

    // ── Context labels for the header ─────────────────────────────────────────
    QString filterLabel = m_filterCombo->currentText();
    QString sortLabel   = m_sortCombo->currentText();
    QString exportDate  = QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm");

    // ── Full HTML document ────────────────────────────────────────────────────
    // Qt's HTML renderer only supports a subset of CSS2 — no flexbox, no gradients,
    // no border-radius, no letter-spacing, no text-transform. Use table-based layout.
    QString html = QString(R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8"/>
<style>
  body { font-family: Arial, sans-serif; font-size: 11px; color: #374151;
         margin: 24px 28px; background: #ffffff; }

  table.hdr { width: 100%; border-collapse: collapse;
              background-color: #4a5e2a; margin-bottom: 14px; }
  table.hdr td { padding: 14px 18px; color: #ffffff; vertical-align: middle; }
  .hdr-title { font-size: 17px; font-weight: bold; color: #ffffff; }
  .hdr-sub   { font-size: 9px; color: #ccddaa; }
  .hdr-right { text-align: right; font-size: 9px; color: #ddeebb; line-height: 2.0; }
  .hdr-right b { color: #ffffff; }

  table.kpi  { width: 100%; border-collapse: collapse; margin-bottom: 12px; }
  table.kpi td { background-color: #f8f6f0; border: 1px solid #ddd8cc;
                 padding: 10px 14px; width: 25%; vertical-align: top; }
  .kpi-lbl { font-size: 8px; font-weight: bold; color: #9ca3af; }
  .kpi-val { font-size: 20px; font-weight: bold; margin: 2px 0; }
  .kpi-sub { font-size: 9px; color: #9ca3af; }

  table.ibar { width: 100%; border-collapse: collapse;
               background-color: #f5f3ef; border: 1px solid #ddd8cc;
               margin-bottom: 10px; }
  table.ibar td { padding: 6px 14px; font-size: 10px; color: #6b7280; }

  p.sec { font-size: 11px; font-weight: bold; color: #374151;
          margin: 0 0 6px 0; padding-left: 8px;
          border-left: 3px solid #8A9A5B; }

  table.emp { width: 100%; border-collapse: collapse; }
  table.emp thead tr { background-color: #4a5e2a; }
  table.emp thead th { padding: 8px 9px; font-size: 8px; font-weight: bold;
                       color: #ffffff; text-align: left; }
  table.emp tbody td { padding: 7px 9px; font-size: 10px;
                       border-bottom: 1px solid #ece8df; vertical-align: middle; }
  table.emp tbody tr.alt { background-color: #f9f7f2; }

  table.foot { width: 100%; border-collapse: collapse;
               border-top: 1px solid #ddd8cc; margin-top: 18px; }
  table.foot td { padding: 8px 0; font-size: 9px; color: #9ca3af; text-align: center; }
</style>
</head>
<body>

<table class="hdr" cellspacing="0" cellpadding="0">
  <tr>
    <td>
      <div class="hdr-title">WoodFlow &mdash; Liste des Employ&eacute;s</div>
      <div class="hdr-sub">Rapport g&eacute;n&eacute;r&eacute; automatiquement depuis le syst&egrave;me de gestion</div>
    </td>
    <td class="hdr-right">
      Date d&rsquo;export&nbsp;: <b>%1</b><br/>
      Filtre&nbsp;: <b>%2</b><br/>
      Tri&nbsp;: <b>%3</b>
    </td>
  </tr>
</table>

<table class="kpi" cellspacing="4" cellpadding="0">
  <tr>
    <td><div class="kpi-lbl">EFFECTIF EXPORT&Eacute;</div>
        <div class="kpi-val" style="color:#8A9A5B;">%4</div>
        <div class="kpi-sub">employ&eacute;s</div></td>
    <td><div class="kpi-lbl">PERFORMANCE MOYENNE</div>
        <div class="kpi-val" style="color:#5B8A9A;">%5</div>
        <div class="kpi-sub">/ 10</div></td>
    <td><div class="kpi-lbl">DISPONIBLES</div>
        <div class="kpi-val" style="color:#9A8A5B;">%6</div>
        <div class="kpi-sub">/ %4</div></td>
    <td><div class="kpi-lbl">MASSE SALARIALE</div>
        <div class="kpi-val" style="color:#7A5B9A;">%7</div>
        <div class="kpi-sub">€ / mois</div></td>
  </tr>
</table>

<table class="ibar" cellspacing="0" cellpadding="0">
  <tr><td>
    Filtre actif&nbsp;: <b>%2</b> &nbsp;&bull;&nbsp;
    Tri&nbsp;: <b>%3</b> &nbsp;&bull;&nbsp;
    Total&nbsp;: <b>%4 employ&eacute;(s)</b>
  </td></tr>
</table>

<p class="sec">D&eacute;tail des employ&eacute;s</p>

<table class="emp" cellspacing="0" cellpadding="0">
  <thead>
    <tr>
      <th>CIN</th><th>Nom complet</th><th>Email</th><th>Poste</th>
      <th>T&eacute;l&eacute;phone</th>
      <th style="text-align:center;">Embauche</th>
      <th style="text-align:right;">Salaire</th>
      <th style="text-align:center;">Perf.</th>
      <th style="text-align:center;">Disponibilit&eacute;</th>
    </tr>
  </thead>
  <tbody>
    %8
  </tbody>
</table>

<table class="foot" cellspacing="0" cellpadding="0">
  <tr><td>
    G&eacute;n&eacute;r&eacute; par <b style="color:#8A9A5B;">WoodFlow</b>
    &nbsp;&bull;&nbsp; %1 &nbsp;&bull;&nbsp; %4 employ&eacute;(s) export&eacute;(s)
  </td></tr>
</table>

</body>
</html>
)")
    .arg(exportDate)                              // %1
    .arg(filterLabel.toHtmlEscaped())             // %2
    .arg(sortLabel.toHtmlEscaped())               // %3
    .arg(employees.size())                        // %4
    .arg(avgPerf, 0, 'f', 1)                     // %5
    .arg(available)                               // %6
    .arg(QString::number(totalSalary, 'f', 0))    // %7
    .arg(rows);                                   // %8

    // ── Render via QTextDocument → QPrinter (landscape A4) ───────────────────
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(printer.pageRect(QPrinter::DevicePixel).size());
    doc.print(&printer);

    QMessageBox::information(this, "Export réussi",
        QString("PDF généré avec succès :\n%1\n\n%2 employé(s) exporté(s).")
            .arg(filePath).arg(employees.size()));
}

void EmployeeManagementPage::onExportCertificate()
{
    Employee e = getSelectedEmployee();
    if (!e.isValid()) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un employé.");
        return;
    }

    // ── Gender heuristic ──────────────────────────────────────────────────────
    QString prenom = e.getPrenom().trimmed();
    bool feminine  = !prenom.isEmpty() &&
                     (prenom.back() == 'a' || prenom.back() == 'A' ||
                      prenom.back() == 'e' || prenom.back() == 'E');
    QString civility = feminine ? "Mme" : "M.";
    QString pronoun  = feminine ? "elle" : "il";

    // ── Seniority & dates ─────────────────────────────────────────────────────
    QDate embauche = e.getDateEmbauche().date();
    QDate today    = QDate::currentDate();
    int years  = embauche.daysTo(today) / 365;
    int months = (embauche.daysTo(today) % 365) / 30;
    QString anciennete;
    if      (years > 0 && months > 0) anciennete = QString("%1 an(s) et %2 mois").arg(years).arg(months);
    else if (years > 0)               anciennete = QString("%1 an(s)").arg(years);
    else                              anciennete = QString("%1 mois").arg(months);

    QLocale frLocale(QLocale::French, QLocale::Tunisia);
    QString certDate  = frLocale.toString(today,    "dd MMMM yyyy");
    QString hireDate  = frLocale.toString(embauche, "dd MMMM yyyy");
    QString refNumber = QString("WF-%1-%2")
                            .arg(today.toString("yyyyMM"))
                            .arg(e.getCin().right(4));

    // ── File save dialog ──────────────────────────────────────────────────────
    QString safeName    = e.getFullName().simplified().replace(' ', '_');
    QString docsDir     = QCoreApplication::applicationDirPath() + "/../docs";
    QDir().mkpath(docsDir);
    QString defaultPath = docsDir + "/" + safeName + "_certif.pdf";
    QString filePath    = QFileDialog::getSaveFileName(
        this, "Enregistrer l'attestation", defaultPath, "PDF Files (*.pdf)");
    if (filePath.isEmpty()) return;

    // ── Logo ──────────────────────────────────────────────────────────────────
    QString logoPath;
    const QStringList logoCandidates = {
        QApplication::applicationDirPath() + "/src/assets/icons/logo1.png",
        QApplication::applicationDirPath() + "/../src/assets/icons/logo1.png",
        QCoreApplication::applicationDirPath() + "/logo1.png"
    };
    for (const QString& lc : logoCandidates)
        if (QFile::exists(lc)) { logoPath = lc; break; }

    // ── Printer setup — A4 portrait, no margins (we paint everything ourselves)
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);

    QPainter p;
    if (!p.begin(&printer)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier PDF.");
        return;
    }

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // ── Coordinate system: device pixels ─────────────────────────────────────
    // QPrinter with HighResolution works in device pixels at printer.resolution() dpi.
    // DO NOT call p.scale() — Qt font engine already accounts for DPI internally.
    // All mm values are converted to pixels explicitly via mm().
    const qreal dpi       = printer.resolution();
    const qreal px_per_mm = dpi / 25.4;
    auto mm = [&](qreal v) { return v * px_per_mm; };

    // ── Page geometry in device pixels ────────────────────────────────────────
    const qreal PW = mm(210.0);
    const qreal PH = mm(297.0);
    const qreal ML = mm(14.0);
    const qreal MR = mm(14.0);
    const qreal TW = PW - ML - MR;

    // ── Colors ────────────────────────────────────────────────────────────────
    const QColor HDR_BG    ("#4a5e2a");
    const QColor ACCENT    ("#2a3a14");
    const QColor MID_GREEN ("#8A9A5B");
    const QColor LABEL_GRAY("#6b7280");
    const QColor TEXT_DARK ("#1a1a1a");
    const QColor CARD_BG   ("#f8f6f0");
    const QColor CARD_ALT  ("#f2efe8");
    const QColor BORDER    ("#ddd8cc");
    const QColor STAMP_COL ("#c5bfb0");
    const QColor FOOT_BG   ("#f0ede4");
    const QColor REF_BG    ("#f5f3ef");
    const QColor PURPOSE_BG("#fffdf7");
    const QColor WHITE     (Qt::white);
    const QColor CCDDAA    ("#ccddaa");
    const QColor DDEEBB    ("#ddeebb");
    const QColor GRAY_LIGHT("#9ca3af");

    // ── Font helper — always use pixel sizes so fonts are DPI-correct ─────────
    // setPixelSize() is immune to painter transforms; setPointSize() is not.
    auto F = [&](const char* family, qreal ptSize, bool bold=false, bool italic=false) {
        QFont f(family);
        f.setPixelSize(qRound(ptSize * dpi / 72.0));
        f.setBold(bold); f.setItalic(italic);
        return f;
    };

    // ── SECTION: HEADER ───────────────────────────────────────────────────────
    const qreal HDR_H = mm(25.4);

    p.setPen(Qt::NoPen);
    p.setBrush(ACCENT);
    p.drawRect(QRectF(0, 0, mm(2.1), HDR_H));

    p.setBrush(HDR_BG);
    p.drawRect(QRectF(mm(2.1), 0, PW - mm(2.1), HDR_H));

    // Logo
    qreal textStartX = ML;
    QPixmap logoPixmap;
    if (!logoPath.isEmpty() && logoPixmap.load(logoPath)) {
        const qreal LW = mm(17.0), LH = mm(17.0);
        qreal logoY = (HDR_H - LH) / 2.0;
        p.drawPixmap(QRect(int(ML), int(logoY), int(LW), int(LH)),
                     logoPixmap.scaled(int(LW), int(LH),
                                       Qt::KeepAspectRatio, Qt::SmoothTransformation));
        textStartX = ML + LW + mm(3.0);
    }

    p.setFont(F("Arial", 16, true));
    p.setPen(WHITE);
    p.drawText(QPointF(textStartX, HDR_H * 0.62), "WoodFlow");

    p.setFont(F("Arial", 7));
    p.setPen(CCDDAA);
    p.drawText(QPointF(textStartX, HDR_H * 0.82),
               QString::fromUtf8("Menuiserie & Aménagement Intérieur — Depuis 2015"));

    // Contact lines (right-aligned)
    p.setFont(F("Arial", 6));
    p.setPen(DDEEBB);
    const QStringList contactLines = {
        QString::fromUtf8("Adresse : Zone Industrielle, Tunis, Tunisie"),
        QString::fromUtf8("Tél : +216 71 000 000   |   Email : contact@woodflow.tn"),
        QString::fromUtf8("RC : B123456789   |   MF : 0012345/A"),
    };
    qreal cy = HDR_H * 0.22;
    QFontMetricsF fmContact(F("Arial", 6));
    for (const QString& line : contactLines) {
        p.drawText(QPointF(PW - MR - fmContact.horizontalAdvance(line), cy), line);
        cy += HDR_H * 0.26;
    }

    // ── SECTION: REFERENCE BAR ────────────────────────────────────────────────
    const qreal REF_Y = HDR_H;
    const qreal REF_H = mm(7.5);

    p.setPen(Qt::NoPen);
    p.setBrush(REF_BG);
    p.drawRect(QRectF(0, REF_Y, PW, REF_H));
    p.setPen(QPen(BORDER, mm(0.18)));
    p.drawLine(QPointF(0, REF_Y + REF_H), QPointF(PW, REF_Y + REF_H));

    QFont f8  = F("Arial", 8);
    QFont f8b = F("Arial", 8, true);
    p.setFont(f8);
    p.setPen(LABEL_GRAY);
    p.drawText(QPointF(ML, REF_Y + REF_H * 0.68),
               QString::fromUtf8("Référence : ") + refNumber);

    p.setFont(f8b);
    p.setPen(TEXT_DARK);
    {
        const QString ds = QString::fromUtf8("Tunis, le ") + certDate;
        p.drawText(QPointF(PW - MR - QFontMetricsF(f8b).horizontalAdvance(ds),
                           REF_Y + REF_H * 0.68), ds);
    }

    // ── SECTION: FOOTER (pinned to bottom) ────────────────────────────────────
    const qreal FOOT_H = mm(7.5);
    const qreal FOOT_Y = PH - FOOT_H;

    p.setPen(Qt::NoPen);
    p.setBrush(FOOT_BG);
    p.drawRect(QRectF(0, FOOT_Y, PW, FOOT_H));
    p.setPen(QPen(MID_GREEN, mm(0.5)));
    p.drawLine(QPointF(0, FOOT_Y), QPointF(PW, FOOT_Y));

    p.setFont(F("Arial", 6));
    p.setPen(LABEL_GRAY);
    p.drawText(QPointF(ML, FOOT_Y + FOOT_H * 0.68),
               QString::fromUtf8("WoodFlow — Zone Industrielle, Tunis — contact@woodflow.tn — +216 71 000 000"));

    QFont fFootB = F("Arial", 6, true);
    p.setFont(fFootB);
    p.setPen(MID_GREEN);
    {
        const QString fr2 = QString::fromUtf8("Réf. ") + refNumber + QString::fromUtf8(" — Confidentiel");
        p.drawText(QPointF(PW - MR - QFontMetricsF(fFootB).horizontalAdvance(fr2),
                           FOOT_Y + FOOT_H * 0.68), fr2);
    }

    // ── BODY ──────────────────────────────────────────────────────────────────
    qreal y = REF_Y + REF_H + mm(8.0);

    // Title
    QFont fTitle = F("Arial", 18, true);
    p.setFont(fTitle);
    p.setPen(TEXT_DARK);
    {
        const QString title = QString::fromUtf8("ATTESTATION DE TRAVAIL");
        p.drawText(QPointF((PW - QFontMetricsF(fTitle).horizontalAdvance(title)) / 2.0,
                           y + mm(6.5)), title);
    }
    y += mm(10.0);
    p.setPen(QPen(MID_GREEN, mm(0.7)));
    p.drawLine(QPointF((PW - mm(49.5)) / 2.0, y), QPointF((PW + mm(49.5)) / 2.0, y));
    y += mm(7.0);

    // Intro paragraph
    QFont f9 = F("Arial", 9);
    p.setFont(f9);
    p.setPen(TEXT_DARK);
    const QString intro = QString::fromUtf8(
        "Je soussigné(e), La Direction des Ressources Humaines de la société WoodFlow, "
        "enregistrée sous le matricule fiscal 0012345/A, sise Zone Industrielle, Tunis, "
        "Tunisie, atteste par la présente que :");
    p.drawText(QRectF(ML, y, TW, mm(22)), Qt::AlignJustify | Qt::TextWordWrap, intro);
    y += QFontMetricsF(f9).height() * 2.0 + mm(5.0);

    // ── Employee info card ────────────────────────────────────────────────────
    const qreal ROW_H  = mm(7.5);
    const qreal CARD_H = 4 * ROW_H;
    const qreal c1x = ML + mm(1.5), c1w = TW * 0.20;
    const qreal c2x = c1x + c1w,    c2w = TW * 0.28;
    const qreal c3x = c2x + c2w,    c3w = TW * 0.20;
    const qreal c4x = c3x + c3w,    c4w = TW - (c4x - ML) - mm(2.0);

    struct CardRow { QString l1, v1, l2, v2; bool boldV; };
    const QList<CardRow> cardRows = {
        { QString::fromUtf8("Nom et prénom :"),  e.getFullName(),    QString::fromUtf8("Numéro CIN :"),      e.getCin(),             true  },
        { QString::fromUtf8("Poste occupé :"),   e.getPoste(),       QString::fromUtf8("Date d'embauche :"), hireDate,               false },
        { QString::fromUtf8("Email pro. :"),     e.getEmail(),       QString::fromUtf8("Ancienneté :"),      anciennete,             false },
        { QString::fromUtf8("Téléphone :"),      e.getTelephone(),   QString::fromUtf8("Statut :"),          e.getDisponibilite(),   true  },
    };

    // Alternating row backgrounds
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 4; ++i) {
        p.setBrush(i % 2 == 0 ? CARD_BG : CARD_ALT);
        p.drawRect(QRectF(ML, y + i * ROW_H, TW, ROW_H));
    }
    // Outer border
    p.setPen(QPen(BORDER, mm(0.18)));
    p.setBrush(Qt::NoBrush);
    p.drawRect(QRectF(ML, y, TW, CARD_H));
    // Vertical separators
    p.drawLine(QPointF(c2x, y), QPointF(c2x, y + CARD_H));
    p.drawLine(QPointF(c3x, y), QPointF(c3x, y + CARD_H));
    p.drawLine(QPointF(c4x, y), QPointF(c4x, y + CARD_H));
    // Horizontal separators
    for (int i = 1; i < 4; ++i)
        p.drawLine(QPointF(ML, y + i * ROW_H), QPointF(ML + TW, y + i * ROW_H));
    // Left accent bar
    p.setPen(Qt::NoPen);
    p.setBrush(MID_GREEN);
    p.drawRect(QRectF(ML, y, mm(1.5), CARD_H));

    QFont fLbl  = F("Arial", 8, true);
    QFont fVal  = F("Arial", 8);
    QFont fValB = F("Arial", 8, true);
    for (int i = 0; i < cardRows.size(); ++i) {
        const CardRow& row = cardRows[i];
        qreal ry = y + i * ROW_H;
        p.setFont(fLbl);  p.setPen(LABEL_GRAY);
        p.drawText(QRectF(c1x + mm(2), ry, c1w - mm(2), ROW_H), Qt::AlignLeft | Qt::AlignVCenter, row.l1);
        p.setFont(row.boldV ? fValB : fVal); p.setPen(TEXT_DARK);
        p.drawText(QRectF(c2x + mm(2), ry, c2w - mm(2), ROW_H), Qt::AlignLeft | Qt::AlignVCenter, row.v1);
        p.setFont(fLbl);  p.setPen(LABEL_GRAY);
        p.drawText(QRectF(c3x + mm(2), ry, c3w - mm(2), ROW_H), Qt::AlignLeft | Qt::AlignVCenter, row.l2);
        p.setFont(row.boldV ? fValB : fVal); p.setPen(TEXT_DARK);
        p.drawText(QRectF(c4x + mm(2), ry, c4w - mm(2), ROW_H), Qt::AlignLeft | Qt::AlignVCenter, row.v2);
    }
    y += CARD_H + mm(6.0);

    // ── Body paragraph (rich text with bold) ─────────────────────────────────
    // QTextDocument is drawn at identity scale; pass pixel-based rect & font size.
    auto drawRichText = [&](const QString& html, const QRectF& rectPx) {
        QTextDocument tdoc;
        QFont docFont("Arial");
        docFont.setPixelSize(qRound(9.0 * dpi / 72.0));
        tdoc.setDefaultFont(docFont);
        tdoc.setHtml(html);
        tdoc.setTextWidth(rectPx.width());
        QAbstractTextDocumentLayout::PaintContext ctx;
        ctx.palette.setColor(QPalette::Text, TEXT_DARK);
        p.save();
        p.translate(rectPx.topLeft());
        tdoc.documentLayout()->draw(&p, ctx);
        p.restore();
        return tdoc.size().height();
    };

    const int fPx = qRound(9.0 * dpi / 72.0);
    const QString bodyHtml = QString::fromUtf8(
        "<span style='font-family:Arial;font-size:%7px;'>"
        "%1 <b>%2</b> est employé(e) au sein de notre société en qualité de "
        "<b>%3</b> depuis le %4, soit une ancienneté de <b>%5</b>. "
        "À ce jour, %6 n'est ni en période d'essai, "
        "ni en procédure de démission ou de licenciement.</span>")
        .arg(civility, e.getFullName().toHtmlEscaped(),
             e.getPoste().toHtmlEscaped(), hireDate.toHtmlEscaped(),
             anciennete.toHtmlEscaped(), pronoun)
        .arg(fPx);
    qreal bodyH = drawRichText(bodyHtml, QRectF(ML, y, TW, mm(30)));
    y += bodyH + mm(6.0);

    // ── Purpose box ───────────────────────────────────────────────────────────
    {
        const QString purposeHtml = QString::fromUtf8(
            "<span style='font-family:Arial;font-size:%1px;font-style:italic;color:#374151;'>"
            "La présente attestation est délivrée à l'intéressé(e) à sa demande et pour "
            "servir et valoir ce que de droit, notamment pour toute démarche administrative, "
            "bancaire ou consulaire.</span>").arg(fPx);

        QTextDocument tdoc;
        QFont pf("Arial", -1, -1, true);
        pf.setPixelSize(fPx);
        tdoc.setDefaultFont(pf);
        tdoc.setHtml(purposeHtml);
        tdoc.setTextWidth(TW - mm(6.0));
        qreal boxH = tdoc.size().height() + mm(6.0);

        p.setPen(Qt::NoPen);
        p.setBrush(PURPOSE_BG);
        p.drawRect(QRectF(ML, y, TW, boxH));
        p.setPen(QPen(STAMP_COL, mm(0.35), Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(QRectF(ML, y, TW, boxH));

        QAbstractTextDocumentLayout::PaintContext ctx;
        ctx.palette.setColor(QPalette::Text, QColor("#374151"));
        p.save();
        p.translate(ML + mm(3.0), y + mm(3.0));
        tdoc.documentLayout()->draw(&p, ctx);
        p.restore();
        y += boxH + mm(9.0);
    }

    // ── Signature block ───────────────────────────────────────────────────────
    QFont fMeta = F("Arial", 7);
    p.setFont(fMeta); p.setPen(LABEL_GRAY);
    p.drawText(QPointF(ML, y + mm(3.5)),  QString::fromUtf8("Document généré le : ") + certDate);
    p.drawText(QPointF(ML, y + mm(7.5)),  QString::fromUtf8("Réf. : ") + refNumber);
    p.setFont(F("Arial", 7, false, true)); p.setPen(GRAY_LIGHT);
    p.drawText(QPointF(ML, y + mm(11.5)), QString::fromUtf8("Valable 3 mois à compter de sa date d'émission."));

    const qreal SIG_X = PW * 0.50;
    const qreal SIG_W = PW * 0.47;

    QFont fSigT = F("Arial", 9, true);
    p.setFont(fSigT); p.setPen(TEXT_DARK);
    {
        const QString st = QString::fromUtf8("Le Directeur des Ressources Humaines");
        qreal stw = QFontMetricsF(fSigT).horizontalAdvance(st);
        p.drawText(QPointF(SIG_X + (SIG_W - mm(18) - stw) / 2.0, y + mm(3.5)), st);
    }
    const qreal SIG_LINE_Y = y + mm(17.0);
    p.setPen(QPen(TEXT_DARK, mm(0.18)));
    p.drawLine(QPointF(SIG_X + mm(4), SIG_LINE_Y), QPointF(SIG_X + SIG_W - mm(20), SIG_LINE_Y));

    p.setFont(F("Arial", 7)); p.setPen(LABEL_GRAY);
    {
        const QString sub = QString::fromUtf8("Direction RH — WoodFlow");
        qreal subw = QFontMetricsF(F("Arial", 7)).horizontalAdvance(sub);
        p.drawText(QPointF(SIG_X + mm(4) + (SIG_W - mm(24) - subw) / 2.0,
                           SIG_LINE_Y + mm(3.5)), sub);
    }

    // Stamp circle
    const qreal SR  = mm(8.0);
    const qreal SCX = SIG_X + SIG_W - mm(10);
    const qreal SCY = SIG_LINE_Y - mm(2);
    p.setPen(QPen(STAMP_COL, mm(0.35), Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(SCX, SCY), SR, SR);
    p.setFont(F("Arial", 6)); p.setPen(STAMP_COL);
    p.drawText(QRectF(SCX - SR, SCY - SR, SR * 2, SR * 2),
               Qt::AlignCenter, QString::fromUtf8("Cachet\nofficiel"));

    p.end();

    QMessageBox::information(this, QString::fromUtf8("Attestation générée"),
        QString::fromUtf8("L'attestation de travail de %1 a été générée :\n%2")
            .arg(e.getFullName()).arg(filePath));
}


void EmployeeManagementPage::onRefreshTable()
{
    m_searchInput->clear();
    m_filterCombo->setCurrentIndex(0);
    m_sortCombo->setCurrentIndex(0);
    loadEmployees();
    populateFilterCombo();   // re-sync on manual refresh too
}

void EmployeeManagementPage::onTableSelectionChanged()
{
    updateButtonStates();
}

// ============================================================================
//  Statistics Tab — rich visual charts
// ============================================================================

// ── Donut/Pie chart widget ───────────────────────────────────────────────────
class PieChartWidget : public QWidget {
public:
    struct Slice { QString label; double value; QColor color; };
    bool donut = true;
    QString centerText;
    QString title;

    PieChartWidget(QWidget* p=nullptr) : QWidget(p) {
        setMinimumSize(220,220);
        setAttribute(Qt::WA_TranslucentBackground);
        setAutoFillBackground(false);
    }
    void setSlices(const QList<Slice>& s) { slices=s; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        int w=width(), h=height();

        // Title
        bool isDarkPie = qApp->styleSheet().contains("0f0f0f");
        QColor pieText = isDarkPie ? QColor("#f0f0f0") : QColor("#374151");
        QColor pieSub  = isDarkPie ? QColor("#9ca3af") : QColor("#4b5563");
        int tH = title.isEmpty() ? 0 : 26;
        if (!title.isEmpty()) {
            QFont tf; tf.setPixelSize(13); tf.setBold(true); p.setFont(tf);
            p.setPen(pieText);
            p.drawText(0,0,w,tH,Qt::AlignCenter,title);
        }

        // Legend height
        int legendRows = (slices.size()+1)/2;
        int legendH = legendRows * 20 + 8;

        int chartH = h - tH - legendH;
        int r = qMin(w, chartH)/2 - 10;
        int cx = w/2, cy = tH + chartH/2;

        double total=0; for(auto& s:slices) total+=s.value;
        if(total==0) return;

        // Draw segments
        double angle = 90.0;
        for(auto& s:slices) {
            double span = 360.0*s.value/total;
            p.setBrush(s.color); p.setPen(Qt::NoPen);
            p.drawPie(cx-r,cy-r,2*r,2*r,(int)(angle*16),-(int)(span*16));
            angle -= span;
        }

        // Donut hole with subtle shadow ring
        if(donut) {
            int hole = r*6/10;
            // Shadow ring
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(0,0,0,18),3));
            p.drawEllipse(cx-hole,cy-hole,2*hole,2*hole);
            // Hole — explicit dark-aware color
            bool isDarkHole = qApp->styleSheet().contains("0f0f0f");
            p.setBrush(QColor(isDarkHole ? "#1e1e1e" : "#FFFFFF")); p.setPen(Qt::NoPen);
            p.drawEllipse(cx-hole,cy-hole,2*hole,2*hole);

        }

        // Draw percentage labels on segments large enough to show them
        angle = 90.0;
        QFont pf; pf.setPixelSize(10); pf.setBold(true); p.setFont(pf);
        for(auto& s:slices) {
            double span = 360.0*s.value/total;
            if(span >= 20) {  // only label segments >= 20 degrees
                double midAngle = (angle - span/2.0) * M_PI / 180.0;
                int labelR = donut ? (r*6/10 + r)/2 : r*6/10;
                int lx2 = (int)(cx + labelR*cos(midAngle));
                int ly2 = (int)(cy - labelR*sin(midAngle));
                double pct2 = s.value*100.0/total;
                p.setPen(QColor(255,255,255,220));
                QString pctStr = QString("%1%").arg(pct2,0,'f',0);
                p.drawText(lx2-20, ly2-8, 40, 16, Qt::AlignCenter, pctStr);
            }
            angle -= span;
        }

        // Segment separator lines — from hole edge to outer rim only (not through center)
        angle = 90.0;
        p.setPen(QPen(QColor(255,255,255,180),1.5));
        int innerR = donut ? r*6/10 : 0;
        for(auto& s:slices) {
            double rad = angle*M_PI/180.0;
            double cosA = cos(rad), sinA = sin(rad);
            p.drawLine((int)(cx + innerR*cosA), (int)(cy - innerR*sinA),
                       (int)(cx + r*cosA),      (int)(cy - r*sinA));
            angle -= 360.0*s.value/total;
        }

        // Legend
        int lx=8, ly = tH + chartH + 8;
        QFont lf; lf.setPixelSize(11); p.setFont(lf);
        for(int i=0;i<slices.size();i++) {
            int col=i%2, row=i/2;
            int ox = col*(w/2)+lx;
            int oy = ly+row*20;
            // Color dot
            p.setBrush(slices[i].color); p.setPen(Qt::NoPen);
            p.drawEllipse(ox,oy+4,10,10);
            p.setPen(pieSub);
            double pct = total > 0 ? slices[i].value * 100.0 / total : 0;
            QString txt = QString("%1  %2%").arg(slices[i].label)
                              .arg(pct, 0, 'f', 1);
            p.drawText(ox+14,oy,w/2-20,18,Qt::AlignVCenter,txt);
        }
    }
private:
    QList<Slice> slices;
};

// ── Vertical bar chart widget ─────────────────────────────────────────────────
class BarChartWidget : public QWidget {
public:
    struct Bar { QString label; double value; QColor color; };
    QString title;
    QString unit;

    BarChartWidget(QWidget* p=nullptr) : QWidget(p) {
        setMinimumSize(200,180);
        setAttribute(Qt::WA_TranslucentBackground);
        setAutoFillBackground(false);
    }
    void setBars(const QList<Bar>& b) { bars=b; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        if(bars.isEmpty()) return;
        int w=width(), h=height();
        int tH=26, botH=44, topPad=24;
        int chartH=h-tH-botH-topPad;

        // Colors — resolve early so all sections can use them
        bool isDark = qApp->styleSheet().contains("0f0f0f");
        QColor gridCol  = isDark ? QColor("#2e2e2e") : QColor("#e5e7eb");
        QColor labelCol = isDark ? QColor("#6b7280") : QColor("#9ca3af");
        QColor titleCol = isDark ? QColor("#f0f0f0") : QColor("#374151");

        // Title
        QFont tf; tf.setPixelSize(13); tf.setBold(true); p.setFont(tf);
        p.setPen(titleCol);
        p.drawText(0,0,w,tH,Qt::AlignCenter,title);

        double maxV=0; for(auto& b:bars) maxV=qMax(maxV,b.value);
        if(maxV==0) return;

        // Grid lines
        p.setPen(QPen(gridCol,1,Qt::DashLine));
        QFont gf; gf.setPixelSize(9); p.setFont(gf);
        for(int i=1;i<=4;i++) {
            int y=tH+topPad+chartH - (int)(chartH*i/4.0);
            p.drawLine(40,y,w-8,y);
            p.setPen(labelCol);
            p.drawText(0,y-8,36,16,Qt::AlignRight|Qt::AlignVCenter,
                       QString::number(maxV*i/4,'f',0));
            p.setPen(QPen(QColor("#e5e7eb"),1,Qt::DashLine));
        }

        int n=bars.size();
        int totalW=w-48;
        int barW=qMax(12,(totalW/n)-8);
        int gap=(totalW - barW*n)/(n+1);

        QFont vf; vf.setPixelSize(10); vf.setBold(true); p.setFont(vf);
        QFont lf; lf.setPixelSize(10); lf.setBold(false);

        for(int i=0;i<n;i++) {
            int bh=(int)(bars[i].value/maxV*chartH);
            int bx=48+gap+(barW+gap)*i;
            int by=tH+topPad+chartH-bh;

            // Shadow
            p.fillRect(bx+2,by+2,barW,bh,QColor(0,0,0,18));

            // Bar with rounded top
            QPainterPath path;
            int rad=qMin(6,barW/3);
            path.moveTo(bx,by+bh);
            path.lineTo(bx,by+rad);
            path.quadTo(bx,by,bx+rad,by);
            path.lineTo(bx+barW-rad,by);
            path.quadTo(bx+barW,by,bx+barW,by+rad);
            path.lineTo(bx+barW,by+bh);
            path.closeSubpath();

            // Gradient fill
            QLinearGradient grad(bx,by,bx,by+bh);
            grad.setColorAt(0,bars[i].color.lighter(115));
            grad.setColorAt(1,bars[i].color);
            p.fillPath(path,grad);

            // Value on top
            p.setPen(titleCol); p.setFont(vf);
            QString valStr=QString::number(bars[i].value,'f',0)+unit;
            p.drawText(bx-4,by-topPad+2,barW+8,topPad-2,Qt::AlignCenter,valStr);

            // Label
            p.setFont(lf); p.setPen(labelCol);
            p.drawText(bx-4,tH+topPad+chartH+4,barW+8,40,
                       Qt::AlignHCenter|Qt::TextWordWrap,bars[i].label);
        }
    }
private:
    QList<Bar> bars;
};

// ── Performance gauge ─────────────────────────────────────────────────────────
class GaugeWidget : public QWidget {
public:
    QString title;
    GaugeWidget(QWidget* p=nullptr) : QWidget(p),m_val(0),m_max(10) {
        setMinimumSize(180,130);
        setAttribute(Qt::WA_TranslucentBackground);
        setAutoFillBackground(false);
    }
    void setValue(double v, double max=10) { m_val=v; m_max=max; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        int w=width(), h=height();
        int tH=22;
        bool isDark = qApp->styleSheet().contains("0f0f0f");

        QFont tf; tf.setPixelSize(12); tf.setBold(true); p.setFont(tf);
        p.setPen(isDark ? QColor("#f0f0f0") : QColor("#374151"));
        p.drawText(0,0,w,tH,Qt::AlignCenter,title);

        int r=qMin(w,h-tH)*9/20;
        int cx=w/2, cy=tH+(h-tH)*8/10;

        // Background arc (180°)
        QPen bgPen(isDark ? QColor("#2e2e2e") : QColor("#e5e7eb"),14,Qt::SolidLine,Qt::RoundCap);
        p.setPen(bgPen); p.setBrush(Qt::NoBrush);
        p.drawArc(cx-r,cy-r,2*r,2*r,0*16,180*16);

        // Value arc
        double ratio = m_max>0 ? qMin(1.0,m_val/m_max) : 0;
        QColor col = ratio>=0.8 ? QColor("#16a34a")
                   : ratio>=0.5 ? QColor("#8A9A5B")
                   : ratio>=0.3 ? QColor("#f59e0b")
                                : QColor("#ef4444");
        QPen valPen(col,14,Qt::SolidLine,Qt::RoundCap);
        p.setPen(valPen);
        p.drawArc(cx-r,cy-r,2*r,2*r,0*16,(int)(180*ratio*16));

        // Value + max drawn together, centered below the arc midpoint
        QString valStr = QString::number(m_val,'f',1);
        QString maxStr = QString(" / %1").arg((int)m_max);

        QFont vf; vf.setPixelSize(22); vf.setBold(true); p.setFont(vf);
        QFontMetrics vfm(vf);
        QFont sf; sf.setPixelSize(11); sf.setBold(false);
        QFontMetrics sfm(sf);

        int valW = vfm.horizontalAdvance(valStr);
        int maxW = sfm.horizontalAdvance(maxStr);
        int totalW = valW + maxW;
        int startX = cx - totalW/2;
        int textY = cy - 4;  // just below arc center

        p.setPen(col);
        p.setFont(vf);
        p.drawText(startX, textY, valStr);

        p.setPen(isDark ? QColor("#6b7280") : QColor("#9ca3af"));
        p.setFont(sf);
        // Align baseline of small text with baseline of large text
        p.drawText(startX + valW, textY - (vfm.ascent() - sfm.ascent()) + vfm.ascent() - sfm.ascent(), maxStr);
    }
private:
    double m_val, m_max;
};

QWidget* EmployeeManagementPage::createStatsTab()
{
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *inner = new QWidget;
    bool dark = qApp->styleSheet().contains("0f0f0f");
    QString pageBg  = dark ? "#111111" : "#F3EFE0";
    QString cardBg  = dark ? "#1e1e1e" : "#FFFFFF";
    QString cardBrd = dark ? "#2e2e2e" : "#E8E4DC";
    QString textCol = dark ? "#f0f0f0" : "#374151";
    QString subCol  = dark ? "#9ca3af" : "#6b7280";

    // Store inner widget ref so refreshStats can re-apply theme
    m_statsInner = inner;
    auto applyStatsTheme = [](QWidget* w) {
        bool d = qApp->styleSheet().contains("0f0f0f");
        QString pg = d?"#111111":"#F3EFE0";
        QString cb = d?"#1e1e1e":"#FFFFFF";
        QString br = d?"#2e2e2e":"#E8E4DC";
        QString tx = d?"#f0f0f0":"#374151";
        w->setStyleSheet(
            QString("QFrame#statCard{background-color:%1;border:1px solid %2;border-radius:14px;}"
                    "QWidget{background-color:%3;}"
                    "QLabel{color:%4;background:transparent;}").arg(cb,br,pg,tx));
    };
    applyStatsTheme(inner);
    // Store lambda result for re-use in refreshStats via a flag
    Q_UNUSED(applyStatsTheme);
    QVBoxLayout *lay = new QVBoxLayout(inner);
    lay->setContentsMargins(8,12,8,24);
    lay->setSpacing(14);

    // ── KPI row ───────────────────────────────────────────────────────────────
    m_kpiRow = new QHBoxLayout();
    m_kpiRow->setSpacing(12);

    auto makeKpi = [&](const QString& title, const QString& color) -> QFrame* {
        QFrame *card = new QFrame; card->setObjectName("statCard");
        card->setMinimumHeight(96);
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(20,16,20,16); cl->setSpacing(2);
        QLabel *t = new QLabel(title);
        t->setStyleSheet(QString("font-size:10px;font-weight:700;letter-spacing:1px;"
                         "color:%1;background:transparent;").arg(subCol));
        QLabel *v = new QLabel("—");
        v->setObjectName("kpiVal");
        v->setStyleSheet(QString("font-size:30px;font-weight:800;color:%1;"
                                 "background:transparent;").arg(color));
        QLabel *s = new QLabel;
        s->setObjectName("kpiSub");
        s->setStyleSheet(QString("font-size:11px;color:%1;background:transparent;").arg(subCol));
        cl->addWidget(t); cl->addWidget(v); cl->addWidget(s); cl->addStretch();
        return card;
    };

    m_kpiTotal = makeKpi("EFFECTIF TOTAL",   "#8A9A5B");
    m_kpiPerf  = makeKpi("PERFORMANCE MOY.", "#5B8A9A");
    m_kpiAvail = makeKpi("DISPONIBLES",      "#9A8A5B");
    m_kpiPay   = makeKpi("MASSE SALARIALE",  "#9A5B8A");
    m_kpiRow->addWidget(m_kpiTotal);
    m_kpiRow->addWidget(m_kpiPerf);
    m_kpiRow->addWidget(m_kpiAvail);
    m_kpiRow->addWidget(m_kpiPay);
    lay->addLayout(m_kpiRow);

    // ── Row 2: Donut (postes) + Pie (dispo) + Gauge (perf) ───────────────────
    QHBoxLayout *row2 = new QHBoxLayout(); row2->setSpacing(12);

    // Donut — postes
    QFrame *c1 = new QFrame; c1->setObjectName("statCard");
    QVBoxLayout *l1 = new QVBoxLayout(c1); l1->setContentsMargins(12,12,12,12);
    m_posteDonut = new PieChartWidget; m_posteDonut->donut=true;
    m_posteDonut->title="Repartition par poste";
    m_posteDonut->setMinimumHeight(280);
    m_posteDonut->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    l1->addWidget(m_posteDonut);

    // Pie — disponibilite
    QFrame *c2 = new QFrame; c2->setObjectName("statCard");
    QVBoxLayout *l2 = new QVBoxLayout(c2); l2->setContentsMargins(12,12,12,12);
    m_dispoPie = new PieChartWidget; m_dispoPie->donut=false;
    m_dispoPie->title="Disponibilite";
    m_dispoPie->setMinimumHeight(280);
    m_dispoPie->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    l2->addWidget(m_dispoPie);

    // Gauge — performance moyenne
    QFrame *c3 = new QFrame; c3->setObjectName("statCard");
    QVBoxLayout *l3 = new QVBoxLayout(c3); l3->setContentsMargins(16,16,16,16); l3->setSpacing(8);
    m_perfGauge = new GaugeWidget;
    m_perfGauge->title="Performance moyenne";
    m_perfGauge->setMinimumSize(180,160);
    m_perfGauge->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    l3->addWidget(m_perfGauge, 1);
    m_absLabel = new QLabel("—");
    m_absLabel->setAlignment(Qt::AlignCenter);
    m_absLabel->setWordWrap(true);
    m_absLabel->setStyleSheet("font-size:11px;background:transparent;");
    l3->addWidget(m_absLabel);

    row2->addWidget(c1,2); row2->addWidget(c2,2); row2->addWidget(c3,2);
    lay->addLayout(row2);

    // ── Row 3: Salary bar chart ───────────────────────────────────────────────
    QFrame *c4 = new QFrame; c4->setObjectName("statCard");
    QVBoxLayout *l4 = new QVBoxLayout(c4); l4->setContentsMargins(16,14,16,14);
    m_salaryBar = new BarChartWidget;
    m_salaryBar->title="Salaire moyen par poste (€)";
    m_salaryBar->unit=" €";
    m_salaryBar->setMinimumHeight(220);
    l4->addWidget(m_salaryBar);
    lay->addWidget(c4);

    // ── Trace table ───────────────────────────────────────────────────────────
    QFrame *traceCard = new QFrame; traceCard->setObjectName("statCard");
    QVBoxLayout *trl = new QVBoxLayout(traceCard);
    trl->setContentsMargins(16,14,16,14); trl->setSpacing(8);
    QLabel *trTitle = new QLabel("Tableau de suivi individuel");
    trTitle->setStyleSheet("font-size:14px;font-weight:700;background:transparent;color:#374151;");
    trl->addWidget(trTitle);

    m_statsTable = new QTableWidget(0,8);
    m_statsTable->setObjectName("empTable");
    m_statsTable->setHorizontalHeaderLabels({
        "Employe","Poste","Performance","Heures","Absences","Conges","Salaire","Tendance"
    });
    m_statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_statsTable->horizontalHeader()->setStretchLastSection(true);
    m_statsTable->setColumnWidth(0,160); m_statsTable->setColumnWidth(1,120);
    m_statsTable->setColumnWidth(2,110); m_statsTable->setColumnWidth(3,90);
    m_statsTable->setColumnWidth(4,80);  m_statsTable->setColumnWidth(5,80);
    m_statsTable->setColumnWidth(6,110);
    m_statsTable->verticalHeader()->setVisible(false);
    m_statsTable->setShowGrid(false);
    m_statsTable->setSortingEnabled(true);
    m_statsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_statsTable->setMinimumHeight(300);
    trl->addWidget(m_statsTable);
    lay->addWidget(traceCard);

    scroll->setWidget(inner);
    return scroll;
}

void EmployeeManagementPage::refreshStats()
{
    // Re-apply dark/light theme to stats inner widget
    if(m_statsInner) {
        bool d = qApp->styleSheet().contains("0f0f0f");
        QString pg = d?"#111111":"#F3EFE0";
        QString cb = d?"#1e1e1e":"#FFFFFF";
        QString br = d?"#2e2e2e":"#E8E4DC";
        QString tx = d?"#f0f0f0":"#374151";
        m_statsInner->setStyleSheet(
            QString("QFrame#statCard{background-color:%1;border:1px solid %2;border-radius:14px;}"
                    "QWidget{background-color:%3;}"
                    "QLabel{color:%4;background:transparent;}").arg(cb,br,pg,tx));
        // Force all charts to repaint with new colors
        for(QWidget* w : m_statsInner->findChildren<QWidget*>())
            w->update();
    }

    // Force card and label colors directly on each statCard frame
    bool d2 = qApp->styleSheet().contains("0f0f0f");
    QString cb2 = d2?"#1e1e1e":"#FFFFFF";
    QString br2 = d2?"#2e2e2e":"#E8E4DC";
    QString tx2 = d2?"#f0f0f0":"#374151";
    QString sc2 = d2?"#6b7280":"#9ca3af";
    if(m_statsInner) {
        for(QFrame* f : m_statsInner->findChildren<QFrame*>("statCard")) {
            f->setStyleSheet(
                QString("QFrame#statCard{background-color:%1;border:1px solid %2;border-radius:14px;}")
                    .arg(cb2, br2));
        }
        for(QLabel* l : m_statsInner->findChildren<QLabel*>()) {
            if(l->objectName()!="kpiVal")
                l->setStyleSheet(l->styleSheet().replace("#374151",tx2)
                                                 .replace("#9ca3af",sc2)
                                                 .replace("#6b7280",sc2));
        }
    }

    QList<Employee> employees = EmployeeDatabase::instance().getAllEmployees();
    if(employees.isEmpty()) return;

    int total=employees.size();
    double totalPerf=0,totalSalary=0,totalAbs=0;
    int available=0;
    QMap<QString,int> byPoste;
    QMap<QString,int> byDisp;
    QMap<QString,double> salByPoste;
    QMap<QString,int>    cntByPoste;

    for(auto& e:employees){
        totalPerf+=e.getPerformance();
        totalSalary+=e.getSalaire();
        totalAbs+=e.getNbJoursAbsence();
        if(e.getDisponibilite()=="Disponible") available++;
        QString poste=e.getPoste().isEmpty()?"Autre":e.getPoste();
        byPoste[poste]++;
        byDisp[e.getDisponibilite().isEmpty()?"Non defini":e.getDisponibilite()]++;
        salByPoste[poste]+=e.getSalaire();
        cntByPoste[poste]++;
    }
    double avgPerf=totalPerf/total;

    // ── KPIs ──────────────────────────────────────────────────────────────────
    auto setKpi=[](QFrame* card,const QString& val,const QString& sub){
        card->findChild<QLabel*>("kpiVal")->setText(val);
        card->findChild<QLabel*>("kpiSub")->setText(sub);
        // Update sub label color for dark mode
        bool d = qApp->styleSheet().contains("0f0f0f");
        QString sc = d?"#6b7280":"#9ca3af";
        card->findChild<QLabel*>("kpiSub")->setStyleSheet(
            QString("font-size:11px;color:%1;background:transparent;").arg(sc));
    };
    setKpi(m_kpiTotal,QString::number(total),"employes");
    setKpi(m_kpiPerf,QString::number(avgPerf,'f',1),"/ 10");
    setKpi(m_kpiAvail,QString::number(available),QString("/ %1").arg(total));
setKpi(m_kpiPay,QString::number(totalSalary,'f',0),"€ / mois");

    // ── Palette ───────────────────────────────────────────────────────────────
    static const QList<QColor> pal={
        QColor("#8A9A5B"),QColor("#5B8A9A"),QColor("#9A8A5B"),
        QColor("#9A5B8A"),QColor("#5B9A8A"),QColor("#8A5B5B"),
        QColor("#6B8A5B"),QColor("#5B6B8A")
    };

    // ── Donut — postes ────────────────────────────────────────────────────────
    QList<PieChartWidget::Slice> postSlices;
    int pi=0;
    for(auto it=byPoste.constBegin();it!=byPoste.constEnd();++it,++pi)
        postSlices.append({it.key(),(double)it.value(),pal[pi%pal.size()]});
    m_posteDonut->setSlices(postSlices);

    // ── Pie — disponibilite ───────────────────────────────────────────────────
    QMap<QString,QColor> dc={
        {"Disponible",QColor("#8A9A5B")},{"Indisponible",QColor("#ef4444")},
        {"En conge",QColor("#f59e0b")},{"En formation",QColor("#5B8A9A")},
        {"Non defini",QColor("#9ca3af")}
    };
    QList<PieChartWidget::Slice> dispSlices;
    for(auto it=byDisp.constBegin();it!=byDisp.constEnd();++it)
        dispSlices.append({it.key(),(double)it.value(),dc.value(it.key(),QColor("#9ca3af"))});
    m_dispoPie->setSlices(dispSlices);

    // ── Gauge ─────────────────────────────────────────────────────────────────
    m_perfGauge->setValue(avgPerf,10);
    m_absLabel->setText(QString("Absences moyennes : %1 j / employe")
                            .arg(totalAbs / total, 0, 'f', 1));

    // ── Salary bars ───────────────────────────────────────────────────────────
    QList<BarChartWidget::Bar> salBars;
    QList<QPair<QString,double>> salList;
    for(auto it=salByPoste.constBegin();it!=salByPoste.constEnd();++it)
        salList.append({it.key(),it.value()/cntByPoste[it.key()]});
    std::sort(salList.begin(),salList.end(),[](auto&a,auto&b){return a.second>b.second;});
    pi=0;
    for(auto& [l,v]:salList){ salBars.append({l,v,pal[pi%pal.size()]}); pi++; }
    m_salaryBar->setBars(salBars);

    // ── Trace table ───────────────────────────────────────────────────────────
    m_statsTable->setSortingEnabled(false);
    m_statsTable->setRowCount(0);
    for(int i=0;i<employees.size();++i){
        const Employee& e=employees[i];
        m_statsTable->insertRow(i);
        m_statsTable->setRowHeight(i,44);
        auto mk=[](const QString& t,Qt::Alignment a=Qt::AlignVCenter|Qt::AlignLeft){
            auto* it=new QTableWidgetItem(t);
            it->setTextAlignment(a);
            it->setFlags(it->flags()&~Qt::ItemIsEditable);
            return it;
        };
        m_statsTable->setItem(i,0,mk(e.getFullName()));
        m_statsTable->setItem(i,1,mk(e.getPoste()));
        double perf=e.getPerformance();
        auto* pi2=mk(QString::number(perf,'f',1)+"/10",Qt::AlignCenter);
        pi2->setForeground(perf>=8?QColor("#16a34a"):perf>=5?QColor("#374151"):QColor("#dc2626"));
        QFont pf=pi2->font(); pf.setBold(perf<5||perf>=8); pi2->setFont(pf);
        m_statsTable->setItem(i,2,pi2);
        auto* hi=mk(QString::number(e.getHeuresTravail(),'f',0)+"h",Qt::AlignCenter);
        if(e.getHeuresTravail()<100) hi->setForeground(QColor("#dc2626"));
        m_statsTable->setItem(i,3,hi);
        auto* ai=mk(QString::number(e.getNbJoursAbsence())+"j",Qt::AlignCenter);
        if(e.getNbJoursAbsence()>3) ai->setForeground(QColor("#dc2626"));
        else if(e.getNbJoursAbsence()==0) ai->setForeground(QColor("#16a34a"));
        m_statsTable->setItem(i,4,ai);
        m_statsTable->setItem(i,5,mk(QString::number(e.getNbJoursConges())+"j",Qt::AlignCenter));
        m_statsTable->setItem(i,6,mk(QString::number(e.getSalaire(),'f',0)+" €",Qt::AlignCenter));
        QString trend; QColor tc;
        if(perf>=avgPerf+1.5){trend="Excellent";tc=QColor("#16a34a");}
        else if(perf>=avgPerf+0.5){trend="Au-dessus";tc=QColor("#22c55e");}
        else if(perf>=avgPerf-0.5){trend="Dans la moyenne";tc=QColor("#6b7280");}
        else if(perf>=avgPerf-1.5){trend="En dessous";tc=QColor("#f97316");}
        else{trend="Critique";tc=QColor("#dc2626");}
        auto* ti=mk(trend,Qt::AlignCenter);
        ti->setForeground(tc);
        QFont tf=ti->font(); tf.setBold(true); ti->setFont(tf);
        m_statsTable->setItem(i,7,ti);
    }
    m_statsTable->setSortingEnabled(true);
}

void EmployeeManagementPage::applyTabTheme()
{
    if (!m_tabs) return;
    bool d = qApp->styleSheet().contains("0f0f0f");
    QString bg  = d ? "#1a1a1a" : "#EDE8D9";
    QString hov = d ? "#222222" : "#E8E2D0";
    QString sel = d ? "#9aaa6b" : "#8A9A5B";
    QString txt = d ? "#9ca3af" : "#6b7280";
    QString act = d ? "#e5e7eb" : "#374151";

    QPalette pal = m_tabs->tabBar()->palette();
    pal.setColor(QPalette::Window, QColor(bg));
    pal.setColor(QPalette::Button, QColor(bg));
    pal.setColor(QPalette::Base,   QColor(bg));
    m_tabs->tabBar()->setAutoFillBackground(true);
    m_tabs->tabBar()->setPalette(pal);
    m_tabs->tabBar()->setStyleSheet(
        QString("QTabBar{background-color:%1;}"
                "QTabBar::tab{background-color:%1;color:%2;border:none;"
                "border-bottom:3px solid transparent;padding:10px 24px;"
                "font-size:13px;font-weight:600;margin-right:4px;}"
                "QTabBar::tab:selected{color:%3;border-bottom:3px solid %3;background-color:%1;}"
                "QTabBar::tab:hover:!selected{color:%4;background-color:%5;}")
            .arg(bg, txt, sel, act, hov));

    QPalette pal2 = m_tabs->palette();
    pal2.setColor(QPalette::Window,     QColor(bg));
    pal2.setColor(QPalette::Base,       QColor(bg));
    pal2.setColor(QPalette::Button,     QColor(bg));
    pal2.setColor(QPalette::WindowText, QColor(txt));
    m_tabs->setPalette(pal2);
}

void EmployeeManagementPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::StyleChange ||
        event->type() == QEvent::ApplicationPaletteChange) {
        applyTabTheme();
        if (m_statsInner) {
            bool d = qApp->styleSheet().contains("0f0f0f");
            QString pg=d?"#111111":"#F3EFE0", cb=d?"#1e1e1e":"#FFFFFF";
            QString br=d?"#2e2e2e":"#E8E4DC", tx=d?"#f0f0f0":"#374151";
            m_statsInner->setStyleSheet(
                QString("QFrame#statCard{background-color:%1;border:1px solid %2;border-radius:14px;}"
                        "QWidget{background-color:%3;}QLabel{color:%4;background:transparent;}")
                    .arg(cb, br, pg, tx));
            for (QWidget* w : m_statsInner->findChildren<QWidget*>())
                w->update();
            for (QFrame* f : m_statsInner->findChildren<QFrame*>("statCard"))
                f->setStyleSheet(QString("QFrame#statCard{background-color:%1;border:1px solid %2;border-radius:14px;}").arg(cb,br));
        }
    }
    QWidget::changeEvent(event);
}
