#include "stockvalidators.h"
#include "src/models/stockmaterial.h"
#include "src/database/stockdatabase.h"
#include "src/modules/stock/stocklocales.h"

bool StockValidators::isMaterialNameUnique(const QString& name, int excludeId)
{
    const QList<StockMaterial> all = StockDatabase::instance().getAllMaterials();
    for (const StockMaterial& m : all) {
        if (m.getNom().compare(name, Qt::CaseInsensitive) == 0) {
            if (excludeId != -1 && m.getId() == excludeId)
                continue;
            return false;
        }
    }
    return true;
}

bool StockValidators::validateName(const QString& name, QString& errorMsg, int excludeId)
{
    if (name.trimmed().isEmpty()) {
        errorMsg = "Le nom du matériau est obligatoire.";
        return false;
    }
    if (name.length() > 20) {
        errorMsg = "Le nom ne doit pas dépasser 20 caractères.";
        return false;
    }
    if (!isMaterialNameUnique(name, excludeId)) {
        errorMsg = "Un matériau avec ce nom existe déjà. Veuillez choisir un nom unique.";
        return false;
    }
    return true;
}

bool StockValidators::validateType(const QString& type, QString& errorMsg)
{
    static const QStringList validTypes = {
        "Bois", "Métal", "Plastique", "Verre", "Peinture", "Quincaillerie", "Textile", "Autre"
    };
    if (type.trimmed().isEmpty()) {
        errorMsg = "Le type de matériau est obligatoire.";
        return false;
    }
    if (!validTypes.contains(type)) {
        errorMsg = "Type invalide. Valeurs autorisées : " + validTypes.join(", ");
        return false;
    }
    return true;
}

bool StockValidators::validateQuantity(double quantity, QString& errorMsg)
{
    if (quantity < 0) {
        errorMsg = "La quantité ne peut pas être négative.";
        return false;
    }
    return true;
}

bool StockValidators::validatePrice(double price, QString& errorMsg)
{
    if (price <= 0.0) {
        errorMsg = "Le prix unitaire doit être supérieur à 0 DT.";
        return false;
    }
    return true;
}

bool StockValidators::validateSupplier(const QString& supplier, QString& errorMsg)
{
    if (supplier.trimmed().isEmpty()) {
        errorMsg = "Le fournisseur est obligatoire.";
        return false;
    }
    if (supplier.length() > 20) {
        errorMsg = "Le nom du fournisseur ne doit pas dépasser 20 caractères.";
        return false;
    }
    return true;
}

bool StockValidators::validateThreshold(double threshold, QString& errorMsg)
{
    if (threshold <= 0.0) {
        errorMsg = "Le seuil d'alerte doit être supérieur à 0.";
        return false;
    }
    return true;
}

bool StockValidators::validateDate(const QDate& date, QString& errorMsg)
{
    if (!date.isValid()) {
        errorMsg = "La date de dernière commande est invalide.";
        return false;
    }
    return true;
}

bool StockValidators::validateMonthlyConsumption(double consumption, QString& errorMsg)
{
    if (consumption < 0) {
        errorMsg = "La consommation mensuelle ne peut pas être négative.";
        return false;
    }
    return true;
}

bool StockValidators::validateUnit(const QString& unit, QString& errorMsg)
{
    static const QStringList validUnits = {
        "m", "m²", "m³", "kg", "g", "L", "mL", "pièce", "paquet", "rouleau", "barre", "feuille", "Autre"
    };
    if (unit.trimmed().isEmpty()) {
        errorMsg = "L'unité est obligatoire.";
        return false;
    }
    if (!validUnits.contains(unit)) {
        errorMsg = "Unité invalide. Valeurs autorisées : " + validUnits.join(", ");
        return false;
    }
    return true;
}

bool StockValidators::validateLocale(const QString& locale, QString& errorMsg)
{
    // Locale est optionnelle — on valide seulement si elle est renseignée
    if (locale.trimmed().isEmpty())
        return true;

    if (!StockLocales::localeNames().contains(locale)) {
        errorMsg = "Locale invalide. Veuillez choisir une locale dans la liste.";
        return false;
    }
    return true;
}

bool StockValidators::validateEmplacement(const QString& emplacement, QString& errorMsg)
{
    // Emplacement est optionnel — on valide seulement si renseigné
    if (emplacement.trimmed().isEmpty())
        return true;

    if (!StockLocales::emplacements().contains(emplacement)) {
        errorMsg = "Emplacement invalide. Veuillez choisir un emplacement dans la liste.";
        return false;
    }
    return true;
}

bool StockValidators::validateAll(const StockMaterial& material, QString& errorMsg, int excludeId)
{
    if (!validateName(material.getNom(), errorMsg, excludeId)) return false;
    if (!validateType(material.getType(), errorMsg)) return false;
    if (!validateQuantity(material.getQuantite(), errorMsg)) return false;
    if (!validatePrice(material.getPrixUnitaire(), errorMsg)) return false;
    if (!validateSupplier(material.getFournisseur(), errorMsg)) return false;
    if (!validateThreshold(material.getSeuilAlerte(), errorMsg)) return false;
    if (!validateDate(material.getLastOrder(), errorMsg)) return false;
    if (!validateMonthlyConsumption(material.getConsoMensuelle(), errorMsg)) return false;
    if (!validateUnit(material.getUnite(), errorMsg)) return false;
    if (!validateLocale(material.getLocale(), errorMsg)) return false;
    if (!validateEmplacement(material.getEmplacement(), errorMsg)) return false;
    return true;
}

bool StockValidators::validateAll(const QString& name, const QString& type, double quantity,
                                  double price, const QString& supplier, double threshold,
                                  const QDate& lastOrder, double monthlyConsumption,
                                  const QString& unit, int /*productId*/,
                                  const QString& locale, const QString& emplacement,
                                  QString& errorMsg, int excludeId)
{
    if (!validateName(name, errorMsg, excludeId)) return false;
    if (!validateType(type, errorMsg)) return false;
    if (!validateQuantity(quantity, errorMsg)) return false;
    if (!validatePrice(price, errorMsg)) return false;
    if (!validateSupplier(supplier, errorMsg)) return false;
    if (!validateThreshold(threshold, errorMsg)) return false;
    if (!validateDate(lastOrder, errorMsg)) return false;
    if (!validateMonthlyConsumption(monthlyConsumption, errorMsg)) return false;
    if (!validateUnit(unit, errorMsg)) return false;
    if (!validateLocale(locale, errorMsg)) return false;
    if (!validateEmplacement(emplacement, errorMsg)) return false;
    return true;
}
