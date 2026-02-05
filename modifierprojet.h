#ifndef MODIFIERPROJET_H
#define MODIFIERPROJET_H

#include <QDialog>

namespace Ui {
class modifierprojet;
}

class modifierprojet : public QDialog
{
    Q_OBJECT

public:
    explicit modifierprojet(QWidget *parent = nullptr);
    ~modifierprojet();

private:
    Ui::modifierprojet *ui;
};

#endif // MODIFIERPROJET_H
