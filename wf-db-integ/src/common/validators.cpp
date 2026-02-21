#include "validators.h"
#include <QRegularExpression>

bool Validators::validateUsername(const QString& username, QString& errorMsg) {
    if (username.isEmpty()) {
        errorMsg = "Le nom d'utilisateur ne peut pas être vide";
        return false;
    }
    if (username.length() > 20) {
        errorMsg = "Le nom d'utilisateur doit contenir moins de 20 caractères";
        return false;
    }
    if (!username[0].isLetter()) {
        errorMsg = "Le nom d'utilisateur doit commencer par une lettre";
        return false;
    }
    return true;
}

bool Validators::validatePassword(const QString& password, QString& errorMsg) {
    if (password.length() < 4) {
        errorMsg = "Le mot de passe doit contenir au moins 4 caractères";
        return false;
    }
    return true;
}

bool Validators::validateEmail(const QString& email, QString& errorMsg) {
    QRegularExpression regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!regex.match(email).hasMatch()) {
        errorMsg = "Format d'email invalide";
        return false;
    }
    return true;
}
