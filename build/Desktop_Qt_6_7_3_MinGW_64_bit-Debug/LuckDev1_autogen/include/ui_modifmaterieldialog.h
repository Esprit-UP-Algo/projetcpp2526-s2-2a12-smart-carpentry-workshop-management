/********************************************************************************
** Form generated from reading UI file 'modifmaterieldialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MODIFMATERIELDIALOG_H
#define UI_MODIFMATERIELDIALOG_H

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

class Ui_ModifMaterielDialog
{
public:
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *ModifMaterielB;
    QPushButton *CancelB;
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
    QLabel *label_8;

    void setupUi(QDialog *ModifMaterielDialog)
    {
        if (ModifMaterielDialog->objectName().isEmpty())
            ModifMaterielDialog->setObjectName("ModifMaterielDialog");
        ModifMaterielDialog->resize(658, 656);
        horizontalLayoutWidget = new QWidget(ModifMaterielDialog);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(80, 520, 511, 80));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        ModifMaterielB = new QPushButton(horizontalLayoutWidget);
        ModifMaterielB->setObjectName("ModifMaterielB");

        horizontalLayout->addWidget(ModifMaterielB);

        CancelB = new QPushButton(horizontalLayoutWidget);
        CancelB->setObjectName("CancelB");

        horizontalLayout->addWidget(CancelB);

        AjouterMaterielGroupBox = new QGroupBox(ModifMaterielDialog);
        AjouterMaterielGroupBox->setObjectName("AjouterMaterielGroupBox");
        AjouterMaterielGroupBox->setGeometry(QRect(80, 120, 511, 381));
        gridLayoutWidget = new QWidget(AjouterMaterielGroupBox);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(30, 60, 451, 291));
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

        label_8 = new QLabel(ModifMaterielDialog);
        label_8->setObjectName("label_8");
        label_8->setGeometry(QRect(180, 50, 311, 31));

        retranslateUi(ModifMaterielDialog);

        QMetaObject::connectSlotsByName(ModifMaterielDialog);
    } // setupUi

    void retranslateUi(QDialog *ModifMaterielDialog)
    {
        ModifMaterielDialog->setWindowTitle(QCoreApplication::translate("ModifMaterielDialog", "Dialog", nullptr));
        ModifMaterielB->setText(QCoreApplication::translate("ModifMaterielDialog", "Valider", nullptr));
        CancelB->setText(QCoreApplication::translate("ModifMaterielDialog", "Annuler", nullptr));
        AjouterMaterielGroupBox->setTitle(QCoreApplication::translate("ModifMaterielDialog", "Informations du Mat\303\251riel", nullptr));
        label_7->setText(QCoreApplication::translate("ModifMaterielDialog", "Consommation mentuelle", nullptr));
        label->setText(QCoreApplication::translate("ModifMaterielDialog", "Type", nullptr));
        label_4->setText(QCoreApplication::translate("ModifMaterielDialog", "Fournisseur", nullptr));
        label_6->setText(QCoreApplication::translate("ModifMaterielDialog", "Date d\303\251rni\303\250re commande", nullptr));
        label_2->setText(QCoreApplication::translate("ModifMaterielDialog", "Nom", nullptr));
        label_5->setText(QCoreApplication::translate("ModifMaterielDialog", "Seuil d'alerte", nullptr));
        label_3->setText(QCoreApplication::translate("ModifMaterielDialog", "Quantit\303\251 en stock", nullptr));
        label_16->setText(QCoreApplication::translate("ModifMaterielDialog", "Prix Unitaire (DT)", nullptr));
        label_8->setText(QCoreApplication::translate("ModifMaterielDialog", "Modifier un mat\303\251riel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ModifMaterielDialog: public Ui_ModifMaterielDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MODIFMATERIELDIALOG_H
