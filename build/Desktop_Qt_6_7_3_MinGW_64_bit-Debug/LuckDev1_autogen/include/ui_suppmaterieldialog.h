/********************************************************************************
** Form generated from reading UI file 'suppmaterieldialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SUPPMATERIELDIALOG_H
#define UI_SUPPMATERIELDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SuppMaterielDialog
{
public:
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *DeleteB;
    QPushButton *CancelB;
    QLabel *label;

    void setupUi(QDialog *SuppMaterielDialog)
    {
        if (SuppMaterielDialog->objectName().isEmpty())
            SuppMaterielDialog->setObjectName("SuppMaterielDialog");
        SuppMaterielDialog->resize(400, 180);
        horizontalLayoutWidget = new QWidget(SuppMaterielDialog);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(20, 80, 351, 80));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        DeleteB = new QPushButton(horizontalLayoutWidget);
        DeleteB->setObjectName("DeleteB");

        horizontalLayout->addWidget(DeleteB);

        CancelB = new QPushButton(horizontalLayoutWidget);
        CancelB->setObjectName("CancelB");

        horizontalLayout->addWidget(CancelB);

        label = new QLabel(SuppMaterielDialog);
        label->setObjectName("label");
        label->setGeometry(QRect(30, 40, 351, 20));

        retranslateUi(SuppMaterielDialog);

        QMetaObject::connectSlotsByName(SuppMaterielDialog);
    } // setupUi

    void retranslateUi(QDialog *SuppMaterielDialog)
    {
        SuppMaterielDialog->setWindowTitle(QCoreApplication::translate("SuppMaterielDialog", "Dialog", nullptr));
        DeleteB->setText(QCoreApplication::translate("SuppMaterielDialog", "Supprimer", nullptr));
        CancelB->setText(QCoreApplication::translate("SuppMaterielDialog", "Annuler", nullptr));
        label->setText(QCoreApplication::translate("SuppMaterielDialog", "Ete vous sure de vouloir supprimer cet \303\251l\303\251ment?", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SuppMaterielDialog: public Ui_SuppMaterielDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SUPPMATERIELDIALOG_H
