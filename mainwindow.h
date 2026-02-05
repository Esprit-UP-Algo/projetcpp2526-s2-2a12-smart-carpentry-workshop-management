#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    // Ajoute ces slots pour chaque bouton
    void on_btnAddMaterial_clicked();
    void on_pushButton_2_clicked();    // Modifier
    void on_pushButton_3_clicked();    // Afficher
    void on_pushButton_4_clicked();    // Supprimer

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
