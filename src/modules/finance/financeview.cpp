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
#include <QSqlQuery>
#include <QSqlError>
#include <QPushButton>
#include <QDir>

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
    m_searchEdit->setPlaceholderText("ref");
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
    m_table->setColumnCount(8);

    QStringList headers = {
        "RÉFÉRENCE", "TYPE", "MODE PAIEMENT", "STATUT",
        "CATÉGORIE", "MONTANT (DT)", "DATE", "PROJET"
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
    m_archiveButton = new QPushButton("Voir les Archives");  // Nouveau bouton

    actionLayout->addWidget(m_addButton);
    actionLayout->addWidget(m_deleteButton);
    actionLayout->addWidget(m_exportButton);
    actionLayout->addWidget(m_statsButton);
    actionLayout->addWidget(m_archiveButton);
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);
}

void FinanceView::setupConnections()
{
    connect(m_addButton, &QPushButton::clicked, this, &FinanceView::onAddClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &FinanceView::onDeleteClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &FinanceView::onExportClicked);
    connect(m_statsButton, &QPushButton::clicked, this, &FinanceView::onStatsClicked);
    connect(m_archiveButton, &QPushButton::clicked, this, &FinanceView::onArchiveClicked);
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

        m_table->setItem(i, 0, new QTableWidgetItem(t.reference));
        m_table->setItem(i, 1, new QTableWidgetItem(t.type));
        m_table->setItem(i, 2, new QTableWidgetItem(t.modePaiement));
        m_table->setItem(i, 3, new QTableWidgetItem(t.statut));
        m_table->setItem(i, 4, new QTableWidgetItem(t.categorie));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(t.montant, 'f', 3)));
        m_table->setItem(i, 6, new QTableWidgetItem(t.date));
        m_table->setItem(i, 7, new QTableWidgetItem(t.nomProjet));

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
            QTableWidgetItem *refItem = m_table->item(row, 0);
            if (!refItem || !refItem->text().contains(search, Qt::CaseInsensitive))
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
    data["CONTRAT_PROJET"] = transaction.contratProjet;

    dialog.setData(data);

    if (dialog.exec() == QDialog::Accepted) {
        if (m_model && m_model->updateTransaction(transaction.reference, dialog.getData())) {
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
        QMessageBox::warning(this, "Aucune sélection",
                             "Veuillez sélectionner une transaction à supprimer.");
        return;
    }

    QString reference = m_table->item(row, 0)->text();
    QString projet = m_table->item(row, 7)->text();

    QString message;
    if (projet != "Aucun projet") {
        message = QString("Cette transaction est liée au projet '%1'.\n\n"
                          "La supprimer pourrait affecter les données du projet.\n\n"
                          "Confirmer la suppression ?")
                      .arg(projet);
    } else {
        message = "Supprimer cette transaction ?\n\nCette action est irréversible.";
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Confirmer la suppression");
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Question);

    QPushButton *ouiButton = msgBox.addButton("Oui", QMessageBox::YesRole);
    QPushButton *nonButton = msgBox.addButton("Non", QMessageBox::NoRole);
    msgBox.setDefaultButton(nonButton);

    msgBox.exec();

    if (msgBox.clickedButton() == ouiButton) {
        if (m_model && m_model->deleteTransaction(reference)) {
            loadTransactions();
            QMessageBox::information(this, "Succès",
                                     "Transaction supprimée avec succès !");
        } else {
            QMessageBox::warning(this, "Erreur",
                                 "Impossible de supprimer la transaction.");
        }
    }
}

void FinanceView::onArchiveClicked()
{
    if (!m_model) return;
    
    auto archives = m_model->loadArchivedTransactions();
    
    QDialog *archiveDialog = new QDialog(this);
    archiveDialog->setWindowTitle("Transactions Supprimees - Archive");
    archiveDialog->setMinimumSize(1000, 600);
    archiveDialog->setStyleSheet(R"(
        QDialog {
            background: #f7f5f0;
        }
        QTableWidget {
            background: white;
            alternate-background-color: #f9f9f9;
            border: 1px solid #ddd;
            border-radius: 5px;
        }
        QHeaderView::section {
            background: #4a6a4e;
            color: white;
            padding: 8px;
            border: none;
        }
    )");
    
    QVBoxLayout *layout = new QVBoxLayout(archiveDialog);
    
    QLabel *title = new QLabel("Historique des Transactions Supprimees");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e2f; margin: 10px;");
    layout->addWidget(title);
    
    QLabel *info = new QLabel(QString("Total des suppressions : %1").arg(archives.size()));
    info->setStyleSheet("color: #666; margin-left: 10px;");
    layout->addWidget(info);
    
    QTableWidget *table = new QTableWidget();
    table->setColumnCount(9);
    QStringList headers = {"REFERENCE", "TYPE", "MODE", "STATUT", "CATEGORIE", 
                          "MONTANT", "DATE", "PROJET", "DATE SUPPRESSION"};
    table->setHorizontalHeaderLabels(headers);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    table->setRowCount(archives.size());
    for (int i = 0; i < archives.size(); ++i) {
        const auto &arch = archives[i];
        table->setItem(i, 0, new QTableWidgetItem(arch.reference));
        table->setItem(i, 1, new QTableWidgetItem(arch.type));
        table->setItem(i, 2, new QTableWidgetItem(arch.modePaiement));
        table->setItem(i, 3, new QTableWidgetItem(arch.statut));
        
        QTableWidgetItem *categorieItem = new QTableWidgetItem(arch.categorie);
        if (arch.categorie == "Recette") {
            categorieItem->setForeground(QBrush(QColor("#28a745")));
        } else {
            categorieItem->setForeground(QBrush(QColor("#dc3545")));
        }
        table->setItem(i, 4, categorieItem);
        
        table->setItem(i, 5, new QTableWidgetItem(QString::number(arch.montant, 'f', 3) + " DT"));
        table->setItem(i, 6, new QTableWidgetItem(arch.date));
        table->setItem(i, 7, new QTableWidgetItem(arch.projet));
        table->setItem(i, 8, new QTableWidgetItem(arch.deletedAt.toString("dd/MM/yyyy hh:mm:ss")));
    }
    
    layout->addWidget(table);
    
    bool isIntegrityValid = m_model->verifyLogIntegrity();
    
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    
    QLabel *integrityLabel = new QLabel();
    if (isIntegrityValid) {
        integrityLabel->setText("Archive non modifiee");
        integrityLabel->setStyleSheet("color: #28a745; font-weight: bold;");
    } else {
        integrityLabel->setText("ATTENTION : L'archive a ete modifiee !");
        integrityLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
    }
    bottomLayout->addWidget(integrityLabel);
    
    QPushButton *exportArchiveBtn = new QPushButton("Exporter l'archive");
    connect(exportArchiveBtn, &QPushButton::clicked, [this, archives]() {
        QString fileName = QFileDialog::getSaveFileName(this, 
            "Exporter l'archive",
            QString("archive_suppressions_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
            "CSV (*.csv)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                QTextStream out(&file);
                out.setEncoding(QStringConverter::Utf8);
                out << "Reference;Type;Mode;Statut;Categorie;Montant;Date;Projet;Date suppression\n";
                for (const auto &arch : archives) {
                    out << arch.reference << ";"
                        << arch.type << ";"
                        << arch.modePaiement << ";"
                        << arch.statut << ";"
                        << arch.categorie << ";"
                        << arch.montant << ";"
                        << arch.date << ";"
                        << arch.projet << ";"
                        << arch.deletedAt.toString("dd/MM/yyyy hh:mm:ss") << "\n";
                }
                file.close();
                QMessageBox::information(this, "Succes", "Archive exportee avec succes !");
            }
        }
    });
    
    bottomLayout->addWidget(exportArchiveBtn);
    
    QPushButton *closeBtn = new QPushButton("Fermer");
    connect(closeBtn, &QPushButton::clicked, archiveDialog, &QDialog::accept);
    bottomLayout->addWidget(closeBtn);
    
    layout->addLayout(bottomLayout);
    
    archiveDialog->exec();
    delete archiveDialog;
}
void FinanceView::onExportClicked()
{
    if (!m_model) return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Exporter le rapport financier",
                                                    QString("rapport_financier_%1.xls").arg(QDate::currentDate().toString("yyyyMMdd")),
                                                    "Fichiers Excel (*.xls);;Fichiers HTML (*.html);;Tous les fichiers (*.*)");

    if (fileName.isEmpty()) return;

    auto stats = m_model->getStatistics();
    auto transactions = m_model->loadTransactions();

    exportToExcelNative(fileName, stats, transactions);
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

    QLabel *title = new QLabel("Tableau de Bord Financier");
    title->setStyleSheet("font-size:24px;font-weight:600;color:#3a5a3e;letter-spacing:-0.3px;margin-bottom:4px;");
    QLabel *subtitle = new QLabel(QString("Analyse de %1 transactions · Montant total : %2 DT")
                                      .arg(totalTransactions)
                                      .arg(totalGeneral, 0, 'f', 3));
    subtitle->setStyleSheet("font-size:13px;color:#7f8c6e;");
    root->addWidget(title);
    root->addWidget(subtitle);

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

    QScrollArea *scroll = new QScrollArea();
    QWidget *scrollContent = new QWidget();
    scrollContent->setObjectName("scrollContent");
    QVBoxLayout *chartsLayout = new QVBoxLayout(scrollContent);
    chartsLayout->setSpacing(22);
    chartsLayout->setContentsMargins(0, 0, 0, 0);
    scroll->setWidget(scrollContent);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto sectionLabel = [](const QString &text) -> QLabel* {
        QLabel *l = new QLabel(text);
        l->setStyleSheet("font-size:15px;font-weight:600;color:#4a6a4e;margin-bottom:10px;"
                         "padding-left:4px;border-left:3px solid #8faa7a;");
        return l;
    };

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

void FinanceView::exportToExcelNative(const QString &fileName,
                                      const FinanceModel::FinanceStats &stats,
                                      const QList<FinanceModel::Transaction> &transactions)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Erreur", "Impossible de créer le fichier.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    double soldeNet = stats.totalRecettes - stats.totalDepenses;
    double montantMoyen = stats.totalGeneral / qMax(1, stats.totalTransactions);

    out << "<html>\n"
        << "<head>\n"
        << "<meta charset=\"UTF-8\">\n"
        << "<title>Rapport Financier</title>\n"
        << "<style>\n"
        << "body { font-family: 'Segoe UI', Arial, sans-serif; margin: 20px; }\n"
        << "h1 { color: #2c3e2f; border-bottom: 2px solid #4a6a4e; padding-bottom: 10px; }\n"
        << "h2 { color: #4a6a4e; margin-top: 25px; }\n"
        << ".kpi-table { width: 100%; border-collapse: collapse; margin: 20px 0; background: #f8f9fa; }\n"
        << ".kpi-table td { padding: 15px; border: 1px solid #dee2e6; }\n"
        << ".kpi-label { font-weight: bold; background: #e9ecef; width: 200px; }\n"
        << ".kpi-value { font-size: 18px; font-weight: bold; }\n"
        << ".recette { color: #28a745; }\n"
        << ".depense { color: #dc3545; }\n"
        << "table { width: 100%; border-collapse: collapse; margin: 20px 0; }\n"
        << "th { background: #4a6a4e; color: white; padding: 10px; border: 1px solid #ddd; }\n"
        << "td { padding: 8px; border: 1px solid #ddd; }\n"
        << "tr:nth-child(even) { background: #f2f2f2; }\n"
        << ".stat-box { margin: 15px 0; padding: 10px; background: #f8f9fa; border-left: 4px solid #4a6a4e; }\n"
        << "footer { margin-top: 30px; padding-top: 10px; border-top: 1px solid #ddd; text-align: center; color: #666; }\n"
        << "</style>\n"
        << "</head>\n"
        << "<body>\n"
        << "<h1>📊 RAPPORT FINANCIER</h1>\n"
        << "<p>Généré le : " << QDate::currentDate().toString("dd/MM/yyyy") << "</p>\n\n"

        << "<h2>INDICATEURS CLÉS</h2>\n"
        << "<table class=\"kpi-table\">\n"
        << " <tr><td class=\"kpi-label\">Total Recettes</td><td class=\"kpi-value recette\">" << QString::number(stats.totalRecettes, 'f', 3) << " DT</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Total Dépenses</td><td class=\"kpi-value depense\">" << QString::number(stats.totalDepenses, 'f', 3) << " DT</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Solde Net</td><td class=\"kpi-value " << (soldeNet >= 0 ? "recette" : "depense") << "\">" << QString::number(soldeNet, 'f', 3) << " DT</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Nombre de Transactions</td><td class=\"kpi-value\">" << stats.totalTransactions << "</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Montant Moyen par Transaction</td><td class=\"kpi-value\">" << QString::number(montantMoyen, 'f', 3) << " DT</td></tr>\n"
        << " <tr><td class=\"kpi-label\">Total Général</td><td class=\"kpi-value\">" << QString::number(stats.totalGeneral, 'f', 3) << " DT</td></tr>\n"
        << "</table>\n\n"

        << "<h2>📋 DÉTAIL DES TRANSACTIONS</h2>\n"
        << "<table>\n"
        << " <thead>\n"
        << "   <tr>\n"
        << "   <th>RÉFÉRENCE</th><th>TYPE</th><th>MODE PAIEMENT</th><th>STATUT</th><th>CATÉGORIE</th><th>MONTANT (DT)</th><th>DATE</th><th>PROJET</th>\n"
        << "   </tr>\n"
        << " </thead>\n"
        << " <tbody>\n";

    for (const auto &t : transactions) {
        QString rowClass = (t.categorie == "Recette") ? "recette" : "depense";
        out << "   <tr>\n"
            << "    <td>" << t.reference << "</td>\n"
            << "    <td>" << t.type << "</td>\n"
            << "    <td>" << t.modePaiement << "</td>\n"
            << "    <td>" << t.statut << "</td>\n"
            << "    <td>" << t.categorie << "</td>\n"
            << "   <td class=\"" << rowClass << "\">" << QString::number(t.montant, 'f', 3) << "</td>\n"
            << "    <td>" << t.date << "</td>\n"
            << "    <td>" << t.nomProjet << "</td>\n"
            << "   </tr>\n";
    }

    out << " </tbody>\n"
        << "</table>\n\n"

        << "<h2>📈 STATISTIQUES DÉTAILLÉES</h2>\n"

        << "<div class=\"stat-box\">\n"
        << "<h3>Par Statut</h3>\n"
        << "<table>\n"
        << "  <tr><th>Statut</th><th>Montant (DT)</th><th>Pourcentage</th></tr>\n";

    for (auto it = stats.statsByStatus.begin(); it != stats.statsByStatus.end(); ++it) {
        double pourcentage = stats.totalGeneral > 0 ? (it.value() / stats.totalGeneral) * 100 : 0;
        out << "  <tr><td>" << it.key() << "</td><td>" << QString::number(it.value(), 'f', 3) << "</td><td>" << QString::number(pourcentage, 'f', 1) << "%</td></tr>\n";
    }

    out << "</table>\n</div>\n\n"

        << "<div class=\"stat-box\">\n"
        << "<h3>Par Mode de Paiement</h3>\n"
        << "<table>\n"
        << "  <tr><th>Mode</th><th>Montant (DT)</th><th>Pourcentage</th></tr>\n";

    for (auto it = stats.statsByMode.begin(); it != stats.statsByMode.end(); ++it) {
        double pourcentage = stats.totalGeneral > 0 ? (it.value() / stats.totalGeneral) * 100 : 0;
        out << "  <tr><td>" << it.key() << "</td><td>" << QString::number(it.value(), 'f', 3) << "</td><td>" << QString::number(pourcentage, 'f', 1) << "%</td></tr>\n";
    }

    out << "</table>\n</div>\n\n"

        << "<div class=\"stat-box\">\n"
        << "<h3>Par Catégorie</h3>\n"
        << "<table>\n"
        << "  <tr><th>Catégorie</th><th>Montant (DT)</th><th>Pourcentage</th></tr>\n";

    for (auto it = stats.statsByCategorie.begin(); it != stats.statsByCategorie.end(); ++it) {
        double pourcentage = stats.totalGeneral > 0 ? (it.value() / stats.totalGeneral) * 100 : 0;
        out << "  <tr><td>" << it.key() << "</td><td>" << QString::number(it.value(), 'f', 3) << "</td><td>" << QString::number(pourcentage, 'f', 1) << "%</td></tr>\n";
    }

    out << "</table>\n</div>\n\n"

        << "<div class=\"stat-box\">\n"
        << "<h3>Évolution Mensuelle</h3>\n"
        << "<table>\n"
        << "  <tr><th>Mois</th><th>Montant (DT)</th></tr>\n";

    QStringList moisTries = stats.statsByMonth.keys();
    std::sort(moisTries.begin(), moisTries.end(), [](const QString &a, const QString &b) {
        return QDate::fromString("01/" + a, "dd/MM/yyyy") < QDate::fromString("01/" + b, "dd/MM/yyyy");
    });

    for (const QString &mois : moisTries) {
        out << "  <tr><td>" << mois << "</td><td>" << QString::number(stats.statsByMonth[mois], 'f', 3) << " DT</td></tr>\n";
    }

    out << "</table>\n</div>\n\n"

        << "<h2>📝 RÉSUMÉ</h2>\n"
        << "<table>\n"
        << "  <tr><td width=\"300\"><strong>Période</strong></td><td>Toutes les transactions</td></tr>\n"
        << "  <tr><td><strong>Performance</strong></td><td>" << (soldeNet >= 0 ? "📈 Bénéficiaire" : "📉 Déficitaire") << "</td></tr>\n"
        << "  <tr><td><strong>Ratio Recettes/Dépenses</strong></td><td>"
        << (stats.totalDepenses > 0 ? QString::number((stats.totalRecettes / stats.totalDepenses) * 100, 'f', 1) : "N/A")
        << "%</td></tr>\n"
        << "</table>\n\n"

        << "<footer>\n"
        << "  <p>Document généré automatiquement par le Système de Gestion Financière</p>\n"
        << "  <p>© " << QDate::currentDate().toString("yyyy") << " - Tous droits réservés</p>\n"
        << "</footer>\n"

        << "</body>\n"
        << "</html>";

    file.close();

    QString excelFileName = fileName;
    if (!excelFileName.endsWith(".xls", Qt::CaseInsensitive)) {
        excelFileName = fileName.left(fileName.lastIndexOf('.')) + ".xls";
        QFile::rename(fileName, excelFileName);
    }

    QMessageBox::information(this, "Export réussi",
                             QString("✅ Le rapport financier a été généré avec succès !\n\n"
                                     "📁 Fichier : %1\n\n"
                                     "📊 Le rapport contient :\n"
                                     "• Indicateurs clés (KPIs)\n"
                                     "• Détail de toutes les transactions\n"
                                     "• Statistiques par statut, mode, catégorie\n"
                                     "• Évolution mensuelle\n"
                                     "• Résumé financier\n\n"
                                     "💡 Ouvrez le fichier avec Excel pour une meilleure expérience.")
                                 .arg(excelFileName));
}