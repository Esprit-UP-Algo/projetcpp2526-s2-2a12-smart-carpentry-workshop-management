/********************************************************************************
** Form generated from reading UI file 'loginpage.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINPAGE_H
#define UI_LOGINPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginPage
{
public:
    QVBoxLayout *vboxLayout;
    QSpacerItem *spacerItem;
    QFrame *auth_card;
    QVBoxLayout *vboxLayout1;
    QLabel *logoLabel;
    QSpacerItem *spacerItem1;
    QLabel *auth_title;
    QLabel *auth_subtitle;
    QSpacerItem *spacerItem2;
    QLabel *label;
    QLineEdit *usernameInput;
    QSpacerItem *spacerItem3;
    QLabel *label1;
    QLineEdit *passwordInput;
    QSpacerItem *spacerItem4;
    QPushButton *loginButton;
    QSpacerItem *spacerItem5;
    QPushButton *registerLink;
    QSpacerItem *spacerItem6;

    void setupUi(QWidget *LoginPage)
    {
        if (LoginPage->objectName().isEmpty())
            LoginPage->setObjectName("LoginPage");
        vboxLayout = new QVBoxLayout(LoginPage);
        vboxLayout->setObjectName("vboxLayout");
        spacerItem = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout->addItem(spacerItem);

        auth_card = new QFrame(LoginPage);
        auth_card->setObjectName("auth_card");
        auth_card->setMinimumSize(QSize(420, 0));
        auth_card->setMaximumSize(QSize(420, 16777215));
        vboxLayout1 = new QVBoxLayout(auth_card);
        vboxLayout1->setSpacing(6);
        vboxLayout1->setObjectName("vboxLayout1");
        vboxLayout1->setContentsMargins(45, 50, 45, 50);
        logoLabel = new QLabel(auth_card);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setMinimumSize(QSize(80, 80));
        logoLabel->setMaximumSize(QSize(80, 80));
        logoLabel->setAlignment(Qt::AlignCenter);
        logoLabel->setScaledContents(true);

        vboxLayout1->addWidget(logoLabel, 0, Qt::AlignCenter);

        spacerItem1 = new QSpacerItem(20, 16, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem1);

        auth_title = new QLabel(auth_card);
        auth_title->setObjectName("auth_title");
        auth_title->setAlignment(Qt::AlignCenter);

        vboxLayout1->addWidget(auth_title);

        auth_subtitle = new QLabel(auth_card);
        auth_subtitle->setObjectName("auth_subtitle");
        auth_subtitle->setAlignment(Qt::AlignCenter);

        vboxLayout1->addWidget(auth_subtitle);

        spacerItem2 = new QSpacerItem(20, 32, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem2);

        label = new QLabel(auth_card);
        label->setObjectName("label");

        vboxLayout1->addWidget(label);

        usernameInput = new QLineEdit(auth_card);
        usernameInput->setObjectName("usernameInput");

        vboxLayout1->addWidget(usernameInput);

        spacerItem3 = new QSpacerItem(20, 16, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem3);

        label1 = new QLabel(auth_card);
        label1->setObjectName("label1");

        vboxLayout1->addWidget(label1);

        passwordInput = new QLineEdit(auth_card);
        passwordInput->setObjectName("passwordInput");
        passwordInput->setEchoMode(QLineEdit::Password);

        vboxLayout1->addWidget(passwordInput);

        spacerItem4 = new QSpacerItem(20, 24, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem4);

        loginButton = new QPushButton(auth_card);
        loginButton->setObjectName("loginButton");

        vboxLayout1->addWidget(loginButton);

        spacerItem5 = new QSpacerItem(20, 12, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem5);

        registerLink = new QPushButton(auth_card);
        registerLink->setObjectName("registerLink");

        vboxLayout1->addWidget(registerLink);


        vboxLayout->addWidget(auth_card, 0, Qt::AlignCenter);

        spacerItem6 = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout->addItem(spacerItem6);


        retranslateUi(LoginPage);

        loginButton->setDefault(true);


        QMetaObject::connectSlotsByName(LoginPage);
    } // setupUi

    void retranslateUi(QWidget *LoginPage)
    {
        logoLabel->setText(QString());
        auth_title->setText(QCoreApplication::translate("LoginPage", "Bienvenue", nullptr));
        auth_subtitle->setText(QCoreApplication::translate("LoginPage", "Connectez-vous pour continuer", nullptr));
        label->setText(QCoreApplication::translate("LoginPage", "Nom d'utilisateur", nullptr));
        usernameInput->setPlaceholderText(QCoreApplication::translate("LoginPage", "Entrez votre nom d'utilisateur", nullptr));
        label1->setText(QCoreApplication::translate("LoginPage", "Mot de passe", nullptr));
        passwordInput->setPlaceholderText(QCoreApplication::translate("LoginPage", "Entrez votre mot de passe", nullptr));
        loginButton->setObjectName(QCoreApplication::translate("LoginPage", "primary_button", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginPage", "Se connecter", nullptr));
        registerLink->setObjectName(QCoreApplication::translate("LoginPage", "text_button", nullptr));
        registerLink->setText(QCoreApplication::translate("LoginPage", "Pas de compte ? Inscrivez-vous", nullptr));
        (void)LoginPage;
    } // retranslateUi

};

namespace Ui {
    class LoginPage: public Ui_LoginPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPAGE_H
