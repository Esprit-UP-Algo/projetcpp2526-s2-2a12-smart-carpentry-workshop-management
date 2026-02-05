#ifndef ADDMATERIALDIALOG_H
#define ADDMATERIALDIALOG_H

#include <QDialog>

namespace Ui {
class AddMaterialDialog;
}

class AddMaterialDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddMaterialDialog(QWidget *parent = nullptr);
    ~AddMaterialDialog();

private:
    Ui::AddMaterialDialog *ui;
};

#endif // ADDMATERIALDIALOG_H
