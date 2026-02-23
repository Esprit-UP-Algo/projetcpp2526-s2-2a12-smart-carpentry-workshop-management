#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include "../../models/employee.h"

namespace Ui { class LoginPage; }

class LoginPage : public QWidget {
    Q_OBJECT
    
public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();
    
signals:
    void loginSuccess(const Employee& employee);
    void switchToRegister();
    
private slots:
    void onLoginClicked();
    
private:
    Ui::LoginPage *ui;
};

#endif
