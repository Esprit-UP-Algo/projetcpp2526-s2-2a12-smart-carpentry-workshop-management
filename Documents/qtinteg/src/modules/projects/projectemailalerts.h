#ifndef PROJECTEMAILALERTS_H
#define PROJECTEMAILALERTS_H

/*  ProjectEmailAlerts
 *  ──────────────────────────────────────────────────────────────
 *  Opens a dialog listing overdue projects.
 *  User picks one, sees the chef's email, and clicks Send.
 *
 *  Requires: QT += network
 *  SMTP: Gmail port 465 SSL (no external API)
 */

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include "src/models/projet.h"

class ProjectEmailAlertDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectEmailAlertDialog(QWidget *parent = nullptr);

private slots:
    void onProjectSelected();
    void onSendClicked();

private:
    void setupUI();
    bool sendMail(const QString& toAddress, const QString& toName,
                  const QString& subject,   const QString& body,
                  QString& errorOut);

    QListWidget  *m_projectList;
    QLabel       *m_chefNameLabel;
    QLabel       *m_chefEmailLabel;
    QLabel       *m_deadlineLabel;
    QPushButton  *m_sendBtn;
    QLabel       *m_statusLabel;

    QList<Projet> m_overdueProjects;

    // ── SMTP config ── change these ──────────────────────────
    static constexpr const char* SMTP_HOST   = "smtp.gmail.com";
    static constexpr int         SMTP_PORT   = 465;
    static constexpr const char* SMTP_USER   = "salhisouha65@gmail.com";
    static constexpr const char* SMTP_PASS   = "qwmy lwki ytnf dlxd";
    static constexpr const char* SENDER_NAME = "WoodFlow";
};

#endif // PROJECTEMAILALERTS_H
