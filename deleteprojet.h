#ifndef DELETEPROJET_H
#define DELETEPROJET_H

#include <QDialog>

namespace Ui {
class deleteprojet;
}

class deleteprojet : public QDialog
{
    Q_OBJECT

public:
    explicit deleteprojet(QWidget *parent = nullptr);
    ~deleteprojet();

private:
    Ui::deleteprojet *ui;
};

#endif // DELETEPROJET_H
