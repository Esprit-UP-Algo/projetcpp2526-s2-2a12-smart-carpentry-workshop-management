#include "loginpage.h"
#include "ui_loginpage.h"
#include "../../database/employeedatabase.h"
#include <QMessageBox>
#include <QPixmap>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginPage)
{
    ui->setupUi(this);

    QPixmap logo("src/assets/icons/logo1.png");
    if (!logo.isNull())
        ui->logoLabel->setPixmap(logo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    connect(ui->loginButton,  &QPushButton::clicked, this, &LoginPage::onLoginClicked);
    connect(ui->registerLink, &QPushButton::clicked, this, &LoginPage::switchToRegister);

    connect(ui->usernameInput, &QLineEdit::returnPressed, [this]() { ui->passwordInput->setFocus(); });
    connect(ui->passwordInput, &QLineEdit::returnPressed, this, &LoginPage::onLoginClicked);
}

LoginPage::~LoginPage() { delete ui; }

void LoginPage::onLoginClicked()
{
    QString cin      = ui->usernameInput->text().trimmed();
    QString password = ui->passwordInput->text();

    // Basic validation
    if (cin.isEmpty()) {
        QMessageBox::warning(this, "Échec de connexion", "Le CIN est requis.");
        return;
    }
    if (cin.length() != 8 || !cin.toLongLong()) {
        QMessageBox::warning(this, "Échec de connexion", "Le CIN doit contenir exactement 8 chiffres.");
        return;
    }
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Échec de connexion", "Le mot de passe est requis.");
        return;
    }

    // Authenticate against DB (CIN + MOT_DE_PASSE, plain text)
    Employee employee = EmployeeDatabase::instance().authenticate(cin, password);

    if (!employee.isValid()) {
        QMessageBox::warning(this, "Échec de connexion",
            "CIN ou mot de passe incorrect.\n"
            "Vérifiez vos identifiants ou créez un compte.");
        return;
    }

    emit loginSuccess(employee);
}
