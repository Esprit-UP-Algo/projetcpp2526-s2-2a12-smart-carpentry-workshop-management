#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <QString>

class Validators {
public:
    static bool validateUsername(const QString& username, QString& errorMsg);
    static bool validatePassword(const QString& password, QString& errorMsg);
    static bool validateEmail(const QString& email, QString& errorMsg);
};

#endif
