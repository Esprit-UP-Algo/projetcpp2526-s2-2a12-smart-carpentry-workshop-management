/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *vboxLayout;
    QLabel *label_title;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *lineEdit_id;
    QLabel *label1;
    QDoubleSpinBox *spinBox_montant;
    QLabel *label2;
    QDateEdit *dateEdit_tran;
    QLabel *label3;
    QLineEdit *lineEdit_type;
    QLabel *label4;
    QLineEdit *lineEdit_mode;
    QLabel *label5;
    QLineEdit *lineEdit_statut;
    QLabel *label6;
    QLineEdit *lineEdit_categorie;
    QLabel *label7;
    QLineEdit *lineEdit_client;
    QLabel *label8;
    QTextEdit *textEdit_description;
    QHBoxLayout *hboxLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_reset;
    QPushButton *pushButton_ajouter;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        vboxLayout = new QVBoxLayout(centralWidget);
        vboxLayout->setObjectName("vboxLayout");
        label_title = new QLabel(centralWidget);
        label_title->setObjectName("label_title");
        label_title->setAlignment(Qt::AlignCenter);

        vboxLayout->addWidget(label_title);

        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName("groupBox");
        formLayout = new QFormLayout(groupBox);
        formLayout->setObjectName("formLayout");
        label = new QLabel(groupBox);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        lineEdit_id = new QLineEdit(groupBox);
        lineEdit_id->setObjectName("lineEdit_id");

        formLayout->setWidget(0, QFormLayout::FieldRole, lineEdit_id);

        label1 = new QLabel(groupBox);
        label1->setObjectName("label1");

        formLayout->setWidget(1, QFormLayout::LabelRole, label1);

        spinBox_montant = new QDoubleSpinBox(groupBox);
        spinBox_montant->setObjectName("spinBox_montant");
        spinBox_montant->setMaximum(999999999.000000000000000);

        formLayout->setWidget(1, QFormLayout::FieldRole, spinBox_montant);

        label2 = new QLabel(groupBox);
        label2->setObjectName("label2");

        formLayout->setWidget(2, QFormLayout::LabelRole, label2);

        dateEdit_tran = new QDateEdit(groupBox);
        dateEdit_tran->setObjectName("dateEdit_tran");
        dateEdit_tran->setCalendarPopup(true);

        formLayout->setWidget(2, QFormLayout::FieldRole, dateEdit_tran);

        label3 = new QLabel(groupBox);
        label3->setObjectName("label3");

        formLayout->setWidget(3, QFormLayout::LabelRole, label3);

        lineEdit_type = new QLineEdit(groupBox);
        lineEdit_type->setObjectName("lineEdit_type");

        formLayout->setWidget(3, QFormLayout::FieldRole, lineEdit_type);

        label4 = new QLabel(groupBox);
        label4->setObjectName("label4");

        formLayout->setWidget(4, QFormLayout::LabelRole, label4);

        lineEdit_mode = new QLineEdit(groupBox);
        lineEdit_mode->setObjectName("lineEdit_mode");

        formLayout->setWidget(4, QFormLayout::FieldRole, lineEdit_mode);

        label5 = new QLabel(groupBox);
        label5->setObjectName("label5");

        formLayout->setWidget(5, QFormLayout::LabelRole, label5);

        lineEdit_statut = new QLineEdit(groupBox);
        lineEdit_statut->setObjectName("lineEdit_statut");

        formLayout->setWidget(5, QFormLayout::FieldRole, lineEdit_statut);

        label6 = new QLabel(groupBox);
        label6->setObjectName("label6");

        formLayout->setWidget(6, QFormLayout::LabelRole, label6);

        lineEdit_categorie = new QLineEdit(groupBox);
        lineEdit_categorie->setObjectName("lineEdit_categorie");

        formLayout->setWidget(6, QFormLayout::FieldRole, lineEdit_categorie);

        label7 = new QLabel(groupBox);
        label7->setObjectName("label7");

        formLayout->setWidget(7, QFormLayout::LabelRole, label7);

        lineEdit_client = new QLineEdit(groupBox);
        lineEdit_client->setObjectName("lineEdit_client");

        formLayout->setWidget(7, QFormLayout::FieldRole, lineEdit_client);

        label8 = new QLabel(groupBox);
        label8->setObjectName("label8");

        formLayout->setWidget(8, QFormLayout::LabelRole, label8);

        textEdit_description = new QTextEdit(groupBox);
        textEdit_description->setObjectName("textEdit_description");

        formLayout->setWidget(8, QFormLayout::FieldRole, textEdit_description);


        vboxLayout->addWidget(groupBox);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName("hboxLayout");
        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hboxLayout->addItem(horizontalSpacer);

        pushButton_reset = new QPushButton(centralWidget);
        pushButton_reset->setObjectName("pushButton_reset");

        hboxLayout->addWidget(pushButton_reset);

        pushButton_ajouter = new QPushButton(centralWidget);
        pushButton_ajouter->setObjectName("pushButton_ajouter");

        hboxLayout->addWidget(pushButton_ajouter);


        vboxLayout->addLayout(hboxLayout);

        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Gestion des Transactions", nullptr));
        label_title->setText(QCoreApplication::translate("MainWindow", "Nouvelle Transaction", nullptr));
        label_title->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size:22px;font-weight:bold;margin-bottom:10px;", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "D\303\251tails de la transaction", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Id", nullptr));
        label1->setText(QCoreApplication::translate("MainWindow", "Montant", nullptr));
        spinBox_montant->setSuffix(QCoreApplication::translate("MainWindow", " DT", nullptr));
        label2->setText(QCoreApplication::translate("MainWindow", "Date", nullptr));
        label3->setText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        label4->setText(QCoreApplication::translate("MainWindow", "Mode paiement", nullptr));
        label5->setText(QCoreApplication::translate("MainWindow", "Statut", nullptr));
        label6->setText(QCoreApplication::translate("MainWindow", "Cat\303\251gorie", nullptr));
        label7->setText(QCoreApplication::translate("MainWindow", "Client", nullptr));
        label8->setText(QCoreApplication::translate("MainWindow", "Description", nullptr));
        pushButton_reset->setText(QCoreApplication::translate("MainWindow", "R\303\251initialiser", nullptr));
        pushButton_ajouter->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        pushButton_ajouter->setStyleSheet(QCoreApplication::translate("MainWindow", "background:#2ecc71;color:white;font-weight:bold;padding:6px 14px;", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
