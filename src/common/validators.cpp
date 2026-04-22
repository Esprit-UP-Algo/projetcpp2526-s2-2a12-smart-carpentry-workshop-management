#include "validators.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QObject>

// ============================================================================
//  Runtime validation
// ============================================================================

bool Validators::validateCin(const QString& cin, QString& errorMsg)
{
    if (cin.isEmpty()) { errorMsg = "Le CIN est obligatoire."; return false; }
    if (cin.length() != 8) { errorMsg = "Le CIN doit contenir exactement 8 chiffres."; return false; }
    for (QChar c : cin) {
        if (!c.isDigit()) { errorMsg = "Le CIN ne doit contenir que des chiffres."; return false; }
    }
    return true;
}

bool Validators::validatePassword(const QString& password, QString& errorMsg)
{
    if (password.isEmpty()) { errorMsg = "Le mot de passe est obligatoire."; return false; }
    if (password.length() < 6) { errorMsg = "Le mot de passe doit contenir au moins 6 caractères."; return false; }
    return true;
}

bool Validators::validateEmail(const QString& email, QString& errorMsg)
{
    if (email.isEmpty()) { errorMsg = "L'email est obligatoire."; return false; }
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)");
    if (!regex.match(email).hasMatch()) { errorMsg = "Format d'email invalide (ex: prenom.nom@woodflow.tn)."; return false; }
    return true;
}

bool Validators::validateName(const QString& name, const QString& fieldName, QString& errorMsg)
{
    if (name.isEmpty()) { errorMsg = fieldName + " est obligatoire."; return false; }
    if (name.length() < 2) { errorMsg = fieldName + " doit contenir au moins 2 caractères."; return false; }
    QRegularExpression regex(R"(^[a-zA-ZÀ-ÿ\s\-']+$)");
    if (!regex.match(name).hasMatch()) { errorMsg = fieldName + " ne doit contenir que des lettres."; return false; }
    return true;
}

bool Validators::validatePhone(const QString& phone, QString& errorMsg)
{
    if (phone.isEmpty()) return true;  // optional field
    QString digits = phone;
    digits.remove(QRegularExpression(R"([\s\-\+\(\)])"));
    if (digits.length() < 8 || digits.length() > 15) { errorMsg = "Numéro de téléphone invalide."; return false; }
    for (QChar c : digits) {
        if (!c.isDigit()) { errorMsg = "Le téléphone ne doit contenir que des chiffres."; return false; }
    }
    return true;
}

bool Validators::validateUsername(const QString& username, QString& errorMsg)
{
    return validateCin(username, errorMsg);
}

// ============================================================================
//  Live input setup — blocks bad chars while typing + shows inline errors
// ============================================================================

static QLabel* makeErrorLabel(QLineEdit* input)
{
    // If no label provided, create one and insert it after the input
    // (only works if input has a parent with a QVBoxLayout — safe fallback)
    Q_UNUSED(input);
    return nullptr;
}

static void attachLiveError(QLineEdit* input, QLabel* errorLabel,
                             std::function<bool(const QString&, QString&)> validator)
{
    if (!errorLabel) return;
    errorLabel->setStyleSheet("color: #ef4444; font-size: 11px; background: transparent;");
    errorLabel->hide();

    QObject::connect(input, &QLineEdit::textChanged, input, [input, errorLabel, validator](const QString& text) {
        Q_UNUSED(input);
        if (text.isEmpty()) { errorLabel->hide(); return; }
        QString err;
        if (!validator(text, err)) {
            errorLabel->setText(err);
            errorLabel->show();
        } else {
            errorLabel->hide();
        }
    });
}

void Validators::setupCinInput(QLineEdit* input, QLabel* errorLabel)
{
    // Block non-digits at the keyboard level
    input->setValidator(new QRegularExpressionValidator(QRegularExpression(R"(\d{0,8})"), input));
    input->setMaxLength(8);
    input->setPlaceholderText("8 chiffres");

    attachLiveError(input, errorLabel, [](const QString& v, QString& e) {
        if (v.length() > 0 && v.length() < 8) { e = "Le CIN doit contenir 8 chiffres."; return false; }
        return true;
    });
}

void Validators::setupEmailInput(QLineEdit* input, QLabel* errorLabel)
{
    // Allow valid email chars only while typing
    input->setValidator(new QRegularExpressionValidator(
        QRegularExpression(R"([a-zA-Z0-9._%+\-@]*)"), input));

    attachLiveError(input, errorLabel, [](const QString& v, QString& e) {
        if (v.contains('@')) return validateEmail(v, e);
        return true;  // don't show error until @ is typed
    });
}

void Validators::setupNameInput(QLineEdit* input, QLabel* errorLabel)
{
    // Letters, spaces, hyphens, apostrophes only
    input->setValidator(new QRegularExpressionValidator(
        QRegularExpression(R"([a-zA-ZÀ-ÿ\s\-']*)"), input));

    attachLiveError(input, errorLabel, [](const QString& v, QString& e) {
        if (v.length() >= 2) return true;
        if (v.length() == 1) { e = "Minimum 2 caractères."; return false; }
        return true;
    });
}

void Validators::setupPhoneInput(QLineEdit* input, QLabel* errorLabel)
{
    // Digits, spaces, +, -, (, ) only
    input->setValidator(new QRegularExpressionValidator(
        QRegularExpression(R"([\d\s\+\-\(\)]{0,15})"), input));

    attachLiveError(input, errorLabel, validatePhone);
}

void Validators::setupPasswordInput(QLineEdit* input, QLabel* errorLabel)
{
    // No validator — passwords can contain anything
    attachLiveError(input, errorLabel, [](const QString& v, QString& e) {
        if (v.length() > 0 && v.length() < 6) { e = "Minimum 6 caractères."; return false; }
        return true;
    });
}
