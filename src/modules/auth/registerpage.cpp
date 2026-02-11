#include "registerpage.h"
#include "ui_registerpage.h"
#include "../../common/validators.h"
#include "../../database/employeedatabase.h"
#include <QMessageBox>
#include <QPixmap>
#include <QDateTime>

RegisterPage::RegisterPage(QWidget *parent) 
    : QWidget(parent), ui(new Ui::RegisterPage) {
    ui->setupUi(this);
    
    // Load and set logo
    QPixmap logo("src/assets/icons/logo1.png");
    if (!logo.isNull()) {
        ui->logoLabel->setPixmap(logo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    
    connect(ui->registerButton, &QPushButton::clicked, this, &RegisterPage::onRegisterClicked);
    connect(ui->loginLink, &QPushButton::clicked, this, &RegisterPage::switchToLogin);
    
    // Enter key navigation
    connect(ui->usernameInput, &QLineEdit::returnPressed, [this]() {
        ui->emailInput->setFocus();
    });
    connect(ui->emailInput, &QLineEdit::returnPressed, [this]() {
        ui->passwordInput->setFocus();
    });
    connect(ui->passwordInput, &QLineEdit::returnPressed, [this]() {
        ui->confirmInput->setFocus();
    });
    connect(ui->confirmInput, &QLineEdit::returnPressed, this, &RegisterPage::onRegisterClicked);
}

RegisterPage::~RegisterPage() {
    delete ui;
}

void RegisterPage::onRegisterClicked() {
    QString cin = ui->usernameInput->text().trimmed();
    QString email = ui->emailInput->text().trimmed();
    QString password = ui->passwordInput->text();
    QString confirm = ui->confirmInput->text();
    
    QString error;
    
    // Validate CIN (use username field for CIN)
    if (cin.isEmpty()) {
        QMessageBox::warning(this, "Échec d'inscription", "Le CIN est requis");
        return;
    }
    
    if (cin.length() != 8) {
        QMessageBox::warning(this, "Échec d'inscription", 
            "Le CIN doit contenir exactement 8 chiffres");
        return;
    }
    
    bool isNumber;
    cin.toLongLong(&isNumber);
    if (!isNumber) {
        QMessageBox::warning(this, "Échec d'inscription", 
            "Le CIN ne doit contenir que des chiffres");
        return;
    }
    
    // Validate email
    if (!Validators::validateEmail(email, error)) {
        QMessageBox::warning(this, "Échec d'inscription", error);
        return;
    }
    
    // Validate password
    if (!Validators::validatePassword(password, error)) {
        QMessageBox::warning(this, "Échec d'inscription", error);
        return;
    }
    
    if (password != confirm) {
        QMessageBox::warning(this, "Échec d'inscription", 
            "Les mots de passe ne correspondent pas");
        return;
    }
    
    // Check if CIN already exists
    QList<Employee> existingEmployees = EmployeeDatabase::instance().searchByCin(cin);
    if (!existingEmployees.isEmpty()) {
        QMessageBox::warning(this, "Échec d'inscription", 
            "Un employé avec ce CIN existe déjà.\n\nVeuillez vous connecter ou utiliser un autre CIN.");
        return;
    }
    
    // Create new employee with basic information
    // Note: Additional employee details can be filled in after first login
    Employee newEmployee;
    newEmployee.setId(EmployeeDatabase::instance().generateNextId());
    newEmployee.setCin(cin);
    newEmployee.setEmail(email);
    
    // Set default values - user will complete profile later
    newEmployee.setNom("À compléter");
    newEmployee.setPrenom("Nouvel employé");
    newEmployee.setPoste("Apprenti");  // Default role for new employees
    newEmployee.setDateEmbauche(QDateTime::currentDateTime());
    newEmployee.setSalaire(1200.0);  // Default minimum salary
    newEmployee.setDisponibilite("Disponible");
    newEmployee.setPerformance(7.0);
    newEmployee.setNbJoursConges(0);
    newEmployee.setNbJoursAbsence(0);
    newEmployee.setHeuresTravail(0.0);
    
    // Add to database
    if (EmployeeDatabase::instance().addEmployee(newEmployee)) {
        QMessageBox::information(this, "Inscription réussie", 
            "Votre compte a été créé avec succès!\n\n"
            "CIN: " + cin + "\n\n"
            "Vous pouvez maintenant vous connecter.\n"
            "N'oubliez pas de compléter votre profil après la connexion.");
        emit switchToLogin();
    } else {
        QMessageBox::critical(this, "Erreur", 
            "Une erreur s'est produite lors de la création du compte.\n\n"
            "Veuillez réessayer ou contacter l'administrateur.");
    }
}
