/********************************************************************************
** Form generated from reading UI file 'affichermaterieldialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AFFICHERMATERIELDIALOG_H
#define UI_AFFICHERMATERIELDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AfficherMaterielDialog
{
public:
    QGroupBox *groupBox_2;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpacerItem *horizontalSpacer;
    QLabel *label_10;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *quantityLabel;
    QLabel *label_8;
    QLabel *supplierLabel;
    QLabel *unitPriceLabel;
    QLabel *label_7;
    QLabel *label_2;
    QLabel *label_6;
    QLabel *lastOrderDateLabel;
    QLabel *label_4;
    QLabel *monthlyConsumptionLabel;
    QLabel *locationLabel;
    QLabel *label_3;
    QLabel *alertThresholdLabel;
    QLabel *label_5;
    QLabel *label_9;
    QLabel *TypeLabel;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton_2;
    QPushButton *pushButton;

    void setupUi(QDialog *AfficherMaterielDialog)
    {
        if (AfficherMaterielDialog->objectName().isEmpty())
            AfficherMaterielDialog->setObjectName("AfficherMaterielDialog");
        AfficherMaterielDialog->resize(591, 577);
        groupBox_2 = new QGroupBox(AfficherMaterielDialog);
        groupBox_2->setObjectName("groupBox_2");
        groupBox_2->setGeometry(QRect(60, 10, 471, 461));
        horizontalLayoutWidget = new QWidget(groupBox_2);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(60, 40, 341, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_10 = new QLabel(horizontalLayoutWidget);
        label_10->setObjectName("label_10");

        horizontalLayout->addWidget(label_10);

        groupBox = new QGroupBox(groupBox_2);
        groupBox->setObjectName("groupBox");
        groupBox->setGeometry(QRect(40, 110, 391, 311));
        QFont font;
        font.setWeight(QFont::DemiBold);
        groupBox->setFont(font);
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName("gridLayout");
        quantityLabel = new QLabel(groupBox);
        quantityLabel->setObjectName("quantityLabel");
        quantityLabel->setStyleSheet(QString::fromUtf8("font-weight: bold;"));

        gridLayout->addWidget(quantityLabel, 1, 1, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName("label_8");

        gridLayout->addWidget(label_8, 7, 0, 1, 1);

        supplierLabel = new QLabel(groupBox);
        supplierLabel->setObjectName("supplierLabel");

        gridLayout->addWidget(supplierLabel, 3, 1, 1, 1);

        unitPriceLabel = new QLabel(groupBox);
        unitPriceLabel->setObjectName("unitPriceLabel");
        unitPriceLabel->setStyleSheet(QString::fromUtf8("font-weight: bold;"));

        gridLayout->addWidget(unitPriceLabel, 2, 1, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName("label_7");

        gridLayout->addWidget(label_7, 6, 0, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");

        gridLayout->addWidget(label_6, 5, 0, 1, 1);

        lastOrderDateLabel = new QLabel(groupBox);
        lastOrderDateLabel->setObjectName("lastOrderDateLabel");

        gridLayout->addWidget(lastOrderDateLabel, 5, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName("label_4");

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        monthlyConsumptionLabel = new QLabel(groupBox);
        monthlyConsumptionLabel->setObjectName("monthlyConsumptionLabel");

        gridLayout->addWidget(monthlyConsumptionLabel, 7, 1, 1, 1);

        locationLabel = new QLabel(groupBox);
        locationLabel->setObjectName("locationLabel");

        gridLayout->addWidget(locationLabel, 6, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        alertThresholdLabel = new QLabel(groupBox);
        alertThresholdLabel->setObjectName("alertThresholdLabel");

        gridLayout->addWidget(alertThresholdLabel, 4, 1, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName("label_5");

        gridLayout->addWidget(label_5, 4, 0, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName("label_9");

        gridLayout->addWidget(label_9, 0, 0, 1, 1);

        TypeLabel = new QLabel(groupBox);
        TypeLabel->setObjectName("TypeLabel");

        gridLayout->addWidget(TypeLabel, 0, 1, 1, 1);

        horizontalLayoutWidget_2 = new QWidget(AfficherMaterielDialog);
        horizontalLayoutWidget_2->setObjectName("horizontalLayoutWidget_2");
        horizontalLayoutWidget_2->setGeometry(QRect(90, 480, 411, 80));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        pushButton_2 = new QPushButton(horizontalLayoutWidget_2);
        pushButton_2->setObjectName("pushButton_2");

        horizontalLayout_2->addWidget(pushButton_2);

        pushButton = new QPushButton(horizontalLayoutWidget_2);
        pushButton->setObjectName("pushButton");

        horizontalLayout_2->addWidget(pushButton);


        retranslateUi(AfficherMaterielDialog);

        QMetaObject::connectSlotsByName(AfficherMaterielDialog);
    } // setupUi

    void retranslateUi(QDialog *AfficherMaterielDialog)
    {
        AfficherMaterielDialog->setWindowTitle(QCoreApplication::translate("AfficherMaterielDialog", "Dialog", nullptr));
        groupBox_2->setTitle(QString());
        label->setText(QCoreApplication::translate("AfficherMaterielDialog", "Nom du mat\303\251riel", nullptr));
        label_10->setText(QCoreApplication::translate("AfficherMaterielDialog", "ID: 000", nullptr));
        groupBox->setTitle(QCoreApplication::translate("AfficherMaterielDialog", "Informations D\303\251taill\303\251es", nullptr));
        quantityLabel->setText(QCoreApplication::translate("AfficherMaterielDialog", "0", nullptr));
        label_8->setText(QCoreApplication::translate("AfficherMaterielDialog", "Consommation mensuelle:", nullptr));
        supplierLabel->setText(QCoreApplication::translate("AfficherMaterielDialog", "Non sp\303\251cifi\303\251", nullptr));
        unitPriceLabel->setText(QCoreApplication::translate("AfficherMaterielDialog", "\342\202\254 0.00", nullptr));
        label_7->setText(QCoreApplication::translate("AfficherMaterielDialog", "Localisation:", nullptr));
        label_2->setText(QCoreApplication::translate("AfficherMaterielDialog", "Quantit\303\251 en stock:", nullptr));
        label_6->setText(QCoreApplication::translate("AfficherMaterielDialog", "Date derni\303\250re commande:", nullptr));
        lastOrderDateLabel->setText(QCoreApplication::translate("AfficherMaterielDialog", "00/00/0000", nullptr));
        label_4->setText(QCoreApplication::translate("AfficherMaterielDialog", "Fournisseur:", nullptr));
        monthlyConsumptionLabel->setText(QCoreApplication::translate("AfficherMaterielDialog", "0", nullptr));
        locationLabel->setText(QCoreApplication::translate("AfficherMaterielDialog", "Non sp\303\251cifi\303\251", nullptr));
        label_3->setText(QCoreApplication::translate("AfficherMaterielDialog", "Prix unitaire:", nullptr));
        alertThresholdLabel->setText(QCoreApplication::translate("AfficherMaterielDialog", "0", nullptr));
        label_5->setText(QCoreApplication::translate("AfficherMaterielDialog", "Seuil d'alerte:", nullptr));
        label_9->setText(QCoreApplication::translate("AfficherMaterielDialog", "Type", nullptr));
        TypeLabel->setText(QCoreApplication::translate("AfficherMaterielDialog", "Bois", nullptr));
        pushButton_2->setText(QCoreApplication::translate("AfficherMaterielDialog", "Exporter", nullptr));
        pushButton->setText(QCoreApplication::translate("AfficherMaterielDialog", "Fermer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AfficherMaterielDialog: public Ui_AfficherMaterielDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AFFICHERMATERIELDIALOG_H
