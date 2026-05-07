#ifndef SESSION_H
#define SESSION_H

#include <QString>
#include "../models/employee.h"

class Session {
public:
    static Employee* getCurrentEmployee();
    static bool isAuthenticated();
    static void setCurrentEmployee(const Employee& employee);
    static void setAuthToken(const QString& token);
    static QString getAuthToken();
    static void logout();
    
private:
    static Employee* s_currentEmployee;
    static QString s_authToken;
};

#endif
