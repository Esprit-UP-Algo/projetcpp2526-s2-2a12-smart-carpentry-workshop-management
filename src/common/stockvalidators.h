#ifndef STOCKVALIDATORS_H
#define STOCKVALIDATORS_H

#include <QString>
#include <QDate>

class StockMaterial;

class StockValidators
{
public:
    static bool validateName(const QString& name, QString& errorMsg, int excludeId = -1);
    static bool validateType(const QString& type, QString& errorMsg);
    static bool validateQuantity(double quantity, QString& errorMsg);
    static bool validatePrice(double price, QString& errorMsg);
    static bool validateSupplier(const QString& supplier, QString& errorMsg);
    static bool validateThreshold(double threshold, QString& errorMsg);
    static bool validateDate(const QDate& date, QString& errorMsg);
    static bool validateMonthlyConsumption(double consumption, QString& errorMsg);
    static bool validateUnit(const QString& unit, QString& errorMsg);
    static bool validateLocale(const QString& locale, QString& errorMsg);
    static bool validateEmplacement(const QString& emplacement, QString& errorMsg);

    static bool validateAll(const StockMaterial& material, QString& errorMsg, int excludeId = -1);
    static bool validateAll(const QString& name, const QString& type, double quantity,
                            double price, const QString& supplier, double threshold,
                            const QDate& lastOrder, double monthlyConsumption,
                            const QString& unit, int productId,
                            const QString& locale, const QString& emplacement,
                            QString& errorMsg, int excludeId = -1);

private:
    static bool isMaterialNameUnique(const QString& name, int excludeId = -1);
};

#endif // STOCKVALIDATORS_H
