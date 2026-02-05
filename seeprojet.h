#ifndef SEEPROJET_H
#define SEEPROJET_H

#include <QDialog>
#include "deleteprojet.h"
#include "modifierprojet.h"

namespace Ui {
class seeprojet;
}

class seeprojet : public QDialog
{
    Q_OBJECT

public:
    explicit seeprojet(QWidget *parent = nullptr);
    ~seeprojet();

private slots:
    void on_sup_clicked();
    void on_mod_clicked();

private:
    Ui::seeprojet *ui;
    deleteprojet *deleteProjet;
    modifierprojet *modifierProjet;
};

#endif // SEEPROJET_H
