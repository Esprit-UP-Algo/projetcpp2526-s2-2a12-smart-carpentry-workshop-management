#ifndef AFFICHERMATERIELDIALOG_H
#define AFFICHERMATERIELDIALOG_H

#include <QDialog>

namespace Ui {
class AfficherMaterielDialog;
}

class AfficherMaterielDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AfficherMaterielDialog(QWidget *parent = nullptr);
    ~AfficherMaterielDialog();

private:
    Ui::AfficherMaterielDialog *ui;
};

#endif // AFFICHERMATERIELDIALOG_H
