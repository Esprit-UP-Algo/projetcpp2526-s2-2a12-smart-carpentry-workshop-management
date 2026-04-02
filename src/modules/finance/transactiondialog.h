#ifndef TRANSACTIONDIALOG_H
#define TRANSACTIONDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>

class QComboBox;
class QLineEdit;
class QDateEdit;
class QDialogButtonBox;

class TransactionDialog : public QDialog
{
    Q_OBJECT  // ← IMPORTANT : Cette macro est nécessaire
public:
    explicit TransactionDialog(QWidget *parent = nullptr);

    void setData(const QMap<QString, QString> &data);
    QMap<QString, QString> getData() const;

private:
    void setupUI();

    QComboBox *m_typeCombo;
    QComboBox *m_modeCombo;
    QComboBox *m_statutCombo;
    QComboBox *m_categoryCombo;
    QLineEdit *m_montantEdit;
    QDateEdit *m_dateEdit;
    QDialogButtonBox *m_buttonBox;
};

#endif // TRANSACTIONDIALOG_H
