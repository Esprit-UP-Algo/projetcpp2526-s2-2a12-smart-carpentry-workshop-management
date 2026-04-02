#include "financeview.h"
#include "financemodel.h"
#include "transactiondialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>
#include <QDebug>
#include <QScrollArea>
#include <QPainter>
#include <QGridLayout>
#include <QDate>

FinanceView::FinanceView(QWidget *parent)
    : QWidget(parent)
    , m_model(nullptr)
{
    setupUI();
    setupConnections();
}

FinanceView::~FinanceView()
{
}

void FinanceView::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(18);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Section filtres
    QFrame *filterFrame = new QFrame();
    filterFrame->setObjectName("searchFrame");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(0, 0, 0, 10);
    filterLayout->setSpacing(10);

    QLabel *searchLabel = new QLabel("Recherche");
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("ID");
    m_searchEdit->setMinimumWidth(200);

    QLabel *typeLabel = new QLabel("Type :");
    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({"Tous", "Facture", "Devis", "Acompte"});

    QLabel *categoryLabel = new QLabel("Catégorie :");
    m_categoryCombo = new QComboBox();
    m_categoryCombo->addItems({"Toutes", "Recette", "Depense"});

    m_filterButton = new QPushButton("Filtrer");
    m_resetButton = new QPushButton("Réinitialiser");

    filterLayout->addWidget(searchLabel);
    filterLayout->addWidget(m_searchEdit);
    filterLayout->addWidget(typeLabel);
    filterLayout->addWidget(m_typeCombo);
    filterLayout->addWidget(categoryLabel);
    filterLayout->addWidget(m_categoryCombo);
    filterLayout->addWidget(m_filterButton);
    filterLayout->addWidget(m_resetButton);
    filterLayout->addStretch();

    mainLayout->addWidget(filterFrame);

    // Table des transactions
    m_table = new QTableWidget();
    m_table->setObjectName("financeTable");
    m_table->setColumnCount(7);

    QStringList headers = {
        "ID", "TYPE", "MODE PAIEMENT", "STATUT",
        "CATÉGORIE", "MONTANT (DT)", "DATE"
    };
    m_table->setHorizontalHeaderLabels(headers);

    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_table, 1);

    // Barre d'actions
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);

    m_addButton = new QPushButton("+ Nouvelle Transaction");
    m_deleteButton = new QPushButton("Supprimer");
    m_exportButton = new QPushButton("Exporter Excel");
    m_statsButton = new QPushButton("Statistiques détaillées");

    actionLayout->addWidget(m_addButton);
    actionLayout->addWidget(m_deleteButton);
    actionLayout->addWidget(m_exportButton);
    actionLayout->addWidget(m_statsButton);
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);
}

void FinanceView::setupConnections()
{
    connect(m_addButton, &QPushButton::clicked, this, &FinanceView::onAddClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &FinanceView::onDeleteClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &FinanceView::onExportClicked);
    connect(m_statsButton, &QPushButton::clicked, this, &FinanceView::onStatsClicked);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &FinanceView::onCellDoubleClicked);
    connect(m_filterButton, &QPushButton::clicked, this, &FinanceView::onFilterChanged);
    connect(m_resetButton, &QPushButton::clicked, this, &FinanceView::onResetFilters);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &FinanceView::onFilterChanged);
    connect(m_typeCombo, &QComboBox::currentTextChanged, this, &FinanceView::onFilterChanged);
    connect(m_categoryCombo, &QComboBox::currentTextChanged, this, &FinanceView::onFilterChanged);
}

void FinanceView::setModel(FinanceModel *model)
{
    m_model = model;
    connect(m_model, &FinanceModel::dataChanged, this, &FinanceView::onModelDataChanged);
    connect(m_model, &FinanceModel::errorOccurred, this, &FinanceView::onModelError);
    loadTransactions();
}

void FinanceView::loadTransactions()
{
    if (m_model) {
        m_currentTransactions = m_model->loadTransactions();
        refreshTable();
    }
}

void FinanceView::refreshTable()
{
    m_table->setRowCount(0);

    for (int i = 0; i < m_currentTransactions.size(); ++i) {
        const auto &t = m_currentTransactions[i];
        m_table->insertRow(i);

        m_table->setItem(i, 0, new QTableWidgetItem(t.id));
        m_table->setItem(i, 1, new QTableWidgetItem(t.type));
        m_table->setItem(i, 2, new QTableWidgetItem(t.modePaiement));
        m_table->setItem(i, 3, new QTableWidgetItem(t.statut));
        m_table->setItem(i, 4, new QTableWidgetItem(t.categorie));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(t.montant, 'f', 3)));
        m_table->setItem(i, 6, new QTableWidgetItem(t.date));

        m_table->setRowHeight(i, 52);
    }
}

void FinanceView::applyFilters()
{
    QString search = m_searchEdit->text().trimmed();
    QString type = m_typeCombo->currentText();
    QString category = m_categoryCombo->currentText();

    for (int row = 0; row < m_table->rowCount(); row++) {
        bool visible = true;

        if (!search.isEmpty()) {
            QTableWidgetItem *idItem = m_table->item(row, 0);
            if (!idItem || !idItem->text().contains(search, Qt::CaseInsensitive))
                visible = false;
        }

        if (visible && type != "Tous") {
            QTableWidgetItem *typeItem = m_table->item(row, 1);
            if (!typeItem || typeItem->text() != type)
                visible = false;
        }

        if (visible && category != "Toutes") {
            QTableWidgetItem *catItem = m_table->item(row, 4);
            if (!catItem || catItem->text() != category)
                visible = false;
        }

        m_table->setRowHidden(row, !visible);
    }
}

void FinanceView::onAddClicked()
{
    TransactionDialog dialog(this);
    dialog.setWindowTitle("Nouvelle Transaction");

    if (dialog.exec() == QDialog::Accepted) {
        if (m_model && m_model->insertTransaction(dialog.getData())) {
            loadTransactions();
            QMessageBox::information(this, "Succès", "Transaction ajoutée avec succès !");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible d'ajouter la transaction.");
        }
    }
}

void FinanceView::onEditClicked(int row)
{
    if (row < 0 || row >= m_currentTransactions.size()) {
        QMessageBox::warning(this, "Erreur", "Aucune transaction sélectionnée.");
        return;
    }

    const auto &transaction = m_currentTransactions[row];

    TransactionDialog dialog(this);
    dialog.setWindowTitle("Modifier Transaction");

    QMap<QString, QString> data;
    data["TYPE_TRAN"] = transaction.type;
    data["MODE_PAIEMENT"] = transaction.modePaiement;
    data["STATUT_TRAN"] = transaction.statut;
    data["CATEGORIE_TRAN"] = transaction.categorie;
    data["MONTANT_TRAN"] = QString::number(transaction.montant, 'f', 3);
    data["DATE_TRAN"] = transaction.date;
    dialog.setData(data);

    if (dialog.exec() == QDialog::Accepted) {
        if (m_model && m_model->updateTransaction(transaction.id, dialog.getData())) {
            loadTransactions();
            QMessageBox::information(this, "Succès", "Transaction modifiée avec succès !");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible de modifier la transaction.");
        }
    }
}

void FinanceView::onDeleteClicked()
{
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Aucune sélection", "Veuillez sélectionner une transaction à supprimer.");
        return;
    }

    QString id = m_table->item(row, 0)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmer la suppression",
        "Supprimer cette transaction ?\n\nCette action est irréversible.",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_model && m_model->deleteTransaction(id)) {
            loadTransactions();
            QMessageBox::information(this, "Succès", "Transaction supprimée avec succès !");
        } else {
            QMessageBox::warning(this, "Erreur", "Impossible de supprimer la transaction.");
        }
    }
}

void FinanceView::onExportClicked()
{
    if (!m_model) return;

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter le bilan financier",
                                                    QString("bilan_financier_%1.xls").arg(QDate::currentDate().toString("yyyyMMdd")),
                                                    "Fichiers Excel (*.xls);;Tous les fichiers (*.*)");

    if (fileName.isEmpty()) return;

    auto stats = m_model->getStatistics();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Erreur", "Impossible de créer le fichier.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "BILAN FINANCIER\n";
    out << "Généré le " << QDate::currentDate().toString("dd/MM/yyyy") << "\n\n";
    out << "INDICATEURS CLÉS\n";
    out << "Total Recettes: " << QString::number(stats.totalRecettes, 'f', 3) << " DT\n";
    out << "Total Dépenses: " << QString::number(stats.totalDepenses, 'f', 3) << " DT\n";
    out << "Solde Net: " << QString::number(stats.totalRecettes - stats.totalDepenses, 'f', 3) << " DT\n\n";
    out << "Total Transactions: " << stats.totalTransactions << "\n";

    file.close();

    QMessageBox::information(this, "Export réussi", "Le bilan financier a été exporté avec succès.");
}

void FinanceView::onStatsClicked()
{
    if (!m_model) return;

    auto stats = m_model->getStatistics();

    double totalRecettes     = stats.totalRecettes;
    double totalDepenses     = stats.totalDepenses;
    double totalGeneral      = stats.totalGeneral;
    int    totalTransactions = stats.totalTransactions;

    QMap<QString, double> statsByStatus    = stats.statsByStatus;
    QMap<QString, double> statsByMode      = stats.statsByMode;
    QMap<QString, double> statsByCategorie = stats.statsByCategorie;
    QMap<QString, double> statsByMonth     = stats.statsByMonth;

    // ── Dialog shell ─────────────────────────────────────────────────
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Statistiques Financières");
    dlg->setMinimumSize(1000, 720);
    dlg->setStyleSheet(R"(
        QDialog {
            background: #f7f5f0;
            color: #2c3e2f;
            font-family: 'Segoe UI', 'Inter', system-ui, sans-serif;
        }
        QLabel { color: #2c3e2f; }
        QPushButton#closeBtn {
            background: #7c8f6e;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 32px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton#closeBtn:hover { background: #6b7d5e; }
        QScrollArea { border: none; background: transparent; }
        QWidget#scrollContent { background: transparent; }
        QScrollBar:vertical { background: #e8e2d8; width: 8px; border-radius: 4px; }
        QScrollBar::handle:vertical { background: #c2b6a6; border-radius: 4px; min-height: 20px; }
        QScrollBar::handle:vertical:hover { background: #a69682; }
    )");

    QVBoxLayout *root = new QVBoxLayout(dlg);
    root->setContentsMargins(32, 28, 32, 28);
    root->setSpacing(24);

    // ── Header ───────────────────────────────────────────────────────
    QLabel *title = new QLabel("Tableau de Bord Financier");
    title->setStyleSheet("font-size:24px;font-weight:600;color:#3a5a3e;letter-spacing:-0.3px;margin-bottom:4px;");
    QLabel *subtitle = new QLabel(QString("Analyse de %1 transactions · Montant total : %2 DT")
                                      .arg(totalTransactions)
                                      .arg(totalGeneral, 0, 'f', 3));
    subtitle->setStyleSheet("font-size:13px;color:#7f8c6e;");
    root->addWidget(title);
    root->addWidget(subtitle);

    // ── KPI row ──────────────────────────────────────────────────────
    auto makeKpi = [](const QString &label, const QString &value, const QString &color) -> QFrame* {
        QFrame *card = new QFrame();
        card->setMinimumHeight(95);
        card->setStyleSheet("QFrame{background:#ffffff;border-radius:12px;border:1px solid #e2dcd2;}");
        QVBoxLayout *l = new QVBoxLayout(card);
        l->setContentsMargins(18, 14, 18, 14);
        l->setSpacing(8);
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size:11px;color:#9b8e7c;font-weight:500;letter-spacing:0.3px;text-transform:uppercase;");
        QLabel *val = new QLabel(value);
        val->setStyleSheet(QString("font-size:24px;font-weight:700;color:%1;").arg(color));
        l->addWidget(lbl);
        l->addWidget(val);
        return card;
    };

    QHBoxLayout *kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(14);
    kpiRow->addWidget(makeKpi("Total Recettes",
                              QString::number(totalRecettes, 'f', 3) + " DT", "#6f8f5e"));
    kpiRow->addWidget(makeKpi("Total Dépenses",
                              QString::number(totalDepenses, 'f', 3) + " DT", "#b5826e"));
    kpiRow->addWidget(makeKpi("Solde Net",
                              QString::number(totalRecettes - totalDepenses, 'f', 3) + " DT",
                              (totalRecettes - totalDepenses) >= 0 ? "#6f8f5e" : "#b5826e"));
    kpiRow->addWidget(makeKpi("En Attente",
                              QString::number(statsByStatus.value("En attente", 0), 'f', 3) + " DT", "#caa87b"));
    root->addLayout(kpiRow);

    // ── Scroll area ──────────────────────────────────────────────────
    QScrollArea *scroll = new QScrollArea();
    QWidget *scrollContent = new QWidget();
    scrollContent->setObjectName("scrollContent");
    QVBoxLayout *chartsLayout = new QVBoxLayout(scrollContent);
    chartsLayout->setSpacing(22);
    chartsLayout->setContentsMargins(0, 0, 0, 0);
    scroll->setWidget(scrollContent);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // ── Helper: section label ────────────────────────────────────────
    auto sectionLabel = [](const QString &text) -> QLabel* {
        QLabel *l = new QLabel(text);
        l->setStyleSheet("font-size:15px;font-weight:600;color:#4a6a4e;margin-bottom:10px;"
                         "padding-left:4px;border-left:3px solid #8faa7a;");
        return l;
    };

    // ── Bar chart helper ─────────────────────────────────────────────
    auto makeBarChart = [&](const QString &sectionTitle,
                            const QMap<QString,double> &data,
                            const QList<QString> &colorList) -> QWidget*
    {
        if (data.isEmpty()) return nullptr;
        QWidget *w = new QWidget();
        QVBoxLayout *vl = new QVBoxLayout(w);
        vl->setContentsMargins(0,0,0,0);
        vl->setSpacing(10);
        vl->addWidget(sectionLabel(sectionTitle));

        QFrame *card = new QFrame();
        card->setStyleSheet("QFrame{background:#ffffff;border-radius:14px;border:1px solid #e2dcd2;}");
        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(22, 20, 22, 20);
        cl->setSpacing(12);

        double maxVal = 0;
        for (auto v : data) maxVal = qMax(maxVal, v);
        if (maxVal == 0) maxVal = 1;

        QList<QString> colors = colorList.isEmpty()
                                    ? QList<QString>{"#8faa7a","#b5826e","#caa87b","#a8967a","#9bb88b"}
                                    : colorList;
        int colorIdx = 0;

        for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
            QString key   = it.key();
            double  val   = it.value();
            QString color = colors[colorIdx++ % colors.size()];

            QHBoxLayout *row = new QHBoxLayout();
            row->setSpacing(12);

            QLabel *keyLbl = new QLabel(key);
            keyLbl->setFixedWidth(130);
            keyLbl->setStyleSheet("font-size:12px;font-weight:500;color:#6a5e4e;");
            row->addWidget(keyLbl);

            QFrame *barBg = new QFrame();
            barBg->setFixedHeight(30);
            barBg->setStyleSheet("QFrame{background:#efe6dc;border-radius:6px;}");
            barBg->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

            QFrame *barFill = new QFrame(barBg);
            barFill->setFixedHeight(30);
            barFill->setStyleSheet(QString("QFrame{background:%1;border-radius:6px;}").arg(color));
            barFill->setFixedWidth(qMax(30, static_cast<int>((val / maxVal) * 300)));

            QLabel *valLbl = new QLabel(QString("%1 DT").arg(val, 0, 'f', 3));
            valLbl->setFixedWidth(120);
            valLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            valLbl->setStyleSheet(QString("font-size:12px;font-weight:600;color:%1;").arg(color));

            row->addWidget(barBg);
            row->addWidget(valLbl);
            cl->addLayout(row);
        }
        vl->addWidget(card);
        return w;
    };

    // ── Donut widget ─────────────────────────────────────────────────
    class DonutWidget : public QWidget {
    public:
        struct Slice { QString label; double value; QColor color; };
        QList<Slice> slices;
        DonutWidget(QWidget *p = nullptr) : QWidget(p) { setMinimumSize(220, 220); }
        void paintEvent(QPaintEvent*) override {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);
            double total = 0;
            for (auto &s : slices) total += s.value;
            if (total == 0) return;
            QRectF rect(20, 20, width()-40, height()-40);
            double start = -90;
            for (auto &s : slices) {
                double span = (s.value / total) * 360.0;
                painter.setBrush(s.color);
                painter.setPen(Qt::NoPen);
                painter.drawPie(rect, static_cast<int>(start*16), static_cast<int>(span*16));
                start += span;
            }
            double inset = qMin(rect.width(), rect.height()) * 0.38;
            QRectF inner = rect.adjusted(inset, inset, -inset, -inset);
            painter.setBrush(QColor("#f7f5f0"));
            painter.drawEllipse(inner);
            painter.setPen(QColor("#6a5e4e"));
            QFont f = painter.font();
            f.setPixelSize(14); f.setBold(true);
            painter.setFont(f);
            painter.drawText(inner, Qt::AlignCenter, QString::number(total,'f',0)+"\nDT");
        }
    };

    // ── Donut: Recettes vs Dépenses ──────────────────────────────────
    {
        QFrame *card = new QFrame();
        card->setStyleSheet("QFrame{background:#ffffff;border-radius:14px;border:1px solid #e2dcd2;}");
        QHBoxLayout *hl = new QHBoxLayout(card);
        hl->setContentsMargins(22,22,22,22);
        hl->setSpacing(28);

        DonutWidget *donut = new DonutWidget();
        donut->slices = {{"Recette", totalRecettes, QColor("#8faa7a")},
                         {"Depense", totalDepenses, QColor("#b5826e")}};
        donut->setFixedSize(200, 200);

        QVBoxLayout *legend = new QVBoxLayout();
        legend->setSpacing(10);
        QLabel *chartTitle = new QLabel("Répartition Recettes / Dépenses");
        chartTitle->setStyleSheet("font-size:14px;font-weight:600;color:#4a6a4e;");
        legend->addWidget(chartTitle);
        legend->addSpacing(6);

        double catTotal = totalRecettes + totalDepenses;

        auto makeRow = [&](const QString &colorHex, const QString &lblText,
                           double val, const QString &valColor) {
            QHBoxLayout *row = new QHBoxLayout();
            row->setSpacing(8);
            QFrame *dot = new QFrame();
            dot->setFixedSize(10,10);
            dot->setStyleSheet(QString("background:%1;border-radius:5px;").arg(colorHex));
            QLabel *lbl = new QLabel(lblText);
            lbl->setStyleSheet("font-size:12px;font-weight:500;color:#6a5e4e;");
            QLabel *valLbl = new QLabel(QString("%1 DT").arg(val, 0, 'f', 3));
            valLbl->setStyleSheet(QString("font-size:12px;font-weight:600;color:%1;").arg(valColor));
            int pct = catTotal > 0 ? static_cast<int>(val / catTotal * 100) : 0;
            QLabel *pctLbl = new QLabel(QString("(%1%)").arg(pct));
            pctLbl->setStyleSheet("font-size:11px;color:#9b8e7c;");
            row->addWidget(dot); row->addWidget(lbl); row->addStretch();
            row->addWidget(valLbl); row->addWidget(pctLbl);
            legend->addLayout(row);
        };

        makeRow("#8faa7a", "Recettes", totalRecettes, "#6f8f5e");
        makeRow("#b5826e", "Dépenses", totalDepenses, "#b5826e");
        legend->addStretch();

        hl->addWidget(donut);
        hl->addLayout(legend);

        QVBoxLayout *wrap = new QVBoxLayout();
        wrap->setContentsMargins(0,0,0,0);
        wrap->addWidget(sectionLabel("Répartition Financière"));
        wrap->addWidget(card);
        QWidget *ww = new QWidget();
        ww->setLayout(wrap);
        chartsLayout->addWidget(ww);
    }

    // ── Bars ─────────────────────────────────────────────────────────
    if (!statsByStatus.isEmpty())
        if (auto *w = makeBarChart("Montant par Statut", statsByStatus,
                                   {"#8faa7a","#caa87b","#b5826e","#a8967a"}))
            chartsLayout->addWidget(w);

    if (!statsByMode.isEmpty())
        if (auto *w = makeBarChart("Montant par Mode de Paiement", statsByMode,
                                   {"#8faa7a","#b5826e","#caa87b","#a8967a","#9bb88b"}))
            chartsLayout->addWidget(w);

    if (!statsByMonth.isEmpty()) {
        QMap<QString, double> sortedMonths;
        QStringList months = statsByMonth.keys();
        std::sort(months.begin(), months.end(), [](const QString &a, const QString &b) {
            return QDate::fromString("01/"+a, "dd/MM/yyyy") < QDate::fromString("01/"+b, "dd/MM/yyyy");
        });
        for (const QString &m : months) sortedMonths[m] = statsByMonth[m];
        if (auto *w = makeBarChart("Évolution Mensuelle", sortedMonths,
                                   {"#8faa7a","#9bb88b","#a8967a","#caa87b","#b5826e"}))
            chartsLayout->addWidget(w);
    }

    // ── Summary card ─────────────────────────────────────────────────
    QFrame *summaryCard = new QFrame();
    summaryCard->setStyleSheet("QFrame{background:#ffffff;border-radius:14px;border:1px solid #e2dcd2;}");
    QVBoxLayout *summaryLayout = new QVBoxLayout(summaryCard);
    summaryLayout->setContentsMargins(22, 18, 22, 18);
    summaryLayout->setSpacing(10);
    QLabel *summaryTitle = new QLabel("Résumé Financier");
    summaryTitle->setStyleSheet("font-size:14px;font-weight:600;color:#4a6a4e;");
    summaryLayout->addWidget(summaryTitle);

    double montantMoyen = totalTransactions > 0 ? totalGeneral / totalTransactions : 0;
    QGridLayout *summaryGrid = new QGridLayout();
    summaryGrid->setSpacing(10);
    QStringList summaryItems = {
        "Total Transactions", QString::number(totalTransactions),
        "Montant Moyen",      QString::number(montantMoyen, 'f', 3) + " DT",
        "Bénéfice Net",       QString::number(totalRecettes - totalDepenses, 'f', 3) + " DT",
        "Taux de Recettes",   totalGeneral > 0
            ? QString::number(totalRecettes/totalGeneral*100,'f',1)+"%"
            : "0%"
    };
    for (int i = 0; i < summaryItems.size(); i += 2) {
        QLabel *lbl = new QLabel(summaryItems[i]);
        lbl->setStyleSheet("font-size:12px;color:#9b8e7c;");
        QLabel *val = new QLabel(summaryItems[i+1]);
        val->setStyleSheet("font-size:13px;font-weight:600;color:#6a5e4e;");
        summaryGrid->addWidget(lbl, i/2, 0);
        summaryGrid->addWidget(val, i/2, 1);
    }
    summaryLayout->addLayout(summaryGrid);
    chartsLayout->addWidget(summaryCard);

    root->addWidget(scroll, 1);

    // ── Close button ─────────────────────────────────────────────────
    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setObjectName("closeBtn");
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);

    dlg->exec();
    delete dlg;
}
void FinanceView::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    onEditClicked(row);
}

void FinanceView::onFilterChanged()
{
    applyFilters();
}

void FinanceView::onResetFilters()
{
    m_searchEdit->clear();
    m_typeCombo->setCurrentIndex(0);
    m_categoryCombo->setCurrentIndex(0);
    for (int row = 0; row < m_table->rowCount(); row++)
        m_table->setRowHidden(row, false);
}

void FinanceView::onModelDataChanged()
{
    loadTransactions();
}

void FinanceView::onModelError(const QString &error)
{
    QMessageBox::warning(this, "Erreur Base de données", error);
}
