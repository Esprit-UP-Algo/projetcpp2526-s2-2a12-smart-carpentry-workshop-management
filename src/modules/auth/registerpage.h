#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include <QWidget>
#include "../../models/employee.h"

namespace Ui { class RegisterPage; }

class RegisterPage : public QWidget {
    Q_OBJECT
    
public:
    explicit RegisterPage(QWidget *parent = nullptr);
    ~RegisterPage();
    
signals:
    void registerSuccess(const Employee& employee);
    void switchToLogin();
    
private slots:
    void onRegisterClicked();
    
private:
    Ui::RegisterPage *ui;
};

#endif
