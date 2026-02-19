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
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGroupBox *groupBox;
    QLabel *label_cin;
    QLineEdit *lineEdit_cin;
    QLabel *label_nom;
    QLineEdit *lineEdit_nom;
    QLabel *label_prenom;
    QLineEdit *lineEdit_prenom;
    QLabel *label_poste;
    QLineEdit *lineEdit_poste;
    QLabel *label_email;
    QLineEdit *lineEdit_email;
    QLabel *label_tel;
    QLineEdit *lineEdit_tel;
    QLabel *label_date;
    QDateEdit *dateEdit_embauche;
    QLabel *label_salaire;
    QDoubleSpinBox *spinBox_salaire;
    QLabel *label_dispo;
    QComboBox *comboBox_dispo;
    QLabel *label_comp;
    QLineEdit *lineEdit_competences;
    QLabel *label_perf;
    QDoubleSpinBox *spinBox_perf;
    QLabel *label_njc;
    QSpinBox *spinBox_njc;
    QLabel *label_nja;
    QSpinBox *spinBox_nja;
    QLabel *label_hdt;
    QDoubleSpinBox *spinBox_hdt;
    QLabel *label_required;
    QPushButton *pushButton_ajouter;
    QPushButton *pushButton_reset;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(480, 600);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName("groupBox");
        groupBox->setGeometry(QRect(20, 10, 440, 510));
        label_cin = new QLabel(groupBox);
        label_cin->setObjectName("label_cin");
        label_cin->setGeometry(QRect(10, 30, 140, 20));
        lineEdit_cin = new QLineEdit(groupBox);
        lineEdit_cin->setObjectName("lineEdit_cin");
        lineEdit_cin->setGeometry(QRect(160, 28, 260, 24));
        label_nom = new QLabel(groupBox);
        label_nom->setObjectName("label_nom");
        label_nom->setGeometry(QRect(10, 65, 140, 20));
        lineEdit_nom = new QLineEdit(groupBox);
        lineEdit_nom->setObjectName("lineEdit_nom");
        lineEdit_nom->setGeometry(QRect(160, 63, 260, 24));
        label_prenom = new QLabel(groupBox);
        label_prenom->setObjectName("label_prenom");
        label_prenom->setGeometry(QRect(10, 100, 140, 20));
        lineEdit_prenom = new QLineEdit(groupBox);
        lineEdit_prenom->setObjectName("lineEdit_prenom");
        lineEdit_prenom->setGeometry(QRect(160, 98, 260, 24));
        label_poste = new QLabel(groupBox);
        label_poste->setObjectName("label_poste");
        label_poste->setGeometry(QRect(10, 135, 140, 20));
        lineEdit_poste = new QLineEdit(groupBox);
        lineEdit_poste->setObjectName("lineEdit_poste");
        lineEdit_poste->setGeometry(QRect(160, 133, 260, 24));
        label_email = new QLabel(groupBox);
        label_email->setObjectName("label_email");
        label_email->setGeometry(QRect(10, 170, 140, 20));
        lineEdit_email = new QLineEdit(groupBox);
        lineEdit_email->setObjectName("lineEdit_email");
        lineEdit_email->setGeometry(QRect(160, 168, 260, 24));
        label_tel = new QLabel(groupBox);
        label_tel->setObjectName("label_tel");
        label_tel->setGeometry(QRect(10, 205, 140, 20));
        lineEdit_tel = new QLineEdit(groupBox);
        lineEdit_tel->setObjectName("lineEdit_tel");
        lineEdit_tel->setGeometry(QRect(160, 203, 260, 24));
        label_date = new QLabel(groupBox);
        label_date->setObjectName("label_date");
        label_date->setGeometry(QRect(10, 240, 140, 20));
        dateEdit_embauche = new QDateEdit(groupBox);
        dateEdit_embauche->setObjectName("dateEdit_embauche");
        dateEdit_embauche->setGeometry(QRect(160, 238, 160, 24));
        dateEdit_embauche->setCalendarPopup(true);
        label_salaire = new QLabel(groupBox);
        label_salaire->setObjectName("label_salaire");
        label_salaire->setGeometry(QRect(10, 275, 140, 20));
        spinBox_salaire = new QDoubleSpinBox(groupBox);
        spinBox_salaire->setObjectName("spinBox_salaire");
        spinBox_salaire->setGeometry(QRect(160, 273, 160, 24));
        spinBox_salaire->setMaximum(999999.989999999990687);
        label_dispo = new QLabel(groupBox);
        label_dispo->setObjectName("label_dispo");
        label_dispo->setGeometry(QRect(10, 310, 140, 20));
        comboBox_dispo = new QComboBox(groupBox);
        comboBox_dispo->addItem(QString());
        comboBox_dispo->addItem(QString());
        comboBox_dispo->addItem(QString());
        comboBox_dispo->setObjectName("comboBox_dispo");
        comboBox_dispo->setGeometry(QRect(160, 308, 180, 24));
        label_comp = new QLabel(groupBox);
        label_comp->setObjectName("label_comp");
        label_comp->setGeometry(QRect(10, 345, 140, 20));
        lineEdit_competences = new QLineEdit(groupBox);
        lineEdit_competences->setObjectName("lineEdit_competences");
        lineEdit_competences->setGeometry(QRect(160, 343, 260, 24));
        label_perf = new QLabel(groupBox);
        label_perf->setObjectName("label_perf");
        label_perf->setGeometry(QRect(10, 380, 140, 20));
        spinBox_perf = new QDoubleSpinBox(groupBox);
        spinBox_perf->setObjectName("spinBox_perf");
        spinBox_perf->setGeometry(QRect(160, 378, 100, 24));
        spinBox_perf->setMaximum(10.000000000000000);
        label_njc = new QLabel(groupBox);
        label_njc->setObjectName("label_njc");
        label_njc->setGeometry(QRect(10, 415, 50, 20));
        spinBox_njc = new QSpinBox(groupBox);
        spinBox_njc->setObjectName("spinBox_njc");
        spinBox_njc->setGeometry(QRect(60, 413, 70, 24));
        label_nja = new QLabel(groupBox);
        label_nja->setObjectName("label_nja");
        label_nja->setGeometry(QRect(145, 415, 50, 20));
        spinBox_nja = new QSpinBox(groupBox);
        spinBox_nja->setObjectName("spinBox_nja");
        spinBox_nja->setGeometry(QRect(200, 413, 70, 24));
        label_hdt = new QLabel(groupBox);
        label_hdt->setObjectName("label_hdt");
        label_hdt->setGeometry(QRect(285, 415, 50, 20));
        spinBox_hdt = new QDoubleSpinBox(groupBox);
        spinBox_hdt->setObjectName("spinBox_hdt");
        spinBox_hdt->setGeometry(QRect(340, 413, 85, 24));
        spinBox_hdt->setMaximum(99999.990000000005239);
        label_required = new QLabel(groupBox);
        label_required->setObjectName("label_required");
        label_required->setGeometry(QRect(10, 465, 300, 20));
        pushButton_ajouter = new QPushButton(centralWidget);
        pushButton_ajouter->setObjectName("pushButton_ajouter");
        pushButton_ajouter->setGeometry(QRect(300, 535, 110, 32));
        pushButton_reset = new QPushButton(centralWidget);
        pushButton_reset->setObjectName("pushButton_reset");
        pushButton_reset->setGeometry(QRect(170, 535, 110, 32));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 480, 21));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Ajouter un Employ\303\251", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Informations de l'Employ\303\251", nullptr));
        label_cin->setText(QCoreApplication::translate("MainWindow", "CIN *", nullptr));
        lineEdit_cin->setPlaceholderText(QCoreApplication::translate("MainWindow", "ex: 12345678", nullptr));
        label_nom->setText(QCoreApplication::translate("MainWindow", "Nom *", nullptr));
        label_prenom->setText(QCoreApplication::translate("MainWindow", "Pr\303\251nom *", nullptr));
        label_poste->setText(QCoreApplication::translate("MainWindow", "Poste", nullptr));
        label_email->setText(QCoreApplication::translate("MainWindow", "Email", nullptr));
        lineEdit_email->setPlaceholderText(QCoreApplication::translate("MainWindow", "ex: nom@email.com", nullptr));
        label_tel->setText(QCoreApplication::translate("MainWindow", "Num\303\251ro T\303\251l", nullptr));
        label_date->setText(QCoreApplication::translate("MainWindow", "Date d'embauche", nullptr));
        dateEdit_embauche->setDisplayFormat(QCoreApplication::translate("MainWindow", "dd/MM/yyyy", nullptr));
        label_salaire->setText(QCoreApplication::translate("MainWindow", "Salaire", nullptr));
        spinBox_salaire->setSuffix(QCoreApplication::translate("MainWindow", " DT", nullptr));
        label_dispo->setText(QCoreApplication::translate("MainWindow", "Disponibilit\303\251", nullptr));
        comboBox_dispo->setItemText(0, QCoreApplication::translate("MainWindow", "Disponible", nullptr));
        comboBox_dispo->setItemText(1, QCoreApplication::translate("MainWindow", "Indisponible", nullptr));
        comboBox_dispo->setItemText(2, QCoreApplication::translate("MainWindow", "En conge", nullptr));

        label_comp->setText(QCoreApplication::translate("MainWindow", "Comp\303\251tences", nullptr));
        lineEdit_competences->setPlaceholderText(QCoreApplication::translate("MainWindow", "ex: C++, Qt, Oracle", nullptr));
        label_perf->setText(QCoreApplication::translate("MainWindow", "Performance (0-10)", nullptr));
        label_njc->setText(QCoreApplication::translate("MainWindow", "NJC", nullptr));
        label_nja->setText(QCoreApplication::translate("MainWindow", "NJA", nullptr));
        label_hdt->setText(QCoreApplication::translate("MainWindow", "HDT", nullptr));
        label_required->setText(QCoreApplication::translate("MainWindow", "* Champs obligatoires", nullptr));
        label_required->setStyleSheet(QCoreApplication::translate("MainWindow", "color: gray; font-size: 10px;", nullptr));
        pushButton_ajouter->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        pushButton_reset->setText(QCoreApplication::translate("MainWindow", "R\303\251initialiser", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
