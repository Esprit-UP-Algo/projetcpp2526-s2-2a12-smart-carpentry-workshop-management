#include "forgotpasswordpage.h"
#include "../../database/employeedatabase.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QStackedWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QDateTime>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QUrl>
#include <QDebug>
#include <cstring>

// ── small UI helpers ─────────────────────────────────────────────────────────
static QFrame* makeCard() {
    QFrame *c = new QFrame;
    c->setObjectName("auth_card");
    c->setMinimumWidth(440); c->setMaximumWidth(440);
    return c;
}
static QLabel* makeTitle(const QString& t) {
    QLabel *l = new QLabel(t); l->setObjectName("auth_title");
    l->setAlignment(Qt::AlignCenter); return l;
}
static QLabel* makeSubtitle(const QString& t) {
    QLabel *l = new QLabel(t); l->setObjectName("auth_subtitle");
    l->setAlignment(Qt::AlignCenter); l->setWordWrap(true); return l;
}
static QLabel* makeError() {
    QLabel *l = new QLabel; l->setObjectName("authError");
    l->setAlignment(Qt::AlignCenter); l->setWordWrap(true);
    l->setStyleSheet("color:#ef4444;font-size:12px;background:transparent;");
    l->hide(); return l;
}
static QLineEdit* makeInput(const QString& ph, bool pw = false) {
    QLineEdit *e = new QLineEdit; e->setPlaceholderText(ph); e->setFixedHeight(42);
    if (pw) e->setEchoMode(QLineEdit::Password); return e;
}
static QPushButton* makePrimaryBtn(const QString& t) {
    QPushButton *b = new QPushButton(t); b->setObjectName("primary_button");
    b->setFixedHeight(44); b->setCursor(Qt::PointingHandCursor); return b;
}
static QPushButton* makeTextBtn(const QString& t) {
    QPushButton *b = new QPushButton(t); b->setObjectName("text_button");
    b->setCursor(Qt::PointingHandCursor); return b;
}

// ── constructor ───────────────────────────────────────────────────────────────
ForgotPasswordPage::ForgotPasswordPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ForgotPasswordPage::setupUI()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(buildStep1());   // 0
    m_stack->addWidget(buildStep2());   // 1
    m_stack->addWidget(buildStep3());   // 2
    m_stack->setCurrentIndex(0);
    root->addStretch();
    root->addWidget(m_stack, 0, Qt::AlignCenter);
    root->addStretch();
}

// ── Step 1 : email input ──────────────────────────────────────────────────────
QWidget* ForgotPasswordPage::buildStep1()
{
    m_step1 = new QWidget;
    QVBoxLayout *outer = new QVBoxLayout(m_step1);
    outer->setContentsMargins(0,0,0,0);
    QFrame *card = makeCard();
    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(45,50,45,50); lay->setSpacing(6);

    lay->addWidget(makeTitle("Mot de passe oublié"));
    lay->addWidget(makeSubtitle("Entrez votre adresse email professionnelle.\nNous vérifierons votre compte."));
    lay->addSpacing(24);
    lay->addWidget(new QLabel("Adresse email"));
    m_emailInput = makeInput("prenom.nom@woodflow.tn");
    lay->addWidget(m_emailInput);
    lay->addSpacing(8);
    m_step1Error = makeError(); lay->addWidget(m_step1Error);
    lay->addSpacing(16);
    m_checkEmailBtn = makePrimaryBtn("Vérifier mon email");
    lay->addWidget(m_checkEmailBtn);
    lay->addSpacing(8);
    QPushButton *backBtn = makeTextBtn("← Retour à la connexion");
    lay->addWidget(backBtn);
    outer->addWidget(card, 0, Qt::AlignCenter);

    connect(m_checkEmailBtn, &QPushButton::clicked, this, &ForgotPasswordPage::onCheckEmail);
    connect(m_emailInput, &QLineEdit::returnPressed, this, &ForgotPasswordPage::onCheckEmail);
    connect(backBtn, &QPushButton::clicked, this, &ForgotPasswordPage::switchToLogin);
    return m_step1;
}

// ── Step 2 : 2FA (QR first-time OR just code if already set up) ───────────────
QWidget* ForgotPasswordPage::buildStep2()
{
    m_step2 = new QWidget;
    QVBoxLayout *outer = new QVBoxLayout(m_step2);
    outer->setContentsMargins(0,0,0,0);
    QFrame *card = new QFrame;
    card->setObjectName("auth_card");
    card->setMinimumWidth(400); card->setMaximumWidth(400);
    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(45,30,45,30); lay->setSpacing(8);

    lay->addWidget(makeTitle("Vérification 2FA"));

    // Short scan instruction — always visible
    m_qrInstruction = makeSubtitle("Ouvrez Microsoft Authenticator et entrez le code.");
    lay->addWidget(m_qrInstruction);

    // QR block — shown only for first-time setup, hidden when secret already exists
    m_qrLabel = new QLabel;
    m_qrLabel->setObjectName("qrLabel");
    m_qrLabel->setAlignment(Qt::AlignCenter);
    m_qrLabel->setMinimumSize(300, 300);
    m_qrLabel->setMaximumSize(300, 300);
    m_qrLabel->setStyleSheet("border:1px solid #e5e7eb; background:white; border-radius:4px;");
    lay->addWidget(m_qrLabel, 0, Qt::AlignCenter);

    // Manual key label — separate from instruction, shown only for new setup
    m_manualKeyLabel = new QLabel;
    m_manualKeyLabel->setObjectName("auth_subtitle");
    m_manualKeyLabel->setAlignment(Qt::AlignCenter);
    m_manualKeyLabel->setWordWrap(true);
    m_manualKeyLabel->setStyleSheet(
        "font-size:11px; color:#6b7280; background:transparent; "
        "padding:6px; border:1px dashed #d1d5db; border-radius:4px;");
    lay->addWidget(m_manualKeyLabel);

    lay->addWidget(new QLabel("Code de vérification (6 chiffres)"));
    m_totpInput = makeInput("000000");
    m_totpInput->setMaxLength(6);
    m_totpInput->setAlignment(Qt::AlignCenter);
    QFont f = m_totpInput->font();
    f.setLetterSpacing(QFont::AbsoluteSpacing, 6); f.setPointSize(16);
    m_totpInput->setFont(f);
    lay->addWidget(m_totpInput);

    m_step2Error = makeError(); lay->addWidget(m_step2Error);
    m_verifyBtn = makePrimaryBtn("Vérifier le code");
    lay->addWidget(m_verifyBtn);
    QPushButton *backBtn = makeTextBtn("← Retour");
    lay->addWidget(backBtn);
    outer->addWidget(card, 0, Qt::AlignCenter);

    connect(m_verifyBtn, &QPushButton::clicked, this, &ForgotPasswordPage::onVerifyCode);
    connect(m_totpInput, &QLineEdit::returnPressed, this, &ForgotPasswordPage::onVerifyCode);
    connect(backBtn, &QPushButton::clicked, [this](){ m_stack->setCurrentIndex(0); });
    return m_step2;
}

// ── Step 3 : new password ─────────────────────────────────────────────────────
QWidget* ForgotPasswordPage::buildStep3()
{
    m_step3 = new QWidget;
    QVBoxLayout *outer = new QVBoxLayout(m_step3);
    outer->setContentsMargins(0,0,0,0);
    QFrame *card = makeCard();
    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(45,50,45,50); lay->setSpacing(6);

    lay->addWidget(makeTitle("Nouveau mot de passe"));
    lay->addWidget(makeSubtitle("Choisissez un nouveau mot de passe sécurisé."));
    lay->addSpacing(24);
    lay->addWidget(new QLabel("Nouveau mot de passe"));
    m_newPassInput = makeInput("Minimum 6 caractères", true);
    lay->addWidget(m_newPassInput);
    lay->addSpacing(12);
    lay->addWidget(new QLabel("Confirmer le mot de passe"));
    m_confirmPassInput = makeInput("Répétez le mot de passe", true);
    lay->addWidget(m_confirmPassInput);
    lay->addSpacing(8);
    m_step3Error = makeError(); lay->addWidget(m_step3Error);
    lay->addSpacing(16);
    m_resetBtn = makePrimaryBtn("Réinitialiser le mot de passe");
    lay->addWidget(m_resetBtn);
    outer->addWidget(card, 0, Qt::AlignCenter);

    connect(m_resetBtn, &QPushButton::clicked, this, &ForgotPasswordPage::onResetPassword);
    connect(m_confirmPassInput, &QLineEdit::returnPressed, this, &ForgotPasswordPage::onResetPassword);
    return m_step3;
}

// ── configure step 2 appearance ───────────────────────────────────────────────

// First time: generate new secret, show QR
void ForgotPasswordPage::configureStep2ForNewSetup()
{
    m_isNewSetup = true;
    generateTotpSecret();

    m_qrInstruction->setText("Scannez ce QR avec Microsoft Authenticator,\npuis entrez le code à 6 chiffres.");
    m_qrLabel->show();
    m_qrLabel->setText("⏳ Chargement...");
    m_manualKeyLabel->setText(QString("Clé manuelle : %1").arg(m_totpSecret));
    m_manualKeyLabel->show();

    renderQrCode();
}

// Returning user: secret already in DB — just ask for code, no QR
void ForgotPasswordPage::configureStep2ForExistingSecret()
{
    m_isNewSetup      = false;
    m_totpSecret      = m_employee.getTotpSecret();
    m_lastUsedCounter = 0;

    m_qrInstruction->setText(
        QString("Ouvrez Microsoft Authenticator\net entrez le code WoodFlow pour\n%1")
            .arg(m_employee.getEmail()));
    m_qrLabel->hide();
    m_qrLabel->clear();
    m_manualKeyLabel->hide();
}

// ── Step 1 slot ───────────────────────────────────────────────────────────────
void ForgotPasswordPage::onCheckEmail()
{
    m_step1Error->hide();
    QString email = m_emailInput->text().trimmed();

    if (email.isEmpty() || !email.contains('@') || !email.contains('.')) {
        m_step1Error->setText("Veuillez entrer une adresse email valide.");
        m_step1Error->show(); return;
    }

    // Look up employee by email
    m_employee = Employee();
    for (const Employee& e : EmployeeDatabase::instance().getAllEmployees()) {
        if (e.getEmail().trimmed().toLower() == email.toLower()) {
            m_employee = e; break;
        }
    }

    if (!m_employee.isValid()) {
        m_step1Error->setText(
            "Aucun compte trouvé avec cette adresse email.\n"
            "Vérifiez l'adresse ou contactez un administrateur.");
        m_step1Error->show(); return;
    }

    m_totpInput->clear();
    m_step2Error->hide();

    // Smart branch: does this account already have 2FA?
    if (m_employee.hasTwoFactor()) {
        configureStep2ForExistingSecret();   // just ask for code
    } else {
        configureStep2ForNewSetup();         // show QR, set it up
    }

    m_stack->setCurrentIndex(1);
}

// ── Step 2 slot ───────────────────────────────────────────────────────────────
void ForgotPasswordPage::onVerifyCode()
{
    m_step2Error->hide();
    QString code = m_totpInput->text().trimmed();

    if (code.length() != 6) {
        m_step2Error->setText("Le code doit contenir exactement 6 chiffres.");
        m_step2Error->show(); return;
    }

    if (!verifyTotp(code)) {
        m_step2Error->setText("Code incorrect ou expiré. Réessayez avec un nouveau code.");
        m_step2Error->show(); m_totpInput->clear(); m_totpInput->setFocus(); return;
    }

    // First-time setup: persist the secret to DB so next time QR is skipped
    if (m_isNewSetup) {
        if (!EmployeeDatabase::instance().saveTotpSecret(m_employee.getId(), m_totpSecret)) {
            m_step2Error->setText("Erreur lors de la sauvegarde du secret 2FA. Réessayez.");
            m_step2Error->show(); return;
        }
        qDebug() << "[ForgotPassword] 2FA secret saved for employee" << m_employee.getId();
    }

    m_newPassInput->clear();
    m_confirmPassInput->clear();
    m_step3Error->hide();
    m_stack->setCurrentIndex(2);
}

// ── Step 3 slot ───────────────────────────────────────────────────────────────
void ForgotPasswordPage::onResetPassword()
{
    m_step3Error->hide();
    QString pass = m_newPassInput->text(), confirm = m_confirmPassInput->text();

    if (pass.length() < 6) {
        m_step3Error->setText("Le mot de passe doit contenir au moins 6 caractères.");
        m_step3Error->show(); return;
    }
    if (pass != confirm) {
        m_step3Error->setText("Les mots de passe ne correspondent pas.");
        m_step3Error->show(); return;
    }

    m_employee.setMotDePasse(pass);
    if (!EmployeeDatabase::instance().updateEmployee(m_employee)) {
        m_step3Error->setText("Erreur lors de la mise à jour. Contactez un administrateur.");
        m_step3Error->show(); return;
    }

    QMessageBox::information(this, "Succès",
        "Mot de passe réinitialisé avec succès !\nVous pouvez maintenant vous connecter.");

    // Reset page state for next use
    m_emailInput->clear();
    m_stack->setCurrentIndex(0);
    emit switchToLogin();
}

// ── QR code fetch ─────────────────────────────────────────────────────────────
void ForgotPasswordPage::generateTotpSecret()
{
    QByteArray raw(20, 0);
    for (int i = 0; i < 20; ++i) raw[i] = (char)(QRandomGenerator::global()->bounded(256));
    m_totpSecret      = base32Encode(raw);
    m_lastUsedCounter = 0;
}

void ForgotPasswordPage::renderQrCode()
{
    // Build URI with string concatenation — avoids QString::arg() misinterpreting
    // '%' characters in the email or secret as format specifiers
    QString uri = "otpauth://totp/WoodFlow:"
                  + m_employee.getEmail()
                  + "?secret=" + m_totpSecret
                  + "&issuer=WoodFlow&algorithm=SHA1&digits=6&period=30";

    qDebug() << "[ForgotPassword] otpauth URI:" << uri;

    // Encode ONLY the uri value for the query string — single pass, no double encoding
    // Build as raw string so Qt never touches the already-encoded data value
    QString apiUrlStr = "https://api.qrserver.com/v1/create-qr-code/?size=400x400&ecc=M&margin=4&format=png&data="
                        + QString::fromUtf8(QUrl::toPercentEncoding(uri));

    qDebug() << "[ForgotPassword] API URL:" << apiUrlStr;

    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        connect(m_nam, &QNetworkAccessManager::finished,
                this, &ForgotPasswordPage::onQrImageReady);
    }

    QUrl qurl(apiUrlStr);
    QNetworkRequest req(qurl);
    req.setRawHeader("User-Agent", "WoodFlow/1.0");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    m_nam->get(req);
}

void ForgotPasswordPage::onQrImageReady(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        m_qrLabel->setText("❌ Réseau indisponible\nUtilisez la clé manuelle ci-dessus.");
        qDebug() << "[ForgotPassword] QR fetch error:" << reply->errorString();
        return;
    }
    QPixmap qr;
    if (qr.loadFromData(reply->readAll(), "PNG")) {
        m_qrLabel->setPixmap(qr.scaled(300, 300, Qt::KeepAspectRatio, Qt::FastTransformation));
        m_qrLabel->setText("");
    } else {
        m_qrLabel->setText("Impossible de décoder le QR.\nUtilisez la clé manuelle.");
    }
}

// ── TOTP (RFC 6238) ───────────────────────────────────────────────────────────
bool ForgotPasswordPage::verifyTotp(const QString& code)
{
    QByteArray secret = base32Decode(m_totpSecret);
    bool ok;
    quint32 entered = code.toUInt(&ok);
    if (!ok) return false;

    quint64 now = (quint64)QDateTime::currentSecsSinceEpoch() / 30;
    for (int delta = -1; delta <= 1; ++delta) {
        quint64 counter = now + (quint64)delta;
        if (counter == m_lastUsedCounter) continue;
        if (totp(secret, delta) == entered) { m_lastUsedCounter = counter; return true; }
    }
    return false;
}

QByteArray ForgotPasswordPage::hmacSha1(const QByteArray& key, const QByteArray& msg) const
{
    return QMessageAuthenticationCode::hash(msg, key, QCryptographicHash::Sha1);
}

quint32 ForgotPasswordPage::hotp(const QByteArray& secret, quint64 counter) const
{
    QByteArray msg(8, 0);
    for (int i = 7; i >= 0; --i) { msg[i] = (char)(counter & 0xFF); counter >>= 8; }
    QByteArray hash = hmacSha1(secret, msg);
    int offset = (quint8)hash[hash.size()-1] & 0x0F;
    quint32 bin = ((quint8)hash[offset]   & 0x7F) << 24
                | ((quint8)hash[offset+1] & 0xFF) << 16
                | ((quint8)hash[offset+2] & 0xFF) << 8
                | ((quint8)hash[offset+3] & 0xFF);
    return bin % 1000000;
}

quint32 ForgotPasswordPage::totp(const QByteArray& secret, int window) const
{
    quint64 counter = (quint64)QDateTime::currentSecsSinceEpoch() / 30 + (quint64)window;
    return hotp(secret, counter);
}

QString ForgotPasswordPage::base32Encode(const QByteArray& data)
{
    static const char* A = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    QString out; int buf = 0, bits = 0;
    for (quint8 b : data) {
        buf = (buf << 8) | b; bits += 8;
        while (bits >= 5) { bits -= 5; out += A[(buf >> bits) & 0x1F]; }
    }
    if (bits > 0) out += A[(buf << (5-bits)) & 0x1F];
    return out;
}

QByteArray ForgotPasswordPage::base32Decode(const QString& encoded)
{
    static const char* A = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    QByteArray out; int buf = 0, bits = 0;
    for (QChar ch : encoded.toUpper()) {
        const char* p = strchr(A, ch.toLatin1()); if (!p) continue;
        buf = (buf << 5) | (int)(p - A); bits += 5;
        if (bits >= 8) { bits -= 8; out += (char)((buf >> bits) & 0xFF); }
    }
    return out;
}
