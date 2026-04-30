#ifndef FINANCEMODEL_H
#define FINANCEMODEL_H

#include <QObject>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QMap>
#include <QList>
#include <QVariant>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>

class FinanceView;

class FinanceModel : public QObject
{
    Q_OBJECT
public:
    struct Transaction {
        QString reference;
        QString type;
        QString modePaiement;
        QString statut;
        QString categorie;
        double montant;
        QString date;
        QString contratProjet;
        QString nomProjet;
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
    
    struct DeletedTransaction {
        QString reference;
        QString type;
        QString modePaiement;
        QString statut;
        QString categorie;
        double montant;
        QString date;
        QString projet;
        QString deletedBy;
        QDateTime deletedAt;
        QString signature;
    };

    explicit FinanceModel(QObject *parent = nullptr);

    QList<Transaction> loadTransactions();
    bool insertTransaction(const QMap<QString, QString> &data);
    bool updateTransaction(const QString &reference, const QMap<QString, QString> &data);
    bool deleteTransaction(const QString &reference);
    FinanceStats getStatistics();
    QSqlDatabase getDatabase();
    
    // ARCHIVAGE
    bool archiveDeletedTransaction(const Transaction &transaction, const QString &deletedBy);
    QList<DeletedTransaction> loadArchivedTransactions();
    bool verifyLogIntegrity();
    QString generateLogSignature(const QString &content);
    QString getLogFilePath();
    
    // RFID Card Access Verification
    struct RFIDVerificationResult {
        bool isValid = false;
        QString errorMessage;
        enum ErrorType { SUCCESS = 0, CARD_NOT_FOUND = 1, ROLE_MISMATCH = 2, CARD_CODE_MISMATCH = 3 } errorType;
    };
    RFIDVerificationResult verifyRFIDCardAccess(const QString &rfidCode);  

signals:
    void dataChanged();
    void errorOccurred(const QString &error);

private:
    QString generateReference();
    void createLogFileIfNotExists();  // ← Garder en private

};

#endif // FINANCEMODEL_H