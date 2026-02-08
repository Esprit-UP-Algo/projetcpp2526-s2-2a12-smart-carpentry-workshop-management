#include "employeedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDate>
#include <QTime>

EmployeeDialog::EmployeeDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    
    setMinimumWidth(650);
    setModal(true);
}

void EmployeeDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    
    // Informations personnelles
    QGroupBox *personalGroup = new QGroupBox("Informations personnelles", this);
    QFormLayout *personalForm = new QFormLayout(personalGroup);
    personalForm->setSpacing(12);
    
    m_cinInput = new QLineEdit(this);
    m_cinInput->setPlaceholderText("Ex: 12345678");
    personalForm->addRow("CIN *:", m_cinInput);
    
    m_nomInput = new QLineEdit(this);
    m_nomInput->setPlaceholderText("Nom de famille");
    personalForm->addRow("Nom *:", m_nomInput);
    
    m_prenomInput = new QLineEdit(this);
    m_prenomInput->setPlaceholderText("Prénom");
    personalForm->addRow("Prénom *:", m_prenomInput);
    
    m_posteCombo = new QComboBox(this);
    m_posteCombo->addItems({"Menuisier", "Chef d'équipe", "Apprenti"});
    personalForm->addRow("Poste *:", m_posteCombo);
    
    // Contact
    QGroupBox *contactGroup = new QGroupBox("Coordonnées", this);
    QFormLayout *contactForm = new QFormLayout(contactGroup);
    contactForm->setSpacing(12);
    
    m_emailInput = new QLineEdit(this);
    m_emailInput->setPlaceholderText("exemple@woodflow.tn");
    contactForm->addRow("Email:", m_emailInput);
    
    m_telephoneInput = new QLineEdit(this);
    m_telephoneInput->setPlaceholderText("+216 XX XXX XXX");
    contactForm->addRow("Téléphone:", m_telephoneInput);
    
    // Informations professionnelles
    QGroupBox *professionalGroup = new QGroupBox("Informations professionnelles", this);
    QFormLayout *professionalForm = new QFormLayout(professionalGroup);
    professionalForm->setSpacing(12);
    
    m_dateEmbaucheInput = new QDateEdit(this);
    m_dateEmbaucheInput->setDate(QDate::currentDate());
    m_dateEmbaucheInput->setCalendarPopup(true);
    m_dateEmbaucheInput->setDisplayFormat("dd/MM/yyyy");
    professionalForm->addRow("Date d'embauche:", m_dateEmbaucheInput);
    
    m_salaireInput = new QDoubleSpinBox(this);
    m_salaireInput->setRange(0, 999999);
    m_salaireInput->setDecimals(2);
    m_salaireInput->setSuffix(" TND");
    m_salaireInput->setValue(1500.0);
    professionalForm->addRow("Salaire:", m_salaireInput);
    
    m_competencesInput = new QTextEdit(this);
    m_competencesInput->setPlaceholderText("Ex: Ébénisterie, Pose, Finition (séparés par des virgules)");
    m_competencesInput->setMaximumHeight(80);
    professionalForm->addRow("Compétences:", m_competencesInput);
    
    m_disponibiliteCombo = new QComboBox(this);
    m_disponibiliteCombo->addItems({"Disponible", "En congé", "En formation"});
    professionalForm->addRow("Disponibilité:", m_disponibiliteCombo);
    
    // Performance et suivi
    QGroupBox *trackingGroup = new QGroupBox("Performance et suivi", this);
    QFormLayout *trackingForm = new QFormLayout(trackingGroup);
    trackingForm->setSpacing(12);
    
    m_performanceInput = new QDoubleSpinBox(this);
    m_performanceInput->setRange(0, 10);
    m_performanceInput->setDecimals(1);
    m_performanceInput->setSingleStep(0.1);
    m_performanceInput->setSuffix(" /10");
    m_performanceInput->setValue(7.0);
    trackingForm->addRow("Performance:", m_performanceInput);
    
    m_heuresTravailInput = new QDoubleSpinBox(this);
    m_heuresTravailInput->setRange(0, 500);
    m_heuresTravailInput->setDecimals(1);
    m_heuresTravailInput->setSuffix(" h");
    m_heuresTravailInput->setValue(160.0);
    trackingForm->addRow("Heures de travail:", m_heuresTravailInput);
    
    m_nbJoursCongesInput = new QSpinBox(this);
    m_nbJoursCongesInput->setRange(0, 365);
    m_nbJoursCongesInput->setSuffix(" jours");
    trackingForm->addRow("Jours de congés:", m_nbJoursCongesInput);
    
    m_nbJoursAbsenceInput = new QSpinBox(this);
    m_nbJoursAbsenceInput->setRange(0, 365);
    m_nbJoursAbsenceInput->setSuffix(" jours");
    trackingForm->addRow("Jours d'absence:", m_nbJoursAbsenceInput);
    
    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Enregistrer");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Annuler");
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &EmployeeDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EmployeeDialog::onRejected);
    
    // Add all groups to main layout
    mainLayout->addWidget(personalGroup);
    mainLayout->addWidget(contactGroup);
    mainLayout->addWidget(professionalGroup);
    mainLayout->addWidget(trackingGroup);
    mainLayout->addWidget(buttonBox);
    
    // Apply stylesheet
    setStyleSheet(
        "QGroupBox {"
        "   font-weight: 600;"
        "   border: 1px solid #BDB5AD;"
        "   border-radius: 8px;"
        "   margin-top: 12px;"
        "   padding-top: 12px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   padding: 0 8px;"
        "   color: #4D362D;"
        "}"
        "QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox, QTextEdit {"
        "   padding: 8px;"
        "   border: 1px solid #BDB5AD;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTextEdit:focus {"
        "   border-color: #8A9A5B;"
        "}"
    );
}

Employee EmployeeDialog::getEmployee() const
{
    Employee employee;
    
    employee.setCin(m_cinInput->text().trimmed());
    employee.setNom(m_nomInput->text().trimmed());
    employee.setPrenom(m_prenomInput->text().trimmed());
    employee.setPoste(m_posteCombo->currentText());
    employee.setEmail(m_emailInput->text().trimmed());
    employee.setTelephone(m_telephoneInput->text().trimmed());
    
    // Fix: Create QDateTime from QDate with QTime
    QDate date = m_dateEmbaucheInput->date();
    QDateTime dateTime(date, QTime(0, 0, 0));
    employee.setDateEmbauche(dateTime);
    
    employee.setSalaire(m_salaireInput->value());
    
    // Parse competences from text
    QString competencesText = m_competencesInput->toPlainText();
    QStringList competences = competencesText.split(',', Qt::SkipEmptyParts);
    for (QString& comp : competences) {
        comp = comp.trimmed();
    }
    employee.setCompetences(competences);
    
    employee.setDisponibilite(m_disponibiliteCombo->currentText());
    employee.setPerformance(m_performanceInput->value());
    employee.setNbJoursConges(m_nbJoursCongesInput->value());
    employee.setNbJoursAbsence(m_nbJoursAbsenceInput->value());
    employee.setHeuresTravail(m_heuresTravailInput->value());
    
    return employee;
}

void EmployeeDialog::setEmployee(const Employee& employee)
{
    m_cinInput->setText(employee.getCin());
    m_nomInput->setText(employee.getNom());
    m_prenomInput->setText(employee.getPrenom());
    
    int posteIndex = m_posteCombo->findText(employee.getPoste());
    if (posteIndex >= 0) {
        m_posteCombo->setCurrentIndex(posteIndex);
    }
    
    m_emailInput->setText(employee.getEmail());
    m_telephoneInput->setText(employee.getTelephone());
    m_dateEmbaucheInput->setDate(employee.getDateEmbauche().date());
    m_salaireInput->setValue(employee.getSalaire());
    m_competencesInput->setPlainText(employee.getCompetencesString());
    
    int dispIndex = m_disponibiliteCombo->findText(employee.getDisponibilite());
    if (dispIndex >= 0) {
        m_disponibiliteCombo->setCurrentIndex(dispIndex);
    }
    
    m_performanceInput->setValue(employee.getPerformance());
    m_nbJoursCongesInput->setValue(employee.getNbJoursConges());
    m_nbJoursAbsenceInput->setValue(employee.getNbJoursAbsence());
    m_heuresTravailInput->setValue(employee.getHeuresTravail());
}

bool EmployeeDialog::validateInput()
{
    if (m_cinInput->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Le CIN est obligatoire.");
        m_cinInput->setFocus();
        return false;
    }
    
    if (m_nomInput->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Le nom est obligatoire.");
        m_nomInput->setFocus();
        return false;
    }
    
    if (m_prenomInput->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Le prénom est obligatoire.");
        m_prenomInput->setFocus();
        return false;
    }
    
    return true;
}

void EmployeeDialog::onAccepted()
{
    if (validateInput()) {
        accept();
    }
}

void EmployeeDialog::onRejected()
{
    reject();
}
