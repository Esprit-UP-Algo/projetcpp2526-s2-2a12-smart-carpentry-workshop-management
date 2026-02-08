#ifndef EMPLOYEEDIALOG_H
#define EMPLOYEEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include "../../models/employee.h"

class EmployeeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeDialog(QWidget *parent = nullptr);
    ~EmployeeDialog() = default;
    
    Employee getEmployee() const;
    void setEmployee(const Employee& employee);

private slots:
    void onAccepted();
    void onRejected();

private:
    // Form inputs
    QLineEdit *m_cinInput;
    QLineEdit *m_nomInput;
    QLineEdit *m_prenomInput;
    QComboBox *m_posteCombo;
    QLineEdit *m_emailInput;
    QLineEdit *m_telephoneInput;
    QDateEdit *m_dateEmbaucheInput;
    QDoubleSpinBox *m_salaireInput;
    QTextEdit *m_competencesInput;
    QComboBox *m_disponibiliteCombo;
    QDoubleSpinBox *m_performanceInput;
    QSpinBox *m_nbJoursCongesInput;
    QSpinBox *m_nbJoursAbsenceInput;
    QDoubleSpinBox *m_heuresTravailInput;
    
    void setupUI();
    bool validateInput();
};

#endif // EMPLOYEEDIALOG_H
