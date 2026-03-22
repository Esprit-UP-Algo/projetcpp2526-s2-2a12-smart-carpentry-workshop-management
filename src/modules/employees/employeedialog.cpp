#include "employeedialog.h"
#include "../../models/employee.h"
#include "../../common/validators.h"
#include <QFileDialog>
#include <QImageReader>
#include <QBuffer>
#include <QPainterPath>
#include <QPainter>
#include <QRegularExpression>
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
    m_cinInput = new QLineEdit(); m_cinInput->setFixedHeight(38);
    Validators::setupCinInput(m_cinInput);
    personalForm->addRow("CIN *", m_cinInput);
    m_nomInput = new QLineEdit(); m_nomInput->setFixedHeight(38);
    Validators::setupNameInput(m_nomInput);
    personalForm->addRow("Nom *", m_nomInput);
    m_prenomInput = new QLineEdit(); m_prenomInput->setFixedHeight(38);
    Validators::setupNameInput(m_prenomInput);
    personalForm->addRow("Prenom *", m_prenomInput);
    m_posteCombo = new QComboBox(); m_posteCombo->setFixedHeight(38);
    m_posteCombo->addItems({"Menuisier","Menuisier Senior","Chef Equipe","Designer","Apprenti"});
    personalForm->addRow("Poste *", m_posteCombo);
    scrollLayout->addWidget(personalGroup);

    // ── Photo ────────────────────────────────────────────────────────────────
    QGroupBox *photoGroup = makeGroup("Photo de profil");
    QHBoxLayout *photoLayout = new QHBoxLayout(photoGroup);
    photoLayout->setContentsMargins(16, 14, 16, 14);
    photoLayout->setSpacing(16);

    m_photoLabel = new QLabel();
    m_photoLabel->setFixedSize(80, 80);
    m_photoLabel->setAlignment(Qt::AlignCenter);
    m_photoLabel->setStyleSheet("border: 2px dashed #c8c2ba; border-radius: 40px; background: #f5f0e8;");
    m_photoLabel->setText("Aucune photo");

    QPushButton *pickPhotoBtn = new QPushButton("Choisir une photo");
    pickPhotoBtn->setObjectName("empSecondaryBtn");
    pickPhotoBtn->setFixedHeight(38);
    pickPhotoBtn->setCursor(Qt::PointingHandCursor);

    QPushButton *removePhotoBtn = new QPushButton("Supprimer");
    removePhotoBtn->setObjectName("empDangerBtn");
    removePhotoBtn->setFixedHeight(38);
    removePhotoBtn->setCursor(Qt::PointingHandCursor);

    QLabel *photoHint = new QLabel("JPG/PNG, max 500 KB, sera redimensionnée à 200×200 px.");
    photoHint->setWordWrap(true);
    photoHint->setStyleSheet("color: #9ca3af; font-size: 11px; background: transparent;");

    QVBoxLayout *photoRight = new QVBoxLayout();
    photoRight->addWidget(pickPhotoBtn);
    photoRight->addWidget(removePhotoBtn);
    photoRight->addWidget(photoHint);
    photoRight->addStretch();

    photoLayout->addWidget(m_photoLabel);
    photoLayout->addLayout(photoRight);
    photoLayout->addStretch();
    scrollLayout->addWidget(photoGroup);

    // Helper: crop pixmap into a circle
    auto makeCircular = [](const QPixmap& pix, int size) -> QPixmap {
        QPixmap scaled = pix.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap result(size, size);
        result.fill(Qt::transparent);
        QPainter p(&result);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(0, 0, size, size);
        p.setClipPath(path);
        p.drawPixmap(0, 0, scaled);
        return result;
    };

    connect(pickPhotoBtn, &QPushButton::clicked, this, [this, makeCircular]() {
        QString path = QFileDialog::getOpenFileName(this, "Choisir une photo",
            QDir::homePath(), "Images (*.jpg *.jpeg *.png)");
        if (path.isEmpty()) return;

        QImageReader reader(path);
        reader.setAutoTransform(true);
        QImage img = reader.read();
        if (img.isNull()) { QMessageBox::warning(this, "Erreur", "Impossible de lire l'image."); return; }

        // Resize to 200x200
        img = img.scaled(200, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)
                  .copy(0, 0, 200, 200);

        // Check size <= 500KB
        QBuffer buf; buf.open(QIODevice::WriteOnly);
        img.save(&buf, "JPEG", 85);
        if (buf.data().size() > 500 * 1024) {
            QMessageBox::warning(this, "Photo trop grande", "L'image dépasse 500 KB après compression.");
            return;
        }
        m_photoData = buf.data();

        // Preview
        QPixmap pix;
        pix.loadFromData(m_photoData);
        m_photoLabel->setPixmap(makeCircular(pix, 80));
        m_photoLabel->setStyleSheet("border: none;");
    });

    connect(removePhotoBtn, &QPushButton::clicked, this, [this]() {
        m_photoData.clear();
        m_photoLabel->setPixmap(QPixmap());
        m_photoLabel->setText("Aucune photo");
        m_photoLabel->setStyleSheet("border: 2px dashed #c8c2ba; border-radius: 40px; background: #f5f0e8;");
    });

    // ── Coordonnees ──────────────────────────────────────────────────────────
    QGroupBox *contactGroup = makeGroup("Coordonnees");
    QFormLayout *contactForm = makeForm(contactGroup);
    m_emailInput = new QLineEdit(); m_emailInput->setFixedHeight(38);
    Validators::setupEmailInput(m_emailInput);
    contactForm->addRow("Email", m_emailInput);
    m_telephoneInput = new QLineEdit(); m_telephoneInput->setFixedHeight(38);
    Validators::setupPhoneInput(m_telephoneInput);
    contactForm->addRow("Telephone", m_telephoneInput);
    scrollLayout->addWidget(contactGroup);

    // ── Informations professionnelles ─────────────────────────────────────
    QGroupBox *professionalGroup = makeGroup("Informations professionnelles");
    QFormLayout *professionalForm = makeForm(professionalGroup);
    m_dateEmbaucheInput = new QDateEdit(); m_dateEmbaucheInput->setDate(QDate::currentDate());
    m_dateEmbaucheInput->setCalendarPopup(true); m_dateEmbaucheInput->setDisplayFormat("dd/MM/yyyy"); m_dateEmbaucheInput->setFixedHeight(38);
    professionalForm->addRow("Date d'embauche", m_dateEmbaucheInput);
    m_salaireInput = new QDoubleSpinBox(); m_salaireInput->setRange(0,999999);
    m_salaireInput->setDecimals(2); m_salaireInput->setValue(1500.0);
    m_salaireInput->setFixedHeight(38); m_salaireInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
    {
        QHBoxLayout *salRow = new QHBoxLayout(); salRow->setSpacing(8);
        QLabel *tndLabel = new QLabel("TND"); tndLabel->setFixedWidth(36);
        salRow->addWidget(m_salaireInput); salRow->addWidget(tndLabel);
        professionalForm->addRow("Salaire", salRow);
    }
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
    m_performanceInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
    trackingForm->addRow("Performance", m_performanceInput);
    m_heuresTravailInput = new QDoubleSpinBox(); m_heuresTravailInput->setRange(0,500);
    m_heuresTravailInput->setDecimals(1); m_heuresTravailInput->setSuffix(" h");
    m_heuresTravailInput->setValue(160.0); m_heuresTravailInput->setFixedHeight(38);
    m_heuresTravailInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
    trackingForm->addRow("Heures de travail", m_heuresTravailInput);
    QHBoxLayout *joursLayout = new QHBoxLayout(); joursLayout->setSpacing(16);
    QVBoxLayout *cc = new QVBoxLayout(); cc->setSpacing(4);
    QLabel *clbl = new QLabel("Conges (j)"); cc->addWidget(clbl);
    m_nbJoursCongesInput = new QSpinBox(); m_nbJoursCongesInput->setRange(0,365); m_nbJoursCongesInput->setFixedHeight(38);
    m_nbJoursCongesInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
    cc->addWidget(m_nbJoursCongesInput);
    QVBoxLayout *ac = new QVBoxLayout(); ac->setSpacing(4);
    QLabel *albl = new QLabel("Absences (j)"); ac->addWidget(albl);
    m_nbJoursAbsenceInput = new QSpinBox(); m_nbJoursAbsenceInput->setRange(0,365); m_nbJoursAbsenceInput->setFixedHeight(38);
    m_nbJoursAbsenceInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ac->addWidget(m_nbJoursAbsenceInput);
    joursLayout->addLayout(cc); joursLayout->addLayout(ac); joursLayout->addStretch();
    trackingForm->addRow("Jours", joursLayout);
    scrollLayout->addWidget(trackingGroup);

    // ── Authentification ──────────────────────────────────────────────────
    QGroupBox *authGroup = makeGroup("Authentification");
    QFormLayout *authForm = makeForm(authGroup);
    QHBoxLayout *passLayout = new QHBoxLayout(); passLayout->setSpacing(6);
    m_passwordInput = new QLineEdit(); m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setFixedHeight(38);
    Validators::setupPasswordInput(m_passwordInput);
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
    // Hint shown only on hover over the password field
    m_passwordInput->setToolTip("En mode édition, laisser vide pour conserver le mot de passe actuel.");
    scrollLayout->addWidget(authGroup);

    // ── Permissions ───────────────────────────────────────────────────────
    QGroupBox *permGroup = makeGroup("Permissions d'acces");
    QVBoxLayout *permLayout = new QVBoxLayout(permGroup);
    permLayout->setContentsMargins(16, 14, 16, 14);
    permLayout->setSpacing(10);

    QLabel *permHint = new QLabel("Selectionnez les modules auxquels cet employe peut acceder.");
    permHint->setWordWrap(true);
    permLayout->addWidget(permHint);

    QHBoxLayout *permRow = new QHBoxLayout();
    permRow->setSpacing(16);

    m_permEmployeCheck     = new QCheckBox("Employes");
    m_permMateriauCheck    = new QCheckBox("Materiaux");
    m_permProduitCheck     = new QCheckBox("Produits");
    m_permProjetCheck      = new QCheckBox("Projets");
    m_permTransactionsCheck= new QCheckBox("Transactions");

    // Default: all checked
    m_permEmployeCheck->setChecked(true);
    m_permMateriauCheck->setChecked(true);
    m_permProduitCheck->setChecked(true);
    m_permProjetCheck->setChecked(true);
    m_permTransactionsCheck->setChecked(true);


    permRow->addWidget(m_permProjetCheck);
    permRow->addWidget(m_permEmployeCheck);
    permRow->addWidget(m_permMateriauCheck);
    permRow->addWidget(m_permProduitCheck);
    permRow->addWidget(m_permTransactionsCheck);
    permRow->addStretch();
    permLayout->addLayout(permRow);

    // Binary label — hidden from UI, kept for internal use
    m_permBinaryLabel = new QLabel("11111 = 31");
    m_permBinaryLabel->hide();

    // Update binary label whenever a checkbox changes
    auto updateBinaryLabel = [this]() {
        int val = 0;
        if (m_permEmployeCheck->isChecked())      val |= Employee::PERM_EMPLOYE;
        if (m_permMateriauCheck->isChecked())     val |= Employee::PERM_MATERIAU;
        if (m_permProduitCheck->isChecked())      val |= Employee::PERM_PRODUIT;
        if (m_permProjetCheck->isChecked())       val |= Employee::PERM_PROJET;
        if (m_permTransactionsCheck->isChecked()) val |= Employee::PERM_TRANSACTIONS;
        QString binary = QString("%1").arg(val, 5, 2, QChar('0'));
        m_permBinaryLabel->setText(QString("%1 = %2").arg(binary).arg(val));
    };

    connect(m_permEmployeCheck,      &QCheckBox::toggled, updateBinaryLabel);
    connect(m_permMateriauCheck,     &QCheckBox::toggled, updateBinaryLabel);
    connect(m_permProduitCheck,      &QCheckBox::toggled, updateBinaryLabel);
    connect(m_permProjetCheck,       &QCheckBox::toggled, updateBinaryLabel);
    connect(m_permTransactionsCheck, &QCheckBox::toggled, updateBinaryLabel);

    scrollLayout->addWidget(permGroup);
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

    applyTheme(dark);

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
    // Strip SQL injection chars: quotes, semicolons, dashes, comment markers, parens
    compText.remove(QRegularExpression(R"(['";\-\-\(\)\*\/\\])"));
    // Keep only letters, digits, spaces, commas, accented chars
    compText.remove(QRegularExpression(R"([^a-zA-ZÀ-ÿ0-9\s,])"));
    QStringList comps = compText.split(',', Qt::SkipEmptyParts);
    for (QString& c : comps) c = c.trimmed();
    e.setCompetences(comps);
    e.setDisponibilite(m_disponibiliteCombo->currentText());
    e.setPerformance(m_performanceInput->value());
    e.setNbJoursConges(m_nbJoursCongesInput->value());
    e.setNbJoursAbsence(m_nbJoursAbsenceInput->value());
    e.setHeuresTravail(m_heuresTravailInput->value());
    e.setMotDePasse(m_passwordInput->text());
    int perms = 0;
    if (m_permEmployeCheck->isChecked())      perms |= Employee::PERM_EMPLOYE;
    if (m_permMateriauCheck->isChecked())     perms |= Employee::PERM_MATERIAU;
    if (m_permProduitCheck->isChecked())      perms |= Employee::PERM_PRODUIT;
    if (m_permProjetCheck->isChecked())       perms |= Employee::PERM_PROJET;
    if (m_permTransactionsCheck->isChecked()) perms |= Employee::PERM_TRANSACTIONS;
    e.setPermissions(perms);
    e.setPhoto(m_photoData);
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
    int p = employee.getPermissions();
    m_permEmployeCheck->setChecked(p & Employee::PERM_EMPLOYE);
    m_permMateriauCheck->setChecked(p & Employee::PERM_MATERIAU);
    m_permProduitCheck->setChecked(p & Employee::PERM_PRODUIT);
    m_permProjetCheck->setChecked(p & Employee::PERM_PROJET);
    m_permTransactionsCheck->setChecked(p & Employee::PERM_TRANSACTIONS);
    // Photo
    m_photoData = employee.getPhoto();
    if (!m_photoData.isEmpty()) {
        QPixmap pix; pix.loadFromData(m_photoData);
        QPixmap result(80, 80);
        result.fill(Qt::transparent);
        QPainter p2(&result);
        p2.setRenderHint(QPainter::Antialiasing);
        QPainterPath path2;
        path2.addEllipse(0, 0, 80, 80);
        p2.setClipPath(path2);
        p2.drawPixmap(0, 0, pix.scaled(80, 80, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        m_photoLabel->setPixmap(result);
        m_photoLabel->setStyleSheet("border: none;");
    } else {
        m_photoLabel->setPixmap(QPixmap());
        m_photoLabel->setText("Aucune photo");
        m_photoLabel->setStyleSheet("border: 2px dashed #c8c2ba; border-radius: 40px; background: #f5f0e8;");
    }

    // Trigger binary label update
    QString binary = QString("%1").arg(p, 5, 2, QChar('0'));
    m_permBinaryLabel->setText(QString("%1 = %2").arg(binary).arg(p));
}

bool EmployeeDialog::validateInput() {
    QString err;
    if (!Validators::validateCin(m_cinInput->text().trimmed(), err))
        { QMessageBox::warning(this, "Validation", err); m_cinInput->setFocus(); return false; }
    if (!Validators::validateName(m_nomInput->text().trimmed(), "Le nom", err))
        { QMessageBox::warning(this, "Validation", err); m_nomInput->setFocus(); return false; }
    if (!Validators::validateName(m_prenomInput->text().trimmed(), "Le prenom", err))
        { QMessageBox::warning(this, "Validation", err); m_prenomInput->setFocus(); return false; }
    if (!m_emailInput->text().trimmed().isEmpty()) {
        if (!Validators::validateEmail(m_emailInput->text().trimmed(), err))
            { QMessageBox::warning(this, "Validation", err); m_emailInput->setFocus(); return false; }
    }
    if (!m_telephoneInput->text().trimmed().isEmpty()) {
        if (!Validators::validatePhone(m_telephoneInput->text().trimmed(), err))
            { QMessageBox::warning(this, "Validation", err); m_telephoneInput->setFocus(); return false; }
    }
    if (!m_editMode && m_passwordInput->text().isEmpty())
        { QMessageBox::warning(this, "Validation", "Un mot de passe est obligatoire pour un nouvel employe."); m_passwordInput->setFocus(); return false; }
    if (!m_passwordInput->text().isEmpty()) {
        if (!Validators::validatePassword(m_passwordInput->text(), err))
            { QMessageBox::warning(this, "Validation", err); m_passwordInput->setFocus(); return false; }
    }
    return true;
}

void EmployeeDialog::onAccepted() { if (validateInput()) accept(); }
void EmployeeDialog::onRejected() { reject(); }


void EmployeeDialog::applyTheme(bool dark)
{
    // Redefine palette here so this method is self-contained and callable any time
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

    // Update checkbox palette — guard against being called before widgets are created
    if (m_permEmployeCheck) {
        QPalette pal;
        pal.setColor(QPalette::Base,       QColor(inputBg));
        pal.setColor(QPalette::Window,     QColor(inputBg));
        pal.setColor(QPalette::Text,       QColor(text));
        pal.setColor(QPalette::ButtonText, Qt::white);
        for (QCheckBox* cb : {m_permEmployeCheck, m_permMateriauCheck,
                              m_permProduitCheck, m_permProjetCheck,
                              m_permTransactionsCheck}) {
            cb->setPalette(pal);
        }
    }

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

    // SpinBox — no buttons (user types values directly, cleaner look)
    ss += R"(
        #employeeDialog QSpinBox::up-button,   #employeeDialog QDoubleSpinBox::up-button,
        #employeeDialog QSpinBox::down-button, #employeeDialog QDoubleSpinBox::down-button {
            width: 0; height: 0; border: none;
        }
    )";
    // DateEdit — clean dropdown button with unicode arrow via subcontrol
    ss += QString(R"(
        #employeeDialog QDateEdit::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: center right;
            width: 28px;
            border: none;
            border-left: 1px solid %1;
            border-top-right-radius: 7px;
            border-bottom-right-radius: 7px;
            background: transparent;
        }
        #employeeDialog QDateEdit::down-arrow {
            image: none;
            width: 8px; height: 8px;
            border-left: 2px solid %2;
            border-bottom: 2px solid %2;
            border-top: none;
            border-right: none;
            margin-right: 6px;
        }
    )").arg(inputBrd, label);

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

    // Checkbox styling — light/dark aware, no external image needed
    // Checked state uses a green fill; the checkmark is painted by Qt's style engine
    // but we override the palette so it draws white on green instead of black on black.
    QString checkOn  = dark ? "#8A9A5B" : "#8A9A5B";   // green fill when checked
    QString checkBrd = dark ? "#3a3a3a" : "#c8c2ba";   // border when unchecked
    QString checkBg  = dark ? "#2a2a2a" : "#ffffff";   // bg when unchecked
    ss += QString(R"(
        #employeeDialog QCheckBox {
            color: %1;
            font-size: 13px;
            font-weight: 500;
            spacing: 8px;
            background: transparent;
        }
        #employeeDialog QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid %2;
            border-radius: 5px;
            background-color: %3;
        }
        #employeeDialog QCheckBox::indicator:hover {
            border-color: %4;
        }
        #employeeDialog QCheckBox::indicator:checked {
            background-color: %4;
            border-color: %4;
            /* Qt will draw its own checkmark glyph — we just ensure it's visible
               by setting a contrasting background. The glyph color follows QPalette. */
        }
        #employeeDialog QCheckBox::indicator:unchecked {
            background-color: %3;
            border: 2px solid %2;
        }
    )").arg(text, checkBrd, checkBg, checkOn);

    // Calendar — just a slightly different background so it's visually distinct
    QString calBg = dark ? "#2a2a2a" : "#f0ece0";
    ss += QString(R"(
        #employeeDialog QDateEdit QCalendarWidget QWidget {
            background-color: %1;
        }
    )").arg(calBg);

    // Tooltip styling
    ss += R"(
        QToolTip {
            background-color: #f9f7f4;
            color: #6b7280;
            border: 1px solid #e2ddd6;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 12px;
        }
    )";

    setStyleSheet(ss);
}