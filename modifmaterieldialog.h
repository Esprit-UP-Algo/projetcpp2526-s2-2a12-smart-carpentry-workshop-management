#ifndef MODIFMATERIELDIALOG_H
#define MODIFMATERIELDIALOG_H

#include <QDialog>

namespace Ui {
class ModifMaterielDialog;
}

class ModifMaterielDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModifMaterielDialog(QWidget *parent = nullptr);
    ~ModifMaterielDialog();

private:
    Ui::ModifMaterielDialog *ui;
};

#endif // MODIFMATERIELDIALOG_H
