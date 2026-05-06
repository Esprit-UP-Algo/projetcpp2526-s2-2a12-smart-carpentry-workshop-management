// ─────────────────────────────────────────────────────────────────────────────
//  pincodesdialog.cpp
//
//  Drop-in Arduino PIN manager for WoodFlow.
//  • Reuses Connection::CONN_NAME — never opens a second DB connection.
//  • Silently calls INIT (CREATE TABLE IF NOT EXISTS) on open.
//  • Themed to match employeedialog.cpp: same dark-detection, same palette
//    variables, same object-name conventions (#empGroup, #empSecondaryBtn, …).
// ─────────────────────────────────────────────────────────────────────────────
#include "pincodesdialog.h"
#include "../../database/connection.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────

PinCodesDialog::PinCodesDialog(const QString& cin,
                               const QString& employeeName,
                               QWidget* parent)
    : QDialog(parent)
    , m_cin(cin)
    , m_employeeName(employeeName)
{
    setModal(true);
    setWindowTitle(QString("Codes PIN  —  %1").arg(employeeName));
    setMinimumSize(620, 520);
    setMaximumSize(760, 720);
    setObjectName("pinCodesDialog");

    // 1. Ensure the ACCESS_CODES table exists (silent, no popup on success).
    ensureTable();

    // 2. Build the UI (reads dark mode flag for theme).
    setupUI();
    applyTheme();

    // 3. Load existing codes for this employee.
    loadCodes();
    updateButtonStates();
}

// ─────────────────────────────────────────────────────────────────────────────
//  DB — table init
// ─────────────────────────────────────────────────────────────────────────────

bool PinCodesDialog::ensureTable()
{
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    if (!db.isOpen()) {
        qWarning() << "[PinCodesDialog] DB connection not open.";
        return false;
    }

    QSqlQuery check(db);
    check.prepare("SELECT COUNT(*) FROM user_tables WHERE table_name = 'ACCESS_CODES'");
    if (!check.exec() || !check.next()) {
        qWarning() << "[PinCodesDialog] ensureTable check failed:" << check.lastError().text();
        return false;
    }
    if (check.value(0).toInt() > 0)
        return true;   // already exists — nothing to do

    QSqlQuery create(db);
    create.prepare(
        "CREATE TABLE ACCESS_CODES ("
        "  CIN      VARCHAR2(20),"
        "  PIN_CODE CHAR(6),"
        "  EXPIRES  DATE,"
        "  PRIMARY KEY (CIN, PIN_CODE)"
        ")"
    );
    if (!create.exec()) {
        qWarning() << "[PinCodesDialog] CREATE TABLE failed:" << create.lastError().text();
        return false;
    }
    qDebug() << "[PinCodesDialog] ACCESS_CODES table created.";
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  DB — load codes for this CIN
// ─────────────────────────────────────────────────────────────────────────────

void PinCodesDialog::loadCodes()
{
    m_table->setRowCount(0);
    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    if (!db.isOpen()) return;

    QSqlQuery q(db);
    q.prepare(
        "SELECT PIN_CODE, "
        "       TO_CHAR(EXPIRES, 'DD/MM/YYYY') AS EXPIRES_STR, "
        "       CASE WHEN EXPIRES >= SYSDATE THEN 'Valide' ELSE 'Expire' END AS STATUT "
        "FROM ACCESS_CODES "
        "WHERE CIN = :cin "
        "ORDER BY EXPIRES DESC"
    );
    q.bindValue(":cin", m_cin);
    if (!q.exec()) {
        qWarning() << "[PinCodesDialog] loadCodes failed:" << q.lastError().text();
        return;
    }

    bool dark = qApp->styleSheet().contains("0f0f0f");
    while (q.next()) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setRowHeight(row, 44);

        auto mk = [](const QString& t) {
            auto* it = new QTableWidgetItem(t);
            it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            it->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            return it;
        };

        m_table->setItem(row, 0, mk(q.value("PIN_CODE").toString().trimmed()));
        m_table->setItem(row, 1, mk(q.value("EXPIRES_STR").toString()));

        QString status = q.value("STATUT").toString();
        auto* statusItem = mk(status);
        if (status == "Valide")
            statusItem->setForeground(QColor(dark ? "#86efac" : "#16a34a"));
        else
            statusItem->setForeground(QColor(dark ? "#fca5a5" : "#dc2626"));
        m_table->setItem(row, 2, statusItem);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DB — CRUD
// ─────────────────────────────────────────────────────────────────────────────

QString PinCodesDialog::cmdAddCode(const QString& pin, int days)
{
    if (pin.isEmpty())      return "ERR:PIN vide";
    if (pin.length() != 6)  return "ERR:Le PIN doit contenir exactement 6 chiffres";
    if (days < 1 || days > 365) return "ERR:Duree invalide (1–365 jours)";

    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);

    // Check if this PIN is already used by this employee
    QSqlQuery dupSelf(db);
    dupSelf.prepare("SELECT COUNT(*) FROM ACCESS_CODES WHERE CIN=:cin AND PIN_CODE=:pin");
    dupSelf.bindValue(":cin", m_cin);
    dupSelf.bindValue(":pin", pin);
    if (!dupSelf.exec() || !dupSelf.next()) return "ERR:Verification doublon echouee";
    if (dupSelf.value(0).toInt() > 0)       return "ERR:Ce PIN existe deja pour cet employe";

    // Check if this PIN is already used by ANY other employee (PINs must be globally unique)
    QSqlQuery dupGlobal(db);
    dupGlobal.prepare("SELECT CIN FROM ACCESS_CODES WHERE PIN_CODE=:pin AND CIN <> :cin AND ROWNUM=1");
    dupGlobal.bindValue(":pin", pin);
    dupGlobal.bindValue(":cin", m_cin);
    if (!dupGlobal.exec() || !dupGlobal.next()) { /* no match — safe to proceed */ }
    else return "ERR:Ce PIN est deja attribue a un autre employe";

    QSqlQuery q(db);
    q.prepare(QString("INSERT INTO ACCESS_CODES (CIN, PIN_CODE, EXPIRES) "
                      "VALUES (:cin, :pin, SYSDATE + %1)").arg(days));
    q.bindValue(":cin", m_cin);
    q.bindValue(":pin", pin);
    if (!q.exec())
        return "ERR:Insertion echouee — " + q.lastError().text();

    // Fetch confirmed expiry date
    QSqlQuery ex(db);
    ex.prepare("SELECT TO_CHAR(EXPIRES,'DD/MM/YYYY') FROM ACCESS_CODES "
               "WHERE CIN=:cin AND PIN_CODE=:pin");
    ex.bindValue(":cin", m_cin);
    ex.bindValue(":pin", pin);
    ex.exec(); ex.next();
    return QString("OK:Code ajoute — expire le %1").arg(ex.value(0).toString());
}

QString PinCodesDialog::cmdRemoveCode(const QString& pin)
{
    if (pin.isEmpty()) return "ERR:Aucun PIN selectionne";

    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    q.prepare("DELETE FROM ACCESS_CODES WHERE CIN=:cin AND PIN_CODE=:pin");
    q.bindValue(":cin", m_cin);
    q.bindValue(":pin", pin);
    if (!q.exec())
        return "ERR:Suppression echouee — " + q.lastError().text();
    if (q.numRowsAffected() == 0)
        return "ERR:Code introuvable";
    return "OK:Code supprime";
}

QString PinCodesDialog::cmdExtendCode(const QString& pin, int days)
{
    if (pin.isEmpty())          return "ERR:Aucun PIN selectionne";
    if (days < 1 || days > 365) return "ERR:Duree invalide (1–365 jours)";

    QSqlDatabase db = QSqlDatabase::database(Connection::CONN_NAME);
    QSqlQuery q(db);
    // Resets expiry to SYSDATE + days (same behaviour as the original bridge)
    q.prepare(QString("UPDATE ACCESS_CODES SET EXPIRES = SYSDATE + %1 "
                      "WHERE CIN=:cin AND PIN_CODE=:pin").arg(days));
    q.bindValue(":cin", m_cin);
    q.bindValue(":pin", pin);
    if (!q.exec())
        return "ERR:Mise a jour echouee — " + q.lastError().text();
    if (q.numRowsAffected() == 0)
        return "ERR:Code introuvable pour ce CIN/PIN";

    QSqlQuery ex(db);
    ex.prepare("SELECT TO_CHAR(EXPIRES,'DD/MM/YYYY') FROM ACCESS_CODES "
               "WHERE CIN=:cin AND PIN_CODE=:pin");
    ex.bindValue(":cin", m_cin);
    ex.bindValue(":pin", pin);
    ex.exec(); ex.next();
    return QString("OK:Prolonge de %1 jour(s) — nouvelle expiration : %2")
                .arg(days).arg(ex.value(0).toString());
}

// ─────────────────────────────────────────────────────────────────────────────
//  Slots
// ─────────────────────────────────────────────────────────────────────────────

void PinCodesDialog::onAddCode()
{
    QString pin = m_pinInput->text().trimmed();
    int days    = m_daysInput->value();
    QString res = cmdAddCode(pin, days);
    setResult(res);
    if (res.startsWith("OK")) {
        m_pinInput->clear();
        loadCodes();
    }
}

void PinCodesDialog::onRemoveCode()
{
    int row = m_table->currentRow();
    if (row < 0) return;
    QString pin = m_table->item(row, 0)->text().trimmed();

    auto reply = QMessageBox::question(this, "Confirmer la suppression",
        QString("Supprimer le code PIN %1 ?").arg(pin),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QString res = cmdRemoveCode(pin);
    setResult(res);
    if (res.startsWith("OK")) loadCodes();
    updateButtonStates();
}

void PinCodesDialog::onExtendCode()
{
    int row = m_table->currentRow();
    if (row < 0) return;
    QString pin = m_table->item(row, 0)->text().trimmed();
    int days    = m_extDays->value();
    QString res = cmdExtendCode(pin, days);
    setResult(res);
    if (res.startsWith("OK")) loadCodes();
}

void PinCodesDialog::onTableSelectionChanged()
{
    updateButtonStates();
}

void PinCodesDialog::updateButtonStates()
{
    bool has = m_table->currentRow() >= 0;
    m_removeBtn->setEnabled(has);
    m_extendBtn->setEnabled(has);
    m_extDays->setEnabled(has);
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI construction
// ─────────────────────────────────────────────────────────────────────────────

void PinCodesDialog::setupUI()
{
    auto* outerV = new QVBoxLayout(this);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->setSpacing(0);

    // ── Scroll area (matches employeedialog.cpp layout pattern) ──────────────
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* content    = new QWidget;
    auto* contentV   = new QVBoxLayout(content);
    contentV->setContentsMargins(20, 20, 20, 16);
    contentV->setSpacing(14);

    // ── Header — employee name + CIN ──────────────────────────────────────────
    auto* headerLabel = new QLabel(
        QString("<b>%1</b>  <span style='color:#9ca3af;font-size:12px;'>CIN : %2</span>")
            .arg(m_employeeName.toHtmlEscaped(), m_cin));
    headerLabel->setObjectName("pinHeader");
    headerLabel->setTextFormat(Qt::RichText);
    headerLabel->setContentsMargins(4, 0, 0, 4);
    contentV->addWidget(headerLabel);

    // ── Table card ────────────────────────────────────────────────────────────
    auto* tableGroup = new QGroupBox("Codes PIN actifs");
    tableGroup->setObjectName("empGroup");
    auto* tableV = new QVBoxLayout(tableGroup);
    tableV->setContentsMargins(12, 14, 12, 12);
    tableV->setSpacing(8);

    m_table = new QTableWidget(0, 3);
    m_table->setObjectName("empTable");
    m_table->setHorizontalHeaderLabels({"Code PIN", "Expiration", "Statut"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->setColumnWidth(0, 120);
    m_table->setColumnWidth(1, 140);
    m_table->verticalHeader()->hide();
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(false);
    m_table->setShowGrid(false);
    m_table->setMinimumHeight(160);
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &PinCodesDialog::onTableSelectionChanged);
    tableV->addWidget(m_table);

    // ── Row actions (Remove + Extend) ─────────────────────────────────────────
    auto* rowActH = new QHBoxLayout;
    rowActH->setSpacing(8);

    m_removeBtn = new QPushButton("Supprimer le code selectionne");
    m_removeBtn->setObjectName("empDangerBtn");
    m_removeBtn->setFixedHeight(36);
    m_removeBtn->setCursor(Qt::PointingHandCursor);
    m_removeBtn->setEnabled(false);
    connect(m_removeBtn, &QPushButton::clicked, this, &PinCodesDialog::onRemoveCode);

    auto* extLabel = new QLabel("Prolonger de");
    extLabel->setObjectName("empSearchLabel");

    m_extDays = new QSpinBox;
    m_extDays->setRange(1, 365);
    m_extDays->setValue(7);
    m_extDays->setSuffix(" j");
    m_extDays->setFixedHeight(36);
    m_extDays->setFixedWidth(80);
    m_extDays->setEnabled(false);

    m_extendBtn = new QPushButton("Prolonger");
    m_extendBtn->setObjectName("empSecondaryBtn");
    m_extendBtn->setFixedHeight(36);
    m_extendBtn->setCursor(Qt::PointingHandCursor);
    m_extendBtn->setEnabled(false);
    connect(m_extendBtn, &QPushButton::clicked, this, &PinCodesDialog::onExtendCode);

    rowActH->addWidget(m_removeBtn);
    rowActH->addStretch();
    rowActH->addWidget(extLabel);
    rowActH->addWidget(m_extDays);
    rowActH->addWidget(m_extendBtn);

    tableV->addLayout(rowActH);
    contentV->addWidget(tableGroup);

    // ── Add-code card ─────────────────────────────────────────────────────────
    auto* addGroup = new QGroupBox("Ajouter un nouveau code PIN");
    addGroup->setObjectName("empGroup");
    auto* addForm  = new QFormLayout(addGroup);
    addForm->setContentsMargins(16, 14, 16, 14);
    addForm->setSpacing(10);
    addForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    addForm->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_pinInput = new QLineEdit;
    m_pinInput->setPlaceholderText("6 chiffres");
    m_pinInput->setMaxLength(6);
    m_pinInput->setFixedHeight(38);
    m_pinInput->setEchoMode(QLineEdit::Password);
    // Accept digits only
    m_pinInput->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d{0,6}"), m_pinInput));
    addForm->addRow("Code PIN *", m_pinInput);

    m_daysInput = new QSpinBox;
    m_daysInput->setRange(1, 365);
    m_daysInput->setValue(7);
    m_daysInput->setSuffix(" jours");
    m_daysInput->setFixedHeight(38);
    m_daysInput->setFixedWidth(120);
    m_daysInput->setButtonSymbols(QAbstractSpinBox::NoButtons);

    // Wrap in HBox so it left-aligns instead of stretching full width
    auto* daysRow = new QHBoxLayout;
    daysRow->setContentsMargins(0, 0, 0, 0);
    daysRow->addWidget(m_daysInput);
    daysRow->addStretch();
    auto* daysWrapper = new QWidget;
    daysWrapper->setLayout(daysRow);
    addForm->addRow("Valide pendant", daysWrapper);

    m_addBtn = new QPushButton("+ Ajouter le code");
    m_addBtn->setObjectName("empAddBtn");
    m_addBtn->setFixedHeight(40);
    m_addBtn->setFixedWidth(180);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addBtn, &QPushButton::clicked, this, &PinCodesDialog::onAddCode);
    connect(m_pinInput, &QLineEdit::returnPressed, this, &PinCodesDialog::onAddCode);

    // Wrap in HBox so the button left-aligns and doesn't stretch full width
    auto* addBtnRow = new QHBoxLayout;
    addBtnRow->setContentsMargins(0, 4, 0, 0);
    addBtnRow->addWidget(m_addBtn);
    addBtnRow->addStretch();
    auto* addBtnWrapper = new QWidget;
    addBtnWrapper->setLayout(addBtnRow);
    addForm->addRow("", addBtnWrapper);

    contentV->addWidget(addGroup);

    // ── Result feedback label ─────────────────────────────────────────────────
    m_resultLabel = new QLabel;
    m_resultLabel->setObjectName("pinResult");
    m_resultLabel->setWordWrap(true);
    m_resultLabel->setContentsMargins(4, 0, 4, 0);
    m_resultLabel->hide();
    contentV->addWidget(m_resultLabel);

    contentV->addStretch();
    scroll->setWidget(content);
    outerV->addWidget(scroll, 1);

    // ── Footer — close button (matches employeedialog footer pattern) ─────────
    auto* footer = new QFrame;
    footer->setObjectName("dialogFooter");
    footer->setFixedHeight(58);
    auto* footerH = new QHBoxLayout(footer);
    footerH->setContentsMargins(20, 0, 20, 0);
    footerH->addStretch();

    auto* closeBtn = new QPushButton("Fermer");
    closeBtn->setObjectName("dlgCancelBtn");
    closeBtn->setFixedHeight(38);
    closeBtn->setMinimumWidth(100);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    footerH->addWidget(closeBtn);

    outerV->addWidget(footer);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Inline result feedback — mirrors the employeedialog pattern
// ─────────────────────────────────────────────────────────────────────────────

void PinCodesDialog::setResult(const QString& msg)
{
    m_resultLabel->show();
    m_resultLabel->setText(msg);

    bool dark  = qApp->styleSheet().contains("0f0f0f");
    bool isOk  = msg.startsWith("OK");
    bool isErr = msg.startsWith("ERR");

    // display text: strip the prefix
    QString display = msg;
    if (msg.contains(':'))
        display = msg.section(':', 1);
    m_resultLabel->setText(display.trimmed());

    QString bg, border, color;
    if (isOk) {
        bg     = dark ? "#052e16" : "#f0fdf4";
        border = dark ? "#166534" : "#bbf7d0";
        color  = dark ? "#86efac" : "#16a34a";
    } else if (isErr) {
        bg     = dark ? "#450a0a" : "#fef2f2";
        border = dark ? "#7f1d1d" : "#fecaca";
        color  = dark ? "#fca5a5" : "#dc2626";
    } else {
        bg     = dark ? "#1c1408" : "#fffbeb";
        border = dark ? "#854d0e" : "#fde68a";
        color  = dark ? "#fbbf24" : "#d97706";
    }

    m_resultLabel->setStyleSheet(
        QString("QLabel{"
                "  background-color:%1;"
                "  border:1px solid %2;"
                "  border-radius:6px;"
                "  color:%3;"
                "  padding:8px 12px;"
                "  font-size:12px;"
                "  font-weight:500;"
                "}")
            .arg(bg, border, color));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Theme — copied directly from employeedialog's applyTheme() logic
//  Uses the same dark-detection, same palette variable names.
// ─────────────────────────────────────────────────────────────────────────────

void PinCodesDialog::applyTheme()
{
    bool dark = qApp->styleSheet().contains("0f0f0f");

    QString bg       = dark ? "#111111" : "#F3EFE0";
    QString card     = dark ? "#1e1e1e" : "#FFFFFF";
    QString border   = dark ? "#2e2e2e" : "#E8E4DC";
    QString inputBg  = dark ? "#2a2a2a" : "#FFFFFF";
    QString inputBrd = dark ? "#3a3a3a" : "#e2ddd6";
    QString inputBrdH= dark ? "#8A9A5B" : "#BDB5AD";
    QString text     = dark ? "#f0f0f0" : "#1f2937";
    QString label    = dark ? "#9ca3af" : "#6b7280";
    QString green    = "#8A9A5B";
    QString greenH   = "#9aaa6b";
    QString footerBg = dark ? "#1a1a1a" : "#F9F7F4";

    QString ss;

    // ── Dialog background ─────────────────────────────────────────────────────
    ss += QString(R"(
        #pinCodesDialog {
            background-color: %1;
        }
        #pinCodesDialog QWidget {
            background-color: %1;
            color: %2;
        }
    )").arg(bg, text);

    // ── Header label ──────────────────────────────────────────────────────────
    ss += QString(R"(
        #pinHeader {
            font-size: 15px;
            color: %1;
            background: transparent;
        }
    )").arg(text);

    // ── Group boxes (empGroup) ────────────────────────────────────────────────
    ss += QString(R"(
        #empGroup {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
            margin-top: 10px;
            font-size: 13px;
            font-weight: 600;
            color: %3;
        }
        #empGroup::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            left: 14px;
            color: %3;
            background-color: %1;
        }
        #empGroup QFormLayout QLabel {
            font-size: 12px;
            color: %4;
            font-weight: 500;
            background: transparent;
        }
    )").arg(card, border, text, label);

    // ── Search / form label ───────────────────────────────────────────────────
    ss += QString(R"(
        #empSearchLabel {
            color: %1;
            font-size: 12px;
            font-weight: 500;
            background: transparent;
        }
    )").arg(label);

    // ── Table (empTable) ──────────────────────────────────────────────────────
    ss += QString(R"(
        #empTable {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
            gridline-color: transparent;
            color: %3;
        }
        #empTable::item {
            padding: 8px;
            border-bottom: 1px solid %2;
            color: %3;
            background-color: transparent;
        }
        #empTable::item:selected {
            background-color: rgba(138,154,91,0.15);
            color: %3;
        }
        #empTable::item:hover {
            background-color: rgba(138,154,91,0.08);
        }
        #empTable QHeaderView::section {
            background-color: %4;
            color: %5;
            padding: 8px;
            border: none;
            border-right: 1px solid %2;
            border-bottom: 2px solid %2;
            font-weight: 700;
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        #empTable QHeaderView::section:last { border-right: none; }
        #empTable QScrollBar:vertical {
            background: %4; width: 6px; border-radius: 3px; margin: 0;
        }
        #empTable QScrollBar::handle:vertical {
            background: %2; border-radius: 3px; min-height: 24px;
        }
        #empTable QScrollBar::handle:vertical:hover { background: %6; }
        #empTable QScrollBar::add-line:vertical,
        #empTable QScrollBar::sub-line:vertical { height: 0; }
    )").arg(card, border, text, bg, label, green);

    // ── Input fields ──────────────────────────────────────────────────────────
    ss += QString(R"(
        #pinCodesDialog QLineEdit,
        #pinCodesDialog QSpinBox {
            background-color: %1;
            border: 1.5px solid %2;
            border-radius: 8px;
            padding: 0 12px;
            font-size: 13px;
            color: %3;
        }
        #pinCodesDialog QLineEdit:focus,
        #pinCodesDialog QSpinBox:focus  { border-color: %4; }
        #pinCodesDialog QLineEdit:hover,
        #pinCodesDialog QSpinBox:hover  { border-color: %5; }
        #pinCodesDialog QSpinBox::up-button,
        #pinCodesDialog QSpinBox::down-button { width:0; height:0; border:none; }
    )").arg(inputBg, inputBrd, text, green, inputBrdH);

    // ── Primary add button ────────────────────────────────────────────────────
    ss += QString(R"(
        #empAddBtn {
            background-color: %1;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 0 24px;
            font-weight: 600;
            font-size: 13px;
        }
        #empAddBtn:hover    { background-color: %2; }
        #empAddBtn:pressed  { background-color: #6a7a3b; }
        #empAddBtn:disabled { background-color: #BDB5AD; color: #FFFFFF; }
    )").arg(green, greenH);

    // ── Secondary buttons ─────────────────────────────────────────────────────
    ss += QString(R"(
        #empSecondaryBtn {
            background-color: %1;
            color: %2;
            border: 1.5px solid %3;
            border-radius: 6px;
            padding: 0 20px;
            font-weight: 500;
            font-size: 13px;
        }
        #empSecondaryBtn:hover:enabled  { background-color: %4; border-color: %5; }
        #empSecondaryBtn:pressed:enabled { background-color: %4; border-color: %2; }
        #empSecondaryBtn:disabled { background-color: %4; color: %3; border-color: %3; }
    )").arg(card, text, inputBrd, bg, green);

    // ── Danger button ─────────────────────────────────────────────────────────
    ss += QString(R"(
        #empDangerBtn {
            background-color: %1;
            color: #C29B6D;
            border: 1.5px solid %2;
            border-radius: 6px;
            padding: 0 20px;
            font-weight: 500;
            font-size: 13px;
        }
        #empDangerBtn:hover:enabled  { background-color:#C29B6D; color:#FFFFFF; border-color:#C29B6D; }
        #empDangerBtn:pressed:enabled { background-color:#a8845a; border-color:#a8845a; color:#FFFFFF; }
        #empDangerBtn:disabled { background-color:%3; color:%2; border-color:%2; }
    )").arg(card, inputBrd, bg);

    // ── Footer ────────────────────────────────────────────────────────────────
    ss += QString(R"(
        #dialogFooter {
            background-color: %1;
            border-top: 1px solid %2;
            border-bottom-left-radius: 12px;
            border-bottom-right-radius: 12px;
        }
    )").arg(footerBg, border);

    // ── Close button ─────────────────────────────────────────────────────────
    ss += QString(R"(
        #dlgCancelBtn {
            background-color: %1;
            border: 1.5px solid %2;
            border-radius: 8px;
            color: %3;
            font-size: 13px;
            font-weight: 600;
            padding: 0 16px;
        }
        #dlgCancelBtn:hover   { border-color: %4; color: %5; }
        #dlgCancelBtn:pressed { background-color: %6; }
    )").arg(footerBg, inputBrd, label, green, text, border);

    // ── Scrollbar (dialog-level) ──────────────────────────────────────────────
    ss += QString(R"(
        #pinCodesDialog QScrollBar:vertical {
            background: %1; width: 6px; border-radius: 3px; margin: 0;
        }
        #pinCodesDialog QScrollBar::handle:vertical {
            background: %2; border-radius: 3px; min-height: 24px;
        }
        #pinCodesDialog QScrollBar::handle:vertical:hover { background: %3; }
        #pinCodesDialog QScrollBar::add-line:vertical,
        #pinCodesDialog QScrollBar::sub-line:vertical { height: 0; }
    )").arg(border, label, green);

    setStyleSheet(ss);
}
