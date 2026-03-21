#include "employeedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QScrollArea>
#include <QFrame>
#include <QApplication>

EmployeeDialog::EmployeeDialog(QWidget *parent) : QDialog(parent)
{
    setupUI();
    setModal(true);
    setWindowTitle("Fiche Employe");
}

void EmployeeDialog::setupUI()
{
    setMinimumSize(680, 640);
    setMaximumSize(780, 840);
    setObjectName("employeeDialog");

    // ── Dark mode: only check stylesheet content, never the palette ──────────
    // "0f0f0f" is the QMainWindow bg that exists ONLY in style-dark.qss
    bool dark = qApp->styleSheet().contains("0f0f0f");

    // ── Brand palette ─────────────────────────────────────────────────────────
    // Light:  bg=Linen White, card=White, inputs=light grey, text=dark
    // Dark:   bg/card/inputs = dark greys
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
    QString titleBg  = dark ? "#2e2e2e" : "#F5F0E8";
    QString titleFg  = dark ? "#9aaa6b" : "#4D362D";
    QString footerBg = dark ? "#1a1a1a" : "#F9F7F4";
    QString saveFg   = "#FFFFFF";

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // ── Scroll area ──────────────────────────────────────────────────────────
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(20, 20, 20, 20);
    scrollLayout->setSpacing(14);

    auto makeGroup = [](const QString& title) -> QGroupBox* {
        QGroupBox *g = new QGroupBox(title);
        g->setObjectName("empGroup");
        return g;
    };
    auto makeForm = [](QGroupBox *g) -> QFormLayout* {
        QFormLayout *f = new QFormLayout(g);
        f->setSpacing(10);
        f->setContentsMargins(16, 14, 16, 14);
        f->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        f->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        return f;
    };

    // ── Informations personnelles ─────────────────────────────────────────
    QGroupBox *personalGroup = makeGroup("Informations personnelles");
    QFormLayout *personalForm = makeForm(personalGroup);
    m_cinInput = new QLineEdit(); m_cinInput->setPlaceholderText("Ex: 12345678"); m_cinInput->setMaxLength(8); m_cinInput->setFixedHeight(38);
    personalForm->addRow("CIN *", m_cinInput);
    m_nomInput = new QLineEdit(); m_nomInput->setPlaceholderText("Nom de famille"); m_nomInput->setFixedHeight(38);
    personalForm->addRow("Nom *", m_nomInput);
    m_prenomInput = new QLineEdit(); m_prenomInput->setPlaceholderText("Prenom"); m_prenomInput->setFixedHeight(38);
    personalForm->addRow("Prenom *", m_prenomInput);
    m_posteCombo = new QComboBox(); m_posteCombo->setFixedHeight(38);
    m_posteCombo->addItems({"Menuisier","Menuisier Senior","Chef Equipe","Designer","Apprenti"});
    personalForm->addRow("Poste *", m_posteCombo);
    scrollLayout->addWidget(personalGroup);

    // ── Coordonnees ──────────────────────────────────────────────────────────
    QGroupBox *contactGroup = makeGroup("Coordonnees");
    QFormLayout *contactForm = makeForm(contactGroup);
    m_emailInput = new QLineEdit(); m_emailInput->setPlaceholderText("exemple@woodflow.tn"); m_emailInput->setFixedHeight(38);
    contactForm->addRow("Email", m_emailInput);
    m_telephoneInput = new QLineEdit(); m_telephoneInput->setPlaceholderText("+216 XX XXX XXX"); m_telephoneInput->setFixedHeight(38);
    contactForm->addRow("Telephone", m_telephoneInput);
    scrollLayout->addWidget(contactGroup);

    // ── Informations professionnelles ─────────────────────────────────────
    QGroupBox *professionalGroup = makeGroup("Informations professionnelles");
    QFormLayout *professionalForm = makeForm(professionalGroup);
    m_dateEmbaucheInput = new QDateEdit(); m_dateEmbaucheInput->setDate(QDate::currentDate());
    m_dateEmbaucheInput->setCalendarPopup(true); m_dateEmbaucheInput->setDisplayFormat("dd/MM/yyyy"); m_dateEmbaucheInput->setFixedHeight(38);
    professionalForm->addRow("Date d'embauche", m_dateEmbaucheInput);
    m_salaireInput = new QDoubleSpinBox(); m_salaireInput->setRange(0,999999);
    m_salaireInput->setDecimals(2); m_salaireInput->setSuffix(" TND"); m_salaireInput->setValue(1500.0);
    m_salaireInput->setFixedHeight(38); m_salaireInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
    professionalForm->addRow("Salaire", m_salaireInput);
    m_competencesInput = new QTextEdit(); m_competencesInput->setFixedHeight(70);
    m_competencesInput->setPlaceholderText("Ex: Ebenisterie, Pose, Finition (separes par des virgules)");
    professionalForm->addRow("Competences", m_competencesInput);
    m_disponibiliteCombo = new QComboBox(); m_disponibiliteCombo->setFixedHeight(38);
    m_disponibiliteCombo->addItems({"Disponible","Indisponible","En conge"});
    professionalForm->addRow("Disponibilite", m_disponibiliteCombo);
    scrollLayout->addWidget(professionalGroup);

    // ── Performance et suivi ──────────────────────────────────────────────
    QGroupBox *trackingGroup = makeGroup("Performance et suivi");
    QFormLayout *trackingForm = makeForm(trackingGroup);
    m_performanceInput = new QDoubleSpinBox(); m_performanceInput->setRange(0,10);
    m_performanceInput->setDecimals(1); m_performanceInput->setSingleStep(0.5);
    m_performanceInput->setSuffix(" / 10"); m_performanceInput->setValue(7.0); m_performanceInput->setFixedHeight(38);
    trackingForm->addRow("Performance", m_performanceInput);
    m_heuresTravailInput = new QDoubleSpinBox(); m_heuresTravailInput->setRange(0,500);
    m_heuresTravailInput->setDecimals(1); m_heuresTravailInput->setSuffix(" h");
    m_heuresTravailInput->setValue(160.0); m_heuresTravailInput->setFixedHeight(38);
    trackingForm->addRow("Heures de travail", m_heuresTravailInput);
    QHBoxLayout *joursLayout = new QHBoxLayout(); joursLayout->setSpacing(16);
    QVBoxLayout *cc = new QVBoxLayout(); cc->setSpacing(4);
    QLabel *clbl = new QLabel("Conges (j)"); cc->addWidget(clbl);
    m_nbJoursCongesInput = new QSpinBox(); m_nbJoursCongesInput->setRange(0,365); m_nbJoursCongesInput->setFixedHeight(38);
    cc->addWidget(m_nbJoursCongesInput);
    QVBoxLayout *ac = new QVBoxLayout(); ac->setSpacing(4);
    QLabel *albl = new QLabel("Absences (j)"); ac->addWidget(albl);
    m_nbJoursAbsenceInput = new QSpinBox(); m_nbJoursAbsenceInput->setRange(0,365); m_nbJoursAbsenceInput->setFixedHeight(38);
    ac->addWidget(m_nbJoursAbsenceInput);
    joursLayout->addLayout(cc); joursLayout->addLayout(ac); joursLayout->addStretch();
    trackingForm->addRow("Jours", joursLayout);
    scrollLayout->addWidget(trackingGroup);

    // ── Authentification ──────────────────────────────────────────────────
    QGroupBox *authGroup = makeGroup("Authentification");
    QFormLayout *authForm = makeForm(authGroup);
    QHBoxLayout *passLayout = new QHBoxLayout(); passLayout->setSpacing(6);
    m_passwordInput = new QLineEdit(); m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setPlaceholderText("Mot de passe"); m_passwordInput->setFixedHeight(38);
    QPushButton *eyeBtn = new QPushButton("Afficher"); eyeBtn->setObjectName("eyeButton");
    eyeBtn->setFixedHeight(38); eyeBtn->setMinimumWidth(78); eyeBtn->setCheckable(true);
    eyeBtn->setCursor(Qt::PointingHandCursor);
    connect(eyeBtn, &QPushButton::toggled, this, [this, eyeBtn](bool checked){
        m_passwordInput->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        eyeBtn->setText(checked ? "Masquer" : "Afficher");
    });
    passLayout->addWidget(m_passwordInput); passLayout->addWidget(eyeBtn);
    authForm->addRow("Mot de passe *", passLayout);
    m_showPasswordCheck = new QCheckBox(); m_showPasswordCheck->hide();
    QLabel *passHint = new QLabel("En mode edition, laisser vide pour conserver le mot de passe actuel.");
    passHint->setWordWrap(true);
    authForm->addRow("", passHint);
    scrollLayout->addWidget(authGroup);
    scrollLayout->addStretch();

    scroll->setWidget(scrollContent);
    outerLayout->addWidget(scroll, 1);

    // ── Footer ────────────────────────────────────────────────────────────
    QFrame *footer = new QFrame(this);
    footer->setObjectName("dialogFooter");
    footer->setFixedHeight(60);
    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(20, 0, 20, 0);
    footerLayout->setSpacing(10);
    QPushButton *cancelBtn = new QPushButton("Annuler"); cancelBtn->setObjectName("dlgCancelBtn");
    cancelBtn->setFixedHeight(38); cancelBtn->setMinimumWidth(110); cancelBtn->setCursor(Qt::PointingHandCursor);
    QPushButton *saveBtn = new QPushButton("Enregistrer"); saveBtn->setObjectName("dlgSaveBtn");
    saveBtn->setFixedHeight(38); saveBtn->setMinimumWidth(130); saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setDefault(true);
    connect(cancelBtn, &QPushButton::clicked, this, &EmployeeDialog::onRejected);
    connect(saveBtn,   &QPushButton::clicked, this, &EmployeeDialog::onAccepted);
    footerLayout->addStretch(); footerLayout->addWidget(cancelBtn); footerLayout->addWidget(saveBtn);
    outerLayout->addWidget(footer);

    // ── Stylesheet — hardcoded light/dark values, no %arg placeholders ────────
    // Using explicit color strings avoids any Qt string formatting issues.
    QString ss;
    ss += QString("#employeeDialog { background-color: %1; }").arg(bg);
    ss += QString("#employeeDialog > QWidget { background-color: %1; }").arg(bg);
    ss += QString("#employeeDialog QScrollArea { background-color: %1; border: none; }").arg(bg);
    ss += QString("#employeeDialog QScrollArea > QWidget > QWidget { background-color: %1; }").arg(bg);

    // GroupBox card
    ss += QString(R"(
        #employeeDialog QGroupBox {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
            margin-top: 14px;
            padding-top: 12px;
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 0.8px;
        }
    )").arg(card, border);

    // GroupBox title — small badge sitting on the top border
    ss += QString(R"(
        #employeeDialog QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 14px;
            top: 0px;
            padding: 2px 10px;
            background-color: %1;
            color: %2;
            border-radius: 6px;
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 0.6px;
        }
    )").arg(titleBg, titleFg);

    // Labels
    ss += QString(R"(
        #employeeDialog QLabel {
            color: %1;
            background: transparent;
            font-size: 12px;
            font-weight: 500;
        }
    )").arg(label);

    // All input widgets
    ss += QString(R"(
        #employeeDialog QLineEdit,
        #employeeDialog QComboBox,
        #employeeDialog QDateEdit,
        #employeeDialog QSpinBox,
        #employeeDialog QDoubleSpinBox,
        #employeeDialog QTextEdit {
            background-color: %1;
            border: 1.5px solid %2;
            border-radius: 8px;
            padding: 0 12px;
            font-size: 13px;
            color: %3;
        }
        #employeeDialog QLineEdit:focus,
        #employeeDialog QComboBox:focus,
        #employeeDialog QDateEdit:focus,
        #employeeDialog QSpinBox:focus,
        #employeeDialog QDoubleSpinBox:focus,
        #employeeDialog QTextEdit:focus {
            border-color: %4;
        }
        #employeeDialog QLineEdit:hover,
        #employeeDialog QComboBox:hover,
        #employeeDialog QDateEdit:hover,
        #employeeDialog QSpinBox:hover,
        #employeeDialog QDoubleSpinBox:hover,
        #employeeDialog QTextEdit:hover {
            border-color: %5;
        }
        #employeeDialog QTextEdit { padding: 8px 12px; }
    )").arg(inputBg, inputBrd, text, green, inputBrdH);

    // ComboBox
    ss += QString(R"(
        #employeeDialog QComboBox::drop-down { border: none; width: 28px; }
        #employeeDialog QComboBox::down-arrow {
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid %1;
            width: 0; height: 0;
        }
        #employeeDialog QComboBox QAbstractItemView {
            background-color: %2;
            border: 1px solid %3;
            border-radius: 8px;
            selection-background-color: rgba(138,154,91,0.15);
            color: %4;
            font-size: 13px;
            padding: 4px;
        }
    )").arg(label, card, border, text);

    // SpinBox arrows
    ss += QString(R"(
        #employeeDialog QSpinBox::up-button, #employeeDialog QDoubleSpinBox::up-button,
        #employeeDialog QSpinBox::down-button, #employeeDialog QDoubleSpinBox::down-button {
            width: 20px; border: none; background: transparent;
        }
        #employeeDialog QSpinBox::up-arrow, #employeeDialog QDoubleSpinBox::up-arrow {
            border-left: 4px solid transparent; border-right: 4px solid transparent;
            border-bottom: 5px solid %1; width: 0; height: 0;
        }
        #employeeDialog QSpinBox::down-arrow, #employeeDialog QDoubleSpinBox::down-arrow {
            border-left: 4px solid transparent; border-right: 4px solid transparent;
            border-top: 5px solid %1; width: 0; height: 0;
        }
        #employeeDialog QDateEdit::drop-down { border: none; width: 26px; }
        #employeeDialog QDateEdit::down-arrow {
            border-left: 4px solid transparent; border-right: 4px solid transparent;
            border-top: 5px solid %1; width: 0; height: 0;
        }
    )").arg(label);

    // Scrollbar
    ss += QString(R"(
        #employeeDialog QScrollBar:vertical {
            background: %1; width: 6px; border-radius: 3px; margin: 0;
        }
        #employeeDialog QScrollBar::handle:vertical {
            background: %2; border-radius: 3px; min-height: 24px;
        }
        #employeeDialog QScrollBar::handle:vertical:hover { background: %3; }
        #employeeDialog QScrollBar::add-line:vertical,
        #employeeDialog QScrollBar::sub-line:vertical { height: 0; }
    )").arg(border, label, green);

    // Eye button
    ss += QString(R"(
        #eyeButton {
            background-color: %1;
            border: 1.5px solid %2;
            border-radius: 8px;
            color: %3;
            font-size: 12px;
            font-weight: 600;
            padding: 0 10px;
        }
        #eyeButton:hover { border-color: %4; color: %4; }
        #eyeButton:checked { background-color: rgba(138,154,91,0.12); border-color: %4; color: %4; }
    )").arg(inputBg, inputBrd, label, green);

    // Footer
    ss += QString(R"(
        #dialogFooter {
            background-color: %1;
            border-top: 1px solid %2;
        }
    )").arg(footerBg, border);

    // Cancel button
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
        #dlgCancelBtn:hover { border-color: %4; color: %5; }
        #dlgCancelBtn:pressed { background-color: %6; }
    )").arg(footerBg, inputBrd, label, green, text, border);

    // Save button
    ss += QString(R"(
        #dlgSaveBtn {
            background-color: %1;
            border: none;
            border-radius: 8px;
            color: %2;
            font-size: 13px;
            font-weight: 600;
            padding: 0 16px;
        }
        #dlgSaveBtn:hover { background-color: %3; }
        #dlgSaveBtn:pressed { background-color: #7a8a4b; }
    )").arg(green, saveFg, greenH);

    setStyleSheet(ss);
}

void EmployeeDialog::togglePasswordVisibility(bool checked) {
    m_passwordInput->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

Employee EmployeeDialog::getEmployee() const {
    Employee e;
    e.setCin(m_cinInput->text().trimmed());
    e.setNom(m_nomInput->text().trimmed());
    e.setPrenom(m_prenomInput->text().trimmed());
    e.setPoste(m_posteCombo->currentText());
    e.setEmail(m_emailInput->text().trimmed());
    e.setTelephone(m_telephoneInput->text().trimmed());
    e.setDateEmbauche(QDateTime(m_dateEmbaucheInput->date(), QTime(0,0,0)));
    e.setSalaire(m_salaireInput->value());
    QString compText = m_competencesInput->toPlainText();
    QStringList comps = compText.split(',', Qt::SkipEmptyParts);
    for (QString& c : comps) c = c.trimmed();
    e.setCompetences(comps);
    e.setDisponibilite(m_disponibiliteCombo->currentText());
    e.setPerformance(m_performanceInput->value());
    e.setNbJoursConges(m_nbJoursCongesInput->value());
    e.setNbJoursAbsence(m_nbJoursAbsenceInput->value());
    e.setHeuresTravail(m_heuresTravailInput->value());
    e.setMotDePasse(m_passwordInput->text());
    return e;
}

void EmployeeDialog::setEmployee(const Employee& employee) {
    m_editMode = true;
    m_cinInput->setText(employee.getCin());
    m_nomInput->setText(employee.getNom());
    m_prenomInput->setText(employee.getPrenom());
    int idx = m_posteCombo->findText(employee.getPoste());
    if (idx >= 0) m_posteCombo->setCurrentIndex(idx);
    m_emailInput->setText(employee.getEmail());
    m_telephoneInput->setText(employee.getTelephone());
    m_dateEmbaucheInput->setDate(employee.getDateEmbauche().date());
    m_salaireInput->setValue(employee.getSalaire());
    m_competencesInput->setPlainText(employee.getCompetencesString());
    idx = m_disponibiliteCombo->findText(employee.getDisponibilite());
    if (idx >= 0) m_disponibiliteCombo->setCurrentIndex(idx);
    m_performanceInput->setValue(employee.getPerformance());
    m_nbJoursCongesInput->setValue(employee.getNbJoursConges());
    m_nbJoursAbsenceInput->setValue(employee.getNbJoursAbsence());
    m_heuresTravailInput->setValue(employee.getHeuresTravail());
    m_passwordInput->setText(employee.getMotDePasse());
}

bool EmployeeDialog::validateInput() {
    if (m_cinInput->text().trimmed().isEmpty()) { QMessageBox::warning(this,"Validation","Le CIN est obligatoire."); m_cinInput->setFocus(); return false; }
    if (m_cinInput->text().trimmed().length() != 8) { QMessageBox::warning(this,"Validation","Le CIN doit contenir exactement 8 chiffres."); m_cinInput->setFocus(); return false; }
    if (m_nomInput->text().trimmed().isEmpty()) { QMessageBox::warning(this,"Validation","Le nom est obligatoire."); m_nomInput->setFocus(); return false; }
    if (m_prenomInput->text().trimmed().isEmpty()) { QMessageBox::warning(this,"Validation","Le prenom est obligatoire."); m_prenomInput->setFocus(); return false; }
    if (!m_editMode && m_passwordInput->text().isEmpty()) { QMessageBox::warning(this,"Validation","Un mot de passe est obligatoire pour un nouvel employe."); m_passwordInput->setFocus(); return false; }
    return true;
}

void EmployeeDialog::onAccepted() { if (validateInput()) accept(); }
void EmployeeDialog::onRejected() { reject(); }
