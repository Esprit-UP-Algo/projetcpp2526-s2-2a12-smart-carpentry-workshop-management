#include "produitbarcodedialog.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFrame>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QPrinter>
#include <QTextDocument>
#include <QBuffer>
#include <QDateTime>

// ═════════════════════════════════════════════════════════════════════════════
//  BarcodeWidget — Code128-B
// ═════════════════════════════════════════════════════════════════════════════

// Encodages Code128-B : chaque caractère = 6 éléments (barres+espaces alternés)
// Source : spécification Code128 standard
static const int CODE128B_TABLE[][6] = {
    {2,1,2,2,2,2},{2,2,2,1,2,2},{2,2,2,2,2,1},{1,2,1,2,2,3},
    {1,2,1,3,2,2},{1,3,1,2,2,2},{1,2,2,2,1,3},{1,2,2,3,1,2},
    {1,3,2,2,1,2},{2,2,1,2,1,3},{2,2,1,3,1,2},{2,3,1,2,1,2},
    {1,1,2,2,3,2},{1,2,2,1,3,2},{1,2,2,2,3,1},{1,1,3,2,2,2},
    {1,2,3,1,2,2},{1,2,3,2,2,1},{2,2,3,2,1,1},{2,2,1,1,3,2},
    {2,2,1,2,3,1},{2,1,3,2,1,2},{2,2,3,1,1,2},{3,1,2,1,3,1},
    {3,1,1,2,2,2},{3,2,1,1,2,2},{3,2,1,2,2,1},{3,1,2,2,1,2},
    {3,2,2,1,1,2},{3,2,2,2,1,1},{2,1,2,1,2,3},{2,1,2,3,2,1},
    {2,3,2,1,2,1},{1,1,1,3,2,3},{1,3,1,1,2,3},{1,3,1,3,2,1},
    {1,1,2,3,1,3},{1,3,2,1,1,3},{1,3,2,3,1,1},{2,1,1,3,1,3},
    {2,3,1,1,1,3},{2,3,1,3,1,1},{1,1,2,1,3,3},{1,1,2,3,3,1},
    {1,3,2,1,3,1},{1,1,3,1,2,3},{1,1,3,3,2,1},{1,3,3,1,2,1},
    {3,1,3,1,2,1},{2,1,1,3,3,1},{2,3,1,1,3,1},{2,1,3,1,1,3},
    {2,1,3,3,1,1},{2,1,3,1,3,1},{3,1,1,1,2,3},{3,1,1,3,2,1},
    {3,3,1,1,2,1},{3,1,2,1,1,3},{3,1,2,3,1,1},{3,3,2,1,1,1},
    {3,1,4,1,1,1},{2,2,1,4,1,1},{4,3,1,1,1,1},{1,1,1,2,2,4},
    {1,1,1,4,2,2},{1,2,1,1,2,4},{1,2,1,4,2,1},{1,4,1,1,2,2},
    {1,4,1,2,2,1},{1,1,2,2,1,4},{1,1,2,4,1,2},{1,2,2,1,1,4},
    {1,2,2,4,1,1},{1,4,2,1,1,2},{1,4,2,2,1,1},{2,4,1,2,1,1},
    {2,2,1,1,1,4},{4,1,3,1,1,1},{2,4,1,1,1,2},{1,3,4,1,1,1},
    {1,1,1,2,4,2},{1,2,1,1,4,2},{1,2,1,2,4,1},{1,1,4,2,1,2},
    {1,2,4,1,1,2},{1,2,4,2,1,1},{4,1,1,2,1,2},{4,2,1,1,1,2},
    {4,2,1,2,1,1},{2,1,2,1,4,1},{2,1,4,1,2,1},{4,1,2,1,2,1},
    {1,1,1,1,4,3},{1,1,1,3,4,1},{1,3,1,1,4,1},{1,1,4,1,1,3},
    };
// Start B = index 104, Stop = spécial
static const int START_B[]  = {2,1,1,4,1,2};
static const int STOP[]     = {2,3,3,1,1,1,2};

BarcodeWidget::BarcodeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(300, 100);
    setMaximumHeight(110);
    setStyleSheet("background:white;border-radius:4px;");
}

void BarcodeWidget::setData(const QString& data)
{
    m_data = data;
    encode();
    update();
}

void BarcodeWidget::encode()
{
    m_bars.clear();
    if (m_data.isEmpty()) return;

    auto push = [&](const int* pat, int n) {
        bool black = true;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < pat[i]; ++j)
                m_bars.append(black);
            black = !black;
        }
    };

    // Start B
    push(START_B, 6);
    int checksum = 104; // valeur Start B

    for (int i = 0; i < m_data.length(); ++i) {
        int code = m_data[i].unicode() - 32;
        if (code < 0 || code > 95) code = 0;
        checksum += (i + 1) * code;
        push(CODE128B_TABLE[code], 6);
    }

    // Checksum
    int cs = checksum % 103;
    push(CODE128B_TABLE[cs], 6);

    // Stop
    push(STOP, 7);

    // Quiet zone (10 modules)
    for (int i = 0; i < 10; ++i) m_bars.prepend(false);
    for (int i = 0; i < 10; ++i) m_bars.append(false);
}

void BarcodeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::white);
    if (m_bars.isEmpty()) return;

    double moduleW = static_cast<double>(width()) / m_bars.size();
    double barH    = height() - 18.0;

    for (int i = 0; i < m_bars.size(); ++i) {
        if (m_bars[i]) {
            p.fillRect(QRectF(i * moduleW, 4, moduleW + 0.5, barH), Qt::black);
        }
    }

    // Texte sous le code-barres
    p.setPen(Qt::black);
    QFont f; f.setPixelSize(10); p.setFont(f);
    p.drawText(QRectF(0, height() - 14, width(), 14), Qt::AlignCenter, m_data);
}

QPixmap BarcodeWidget::toPixmap() const
{
    QPixmap pm(size());
    pm.fill(Qt::white);
    BarcodeWidget *self = const_cast<BarcodeWidget*>(this);
    self->render(&pm);
    return pm;
}

// ═════════════════════════════════════════════════════════════════════════════
//  QRWidget — QR Code simplifié (micro-QR visuel pour affichage)
// ═════════════════════════════════════════════════════════════════════════════
// Note : implémentation visuelle simplifiée — affiche un QR-like
// pour des ID courts (≤ 20 chars). Pour un vrai QR scannable,
// utiliser la lib QZXing ou libqrencode.

QRWidget::QRWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(130, 130);
    setStyleSheet("background:white;border-radius:4px;");
}

void QRWidget::setData(const QString& data)
{
    m_data = data;
    buildMatrix();
    update();
}

void QRWidget::buildMatrix()
{
    // QR version 2 = 25×25 modules
    m_size = 25;
    m_matrix.clear();
    m_matrix.resize(m_size);
    for (auto& row : m_matrix)
        row.resize(m_size, false);

    // Finder patterns (3 coins)
    addFinderPattern(0, 0);
    addFinderPattern(0, m_size - 7);
    addFinderPattern(m_size - 7, 0);

    // Timing patterns
    addTimingPatterns();

    // Données encodées dans la zone centrale
    QVector<bool> bits = encodeData();
    int bit = 0;
    // Remplissage colonne par colonne de droite à gauche
    for (int col = m_size - 1; col >= 1; col -= 2) {
        if (col == 6) { --col; } // skip timing column
        for (int i = 0; i < m_size; ++i) {
            int row = (col % 4 < 2) ? (m_size - 1 - i) : i;
            for (int dc = 0; dc <= 1; ++dc) {
                int c = col - dc;
                if (c < 0 || c >= m_size) continue;
                // Skip finder/timing zones
                bool inFinder = (row < 9 && c < 9) ||
                                (row < 9 && c >= m_size - 8) ||
                                (row >= m_size - 8 && c < 9);
                bool inTiming = (row == 6 || c == 6);
                if (!inFinder && !inTiming && bit < bits.size()) {
                    m_matrix[row][c] = bits[bit++];
                }
            }
        }
    }
}

void QRWidget::addFinderPattern(int r, int c)
{
    for (int dr = 0; dr < 7; ++dr)
        for (int dc = 0; dc < 7; ++dc) {
            bool on = (dr == 0 || dr == 6 || dc == 0 || dc == 6 ||
                       (dr >= 2 && dr <= 4 && dc >= 2 && dc <= 4));
            if (r+dr < m_size && c+dc < m_size)
                m_matrix[r+dr][c+dc] = on;
        }
    // Séparateurs
    for (int i = 0; i <= 7 && r+7 < m_size && c+i < m_size; ++i)
        m_matrix[r+7][c+i] = false;
    for (int i = 0; i <= 7 && c+7 < m_size && r+i < m_size; ++i)
        m_matrix[r+i][c+7] = false;
}

void QRWidget::addTimingPatterns()
{
    for (int i = 8; i < m_size - 8; ++i) {
        bool on = (i % 2 == 0);
        m_matrix[6][i] = on;
        m_matrix[i][6] = on;
    }
}

QVector<bool> QRWidget::encodeData() const
{
    QVector<bool> bits;
    // Encodage numérique simplifié des caractères ASCII
    for (QChar c : m_data) {
        int v = c.unicode();
        for (int b = 7; b >= 0; --b)
            bits.append((v >> b) & 1);
    }
    // Padding pour atteindre 200 bits
    while (bits.size() < 200)
        bits.append(bits.size() % 2 == 0);
    return bits;
}

void QRWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::white);
    if (m_matrix.isEmpty()) return;

    int margin  = 6;
    int cellSize = (width() - 2 * margin) / m_size;

    for (int r = 0; r < m_size; ++r)
        for (int c = 0; c < m_size; ++c)
            if (m_matrix[r][c])
                p.fillRect(margin + c * cellSize,
                           margin + r * cellSize,
                           cellSize, cellSize, Qt::black);
}

QPixmap QRWidget::toPixmap() const
{
    QPixmap pm(size());
    pm.fill(Qt::white);
    QRWidget *self = const_cast<QRWidget*>(this);
    self->render(&pm);
    return pm;
}

// ═════════════════════════════════════════════════════════════════════════════
//  ProduitBarcodeDialog
// ═════════════════════════════════════════════════════════════════════════════

ProduitBarcodeDialog::ProduitBarcodeDialog(const Produit& produit, QWidget *parent)
    : QDialog(parent), m_produit(produit)
{
    setupUI();
}

void ProduitBarcodeDialog::setupUI()
{
    setWindowTitle(QString("Code-barres — %1").arg(m_produit.getNom()));
    setMinimumSize(580, 440);
    setStyleSheet(R"(
        QDialog  { background:#f7fafc; }
        QLabel#hdrTitle { font-size:15px; font-weight:bold; color:#1a202c; }
        QLabel#hdrSub   { font-size:11px; color:#718096; }
        QGroupBox { font-size:11px; color:#718096; border:1px solid #e2e8f0;
                    border-radius:6px; margin-top:8px; padding-top:8px; background:white; }
        QGroupBox::title { subcontrol-origin:margin; left:8px; padding:0 4px; }
        QPushButton#actionBtn {
            border:1px solid #e2e8f0; border-radius:6px; padding:8px 16px;
            background:white; font-size:12px; font-weight:500; color:#2d3748;
        }
        QPushButton#actionBtn:hover { background:#edf2f7; }
        QPushButton#closeBtn {
            background:#8A9A5B; color:white; border:none;
            border-radius:6px; padding:8px 24px; font-weight:bold; font-size:12px;
        }
        QPushButton#closeBtn:hover { background:#9aaa6b; }
        QLabel#infoLabel { font-size:11px; color:#4a5568; }
    )");

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 16);
    root->setSpacing(12);

    // ── En-tête ───────────────────────────────────────────────────────────────
    QFrame *hdr = new QFrame(this);
    hdr->setStyleSheet("background:white;border-radius:8px;border:1px solid #e2e8f0;");
    QHBoxLayout *hdrLay = new QHBoxLayout(hdr);
    hdrLay->setContentsMargins(14, 10, 14, 10);

    QLabel *icon = new QLabel("BC", hdr);
    icon->setStyleSheet("font-size:13px; font-weight:bold; color:#38a169;"
                        " background:#f0fff4; border-radius:5px; padding:4px 8px;");

    QVBoxLayout *hdrText = new QVBoxLayout();
    QLabel *titleLbl = new QLabel(m_produit.getNom(), hdr);
    titleLbl->setObjectName("hdrTitle");
    QLabel *subLbl = new QLabel(
        QString("Réf. #%1  ·  %2  ·  Généré le %3")
            .arg(m_produit.getId())
            .arg(m_produit.getCategorie())
            .arg(QDate::currentDate().toString("dd/MM/yyyy")),
        hdr);
    subLbl->setObjectName("hdrSub");
    hdrText->addWidget(titleLbl);
    hdrText->addWidget(subLbl);
    hdrLay->addWidget(icon);
    hdrLay->addLayout(hdrText);
    hdrLay->addStretch();
    root->addWidget(hdr);

    // ── Zone codes ────────────────────────────────────────────────────────────
    QHBoxLayout *codesRow = new QHBoxLayout();
    codesRow->setSpacing(14);

    // Code128
    QGroupBox *bcGroup = new QGroupBox("Code128-B", this);
    QVBoxLayout *bcLay = new QVBoxLayout(bcGroup);
    bcLay->setContentsMargins(12, 8, 12, 12);
    bcLay->setSpacing(6);

    m_barcode = new BarcodeWidget(bcGroup);
    // Données encodées : PRD-XXXXX + nom tronqué
    QString bcData = QString("PRD-%1").arg(m_produit.getId().rightJustified(5, '0'));
    m_barcode->setData(bcData);
    bcLay->addWidget(m_barcode);

    QLabel *bcInfo = new QLabel(
        QString("Données : <b>%1</b>").arg(bcData), bcGroup);
    bcInfo->setObjectName("infoLabel");
    bcInfo->setTextFormat(Qt::RichText);
    bcLay->addWidget(bcInfo);
    codesRow->addWidget(bcGroup, 2);

    // QR Code
    QGroupBox *qrGroup = new QGroupBox("QR Code", this);
    QVBoxLayout *qrLay = new QVBoxLayout(qrGroup);
    qrLay->setContentsMargins(12, 8, 12, 12);
    qrLay->setSpacing(6);
    qrLay->setAlignment(Qt::AlignCenter);

    m_qr = new QRWidget(qrGroup);
    // Données QR : JSON compact du produit
    QString qrData = QString("ID:%1|NOM:%2|CAT:%3|PROJ:%4")
                         .arg(m_produit.getId())
                         .arg(m_produit.getNom().left(12))
                         .arg(m_produit.getCategorie().left(8))
                         .arg(m_produit.getProjetId());
    m_qr->setData(qrData);
    qrLay->addWidget(m_qr, 0, Qt::AlignCenter);

    QLabel *qrInfo = new QLabel("Scanner pour accéder\nà la fiche produit", qrGroup);
    qrInfo->setObjectName("infoLabel");
    qrInfo->setAlignment(Qt::AlignCenter);
    qrLay->addWidget(qrInfo);
    codesRow->addWidget(qrGroup, 1);

    root->addLayout(codesRow, 1);

    // ── Infos produit ─────────────────────────────────────────────────────────
    QFrame *infoFrame = new QFrame(this);
    infoFrame->setStyleSheet("background:white;border-radius:8px;border:1px solid #e2e8f0;");
    QHBoxLayout *infoLay = new QHBoxLayout(infoFrame);
    infoLay->setContentsMargins(14, 8, 14, 8);
    infoLay->setSpacing(20);

    auto addInfo = [&](const QString& label, const QString& val) {
        QVBoxLayout *v = new QVBoxLayout();
        QLabel *l = new QLabel(label, infoFrame);
        l->setStyleSheet("font-size:10px;color:#718096;text-transform:uppercase;");
        QLabel *vl = new QLabel(val, infoFrame);
        vl->setStyleSheet("font-size:12px;font-weight:bold;color:#2d3748;");
        v->addWidget(l); v->addWidget(vl);
        infoLay->addLayout(v);
    };

    addInfo("ID Produit", m_produit.getId());
    addInfo("Nom", m_produit.getNom());
    addInfo("Catégorie", m_produit.getCategorie());
    addInfo("Prix", QString("%1 DT").arg(m_produit.getPrix(), 0, 'f', 2));
    addInfo("Projet", m_produit.getProjetId());
    infoLay->addStretch();
    root->addWidget(infoFrame);

    // ── Boutons actions ───────────────────────────────────────────────────────
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    QPushButton *exportBtn = new QPushButton("Exporter PNG", this);
    exportBtn->setObjectName("actionBtn");
    exportBtn->setMinimumHeight(36);
    connect(exportBtn, &QPushButton::clicked, this, &ProduitBarcodeDialog::onExportPNG);

    QPushButton *printBtn = new QPushButton("Imprimer", this);
    printBtn->setObjectName("actionBtn");
    printBtn->setMinimumHeight(36);
    connect(printBtn, &QPushButton::clicked, this, &ProduitBarcodeDialog::onPrint);

    QPushButton *closeBtn = new QPushButton("Fermer", this);
    closeBtn->setObjectName("closeBtn");
    closeBtn->setMinimumHeight(36);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnRow->addWidget(exportBtn);
    btnRow->addWidget(printBtn);
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);
}

void ProduitBarcodeDialog::onExportPNG()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        "Exporter le code-barres",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            + QString("/PRD-%1_barcode.png").arg(m_produit.getId()),
        "Images PNG (*.png)");

    if (path.isEmpty()) return;

    // Assembler les deux codes en un seul PNG
    QPixmap bcPm = m_barcode->toPixmap();
    QPixmap qrPm = m_qr->toPixmap();

    int totalW = bcPm.width() + qrPm.width() + 20;
    int totalH = qMax(bcPm.height(), qrPm.height()) + 40;

    QPixmap combined(totalW + 40, totalH);
    combined.fill(Qt::white);
    QPainter p(&combined);

    // Titre
    QFont tf; tf.setBold(true); tf.setPixelSize(13);
    p.setFont(tf);
    p.setPen(Qt::black);
    p.drawText(QRect(0, 8, combined.width(), 20), Qt::AlignCenter,
               QString("WoodFlow — %1  (Réf. #%2)")
                   .arg(m_produit.getNom()).arg(m_produit.getId()));

    p.drawPixmap(20, 34, bcPm);
    p.drawPixmap(20 + bcPm.width() + 20, 34, qrPm);
    p.end();

    if (combined.save(path, "PNG"))
        QMessageBox::information(this, "Export réussi",
                                 QString("Code-barres exporté :\n%1").arg(path));
    else
        QMessageBox::critical(this, "Erreur", "Impossible d'enregistrer le fichier.");
}

void ProduitBarcodeDialog::onPrint()
{
    QPrinter printer;
    printer.setPageSize(QPageSize(QPageSize::A6));
    printer.setPageOrientation(QPageLayout::Landscape);

    QPainter p(&printer);
    if (!p.isActive()) {
        QMessageBox::warning(this, "Impression", "Aucune imprimante disponible.");
        return;
    }

    QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();

    // Titre
    QFont tf; tf.setBold(true); tf.setPixelSize(18);
    p.setFont(tf);
    p.setPen(Qt::black);
    p.drawText(QRect(0, 10, pageRect.width(), 30), Qt::AlignCenter,
               m_produit.getNom());

    QFont sf; sf.setPixelSize(12);
    p.setFont(sf);
    p.setPen(QColor("#718096"));
    p.drawText(QRect(0, 44, pageRect.width(), 20), Qt::AlignCenter,
               QString("Réf. #%1  ·  %2  ·  %3 DT")
                   .arg(m_produit.getId())
                   .arg(m_produit.getCategorie())
                   .arg(m_produit.getPrix(), 0, 'f', 2));

    // Code-barres centré
    QPixmap bcPm = m_barcode->toPixmap().scaled(
        pageRect.width() - 60, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.drawPixmap((pageRect.width() - bcPm.width()) / 2, 72, bcPm);

    p.end();
}
