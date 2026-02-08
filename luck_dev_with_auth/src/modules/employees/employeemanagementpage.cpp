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

EmployeeManagementPage::EmployeeManagementPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
    loadEmployees();
}

void EmployeeManagementPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);
    
    createToolbar();
    createTable();
    
    mainLayout->addLayout(dynamic_cast<QVBoxLayout*>(layout()));
}

void EmployeeManagementPage::createToolbar()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (!mainLayout) {
        mainLayout = new QVBoxLayout(this);
    }
    
    // Search and filter section
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(10);
    
    QLabel *searchLabel = new QLabel("Rechercher:", this);
    searchLabel->setStyleSheet("font-weight: 500; color: #4D362D;");
    
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("Rechercher par nom, CIN ou poste...");
    m_searchInput->setMinimumWidth(300);
    m_searchInput->setStyleSheet(
        "QLineEdit {"
        "   padding: 8px 12px;"
        "   border: 1px solid #BDB5AD;"
        "   border-radius: 6px;"
        "   font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #8A9A5B;"
        "}"
    );
    
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem("Tous les postes", "all");
    m_filterCombo->addItem("Menuisier", "Menuisier");
    m_filterCombo->addItem("Chef d'équipe", "Chef d'équipe");
    m_filterCombo->addItem("Apprenti", "Apprenti");
    m_filterCombo->setMinimumWidth(180);
    m_filterCombo->setStyleSheet(
        "QComboBox {"
        "   padding: 8px 12px;"
        "   border: 1px solid #BDB5AD;"
        "   border-radius: 6px;"
        "   font-size: 13px;"
        "}"
    );
    
    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItem("Trier par nom", "name");
    m_sortCombo->addItem("Salaire (croissant)", "salary_asc");
    m_sortCombo->addItem("Salaire (décroissant)", "salary_desc");
    m_sortCombo->addItem("Date d'embauche (récent)", "date_desc");
    m_sortCombo->addItem("Date d'embauche (ancien)", "date_asc");
    m_sortCombo->addItem("Performance (meilleur)", "performance_desc");
    m_sortCombo->setMinimumWidth(200);
    m_sortCombo->setStyleSheet(m_filterCombo->styleSheet());
    
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchInput);
    searchLayout->addWidget(m_filterCombo);
    searchLayout->addWidget(m_sortCombo);
    searchLayout->addStretch();
    
    // Action buttons section
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_addButton = new QPushButton("+ Ajouter un employé", this);
    m_addButton->setObjectName("primary_button");
    m_addButton->setMinimumHeight(40);
    m_addButton->setCursor(Qt::PointingHandCursor);
    m_addButton->setStyleSheet(
        "QPushButton#primary_button {"
        "   background-color: #8A9A5B;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 0 24px;"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "}"
        "QPushButton#primary_button:hover {"
        "   background-color: #7a8a4b;"
        "}"
        "QPushButton#primary_button:pressed {"
        "   background-color: #6a7a3b;"
        "}"
    );
    
    m_editButton = new QPushButton("Modifier", this);
    m_editButton->setMinimumHeight(40);
    m_editButton->setCursor(Qt::PointingHandCursor);
    m_editButton->setEnabled(false);
    m_editButton->setStyleSheet(
        "QPushButton {"
        "   background-color: white;"
        "   color: #4D362D;"
        "   border: 1px solid #BDB5AD;"
        "   border-radius: 6px;"
        "   padding: 0 20px;"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover:enabled {"
        "   background-color: #F3EFE0;"
        "   border-color: #8A9A5B;"
        "}"
        "QPushButton:disabled {"
        "   opacity: 0.5;"
        "}"
    );
    
    m_deleteButton = new QPushButton("Supprimer", this);
    m_deleteButton->setMinimumHeight(40);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setEnabled(false);
    m_deleteButton->setStyleSheet(
        "QPushButton {"
        "   background-color: white;"
        "   color: #C29B6D;"
        "   border: 1px solid #BDB5AD;"
        "   border-radius: 6px;"
        "   padding: 0 20px;"
        "   font-weight: 500;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover:enabled {"
        "   background-color: #C29B6D;"
        "   color: white;"
        "   border-color: #C29B6D;"
        "}"
        "QPushButton:disabled {"
        "   opacity: 0.5;"
        "}"
    );
    
    m_exportButton = new QPushButton("Exporter PDF", this);
    m_exportButton->setMinimumHeight(40);
    m_exportButton->setCursor(Qt::PointingHandCursor);
    m_exportButton->setStyleSheet(m_editButton->styleSheet());
    
    m_refreshButton = new QPushButton("Actualiser", this);
    m_refreshButton->setMinimumHeight(40);
    m_refreshButton->setCursor(Qt::PointingHandCursor);
    m_refreshButton->setStyleSheet(m_editButton->styleSheet());
    
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(searchLayout);
    mainLayout->addLayout(buttonLayout);
}

void EmployeeManagementPage::createTable()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    
    m_table = new QTableWidget(this);
    m_table->setColumnCount(11);
    m_table->setHorizontalHeaderLabels({
        "ID", "CIN", "Nom complet", "Poste", "Email", 
        "Téléphone", "Salaire", "Performance", "Disponibilité", 
        "Heures", "Compétences"
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
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setStyleSheet(
        "QTableWidget {"
        "   background-color: white;"
        "   border: 1px solid #BDB5AD;"
        "   border-radius: 8px;"
        "   gridline-color: #F3EFE0;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px;"
        "   border-bottom: 1px solid #F3EFE0;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #E8F0E3;"
        "   color: #4D362D;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F3EFE0;"
        "   color: #4D362D;"
        "   padding: 10px;"
        "   border: none;"
        "   border-right: 1px solid #BDB5AD;"
        "   font-weight: 600;"
        "   font-size: 12px;"
        "}"
    );
    
    mainLayout->addWidget(m_table);
}

void EmployeeManagementPage::setupConnections()
{
    connect(m_addButton, &QPushButton::clicked, this, &EmployeeManagementPage::onAddEmployee);
    connect(m_editButton, &QPushButton::clicked, this, &EmployeeManagementPage::onEditEmployee);
    connect(m_deleteButton, &QPushButton::clicked, this, &EmployeeManagementPage::onDeleteEmployee);
    connect(m_exportButton, &QPushButton::clicked, this, &EmployeeManagementPage::onExportPDF);
    connect(m_refreshButton, &QPushButton::clicked, this, &EmployeeManagementPage::onRefreshTable);
    
    connect(m_searchInput, &QLineEdit::textChanged, this, &EmployeeManagementPage::onSearchTextChanged);
    connect(m_filterCombo, &QComboBox::currentTextChanged, this, &EmployeeManagementPage::onFilterChanged);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EmployeeManagementPage::onSortChanged);
    
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &EmployeeManagementPage::onTableSelectionChanged);
    connect(m_table, &QTableWidget::doubleClicked, this, &EmployeeManagementPage::onEditEmployee);
}

void EmployeeManagementPage::loadEmployees()
{
    QList<Employee> employees = EmployeeDatabase::instance().getAllEmployees();
    loadEmployees(employees);
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
    
    // Poste with badge
    QTableWidgetItem *posteItem = new QTableWidgetItem(employee.getPoste());
    posteItem->setBackground(QColor(getPosteBadgeColor(employee.getPoste())));
    m_table->setItem(row, 3, posteItem);
    
    m_table->setItem(row, 4, new QTableWidgetItem(employee.getEmail()));
    m_table->setItem(row, 5, new QTableWidgetItem(employee.getTelephone()));
    m_table->setItem(row, 6, new QTableWidgetItem(QString::number(employee.getSalaire(), 'f', 2) + " TND"));
    m_table->setItem(row, 7, new QTableWidgetItem(QString::number(employee.getPerformance(), 'f', 1) + "/10"));
    
    // Disponibilité with badge
    QTableWidgetItem *dispItem = new QTableWidgetItem(employee.getDisponibilite());
    dispItem->setBackground(QColor(getDisponibiliteBadgeColor(employee.getDisponibilite())));
    m_table->setItem(row, 8, dispItem);
    
    m_table->setItem(row, 9, new QTableWidgetItem(QString::number(employee.getHeuresTravail(), 'f', 0) + "h"));
    m_table->setItem(row, 10, new QTableWidgetItem(employee.getCompetencesString()));
    
    m_table->setRowHeight(row, 50);
}

Employee EmployeeManagementPage::getSelectedEmployee() const
{
    int row = m_table->currentRow();
    if (row < 0) {
        return Employee();
    }
    
    QString id = m_table->item(row, 0)->text();
    return EmployeeDatabase::instance().getEmployee(id);
}

void EmployeeManagementPage::updateButtonStates()
{
    bool hasSelection = m_table->currentRow() >= 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

QString EmployeeManagementPage::getPosteBadgeColor(const QString& poste) const
{
    if (poste == "Chef d'équipe") return "#E8F0E3";
    if (poste == "Menuisier") return "#F3EFE0";
    if (poste == "Apprenti") return "#FFF3E0";
    return "#F3EFE0";
}

QString EmployeeManagementPage::getDisponibiliteBadgeColor(const QString& disponibilite) const
{
    if (disponibilite == "Disponible") return "#E8F0E3";
    if (disponibilite == "En congé") return "#FFE0E0";
    if (disponibilite == "En formation") return "#E0F0FF";
    return "#F3EFE0";
}

void EmployeeManagementPage::onAddEmployee()
{
    EmployeeDialog dialog(this);
    dialog.setWindowTitle("Ajouter un employé");
    
    if (dialog.exec() == QDialog::Accepted) {
        Employee employee = dialog.getEmployee();
        employee.setId(EmployeeDatabase::instance().generateNextId());
        
        if (EmployeeDatabase::instance().addEmployee(employee)) {
            loadEmployees();
            QMessageBox::information(this, "Succès", "Employé ajouté avec succès!");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible d'ajouter l'employé.");
        }
    }
}

void EmployeeManagementPage::onEditEmployee()
{
    Employee employee = getSelectedEmployee();
    if (!employee.isValid()) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un employé à modifier.");
        return;
    }
    
    EmployeeDialog dialog(this);
    dialog.setWindowTitle("Modifier l'employé");
    dialog.setEmployee(employee);
    
    if (dialog.exec() == QDialog::Accepted) {
        Employee updated = dialog.getEmployee();
        updated.setId(employee.getId()); // Keep the same ID
        
        if (EmployeeDatabase::instance().updateEmployee(updated)) {
            loadEmployees();
            QMessageBox::information(this, "Succès", "Employé modifié avec succès!");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible de modifier l'employé.");
        }
    }
}

void EmployeeManagementPage::onDeleteEmployee()
{
    Employee employee = getSelectedEmployee();
    if (!employee.isValid()) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner un employé à supprimer.");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmer la suppression",
        QString("Êtes-vous sûr de vouloir supprimer %1 ?").arg(employee.getFullName()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (EmployeeDatabase::instance().deleteEmployee(employee.getId())) {
            loadEmployees();
            QMessageBox::information(this, "Succès", "Employé supprimé avec succès!");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible de supprimer l'employé.");
        }
    }
}

void EmployeeManagementPage::onSearchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        onRefreshTable();
        return;
    }
    
    QList<Employee> results = EmployeeDatabase::instance().searchByName(text);
    
    // Also search by CIN
    QList<Employee> cinResults = EmployeeDatabase::instance().searchByCin(text);
    for (const Employee& emp : cinResults) {
        if (!results.contains(emp)) {
            results.append(emp);
        }
    }
    
    loadEmployees(results);
}

void EmployeeManagementPage::onFilterChanged(const QString& filter)
{
    QString filterData = m_filterCombo->currentData().toString();
    
    if (filterData == "all") {
        loadEmployees();
    } else {
        QList<Employee> filtered = EmployeeDatabase::instance().searchByPoste(filterData);
        loadEmployees(filtered);
    }
}

void EmployeeManagementPage::onSortChanged(int index)
{
    QString sortType = m_sortCombo->itemData(index).toString();
    QList<Employee> sorted;
    
    if (sortType == "salary_asc") {
        sorted = EmployeeDatabase::instance().sortBySalaire(true);
    } else if (sortType == "salary_desc") {
        sorted = EmployeeDatabase::instance().sortBySalaire(false);
    } else if (sortType == "date_asc") {
        sorted = EmployeeDatabase::instance().sortByDateEmbauche(true);
    } else if (sortType == "date_desc") {
        sorted = EmployeeDatabase::instance().sortByDateEmbauche(false);
    } else if (sortType == "performance_desc") {
        sorted = EmployeeDatabase::instance().sortByPerformance(false);
    } else {
        sorted = EmployeeDatabase::instance().getAllEmployees();
    }
    
    loadEmployees(sorted);
}

void EmployeeManagementPage::onExportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", 
                                                     "liste_employees.pdf", 
                                                     "PDF Files (*.pdf)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QMessageBox::information(this, "Export", "Fonctionnalité d'export PDF à implémenter");
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
