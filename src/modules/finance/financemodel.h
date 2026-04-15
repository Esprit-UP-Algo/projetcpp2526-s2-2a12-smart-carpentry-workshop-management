#ifndef FINANCEMODEL_H
#define FINANCEMODEL_H

#include <QObject>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QMap>
#include <QList>
#include <QVariant>
class FinanceView;
class FinanceModel : public QObject
{
    Q_OBJECT  // ← IMPORTANT : Cette macro est nécessaire
public:
    struct Transaction {
        QString id;
        QString type;
        QString modePaiement;
        QString statut;
        QString categorie;
        double montant;
        QString date;
    };

    struct FinanceStats {
        double totalRecettes = 0;
        double totalDepenses = 0;
        double totalGeneral = 0;
        int totalTransactions = 0;
        QMap<QString, double> statsByStatus;
        QMap<QString, double> statsByMode;
        QMap<QString, double> statsByCategorie;
        QMap<QString, double> statsByMonth;
    };

    explicit FinanceModel(QObject *parent = nullptr);

    QList<Transaction> loadTransactions();
    bool insertTransaction(const QMap<QString, QString> &data);
    bool updateTransaction(const QString &id, const QMap<QString, QString> &data);
    bool deleteTransaction(const QString &id);
    FinanceStats getStatistics();

signals:
    void dataChanged();
    void errorOccurred(const QString &error);

private:
    QSqlDatabase getDatabase();
};

#endif // FINANCEMODEL_H
