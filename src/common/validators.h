#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <QString>
#include <QLineEdit>
#include <QLabel>

class Validators {
public:
    // ── Runtime validation (called on submit) ────────────────────────────────
    static bool validateCin(const QString& cin, QString& errorMsg);
    static bool validatePassword(const QString& password, QString& errorMsg);
    static bool validateEmail(const QString& email, QString& errorMsg);
    static bool validateName(const QString& name, const QString& fieldName, QString& errorMsg);
    static bool validatePhone(const QString& phone, QString& errorMsg);

    // ── Live input setup (called once after widget creation) ─────────────────
    // Installs a validator + connects textChanged to show inline error label
    static void setupCinInput(QLineEdit* input, QLabel* errorLabel = nullptr);
    static void setupEmailInput(QLineEdit* input, QLabel* errorLabel = nullptr);
    static void setupNameInput(QLineEdit* input, QLabel* errorLabel = nullptr);
    static void setupPhoneInput(QLineEdit* input, QLabel* errorLabel = nullptr);
    static void setupPasswordInput(QLineEdit* input, QLabel* errorLabel = nullptr);

    // ── Legacy ───────────────────────────────────────────────────────────────
    static bool validateUsername(const QString& username, QString& errorMsg);
};

#endif
