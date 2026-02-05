#ifndef SUPPMATERIELDIALOG_H
#define SUPPMATERIELDIALOG_H

#include <QDialog>

namespace Ui {
class SuppMaterielDialog;
}

class SuppMaterielDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SuppMaterielDialog(QWidget *parent = nullptr);
    ~SuppMaterielDialog();

private:
    Ui::SuppMaterielDialog *ui;
};

#endif // SUPPMATERIELDIALOG_H
