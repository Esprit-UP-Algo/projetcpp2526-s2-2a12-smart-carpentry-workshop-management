#ifndef AJOUTPROJET_H
#define AJOUTPROJET_H

#include <QDialog>

namespace Ui {
class AjoutProjet;
}

class AjoutProjet : public QDialog
{
    Q_OBJECT

public:
    explicit AjoutProjet(QWidget *parent = nullptr);
    ~AjoutProjet();

private:
    Ui::AjoutProjet *ui;
};

#endif // AJOUTPROJET_H
