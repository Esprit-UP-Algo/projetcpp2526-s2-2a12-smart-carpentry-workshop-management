#include "projectemailalerts.h"
#include "src/database/projectdatabase.h"
#include "src/database/connection.h"

#include <QApplication>
#include <QSslSocket>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QSplitter>
#include <QScrollArea>

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
ProjectEmailAlertDialog::ProjectEmailAlertDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Alertes Deadline — Envoyer un Email");
    setMinimumSize(680, 460);
    setModal(true);
    setStyleSheet("QDialog { background:#f1f5f9; }");
    setupUI();
}

// ─────────────────────────────────────────────────────────────
//  UI
// ─────────────────────────────────────────────────────────────
void ProjectEmailAlertDialog::setupUI()
{
    // Load overdue projects from DB
    QList<Projet> all = ProjectDatabase::instance().getAllProjets();
    for (const Projet& p : all)
        if (p.isOverdue())
            m_overdueProjects.append(p);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    // Title
    QLabel *title = new QLabel("📧  Alertes Deadline", this);
    title->setStyleSheet("font-size:18px; font-weight:800; color:#1e293b;");
    mainLayout->addWidget(title);

    QLabel *subtitle = new QLabel(
        QString("%1 projet(s) en retard — sélectionnez un projet pour envoyer une alerte au chef.")
            .arg(m_overdueProjects.size()), this);
    subtitle->setStyleSheet("font-size:12px; color:#64748b;");
    mainLayout->addWidget(subtitle);

    // ── Horizontal split ─────────────────────────────────────
    QHBoxLayout *split = new QHBoxLayout();
    split->setSpacing(16);

    // Left: project list
    QFrame *leftCard = new QFrame(this);
    leftCard->setStyleSheet(
        "QFrame{background:white;border-radius:10px;border:1px solid #e2e8f0;}");
    QVBoxLayout *ll = new QVBoxLayout(leftCard);
    ll->setContentsMargins(12, 12, 12, 12);
    ll->setSpacing(8);

    QLabel *listTitle = new QLabel("Projets en retard", leftCard);
    listTitle->setStyleSheet("font-weight:700; font-size:13px; color:#374151;");
    ll->addWidget(listTitle);

    m_projectList = new QListWidget(leftCard);
    m_projectList->setStyleSheet(
        "QListWidget{border:none; background:transparent; font-size:12px;}"
        "QListWidget::item{padding:10px 8px; border-bottom:1px solid #f1f5f9; border-radius:6px;}"
        "QListWidget::item:selected{background:#fee2e2; color:#991b1b;}"
        "QListWidget::item:hover{background:#fef2f2;}");

    if (m_overdueProjects.isEmpty()) {
        m_projectList->addItem("Aucun projet en retard.");
    } else {
        for (const Projet& p : m_overdueProjects) {
            int daysLate = -p.daysRemaining();
            QString label = QString(" %1\n    Client: %2  |  %3 jour(s) de retard")
                                .arg(p.getNom())
                                .arg(p.getClient())
                                .arg(daysLate);
            QListWidgetItem *item = new QListWidgetItem(label);
            item->setForeground(QColor("#991b1b"));
            m_projectList->addItem(item);
        }
    }
    ll->addWidget(m_projectList);
    split->addWidget(leftCard, 1);

    // Right: detail + send panel
    QFrame *rightCard = new QFrame(this);
    rightCard->setStyleSheet(
        "QFrame{background:white;border-radius:10px;border:1px solid #e2e8f0;}");
    QVBoxLayout *rl = new QVBoxLayout(rightCard);
    rl->setContentsMargins(20, 16, 20, 16);
    rl->setSpacing(12);

    QLabel *detailTitle = new QLabel("Détails du destinataire", rightCard);
    detailTitle->setStyleSheet("font-weight:700; font-size:13px; color:#374151;");
    rl->addWidget(detailTitle);

    QFrame *sep = new QFrame(rightCard);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#e2e8f0; max-height:1px; border:none;");
    rl->addWidget(sep);

    // Info rows
    auto makeInfoRow = [&](const QString& label, QLabel*& valueLabel) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lbl = new QLabel(label, rightCard);
        lbl->setStyleSheet("font-size:12px; font-weight:600; color:#6b7280; min-width:100px;");
        valueLabel = new QLabel("—", rightCard);
        valueLabel->setStyleSheet("font-size:12px; color:#1e293b;");
        valueLabel->setWordWrap(true);
        row->addWidget(lbl);
        row->addWidget(valueLabel, 1);
        rl->addLayout(row);
    };

    QLabel *projectNameLabel = nullptr;  // we'll reuse m_ labels for chef info
    makeInfoRow("Projet :",   projectNameLabel);   // placeholder, we'll update via slot
    makeInfoRow("Deadline :", m_deadlineLabel);
    makeInfoRow("Chef :",     m_chefNameLabel);
    makeInfoRow("Email :",    m_chefEmailLabel);

    // store project name label for updating
    // (simpler: just keep it via the list — we update all in onProjectSelected)
    // We'll replace projectNameLabel with a member
    // Re-declare properly:
    m_chefEmailLabel->setStyleSheet(
        "font-size:12px; color:#1d4ed8; font-weight:600;");

    rl->addStretch();

    // Status label (shows success/error after send)
    m_statusLabel = new QLabel("", rightCard);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("font-size:12px; color:#6b7280;");
    rl->addWidget(m_statusLabel);

    // Send button
    m_sendBtn = new QPushButton(" Envoyer l'alerte", rightCard);
    m_sendBtn->setFixedHeight(40);
    m_sendBtn->setEnabled(false);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(
        "QPushButton{background:#e67e22;color:white;border:none;border-radius:8px;"
        "font-size:13px;font-weight:700;}"
        "QPushButton:hover{background:#f39c12;}"
        "QPushButton:disabled{background:#d1d5db;color:#9ca3af;}");
    rl->addWidget(m_sendBtn);

    split->addWidget(rightCard, 1);
    mainLayout->addLayout(split, 1);

    // Close button
    QPushButton *closeBtn = new QPushButton("Fermer", this);
    closeBtn->setFixedHeight(36);
    closeBtn->setStyleSheet(
        "QPushButton{background:#e2e8f0;color:#374151;border:none;border-radius:8px;"
        "font-size:13px;font-weight:600;padding:0 24px;}"
        "QPushButton:hover{background:#cbd5e0;}");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    // Signals
    connect(m_projectList, &QListWidget::currentRowChanged,
            this, &ProjectEmailAlertDialog::onProjectSelected);
    connect(m_sendBtn, &QPushButton::clicked,
            this, &ProjectEmailAlertDialog::onSendClicked);
}

// ─────────────────────────────────────────────────────────────
//  Slot: project selected → fill detail panel
// ─────────────────────────────────────────────────────────────
void ProjectEmailAlertDialog::onProjectSelected()
{
    int row = m_projectList->currentRow();
    if (row < 0 || row >= m_overdueProjects.size()) return;

    const Projet& p = m_overdueProjects[row];

    m_deadlineLabel->setText(p.getDeadline().toString("dd/MM/yyyy")
                             + QString("  (%1 jour(s) de retard)").arg(-p.daysRemaining()));

    // Get chef info from DB
    QString chefName  = ProjectDatabase::instance().getEmployeeName(p.getChefProjet());
    QString chefEmail = "";

    if (p.getChefProjet() != 0) {
        QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
        QSqlQuery q(db);
        q.prepare("SELECT EMAIL_EMP FROM EMPLOYE WHERE ID_EMP = :id");
        q.bindValue(":id", p.getChefProjet());
        if (q.exec() && q.next())
            chefEmail = q.value(0).toString().trimmed();
    }

    m_chefNameLabel->setText(chefName.isEmpty() ? "—" : chefName);

    if (chefEmail.isEmpty()) {
        m_chefEmailLabel->setText("Aucun email enregistré");
        m_chefEmailLabel->setStyleSheet("font-size:12px; color:#ef4444; font-weight:600;");
        m_sendBtn->setEnabled(false);
        m_statusLabel->setText(" Pas d'email pour ce chef de projet.");
    } else {
        m_chefEmailLabel->setText(chefEmail);
        m_chefEmailLabel->setStyleSheet("font-size:12px; color:#1d4ed8; font-weight:600;");
        m_sendBtn->setEnabled(true);
        m_statusLabel->setText("");
    }
}

// ─────────────────────────────────────────────────────────────
//  Slot: send clicked
// ─────────────────────────────────────────────────────────────
void ProjectEmailAlertDialog::onSendClicked()
{
    int row = m_projectList->currentRow();
    if (row < 0 || row >= m_overdueProjects.size()) return;

    const Projet& p = m_overdueProjects[row];

    QString chefName  = m_chefNameLabel->text();
    QString chefEmail = m_chefEmailLabel->text();
    int daysLate      = -p.daysRemaining();

    QString subject = QString("[WoodFlow] Projet en retard : %1").arg(p.getNom());
    QString body =
        QString("Bonjour %1,\n\n"
                "Ceci est une alerte automatique de WoodFlow SARL.\n\n"
                "Le projet dont vous êtes responsable est en retard :\n\n"
                "  Projet   : %2\n"
                "  Client   : %3\n"
                "  Deadline : %4\n"
                "  Retard   : %5 jour(s)\n"
                "  Statut   : %6\n\n"
                "Merci de prendre les mesures nécessaires.\n\n"
                "Cordialement,\n"
                "WoodFlow SARL\n")
            .arg(chefName)
            .arg(p.getNom())
            .arg(p.getClient())
            .arg(p.getDeadline().toString("dd/MM/yyyy"))
            .arg(daysLate)
            .arg(p.getStatus());

    m_sendBtn->setEnabled(false);
    m_sendBtn->setText("Envoi en cours…");
    m_statusLabel->setText("");
    QApplication::processEvents();  // refresh UI before blocking SMTP call

    QString err;
    bool ok = sendMail(chefEmail, chefName, subject, body, err);

    m_sendBtn->setEnabled(true);
    m_sendBtn->setText("Envoyer l'alerte");

    if (ok) {
        m_statusLabel->setStyleSheet("font-size:12px; color:#16a34a; font-weight:600;");
        m_statusLabel->setText(" Email envoyé avec succès à " + chefEmail);
    } else {
        m_statusLabel->setStyleSheet("font-size:12px; color:#ef4444;");
        m_statusLabel->setText(" Échec : " + err);
    }
}

// ─────────────────────────────────────────────────────────────
//  SMTP sender
// ─────────────────────────────────────────────────────────────
bool ProjectEmailAlertDialog::sendMail(const QString& toAddress,
                                       const QString& toName,
                                       const QString& subject,
                                       const QString& body,
                                       QString& errorOut)
{
    QByteArray user64 = QByteArray(SMTP_USER).toBase64();
    QByteArray pass64 = QByteArray(SMTP_PASS).toBase64();

    QSslSocket socket;
    socket.connectToHostEncrypted(SMTP_HOST, SMTP_PORT);

    if (!socket.waitForEncrypted(8000)) {
        errorOut = "Connexion SSL échouée : " + socket.errorString();
        return false;
    }

    auto recv = [&]() -> QString {
        socket.waitForReadyRead(5000);
        return QString::fromUtf8(socket.readAll());
    };
    auto send = [&](const QString& cmd) {
        socket.write((cmd + "\r\n").toUtf8());
        socket.waitForBytesWritten(3000);
    };

    recv(); // 220 banner
    send("EHLO woodflow.local");
    recv();
    send("AUTH LOGIN");
    recv();
    send(QString::fromUtf8(user64));
    recv();
    send(QString::fromUtf8(pass64));
    QString authResp = recv();
    if (!authResp.startsWith("235")) {
        errorOut = "Authentification échouée. Vérifiez SMTP_USER / SMTP_PASS.";
        return false;
    }

    send(QString("MAIL FROM:<%1>").arg(SMTP_USER));
    recv();
    send(QString("RCPT TO:<%1>").arg(toAddress));
    QString rcptResp = recv();
    if (!rcptResp.startsWith("250")) {
        errorOut = "Destinataire refusé : " + rcptResp;
        return false;
    }

    send("DATA");
    recv();

    QString date = QDateTime::currentDateTimeUtc().toString("ddd, dd MMM yyyy hh:mm:ss +0000");
    QString msg =
        QString("Date: %1\r\n"
                "From: %2 <%3>\r\n"
                "To: %4 <%5>\r\n"
                "Subject: %6\r\n"
                "MIME-Version: 1.0\r\n"
                "Content-Type: text/plain; charset=UTF-8\r\n"
                "Content-Transfer-Encoding: 8bit\r\n"
                "\r\n"
                "%7\r\n"
                ".\r\n")
            .arg(date)
            .arg(SENDER_NAME).arg(SMTP_USER)
            .arg(toName).arg(toAddress)
            .arg(subject)
            .arg(body);

    socket.write(msg.toUtf8());
    socket.waitForBytesWritten(5000);
    QString dataResp = recv();
    if (!dataResp.startsWith("250")) {
        errorOut = "Erreur DATA : " + dataResp;
        return false;
    }

    send("QUIT");
    socket.waitForDisconnected(3000);
    return true;
}
