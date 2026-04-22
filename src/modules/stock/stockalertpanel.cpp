#include "stockalertpanel.h"

#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QScreen>
#include <QFocusEvent>
#include <QGraphicsDropShadowEffect>

// ============================================================================
//  StockAlertPanel
// ============================================================================

StockAlertPanel::StockAlertPanel(QWidget *parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);
    setFixedWidth(340);

    // Layout principal du panneau
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(0);

    // Carte blanche avec coins arrondis et ombre portée
    QFrame *card = new QFrame(this);
    card->setObjectName("alertPanelCard");
    card->setStyleSheet(
        "QFrame#alertPanelCard {"
        "  background: palette(base);"
        "  border-radius: 12px;"
        "  border: 1px solid rgba(0,0,0,0.10);"
        "}"
        );

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 55));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // ── En-tête ─────────────────────────────────────────────────────────────
    QFrame *header = new QFrame(card);
    header->setStyleSheet(
        "background: transparent;"
        "border-bottom: 1px solid rgba(0,0,0,0.07);"
        "border-radius: 0px;"
        );
    QHBoxLayout *hLay = new QHBoxLayout(header);
    hLay->setContentsMargins(16, 12, 16, 12);

    QLabel *title = new QLabel("Alertes stock", header);
    {
        QFont f = title->font();
        f.setPixelSize(14);
        f.setBold(true);
        title->setFont(f);
    }

    // Petit point rouge animé
    QLabel *dot = new QLabel(header);
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(
        "background: #e53e3e;"
        "border-radius: 4px;"
        );

    hLay->addWidget(dot);
    hLay->addWidget(title);
    hLay->addStretch();
    cardLayout->addWidget(header);

    // ── Zone scrollable des cartes ───────────────────────────────────────────
    m_scroll = new QScrollArea(card);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setStyleSheet("background: transparent;");
    m_scroll->setMaximumHeight(380);

    m_container = new QWidget();
    m_container->setStyleSheet("background: transparent;");
    m_cardLayout = new QVBoxLayout(m_container);
    m_cardLayout->setContentsMargins(8, 8, 8, 8);
    m_cardLayout->setSpacing(6);

    m_emptyLabel = new QLabel("Aucun matériau en alerte 🎉", m_container);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    {
        QFont f = m_emptyLabel->font();
        f.setPixelSize(12);
        m_emptyLabel->setFont(f);
    }
    m_emptyLabel->setStyleSheet("color: #6b7280; padding: 20px;");
    m_cardLayout->addWidget(m_emptyLabel);
    m_cardLayout->addStretch();

    m_scroll->setWidget(m_container);
    cardLayout->addWidget(m_scroll);

    root->addWidget(card);
}

// ---------------------------------------------------------------------------
//  populate : reconstruit les cartes à partir de la liste fournie
// ---------------------------------------------------------------------------
void StockAlertPanel::populate(const QList<StockMaterial>& alertMaterials)
{
    clearCards();

    if (alertMaterials.isEmpty()) {
        m_emptyLabel->setVisible(true);
        m_cardLayout->addStretch();
        return;
    }

    m_emptyLabel->setVisible(false);

    for (const StockMaterial& mat : alertMaterials)
        buildCard(mat);

    m_cardLayout->addStretch();
}

// ---------------------------------------------------------------------------
//  buildCard : crée une carte pour un matériau en alerte
// ---------------------------------------------------------------------------
void StockAlertPanel::buildCard(const StockMaterial& mat)
{
    bool isDark = qApp->styleSheet().contains("0f0f0f");

    QFrame *card = new QFrame(m_container);
    card->setCursor(Qt::PointingHandCursor);
    card->setObjectName("notifCard");

    // Style de la carte
    QString cardStyle = isDark
                            ? "QFrame#notifCard { background: #2a2a2a; border-radius: 10px; border: 1px solid #3a3a3a; }"
                              "QFrame#notifCard:hover { background: #333333; border: 1px solid #e53e3e; }"
                            : "QFrame#notifCard { background: #fafafa; border-radius: 10px; border: 1px solid #f0f0f0; }"
                              "QFrame#notifCard:hover { background: #fff5f5; border: 1px solid #e53e3e; }";
    card->setStyleSheet(cardStyle);

    QVBoxLayout *vLay = new QVBoxLayout(card);
    vLay->setContentsMargins(12, 10, 12, 10);
    vLay->setSpacing(6);

    // ── Ligne 1 : icône + nom + niveau critique ──────────────────────────────
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(8);

    // Icône d'alerte
    QLabel *icon = new QLabel("!!!", card);
    icon->setStyleSheet("color: #e53e3e; font-size: 14px;");
    icon->setFixedWidth(20);

    // Nom du matériau
    QLabel *nomLbl = new QLabel(mat.getNom(), card);
    {
        QFont f = nomLbl->font();
        f.setPixelSize(13);
        f.setBold(true);
        nomLbl->setFont(f);
    }
    nomLbl->setStyleSheet(isDark ? "color: #f0f0f0;" : "color: #1f2937;");

    // Badge critique / très bas
    double ratio = (mat.getSeuilAlerte() > 0)
                       ? mat.getQuantite() / mat.getSeuilAlerte()
                       : 0.0;
    QString badgeText = ratio <= 0.5 ? "Critique" : "Bas";
    QColor  badgeCol  = ratio <= 0.5 ? QColor("#e53e3e") : QColor("#dd6b20");
    QLabel *badge = new QLabel(badgeText, card);
    {
        QFont f = badge->font();
        f.setPixelSize(9);
        f.setBold(true);
        badge->setFont(f);
    }
    badge->setStyleSheet(QString(
                             "color: white; background: %1; border-radius: 4px; padding: 1px 6px;"
                             ).arg(badgeCol.name()));

    row1->addWidget(icon);
    row1->addWidget(nomLbl, 1);
    row1->addWidget(badge);
    vLay->addLayout(row1);

    // ── Ligne 2 : quantité / seuil ───────────────────────────────────────────
    QLabel *detailLbl = new QLabel(
        QString("Stock : <b>%1</b> %2  |  Seuil : %3 %4")
            .arg(mat.getQuantite(), 0, 'f', 1)
            .arg(mat.getUnite())
            .arg(mat.getSeuilAlerte(), 0, 'f', 1)
            .arg(mat.getUnite()),
        card
        );
    detailLbl->setTextFormat(Qt::RichText);
    {
        QFont f = detailLbl->font();
        f.setPixelSize(11);
        detailLbl->setFont(f);
    }
    detailLbl->setStyleSheet(isDark ? "color: #9ca3af;" : "color: #6b7280;");
    vLay->addWidget(detailLbl);

    // ── Ligne 3 : barre de progression ──────────────────────────────────────
    QProgressBar *bar = new QProgressBar(card);
    bar->setFixedHeight(5);
    bar->setRange(0, 100);
    bar->setValue(qBound(0, (int)(ratio * 100), 100));
    bar->setTextVisible(false);

    QString barColor = ratio <= 0.5 ? "#e53e3e" : "#dd6b20";
    bar->setStyleSheet(QString(
                           "QProgressBar { background: %1; border-radius: 2px; }"
                           "QProgressBar::chunk { background: %2; border-radius: 2px; }"
                           ).arg(isDark ? "#3a3a3a" : "#e5e7eb").arg(barColor));
    vLay->addWidget(bar);

    // ── Ligne 4 : fournisseur ────────────────────────────────────────────────
    if (!mat.getFournisseur().isEmpty()) {
        QLabel *fourn = new QLabel("Fournisseur : " + mat.getFournisseur(), card);
        {
            QFont f = fourn->font();
            f.setPixelSize(10);
            fourn->setFont(f);
        }
        fourn->setStyleSheet(isDark ? "color: #6b7280;" : "color: #9ca3af;");
        vLay->addWidget(fourn);
    }

    m_cardLayout->addWidget(card);

    // Clic sur la carte → fermer le panneau + sélectionner dans la table
    int matId = mat.getId();
    connect(card, &QFrame::destroyed, [](){});

    // Utiliser un eventFilter sur la carte pour intercepter le clic
    card->installEventFilter(this);
    card->setProperty("materialId", matId);
}

// ---------------------------------------------------------------------------
//  eventFilter : intercepte les clics sur les cartes
// ---------------------------------------------------------------------------
bool StockAlertPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QFrame *card = qobject_cast<QFrame*>(watched);
        if (card) {
            int id = card->property("materialId").toInt();
            hide();
            emit alertSelected(id);
            return true;
        }
    }
    return QFrame::eventFilter(watched, event);
}

// ---------------------------------------------------------------------------
//  clearCards : supprime toutes les cartes existantes
// ---------------------------------------------------------------------------
void StockAlertPanel::clearCards()
{
    QLayoutItem *item;
    while ((item = m_cardLayout->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != m_emptyLabel)
            item->widget()->deleteLater();
        delete item;
    }
    m_cardLayout->addWidget(m_emptyLabel);
}

// ---------------------------------------------------------------------------
//  showUnder : affiche le panneau positionné sous l'ancre
// ---------------------------------------------------------------------------
void StockAlertPanel::showUnder(QWidget *anchor)
{
    QPoint globalPos = anchor->mapToGlobal(QPoint(0, anchor->height() + 4));

    // Éviter de déborder à droite de l'écran
    QScreen *screen = QApplication::primaryScreen();
    int screenRight = screen->geometry().right();
    if (globalPos.x() + width() > screenRight)
        globalPos.setX(screenRight - width() - 8);

    move(globalPos);
    show();
    raise();
    setFocus();
}

// ---------------------------------------------------------------------------
//  paintEvent : fond transparent (le style est sur la card interne)
// ---------------------------------------------------------------------------
void StockAlertPanel::paintEvent(QPaintEvent *)
{
    // Fond transparent — le QFrame interne gère son propre fond
}

// ---------------------------------------------------------------------------
//  focusOutEvent : ferme le panneau si on clique ailleurs
// ---------------------------------------------------------------------------
void StockAlertPanel::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    hide();
}
