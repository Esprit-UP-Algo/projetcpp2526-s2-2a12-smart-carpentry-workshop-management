#ifndef FORGOTPASSWORDPAGE_H
#define FORGOTPASSWORDPAGE_H

#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include "../../models/employee.h"

/**
 * ForgotPasswordPage — 3-step password reset with smart 2FA
 *
 * Step 1 : Enter email → verified against DB
 * Step 2a: NO 2FA yet  → show QR to set up Microsoft Authenticator → save secret to DB
 * Step 2b: HAS 2FA     → skip QR, just ask for the 6-digit code
 * Step 3 : Enter new password → saved to DB
 */
class ForgotPasswordPage : public QWidget
{
    Q_OBJECT

public:
    explicit ForgotPasswordPage(QWidget *parent = nullptr);
    ~ForgotPasswordPage() = default;

signals:
    void switchToLogin();

private slots:
    void onCheckEmail();
    void onVerifyCode();
    void onResetPassword();
    void onQrImageReady(QNetworkReply *reply);

private:
    QStackedWidget *m_stack;

    // Step 1 — email
    QWidget     *m_step1;
    QLineEdit   *m_emailInput;
    QPushButton *m_checkEmailBtn;
    QLabel      *m_step1Error;

    // Step 2 — 2FA (QR shown only first time)
    QWidget     *m_step2;
    QLabel      *m_qrLabel;
    QLabel      *m_qrInstruction;
    QLabel      *m_manualKeyLabel;  // shows base32 key for manual entry (new setup only)
    QLineEdit   *m_totpInput;
    QPushButton *m_verifyBtn;
    QLabel      *m_step2Error;

    // Step 3 — new password
    QWidget     *m_step3;
    QLineEdit   *m_newPassInput;
    QLineEdit   *m_confirmPassInput;
    QPushButton *m_resetBtn;
    QLabel      *m_step3Error;

    // State
    Employee m_employee;
    QString  m_totpSecret;          // working secret for this session
    quint64  m_lastUsedCounter = 0;
    bool     m_isNewSetup = false;  // true = first-time QR setup, false = existing secret

    QNetworkAccessManager *m_nam = nullptr;

    // UI helpers
    void setupUI();
    QWidget* buildStep1();
    QWidget* buildStep2();
    QWidget* buildStep3();
    void configureStep2ForNewSetup();
    void configureStep2ForExistingSecret();

    // TOTP
    void generateTotpSecret();
    void renderQrCode();
    bool verifyTotp(const QString& code);

    QByteArray hmacSha1(const QByteArray& key, const QByteArray& msg) const;
    quint32    hotp(const QByteArray& secret, quint64 counter) const;
    quint32    totp(const QByteArray& secret, int window = 0) const;
    static QByteArray base32Decode(const QString& encoded);
    static QString    base32Encode(const QByteArray& data);
};

#endif // FORGOTPASSWORDPAGE_H
