#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ajoutprojet.h"
#include "deleteprojet.h"
#include "modifierprojet.h"
#include "seeprojet.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_ajout_clicked();
    void on_delete_2_clicked();
    void on_modifier_clicked();
    void on_see_clicked();
private:
    Ui::MainWindow *ui;
    AjoutProjet *Ajoutprojet;
    deleteprojet *deleteProjet;
    modifierprojet *modifierProjet;
    seeprojet *seeProjet;
};
#endif // MAINWINDOW_H
