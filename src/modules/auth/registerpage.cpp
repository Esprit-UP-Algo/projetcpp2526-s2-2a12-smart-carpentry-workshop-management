#include "registerpage.h"
#include "ui_registerpage.h"
#include "../../database/employeedatabase.h"
#include <QMessageBox>
#include <QPixmap>
#include <QDateTime>

RegisterPage::RegisterPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::RegisterPage)
{
    ui->setupUi(this);

    // Override placeholder so field reads as CIN (the .ui label text stays "Nom d'utilisateur"
    // unless you also update the .ui file — to change the label text, edit registerpage.ui:
    //   <widget class="QLabel"><property name="text"><string>CIN</string></property></widget>
    ui->usernameInput->setPlaceholderText("Entrez votre CIN (8 chiffres)");

    QPixmap logo("src/assets/icons/logo1.png");
    if (!logo.isNull())
        ui->logoLabel->setPixmap(logo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    connect(ui->registerButton, &QPushButton::clicked, this, &RegisterPage::onRegisterClicked);
    connect(ui->loginLink,      &QPushButton::clicked, this, &RegisterPage::switchToLogin);

    connect(ui->usernameInput, &QLineEdit::returnPressed, [this]() { ui->emailInput->setFocus(); });
    connect(ui->emailInput,    &QLineEdit::returnPressed, [this]() { ui->passwordInput->setFocus(); });
    connect(ui->passwordInput, &QLineEdit::returnPressed, [this]() { ui->confirmInput->setFocus(); });
    connect(ui->confirmInput,  &QLineEdit::returnPressed, this, &RegisterPage::onRegisterClicked);
}

RegisterPage::~RegisterPage() { delete ui; }

// Parse "prenom.nom@domain.com" → ("Prenom", "Nom")
static QPair<QString,QString> parseNameFromEmail(const QString& email)
{
    QString prefix = email.section('@', 0, 0);
    prefix.replace('.', ' ');
    prefix.replace('_', ' ');
    prefix.replace('-', ' ');

    QStringList parts = prefix.split(' ', Qt::SkipEmptyParts);
    for (QString& p : parts)
        if (!p.isEmpty()) p[0] = p[0].toUpper();

    if (parts.size() >= 2)
        return { parts.first(), parts.mid(1).join(" ") };
    if (parts.size() == 1)
        return { parts[0], "À compléter" };
    return { "Nouvel", "Employé" };
}

void RegisterPage::onRegisterClicked()
{
    QString cin      = ui->usernameInput->text().trimmed();
    QString email    = ui->emailInput->text().trimmed();
    QString password = ui->passwordInput->text();
    QString confirm  = ui->confirmInput->text();

    // ── Validate CIN ──────────────────────────────────────────────────────────
    if (cin.isEmpty()) {
        QMessageBox::warning(this, "Échec d'inscription", "Le CIN est requis.");
        ui->usernameInput->setFocus(); return;
    }
    if (cin.length() != 8) {
        QMessageBox::warning(this, "Échec d'inscription", "Le CIN doit contenir exactement 8 chiffres.");
        ui->usernameInput->setFocus(); return;
    }
    bool isNumber;
    cin.toLongLong(&isNumber);
    if (!isNumber) {
        QMessageBox::warning(this, "Échec d'inscription", "Le CIN ne doit contenir que des chiffres.");
        ui->usernameInput->setFocus(); return;
    }

    // ── Validate email ────────────────────────────────────────────────────────
    if (email.isEmpty() || !email.contains('@') || !email.contains('.')) {
        QMessageBox::warning(this, "Échec d'inscription", "Veuillez entrer un email valide.");
        ui->emailInput->setFocus(); return;
    }

    // ── Validate password ─────────────────────────────────────────────────────
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Échec d'inscription", "Le mot de passe est requis.");
        ui->passwordInput->setFocus(); return;
    }
    if (password != confirm) {
        QMessageBox::warning(this, "Échec d'inscription", "Les mots de passe ne correspondent pas.");
        ui->confirmInput->setFocus(); return;
    }

    // ── Check CIN uniqueness ──────────────────────────────────────────────────
    if (!EmployeeDatabase::instance().searchByCin(cin).isEmpty()) {
        QMessageBox::warning(this, "Échec d'inscription",
            "Un employé avec ce CIN existe déjà.\nVeuillez vous connecter.");
        return;
    }

    // ── Parse name from email ─────────────────────────────────────────────────
    auto [prenom, nom] = parseNameFromEmail(email);

    // ── Create employee ───────────────────────────────────────────────────────
    Employee e;
    e.setCin(cin);
    e.setEmail(email);
    e.setMotDePasse(password);
    e.setPrenom(prenom);
    e.setNom(nom);
    e.setPoste("Apprenti");
    e.setDateEmbauche(QDateTime::currentDateTime());
    e.setSalaire(1200.0);
    e.setDisponibilite("Disponible");
    e.setPerformance(7.0);
    e.setNbJoursConges(0);
    e.setNbJoursAbsence(0);
    e.setHeuresTravail(0.0);

    if (EmployeeDatabase::instance().addEmployee(e)) {
        QMessageBox::information(this, "Inscription réussie",
            QString("Compte créé avec succès !\n\n"
                    "Nom détecté : %1 %2\n"
                    "CIN : %3\n\n"
                    "Vous pouvez maintenant vous connecter.")
                .arg(prenom, nom, cin));
        emit switchToLogin();
    } else {
        QMessageBox::critical(this, "Erreur",
            "Erreur lors de la création du compte.\nVérifiez les logs.");
    }
}
