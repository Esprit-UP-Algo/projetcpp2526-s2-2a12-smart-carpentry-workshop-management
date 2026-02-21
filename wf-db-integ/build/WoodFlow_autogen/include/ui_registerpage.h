/********************************************************************************
** Form generated from reading UI file 'registerpage.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTERPAGE_H
#define UI_REGISTERPAGE_H

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

class Ui_RegisterPage
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
    QLineEdit *emailInput;
    QSpacerItem *spacerItem4;
    QLabel *label2;
    QLineEdit *passwordInput;
    QSpacerItem *spacerItem5;
    QLabel *label3;
    QLineEdit *confirmInput;
    QSpacerItem *spacerItem6;
    QPushButton *registerButton;
    QSpacerItem *spacerItem7;
    QPushButton *loginLink;
    QSpacerItem *spacerItem8;

    void setupUi(QWidget *RegisterPage)
    {
        if (RegisterPage->objectName().isEmpty())
            RegisterPage->setObjectName("RegisterPage");
        vboxLayout = new QVBoxLayout(RegisterPage);
        vboxLayout->setObjectName("vboxLayout");
        spacerItem = new QSpacerItem(20, 35, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout->addItem(spacerItem);

        auth_card = new QFrame(RegisterPage);
        auth_card->setObjectName("auth_card");
        auth_card->setMinimumSize(QSize(420, 0));
        auth_card->setMaximumSize(QSize(420, 16777215));
        vboxLayout1 = new QVBoxLayout(auth_card);
        vboxLayout1->setSpacing(6);
        vboxLayout1->setObjectName("vboxLayout1");
        vboxLayout1->setContentsMargins(45, 45, 45, 45);
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

        spacerItem2 = new QSpacerItem(20, 26, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem2);

        label = new QLabel(auth_card);
        label->setObjectName("label");

        vboxLayout1->addWidget(label);

        usernameInput = new QLineEdit(auth_card);
        usernameInput->setObjectName("usernameInput");

        vboxLayout1->addWidget(usernameInput);

        spacerItem3 = new QSpacerItem(20, 12, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem3);

        label1 = new QLabel(auth_card);
        label1->setObjectName("label1");

        vboxLayout1->addWidget(label1);

        emailInput = new QLineEdit(auth_card);
        emailInput->setObjectName("emailInput");

        vboxLayout1->addWidget(emailInput);

        spacerItem4 = new QSpacerItem(20, 12, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem4);

        label2 = new QLabel(auth_card);
        label2->setObjectName("label2");

        vboxLayout1->addWidget(label2);

        passwordInput = new QLineEdit(auth_card);
        passwordInput->setObjectName("passwordInput");
        passwordInput->setEchoMode(QLineEdit::Password);

        vboxLayout1->addWidget(passwordInput);

        spacerItem5 = new QSpacerItem(20, 12, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem5);

        label3 = new QLabel(auth_card);
        label3->setObjectName("label3");

        vboxLayout1->addWidget(label3);

        confirmInput = new QLineEdit(auth_card);
        confirmInput->setObjectName("confirmInput");
        confirmInput->setEchoMode(QLineEdit::Password);

        vboxLayout1->addWidget(confirmInput);

        spacerItem6 = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem6);

        registerButton = new QPushButton(auth_card);
        registerButton->setObjectName("registerButton");

        vboxLayout1->addWidget(registerButton);

        spacerItem7 = new QSpacerItem(20, 14, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout1->addItem(spacerItem7);

        loginLink = new QPushButton(auth_card);
        loginLink->setObjectName("loginLink");

        vboxLayout1->addWidget(loginLink);


        vboxLayout->addWidget(auth_card, 0, Qt::AlignCenter);

        spacerItem8 = new QSpacerItem(20, 35, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vboxLayout->addItem(spacerItem8);


        retranslateUi(RegisterPage);

        registerButton->setDefault(true);


        QMetaObject::connectSlotsByName(RegisterPage);
    } // setupUi

    void retranslateUi(QWidget *RegisterPage)
    {
        logoLabel->setText(QString());
        auth_title->setText(QCoreApplication::translate("RegisterPage", "Cr\303\251er un compte", nullptr));
        auth_subtitle->setText(QCoreApplication::translate("RegisterPage", "Inscrivez-vous pour commencer", nullptr));
        label->setText(QCoreApplication::translate("RegisterPage", "CIN", nullptr));
        usernameInput->setPlaceholderText(QCoreApplication::translate("RegisterPage", "Entrez votre CIN (8 chiffres)", nullptr));
        label1->setText(QCoreApplication::translate("RegisterPage", "Email", nullptr));
        emailInput->setPlaceholderText(QCoreApplication::translate("RegisterPage", "prenom.nom@domaine.com", nullptr));
        label2->setText(QCoreApplication::translate("RegisterPage", "Mot de passe", nullptr));
        passwordInput->setPlaceholderText(QCoreApplication::translate("RegisterPage", "Cr\303\251ez un mot de passe", nullptr));
        label3->setText(QCoreApplication::translate("RegisterPage", "Confirmer le mot de passe", nullptr));
        confirmInput->setPlaceholderText(QCoreApplication::translate("RegisterPage", "Confirmez votre mot de passe", nullptr));
        registerButton->setObjectName(QCoreApplication::translate("RegisterPage", "primary_button", nullptr));
        registerButton->setText(QCoreApplication::translate("RegisterPage", "Cr\303\251er un compte", nullptr));
        loginLink->setObjectName(QCoreApplication::translate("RegisterPage", "text_button", nullptr));
        loginLink->setText(QCoreApplication::translate("RegisterPage", "Vous avez d\303\251j\303\240 un compte ? Connectez-vous", nullptr));
        (void)RegisterPage;
    } // retranslateUi

};

namespace Ui {
    class RegisterPage: public Ui_RegisterPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTERPAGE_H
