#pragma once
// ── PinAssignDialog — assigns a PIN to an employee directly in EMPLOYE ───────
// Usage:
//   PinAssignDialog dlg(cin, this);
//   dlg.exec();
//
// Add to your CMakeLists.txt sources:
//   src/modules/employees/pinassigndialog.h
//   src/modules/employees/pinassigndialog.cpp  (or keep it header-only below)

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QRandomGenerator>

class PinAssignDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PinAssignDialog(const QString& cin, const QString& employeeName,
                             QWidget* parent = nullptr)
        : QDialog(parent), m_cin(cin)
    {
        setWindowTitle("Assign PIN — " + employeeName);
        setFixedWidth(340);

        auto* form = new QFormLayout;

        // PIN field
        m_pinEdit = new QLineEdit;
        m_pinEdit->setMaxLength(6);
        m_pinEdit->setPlaceholderText("6 digits");
        m_pinEdit->setInputMask("999999"); // digits only
        form->addRow("PIN (6 digits):", m_pinEdit);

        // Validity
        m_daysSpin = new QSpinBox;
        m_daysSpin->setRange(1, 365);
        m_daysSpin->setValue(30);
        m_daysSpin->setSuffix(" days");
        form->addRow("Valid for:", m_daysSpin);

        // Info label
        m_infoLabel = new QLabel;
        m_infoLabel->setWordWrap(true);
        m_infoLabel->setStyleSheet("color: gray; font-size: 11px;");

        // Buttons
        auto* genBtn   = new QPushButton("Generate random PIN");
        auto* saveBtn  = new QPushButton("Save PIN");
        auto* clearBtn = new QPushButton("Remove PIN");
        saveBtn->setDefault(true);

        auto* btnLayout = new QHBoxLayout;
        btnLayout->addWidget(clearBtn);
        btnLayout->addStretch();
        btnLayout->addWidget(genBtn);
        btnLayout->addWidget(saveBtn);

        auto* main = new QVBoxLayout(this);
        main->addLayout(form);
        main->addWidget(m_infoLabel);
        main->addLayout(btnLayout);

        // Load current PIN if any
        loadCurrentPin();

        connect(genBtn,   &QPushButton::clicked, this, &PinAssignDialog::generatePin);
        connect(saveBtn,  &QPushButton::clicked, this, &PinAssignDialog::savePin);
        connect(clearBtn, &QPushButton::clicked, this, &PinAssignDialog::removePin);
    }

private slots:
    void generatePin()
    {
        // 6 random digits, never starts with 0
        int pin = QRandomGenerator::global()->bounded(100000, 999999);
        m_pinEdit->setText(QString::number(pin));
    }

    void savePin()
    {
        QString pin = m_pinEdit->text().trimmed();
        if (pin.length() != 6) {
            QMessageBox::warning(this, "Invalid PIN", "PIN must be exactly 6 digits.");
            return;
        }
        int days = m_daysSpin->value();

        QSqlDatabase db = QSqlDatabase::database("oracle_conn");
        QSqlQuery q(db);
        q.prepare(QString(
                      "UPDATE EMPLOYE SET PIN_CODE = :pin, PIN_EXPIRES = SYSDATE + %1 "
                      "WHERE CIN = :cin"
                      ).arg(days));
        q.bindValue(":pin", pin);
        q.bindValue(":cin", m_cin);

        if (!q.exec()) {
            QMessageBox::critical(this, "Error", "Failed to save PIN:\n" + q.lastError().text());
            return;
        }

        QMessageBox::information(this, "Success",
                                 QString("PIN %1 assigned to employee.\nValid for %2 day(s).").arg(pin).arg(days));
        loadCurrentPin();
    }

    void removePin()
    {
        if (QMessageBox::question(this, "Remove PIN",
                                  "Remove this employee's PIN?") != QMessageBox::Yes)
            return;

        QSqlDatabase db = QSqlDatabase::database("oracle_conn");
        QSqlQuery q(db);
        q.prepare("UPDATE EMPLOYE SET PIN_CODE = NULL, PIN_EXPIRES = NULL WHERE CIN = :cin");
        q.bindValue(":cin", m_cin);

        if (!q.exec()) {
            QMessageBox::critical(this, "Error", "Failed to remove PIN:\n" + q.lastError().text());
            return;
        }

        m_pinEdit->clear();
        m_infoLabel->setText("No PIN assigned.");
    }

private:
    void loadCurrentPin()
    {
        QSqlDatabase db = QSqlDatabase::database("oracle_conn");
        QSqlQuery q(db);
        q.prepare("SELECT PIN_CODE, TO_CHAR(PIN_EXPIRES,'YYYY-MM-DD') FROM EMPLOYE WHERE CIN = :cin");
        q.bindValue(":cin", m_cin);
        if (!q.exec() || !q.next() || q.value(0).isNull()) {
            m_infoLabel->setText("No PIN currently assigned.");
            return;
        }
        QString currentPin     = q.value(0).toString().trimmed();
        QString currentExpires = q.value(1).toString().trimmed();
        m_pinEdit->setText(currentPin);
        m_infoLabel->setText(QString("Current PIN: %1  |  Expires: %2")
                                 .arg(currentPin, currentExpires));
    }

    QString     m_cin;
    QLineEdit*  m_pinEdit  = nullptr;
    QSpinBox*   m_daysSpin = nullptr;
    QLabel*     m_infoLabel = nullptr;
};

