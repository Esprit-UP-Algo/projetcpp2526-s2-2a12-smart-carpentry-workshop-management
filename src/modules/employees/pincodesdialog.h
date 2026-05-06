#ifndef PINCODESDIALOG_H
#define PINCODESDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

// ─────────────────────────────────────────────────────────────────────────────
//  PinCodesDialog
//
//  Opened from EmployeeManagementPage with a pre-selected employee.
//  On construction it:
//    1. Reuses the existing oracle_conn (Connection::CONN_NAME) — no new connection.
//    2. Auto-creates ACCESS_CODES if it doesn't exist (silent INIT).
//    3. Loads and displays all PIN codes for the given CIN.
//
//  CRUD provided: Add (with expiry days), Remove single code, Extend expiry.
//  The dialog is themed to match WoodFlow's light/dark palette exactly.
// ─────────────────────────────────────────────────────────────────────────────
class PinCodesDialog : public QDialog
{
    Q_OBJECT

public:
    // Pass the employee's CIN and display name (e.g. "Jean Dupont").
    explicit PinCodesDialog(const QString& cin,
                            const QString& employeeName,
                            QWidget* parent = nullptr);
    ~PinCodesDialog() = default;

private slots:
    void onAddCode();
    void onRemoveCode();
    void onExtendCode();
    void onTableSelectionChanged();

private:
    // ── DB helpers (use Connection::CONN_NAME — already open) ────────────────
    bool    ensureTable();           // CREATE TABLE IF NOT EXISTS
    void    loadCodes();             // SELECT for this CIN

    QString cmdAddCode(const QString& pin, int days);
    QString cmdRemoveCode(const QString& pin);
    QString cmdExtendCode(const QString& pin, int days);

    // ── UI ────────────────────────────────────────────────────────────────────
    void    setupUI();
    void    applyTheme();
    void    setResult(const QString& msg);   // shows inline feedback label
    void    updateButtonStates();

    // ── Data ──────────────────────────────────────────────────────────────────
    QString m_cin;
    QString m_employeeName;

    // ── Widgets ───────────────────────────────────────────────────────────────
    QTableWidget* m_table       = nullptr;

    // Add-code form
    QLineEdit*    m_pinInput    = nullptr;
    QSpinBox*     m_daysInput   = nullptr;
    QPushButton*  m_addBtn      = nullptr;

    // Per-row actions
    QPushButton*  m_removeBtn   = nullptr;
    QSpinBox*     m_extDays     = nullptr;
    QPushButton*  m_extendBtn   = nullptr;

    // Feedback
    QLabel*       m_resultLabel = nullptr;
};

#endif // PINCODESDIALOG_H
