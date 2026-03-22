#include "loginpage.h"
#include "ui_loginpage.h"
#include "../../database/employeedatabase.h"
#include <QMessageBox>
#include <QPixmap>
#include "../../common/validators.h"

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginPage)
{
    ui->setupUi(this);

    QPixmap logo("src/assets/icons/logo1.png");
    if (!logo.isNull())
        ui->logoLabel->setPixmap(logo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // CIN: digits only, max 8, live validation
    Validators::setupCinInput(ui->usernameInput);

    connect(ui->loginButton,  &QPushButton::clicked, this, &LoginPage::onLoginClicked);
    connect(ui->registerLink, &QPushButton::clicked, this, &LoginPage::switchToForgotPassword);

    connect(ui->usernameInput, &QLineEdit::returnPressed, [this]() { ui->passwordInput->setFocus(); });
    connect(ui->passwordInput, &QLineEdit::returnPressed, this, &LoginPage::onLoginClicked);
}

LoginPage::~LoginPage() { delete ui; }

void LoginPage::onLoginClicked()
{
    QString cin      = ui->usernameInput->text().trimmed();
    QString password = ui->passwordInput->text();

    QString cinErr, pwdErr;
    if (!Validators::validateCin(cin, cinErr)) {
        QMessageBox::warning(this, "Échec de connexion", cinErr); return;
    }
    if (!Validators::validatePassword(password, pwdErr)) {
        QMessageBox::warning(this, "Échec de connexion", pwdErr); return;
    }

    Employee employee = EmployeeDatabase::instance().authenticate(cin, password);

    if (!employee.isValid()) {
        QMessageBox::warning(this, "Échec de connexion",
            "CIN ou mot de passe incorrect.\n"
            "Vérifiez vos identifiants ou utilisez 'Mot de passe oublié'.");
        return;
    }

    emit loginSuccess(employee);
}
