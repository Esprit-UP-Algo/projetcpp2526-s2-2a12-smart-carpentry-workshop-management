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
#include <QFrame>

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
    createToolbar();
    createTable();
}

void EmployeeManagementPage::createToolbar()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());

    // Toolbar wrapper — objectName is the QSS anchor
    QFrame *toolbar = new QFrame(this);
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
    m_filterCombo->addItem("Tous les postes", "all");
    m_filterCombo->addItem("Menuisier", "Menuisier");
    m_filterCombo->addItem("Chef d'equipe", "Chef d'equipe");
    m_filterCombo->addItem("Apprenti", "Apprenti");
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

    m_exportButton = new QPushButton("Exporter PDF", toolbar);
    m_exportButton->setObjectName("empSecondaryBtn");
    m_exportButton->setFixedHeight(40);
    m_exportButton->setCursor(Qt::PointingHandCursor);

    m_refreshButton = new QPushButton("Actualiser", toolbar);
    m_refreshButton->setObjectName("empSecondaryBtn");
    m_refreshButton->setFixedHeight(40);
    m_refreshButton->setCursor(Qt::PointingHandCursor);

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();

    toolbarLayout->addLayout(searchLayout);
    toolbarLayout->addLayout(buttonLayout);

    mainLayout->addWidget(toolbar);
}

void EmployeeManagementPage::createTable()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());

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

    mainLayout->addWidget(m_table);
}

void EmployeeManagementPage::setupConnections()
{
    connect(m_addButton,    &QPushButton::clicked, this, &EmployeeManagementPage::onAddEmployee);
    connect(m_editButton,   &QPushButton::clicked, this, &EmployeeManagementPage::onEditEmployee);
    connect(m_deleteButton, &QPushButton::clicked, this, &EmployeeManagementPage::onDeleteEmployee);
    connect(m_exportButton, &QPushButton::clicked, this, &EmployeeManagementPage::onExportPDF);
    connect(m_refreshButton,&QPushButton::clicked, this, &EmployeeManagementPage::onRefreshTable);
    connect(m_searchInput,  &QLineEdit::textChanged, this, &EmployeeManagementPage::onSearchTextChanged);
    connect(m_filterCombo,  &QComboBox::currentTextChanged, this, &EmployeeManagementPage::onFilterChanged);
    connect(m_sortCombo,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EmployeeManagementPage::onSortChanged);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &EmployeeManagementPage::onTableSelectionChanged);
    connect(m_table, &QTableWidget::doubleClicked, this, &EmployeeManagementPage::onEditEmployee);
}

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
    m_table->setItem(row, 6, new QTableWidgetItem(QString::number(employee.getSalaire(), 'f', 2) + " TND"));
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

void EmployeeManagementPage::onAddEmployee()
{
    EmployeeDialog dialog(this);
    dialog.setWindowTitle("Ajouter un employe");
    if (dialog.exec() == QDialog::Accepted) {
        Employee e = dialog.getEmployee();
        e.setId(EmployeeDatabase::instance().generateNextId());
        if (EmployeeDatabase::instance().addEmployee(e))
            { loadEmployees(); QMessageBox::information(this, "Succes", "Employe ajoute!"); }
        else
            QMessageBox::warning(this, "Erreur", "Impossible d'ajouter l'employe.");
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
        if (EmployeeDatabase::instance().updateEmployee(updated))
            { loadEmployees(); QMessageBox::information(this, "Succes", "Employe modifie!"); }
        else
            QMessageBox::warning(this, "Erreur", "Impossible de modifier l'employe.");
    }
}

void EmployeeManagementPage::onDeleteEmployee()
{
    Employee e = getSelectedEmployee();
    if (!e.isValid()) { QMessageBox::warning(this, "Aucune selection", "Selectionnez un employe."); return; }
    auto r = QMessageBox::question(this, "Confirmer",
        QString("Supprimer %1 ?").arg(e.getFullName()), QMessageBox::Yes | QMessageBox::No);
    if (r == QMessageBox::Yes) {
        if (EmployeeDatabase::instance().deleteEmployee(e.getId()))
            { loadEmployees(); QMessageBox::information(this, "Succes", "Employe supprime!"); }
        else
            QMessageBox::warning(this, "Erreur", "Impossible de supprimer l'employe.");
    }
}

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
    QString f = QFileDialog::getSaveFileName(this, "Exporter en PDF", "liste_employees.pdf", "PDF Files (*.pdf)");
    if (!f.isEmpty()) QMessageBox::information(this, "Export", "Export PDF a implementer");
}

void EmployeeManagementPage::onRefreshTable()
{
    m_searchInput->clear();
    m_filterCombo->setCurrentIndex(0);
    m_sortCombo->setCurrentIndex(0);
    loadEmployees();
}

void EmployeeManagementPage::onTableSelectionChanged()
{
    updateButtonStates();
}
