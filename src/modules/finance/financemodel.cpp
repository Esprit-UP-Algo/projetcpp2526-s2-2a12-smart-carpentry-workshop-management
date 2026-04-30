#include "financemodel.h"
#include "src/database/connection.h"
#include <QDebug>
#include <QDate>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QFileInfo>

FinanceModel::FinanceModel(QObject *parent) : QObject(parent) {}

QSqlDatabase FinanceModel::getDatabase()
{
    return QSqlDatabase::database(Connection::CONN_NAME);
}

QString FinanceModel::generateReference()
{
    // Format: TRANS-20241215-0001
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");

    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        return QString("TRANS-%1-0001").arg(dateStr);
    }

    // Compter les transactions du jour
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM TRANSACTIONS WHERE TO_CHAR(DATE_TRAN, 'YYYYMMDD') = :date");
    query.bindValue(":date", dateStr);

    int count = 0;
    if (query.exec() && query.next()) {
        count = query.value(0).toInt() + 1;
    }

    return QString("TRANS-%1-%2").arg(dateStr).arg(count, 4, 10, QChar('0'));
}

QList<FinanceModel::Transaction> FinanceModel::loadTransactions()
{
    QList<Transaction> transactions;
    QSqlDatabase db = getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database not connected!";
        return transactions;
    }

    QSqlQuery query(db);
    query.prepare("SELECT t.REFERENCE, t.TYPE_TRAN, t.MODE_PAIEMENT, t.STATUT_TRAN, "
                  "t.CATEGORIE_TRAN, t.MONTANT_TRAN, "
                  "TO_CHAR(t.DATE_TRAN, 'DD/MM/YYYY') as DATE_TRAN, "
                  "COALESCE(p.NOM_PROJET, 'Aucun projet') as NOM_PROJET "
                  "FROM TRANSACTIONS t "
                  "LEFT JOIN PROJET p ON t.CONTRAT_PROJET = p.ID_PROJET "
                  "ORDER BY t.DATE_TRAN DESC");

    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text();
        emit errorOccurred(query.lastError().text());
        return transactions;
    }

    while (query.next()) {
        Transaction t;
        t.reference = query.value(0).toString();
        t.type = query.value(1).toString();
        t.modePaiement = query.value(2).toString();
        t.statut = query.value(3).toString();
        t.categorie = query.value(4).toString();
        t.montant = query.value(5).toDouble();
        t.date = query.value(6).toString();
        t.nomProjet = query.value(7).toString();
        transactions.append(t);
    }

    return transactions;
}

bool FinanceModel::insertTransaction(const QMap<QString, QString> &data)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);

    // Générer une référence automatique
    QString reference = generateReference();

    QString projetValue = data["CONTRAT_PROJET"];
    bool hasProjet = !projetValue.isEmpty() && projetValue != "0";

    if (hasProjet) {
        query.prepare("INSERT INTO TRANSACTIONS "
                      "(ID_TRAN, REFERENCE, TYPE_TRAN, MODE_PAIEMENT, STATUT_TRAN, "
                      "CATEGORIE_TRAN, MONTANT_TRAN, DATE_TRAN, CONTRAT_PROJET) "
                      "VALUES (SEQ_TRAN.NEXTVAL, :reference, :type, :mode, :statut, "
                      ":categorie, :montant, TO_DATE(:date_str, 'DD/MM/YYYY'), :projet)");
        query.bindValue(":projet", projetValue.toInt());
    } else {
        query.prepare("INSERT INTO TRANSACTIONS "
                      "(ID_TRAN, REFERENCE, TYPE_TRAN, MODE_PAIEMENT, STATUT_TRAN, "
                      "CATEGORIE_TRAN, MONTANT_TRAN, DATE_TRAN) "
                      "VALUES (SEQ_TRAN.NEXTVAL, :reference, :type, :mode, :statut, "
                      ":categorie, :montant, TO_DATE(:date_str, 'DD/MM/YYYY'))");
    }

    query.bindValue(":reference", reference);
    query.bindValue(":type",      data["TYPE_TRAN"]);
    query.bindValue(":mode",      data["MODE_PAIEMENT"]);
    query.bindValue(":statut",    data["STATUT_TRAN"]);
    query.bindValue(":categorie", data["CATEGORIE_TRAN"]);
    query.bindValue(":montant",   data["MONTANT_TRAN"].toDouble());
    query.bindValue(":date_str",  data["DATE_TRAN"]);

    if (!query.exec()) {
        qDebug() << "Insert failed:" << query.lastError().text();
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

bool FinanceModel::updateTransaction(const QString &reference, const QMap<QString, QString> &data)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) return false;

    QSqlQuery query(db);

    QString projetValue = data["CONTRAT_PROJET"];
    bool hasProjet = !projetValue.isEmpty() && projetValue != "0";

    if (hasProjet) {
        query.prepare("UPDATE TRANSACTIONS SET "
                      "TYPE_TRAN = :type, "
                      "MODE_PAIEMENT = :mode, "
                      "STATUT_TRAN = :statut, "
                      "CATEGORIE_TRAN = :categorie, "
                      "MONTANT_TRAN = :montant, "
                      "DATE_TRAN = TO_DATE(:date_str, 'DD/MM/YYYY'), "
                      "CONTRAT_PROJET = :projet "
                      "WHERE REFERENCE = :reference");
        query.bindValue(":projet", projetValue.toInt());
    } else {
        query.prepare("UPDATE TRANSACTIONS SET "
                      "TYPE_TRAN = :type, "
                      "MODE_PAIEMENT = :mode, "
                      "STATUT_TRAN = :statut, "
                      "CATEGORIE_TRAN = :categorie, "
                      "MONTANT_TRAN = :montant, "
                      "DATE_TRAN = TO_DATE(:date_str, 'DD/MM/YYYY'), "
                      "CONTRAT_PROJET = NULL "
                      "WHERE REFERENCE = :reference");
    }

    query.bindValue(":type",      data["TYPE_TRAN"]);
    query.bindValue(":mode",      data["MODE_PAIEMENT"]);
    query.bindValue(":statut",    data["STATUT_TRAN"]);
    query.bindValue(":categorie", data["CATEGORIE_TRAN"]);
    query.bindValue(":montant",   data["MONTANT_TRAN"].toDouble());
    query.bindValue(":date_str",  data["DATE_TRAN"]);
    query.bindValue(":reference", reference);

    if (!query.exec()) {
        qDebug() << "Update failed:" << query.lastError().text();
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

bool FinanceModel::deleteTransaction(const QString &reference)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) return false;

    // D'abord, récupérer la transaction avant suppression
    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT t.REFERENCE, t.TYPE_TRAN, t.MODE_PAIEMENT, t.STATUT_TRAN, "
                        "t.CATEGORIE_TRAN, t.MONTANT_TRAN, TO_CHAR(t.DATE_TRAN, 'DD/MM/YYYY') as DATE_TRAN, "
                        "COALESCE(p.NOM_PROJET, 'Aucun projet') as NOM_PROJET "
                        "FROM TRANSACTIONS t "
                        "LEFT JOIN PROJET p ON t.CONTRAT_PROJET = p.ID_PROJET "
                        "WHERE t.REFERENCE = :reference");
    selectQuery.bindValue(":reference", reference);
    
    if (!selectQuery.exec() || !selectQuery.next()) {
        emit errorOccurred("Transaction non trouvée");
        return false;
    }
    
    // Sauvegarder les données
    Transaction transactionToDelete;
    transactionToDelete.reference = selectQuery.value(0).toString();
    transactionToDelete.type = selectQuery.value(1).toString();
    transactionToDelete.modePaiement = selectQuery.value(2).toString();
    transactionToDelete.statut = selectQuery.value(3).toString();
    transactionToDelete.categorie = selectQuery.value(4).toString();
    transactionToDelete.montant = selectQuery.value(5).toDouble();
    transactionToDelete.date = selectQuery.value(6).toString();
    transactionToDelete.nomProjet = selectQuery.value(7).toString();
    
    // Archiver avant suppression
    QString deletedBy = "Utilisateur";
    if (!archiveDeletedTransaction(transactionToDelete, deletedBy)) {
        emit errorOccurred("Erreur lors de l'archivage");
        return false;
    }
    
    // Procéder à la suppression
    if (!db.transaction()) {
        emit errorOccurred("Impossible de démarrer la transaction");
        return false;
    }
    
    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM TRANSACTIONS WHERE REFERENCE = :reference");
    deleteQuery.bindValue(":reference", reference);

    if (!deleteQuery.exec()) {
        db.rollback();
        emit errorOccurred(deleteQuery.lastError().text());
        return false;
    }
    
    if (!db.commit()) {
        emit errorOccurred("Erreur lors de la validation");
        return false;
    }

    emit dataChanged();
    return true;
}
FinanceModel::FinanceStats FinanceModel::getStatistics()
{
    FinanceStats stats;
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) return stats;

    QSqlQuery query(db);
    query.prepare("SELECT STATUT_TRAN, MODE_PAIEMENT, CATEGORIE_TRAN, MONTANT_TRAN, "
                  "TO_CHAR(DATE_TRAN,'MM/YYYY') as MOIS "
                  "FROM TRANSACTIONS");

    if (!query.exec()) return stats;

    while (query.next()) {
        QString statut    = query.value(0).toString();
        QString mode      = query.value(1).toString();
        QString categorie = query.value(2).toString();
        double  montant   = query.value(3).toDouble();
        QString mois      = query.value(4).toString();

        stats.statsByStatus[statut] += montant;
        stats.statsByMode[mode] += montant;
        stats.statsByCategorie[categorie] += montant;
        stats.statsByMonth[mois] += montant;

        if (categorie == "Recette") {
            stats.totalRecettes += montant;
        } else if (categorie == "Depense") {
            stats.totalDepenses += montant;
        }

        stats.totalGeneral += montant;
        stats.totalTransactions++;
    }

    return stats;
}

// ============ MÉTHODES D'ARCHIVAGE ============

QString FinanceModel::getLogFilePath()
{
    QString logDir;
    
    #ifdef QT_DEBUG
        // En mode debug : utiliser le dossier src/modules/finance
        QString sourcePath = __FILE__;
        QFileInfo fileInfo(sourcePath);
        logDir = fileInfo.absolutePath() + "/logs";
    #else
        // En mode release : utiliser le dossier de l'application
        logDir = QCoreApplication::applicationDirPath() + "/logs";
    #endif
    
    QDir dir;
    if (!dir.exists(logDir)) {
        dir.mkpath(logDir);
    }
    
    qDebug() << "Dossier logs:" << logDir;
    return logDir + "/deleted_transactions.log";
}
void FinanceModel::createLogFileIfNotExists()
{
    QString logFile = getLogFilePath();
    QFile file(logFile);
    
    if (!file.exists()) {
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out << "=== JOURNAL DES SUPPRESSIONS ===\n";
            out << QString("Créé le : %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss"));
            out << "=== FIN D'EN-TÊTE ===\n\n";
            file.close();
            qDebug() << "Fichier log créé:" << logFile;
        } else {
            qDebug() << "Erreur création fichier log:" << file.errorString();
        }
    }
}

bool FinanceModel::archiveDeletedTransaction(const Transaction &transaction, const QString &deletedBy)
{
    // Créer le dossier et le fichier si nécessaire
    createLogFileIfNotExists();
    
    QString logFile = getLogFilePath();
    QFile file(logFile);
    
    qDebug() << "Tentative d'ouverture du fichier:" << logFile;
    
    if (file.exists()) {
        QFile::setPermissions(logFile, QFile::ReadOwner | QFile::WriteOwner | 
                                        QFile::ReadUser | QFile::WriteUser |
                                        QFile::ReadGroup | QFile::WriteGroup |
                                        QFile::ReadOther | QFile::WriteOther);
    }
    
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QString error = file.errorString();
        qDebug() << "Erreur d'ouverture:" << error;
        
        QString backupLogFile = logFile + ".backup";
        QFile backupFile(backupLogFile);
        if (backupFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&backupFile);
            out.setEncoding(QStringConverter::Utf8);
            out << "========================================\n";
            out << "Date suppression: " << QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss") << "\n";
            // NE PAS écrire "Supprimé par"
            out << "Référence: " << transaction.reference << "\n";
            out << "Type: " << transaction.type << "\n";
            out << "Mode paiement: " << transaction.modePaiement << "\n";
            out << "Statut: " << transaction.statut << "\n";
            out << "Catégorie: " << transaction.categorie << "\n";
            out << "Montant: " << QString::number(transaction.montant, 'f', 3) << " DT\n";
            out << "Date transaction: " << transaction.date << "\n";
            out << "Projet: " << transaction.nomProjet << "\n";
            out << "========================================\n\n";
            backupFile.close();
            return true;
        }
        
        emit errorOccurred("Impossible d'ouvrir le fichier d'archivage: " + error);
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // Écrire la transaction supprimée SANS "Supprimé par"
    out << "========================================\n";
    out << "Date suppression: " << QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss") << "\n";
    // Ligne "Supprimé par" SUPPRIMÉE
    out << "Référence: " << transaction.reference << "\n";
    out << "Type: " << transaction.type << "\n";
    out << "Mode paiement: " << transaction.modePaiement << "\n";
    out << "Statut: " << transaction.statut << "\n";
    out << "Catégorie: " << transaction.categorie << "\n";
    out << "Montant: " << QString::number(transaction.montant, 'f', 3) << " DT\n";
    out << "Date transaction: " << transaction.date << "\n";
    out << "Projet: " << transaction.nomProjet << "\n";
    out << "========================================\n\n";
    
    out.flush();
    file.close();
    
    QFile::setPermissions(logFile, QFile::ReadOwner | QFile::ReadUser | 
                                    QFile::ReadGroup | QFile::ReadOther);
    
    qDebug() << "Transaction archivée avec succès:" << transaction.reference;
    return true;
}
QList<FinanceModel::DeletedTransaction> FinanceModel::loadArchivedTransactions()
{
    QList<DeletedTransaction> deletedTransactions;
    QString logFile = getLogFilePath();
    
    if (!QFile::exists(logFile)) {
        qDebug() << "Fichier d'archive n'existe pas";
        return deletedTransactions;
    }
    
    QFile::setPermissions(logFile, QFile::ReadOwner | QFile::WriteOwner | 
                                    QFile::ReadUser | QFile::WriteUser);
    
    QFile file(logFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Impossible d'ouvrir le fichier d'archive";
        return deletedTransactions;
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    
    QFile::setPermissions(logFile, QFile::ReadOwner | QFile::ReadUser | 
                                    QFile::ReadGroup | QFile::ReadOther);
    
    QStringList blocks = content.split("========================================\n");
    
    for (const QString &block : blocks) {
        if (block.trimmed().isEmpty()) continue;
        if (block.contains("=== JOURNAL DES SUPPRESSIONS ===")) continue;
        
        DeletedTransaction dt;
        
        QRegularExpression refRegex("Référence: (.*)");
        QRegularExpression typeRegex("Type: (.*)");
        QRegularExpression modeRegex("Mode paiement: (.*)");
        QRegularExpression statutRegex("Statut: (.*)");
        QRegularExpression catRegex("Catégorie: (.*)");
        QRegularExpression montantRegex("Montant: (.*) DT");
        QRegularExpression dateRegex("Date transaction: (.*)");
        QRegularExpression projetRegex("Projet: (.*)");
        QRegularExpression dateSuppRegex("Date suppression: (.*)");
        
        dt.reference = refRegex.match(block).captured(1).trimmed();
        dt.type = typeRegex.match(block).captured(1).trimmed();
        dt.modePaiement = modeRegex.match(block).captured(1).trimmed();
        dt.statut = statutRegex.match(block).captured(1).trimmed();
        dt.categorie = catRegex.match(block).captured(1).trimmed();
        dt.montant = montantRegex.match(block).captured(1).trimmed().toDouble();
        dt.date = dateRegex.match(block).captured(1).trimmed();
        dt.projet = projetRegex.match(block).captured(1).trimmed();
        dt.deletedBy = "Inconnu";  // Valeur par défaut
        // dt.deletedBy = suppRegex.match(block).captured(1).trimmed(); // SUPPRIMÉ
        
        QString dateSuppStr = dateSuppRegex.match(block).captured(1).trimmed();
        dt.deletedAt = QDateTime::fromString(dateSuppStr, "dd/MM/yyyy hh:mm:ss");
        
        if (!dt.reference.isEmpty()) {
            deletedTransactions.append(dt);
        }
    }
    
    qDebug() << "Chargé" << deletedTransactions.size() << "transactions archivées";
    return deletedTransactions;
}

bool FinanceModel::verifyLogIntegrity()
{
    QString logFile = getLogFilePath();
    if (!QFile::exists(logFile)) {
        return false;
    }
    
    // Vérifier si le fichier est lisible
    QFile file(logFile);
    return file.open(QIODevice::ReadOnly);
}

QString FinanceModel::generateLogSignature(const QString &content)
{
    return QString(QCryptographicHash::hash(content.toUtf8(), QCryptographicHash::Sha256).toHex());
}

FinanceModel::RFIDVerificationResult FinanceModel::verifyRFIDCardAccess(const QString &rfidCode)
{
    RFIDVerificationResult result;
    result.isValid = false;
    
    // Get database connection
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        result.errorMessage = "Connexion à la base de données échouée.";
        result.errorType = RFIDVerificationResult::ErrorType::CARD_NOT_FOUND;
        return result;
    }
    
    // Get current employee
    // Note: You need to include Session header in this file
    // For now, we'll query the card directly first
    
    // Step 1: Check if RFID code exists in CARTE_ACCESS table
    QSqlQuery cardQuery(db);
    cardQuery.prepare("SELECT ROLE FROM CARTE_ACCESS WHERE RFID_CODE = :rfidCode");
    cardQuery.bindValue(":rfidCode", rfidCode);
    
    QString cardRole;
    if (cardQuery.exec() && cardQuery.next()) {
        cardRole = cardQuery.value(0).toString();
    } else {
        result.errorMessage = "La carte RFID n'est pas reconnue ou enregistrée dans le système.";
        result.errorType = RFIDVerificationResult::ErrorType::CARD_NOT_FOUND;
        return result;
    }
    
    // Step 2: Check if card code matches (compare with the scanned code)
    // The code is already matched since we found it in the query above
    
    // Step 3: Get current employee's role and check permission
    // We need to get the logged-in employee info
    // For now, return success if card is found and role exists
    // The actual employee POST_EMP check should be done in FinanceView with Session
    
    result.isValid = true;
    result.errorType = RFIDVerificationResult::ErrorType::SUCCESS;
    result.errorMessage = cardRole;  // Store the card role for validation
    
    return result;
}