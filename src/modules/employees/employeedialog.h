#ifndef EMPLOYEEDIALOG_H
#define EMPLOYEEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QCheckBox>
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
    void togglePasswordVisibility(bool checked);

private:
    // Personal
    QLineEdit      *m_cinInput;
    QLineEdit      *m_nomInput;
    QLineEdit      *m_prenomInput;
    QComboBox      *m_posteCombo;
    // Contact
    QLineEdit      *m_emailInput;
    QLineEdit      *m_telephoneInput;
    // Professional
    QDateEdit      *m_dateEmbaucheInput;
    QDoubleSpinBox *m_salaireInput;
    QTextEdit      *m_competencesInput;
    QComboBox      *m_disponibiliteCombo;
    // Tracking
    QDoubleSpinBox *m_performanceInput;
    QDoubleSpinBox *m_heuresTravailInput;
    QSpinBox       *m_nbJoursCongesInput;
    QSpinBox       *m_nbJoursAbsenceInput;
    // Auth
    QLineEdit      *m_passwordInput;
    QCheckBox      *m_showPasswordCheck;

    bool m_editMode = false;

    void setupUI();
    void applyTheme(bool dark);
    bool validateInput();
};

#endif // EMPLOYEEDIALOG_H
