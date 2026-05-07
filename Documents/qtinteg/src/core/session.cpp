#include "session.h"

Employee* Session::s_currentEmployee = nullptr;
QString Session::s_authToken;

Employee* Session::getCurrentEmployee() {
    return s_currentEmployee;
}

bool Session::isAuthenticated() {
    return s_currentEmployee != nullptr && s_currentEmployee->isValid();
}

void Session::setCurrentEmployee(const Employee& employee) {
    if (s_currentEmployee) {
        delete s_currentEmployee;
    }
    s_currentEmployee = new Employee(employee);
}

void Session::setAuthToken(const QString& token) {
    s_authToken = token;
}

QString Session::getAuthToken() {
    return s_authToken;
}

void Session::logout() {
    if (s_currentEmployee) {
        delete s_currentEmployee;
        s_currentEmployee = nullptr;
    }
    s_authToken.clear();
}
