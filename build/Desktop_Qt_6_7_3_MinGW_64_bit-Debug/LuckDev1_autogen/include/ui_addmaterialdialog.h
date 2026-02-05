/********************************************************************************
** Form generated from reading UI file 'addmaterialdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDMATERIALDIALOG_H
#define UI_ADDMATERIALDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AddMaterialDialog
{
public:
    QGroupBox *AjouterMaterielGroupBox;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QDateEdit *DateCommande;
    QSpinBox *seuilAlerte;
    QLabel *label_7;
    QLineEdit *NomFournisseur;
    QSpinBox *ConsoMentuelle;
    QLabel *label;
    QLabel *label_4;
    QLabel *label_6;
    QSpinBox *QuantiteStock;
    QComboBox *TypeMaeriel;
    QLabel *label_2;
    QLabel *label_5;
    QLabel *label_3;
    QLineEdit *NomMatriel;
    QLabel *label_16;
    QSpinBox *PrixUnitaire;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *AjoutMaterielB;
    QPushButton *CancelB;
    QLabel *label_8;

    void setupUi(QDialog *AddMaterialDialog)
    {
        if (AddMaterialDialog->objectName().isEmpty())
            AddMaterialDialog->setObjectName("AddMaterialDialog");
        AddMaterialDialog->resize(664, 668);
        AjouterMaterielGroupBox = new QGroupBox(AddMaterialDialog);
        AjouterMaterielGroupBox->setObjectName("AjouterMaterielGroupBox");
        AjouterMaterielGroupBox->setGeometry(QRect(40, 110, 511, 431));
        gridLayoutWidget = new QWidget(AjouterMaterielGroupBox);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(30, 50, 451, 351));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        DateCommande = new QDateEdit(gridLayoutWidget);
        DateCommande->setObjectName("DateCommande");

        gridLayout->addWidget(DateCommande, 6, 1, 1, 1);

        seuilAlerte = new QSpinBox(gridLayoutWidget);
        seuilAlerte->setObjectName("seuilAlerte");

        gridLayout->addWidget(seuilAlerte, 5, 1, 1, 1);

        label_7 = new QLabel(gridLayoutWidget);
        label_7->setObjectName("label_7");

        gridLayout->addWidget(label_7, 7, 0, 1, 1);

        NomFournisseur = new QLineEdit(gridLayoutWidget);
        NomFournisseur->setObjectName("NomFournisseur");

        gridLayout->addWidget(NomFournisseur, 4, 1, 1, 1);

        ConsoMentuelle = new QSpinBox(gridLayoutWidget);
        ConsoMentuelle->setObjectName("ConsoMentuelle");

        gridLayout->addWidget(ConsoMentuelle, 7, 1, 1, 1);

        label = new QLabel(gridLayoutWidget);
        label->setObjectName("label");

        gridLayout->addWidget(label, 1, 0, 1, 1);

        label_4 = new QLabel(gridLayoutWidget);
        label_4->setObjectName("label_4");

        gridLayout->addWidget(label_4, 4, 0, 1, 1);

        label_6 = new QLabel(gridLayoutWidget);
        label_6->setObjectName("label_6");

        gridLayout->addWidget(label_6, 6, 0, 1, 1);

        QuantiteStock = new QSpinBox(gridLayoutWidget);
        QuantiteStock->setObjectName("QuantiteStock");

        gridLayout->addWidget(QuantiteStock, 2, 1, 1, 1);

        TypeMaeriel = new QComboBox(gridLayoutWidget);
        TypeMaeriel->setObjectName("TypeMaeriel");

        gridLayout->addWidget(TypeMaeriel, 1, 1, 1, 1);

        label_2 = new QLabel(gridLayoutWidget);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        label_5 = new QLabel(gridLayoutWidget);
        label_5->setObjectName("label_5");

        gridLayout->addWidget(label_5, 5, 0, 1, 1);

        label_3 = new QLabel(gridLayoutWidget);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        NomMatriel = new QLineEdit(gridLayoutWidget);
        NomMatriel->setObjectName("NomMatriel");

        gridLayout->addWidget(NomMatriel, 0, 1, 1, 1);

        label_16 = new QLabel(gridLayoutWidget);
        label_16->setObjectName("label_16");

        gridLayout->addWidget(label_16, 3, 0, 1, 1);

        PrixUnitaire = new QSpinBox(gridLayoutWidget);
        PrixUnitaire->setObjectName("PrixUnitaire");

        gridLayout->addWidget(PrixUnitaire, 3, 1, 1, 1);

        horizontalLayoutWidget = new QWidget(AddMaterialDialog);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(40, 560, 511, 80));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        AjoutMaterielB = new QPushButton(horizontalLayoutWidget);
        AjoutMaterielB->setObjectName("AjoutMaterielB");

        horizontalLayout->addWidget(AjoutMaterielB);

        CancelB = new QPushButton(horizontalLayoutWidget);
        CancelB->setObjectName("CancelB");

        horizontalLayout->addWidget(CancelB);

        label_8 = new QLabel(AddMaterialDialog);
        label_8->setObjectName("label_8");
        label_8->setGeometry(QRect(140, 30, 311, 61));

        retranslateUi(AddMaterialDialog);

        QMetaObject::connectSlotsByName(AddMaterialDialog);
    } // setupUi

    void retranslateUi(QDialog *AddMaterialDialog)
    {
        AddMaterialDialog->setWindowTitle(QCoreApplication::translate("AddMaterialDialog", "Dialog", nullptr));
        AjouterMaterielGroupBox->setTitle(QCoreApplication::translate("AddMaterialDialog", "Informations du Mat\303\251riel", nullptr));
        label_7->setText(QCoreApplication::translate("AddMaterialDialog", "Consommation mentuelle", nullptr));
        label->setText(QCoreApplication::translate("AddMaterialDialog", "Type", nullptr));
        label_4->setText(QCoreApplication::translate("AddMaterialDialog", "Fournisseur", nullptr));
        label_6->setText(QCoreApplication::translate("AddMaterialDialog", "Date d\303\251rni\303\250re commande", nullptr));
        label_2->setText(QCoreApplication::translate("AddMaterialDialog", "Nom", nullptr));
        label_5->setText(QCoreApplication::translate("AddMaterialDialog", "Seuil d'alerte", nullptr));
        label_3->setText(QCoreApplication::translate("AddMaterialDialog", "Quantit\303\251 en stock", nullptr));
        label_16->setText(QCoreApplication::translate("AddMaterialDialog", "Prix Unitaire (DT)", nullptr));
        AjoutMaterielB->setText(QCoreApplication::translate("AddMaterialDialog", "Valider", nullptr));
        CancelB->setText(QCoreApplication::translate("AddMaterialDialog", "Annuler", nullptr));
        label_8->setText(QCoreApplication::translate("AddMaterialDialog", "Ajouter un nouveau mat\303\251riel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AddMaterialDialog: public Ui_AddMaterialDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDMATERIALDIALOG_H
