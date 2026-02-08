#include "loginpage.h"
#include "ui_loginpage.h"
#include "../../common/validators.h"
#include "../../database/employeedatabase.h"
#include <QMessageBox>
#include <QPixmap>

LoginPage::LoginPage(QWidget *parent) 
    : QWidget(parent), ui(new Ui::LoginPage) {
    ui->setupUi(this);
    
    // Load and set logo
    QPixmap logo("src/assets/icons/logo1.png");
    if (!logo.isNull()) {
        ui->logoLabel->setPixmap(logo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginPage::onLoginClicked);
    connect(ui->registerLink, &QPushButton::clicked, this, &LoginPage::switchToRegister);
    
    // Enter key navigation
    connect(ui->usernameInput, &QLineEdit::returnPressed, [this]() {
        ui->passwordInput->setFocus();
    });
    connect(ui->passwordInput, &QLineEdit::returnPressed, this, &LoginPage::onLoginClicked);
}

LoginPage::~LoginPage() {
    delete ui;
}

void LoginPage::onLoginClicked() {
    QString cin = ui->usernameInput->text().trimmed();
    QString password = ui->passwordInput->text();
    
    QString error;
    
    // Validate CIN
    if (cin.isEmpty()) {
        QMessageBox::warning(this, "Échec de connexion", "Le CIN est requis");
        return;
    }
    
    if (cin.length() != 8 || !cin.toLongLong()) {
        QMessageBox::warning(this, "Échec de connexion", "Le CIN doit contenir exactement 8 chiffres");
        return;
    }
    
    // Validate password
    if (!Validators::validatePassword(password, error)) {
        QMessageBox::warning(this, "Échec de connexion", error);
        return;
    }
    
    // Search for employee by CIN in database
    QList<Employee> employees = EmployeeDatabase::instance().searchByCin(cin);
    
    if (employees.isEmpty()) {
        QMessageBox::warning(this, "Échec de connexion", 
            "Aucun employé trouvé avec ce CIN.\n\nVeuillez vérifier votre CIN ou créer un nouveau compte.");
        return;
    }
    
    // For demo purposes, accept any password
    // TODO: In production, verify password against stored hash
    Employee employee = employees.first();
    
    QMessageBox::information(this, "Connexion réussie", 
        QString("Bienvenue, %1 %2!\n\nPoste: %3")
            .arg(employee.getPrenom())
            .arg(employee.getNom())
            .arg(employee.getPoste()));
    
    emit loginSuccess(employee);
}
