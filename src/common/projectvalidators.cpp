#include "projectvalidators.h"
#include "src/database/projectdatabase.h" // to check uniqueness
#include "src/models/projet.h"
#include <QRegularExpression>

// ----------------------------------------------------------------------------
// Private helper
// ----------------------------------------------------------------------------
bool ProjectValidators::isProjectNameUnique(const QString& name, int excludeId)
{
    const QList<Projet>& all = ProjectDatabase::instance().getAllProjets();
    for (const Projet& p : all) {
        if (p.getNom().compare(name, Qt::CaseInsensitive) == 0) {
            if (excludeId != -1 && p.getId() == excludeId)
                continue; // it's the same project being edited
            return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// Individual validators
// ----------------------------------------------------------------------------
bool ProjectValidators::validateProjectName(const QString& name, QString& errorMsg, int excludeId)
{
    if (name.trimmed().isEmpty()) {
        errorMsg = "Le nom du projet est obligatoire.";
        return false;
    }
    if (name.length() > 30) {
        errorMsg = "Le nom du projet ne doit pas dépasser 30 caractères.";
        return false;
    }
    if (!isProjectNameUnique(name, excludeId)) {
        errorMsg = "Un projet avec ce nom existe déjà. Veuillez choisir un nom unique.";
        return false;
    }
    return true;
}

bool ProjectValidators::validateClientName(const QString& client, QString& errorMsg)
{
    if (client.trimmed().isEmpty()) {
        errorMsg = "Le nom du client est obligatoire.";
        return false;
    }
    if (client.length() > 20) {
        errorMsg = "Le nom du client ne doit pas dépasser 20 caractères.";
        return false;
    }
    return true;
}

bool ProjectValidators::validateBudget(double budget, QString& errorMsg)
{
    if (budget <= 0.0) {
        errorMsg = "Le budget doit être supérieur à 0 DT.";
        return false;
    }
    return true;
}

bool ProjectValidators::validateChefProjet(int chefId, QString& errorMsg)
{
    if (chefId == 0) {
        errorMsg = "Veuillez sélectionner un chef de projet (option '— Aucun —' n'est pas autorisée).";
        return false;
    }

    return true;
}

bool ProjectValidators::validateDates(const QDate& start, const QDate& deadline, QString& errorMsg)
{
    if (!start.isValid()) {
        errorMsg = "La date de début est invalide.";
        return false;
    }
    if (!deadline.isValid()) {
        errorMsg = "La deadline est invalide.";
        return false;
    }
    if (deadline < start) {
        errorMsg = "La deadline ne peut pas être antérieure à la date de début.";
        return false;
    }
    // Optional: warn if deadline is too far? Not a hard error but could be useful.
    if (start.daysTo(deadline) > 365*5) {
        errorMsg = "La durée du projet dépasse 5 ans. Est-ce correct ?";
        // Not a blocker, just a warning. We'll treat it as an optional check.
        // Return true anyway, but the caller could show a warning.
    }
    return true;
}

bool ProjectValidators::validateStatus(const QString& status, QString& errorMsg)
{
    static const QStringList validStatuses = {"En cours", "En attente", "Terminé", "Annulé"};
    if (status.trimmed().isEmpty()) {
        errorMsg = "Le statut est obligatoire.";
        return false;
    }
    if (!validStatuses.contains(status)) {
        errorMsg = "Statut invalide. Valeurs autorisées : " + validStatuses.join(", ");
        return false;
    }
    return true;
}

bool ProjectValidators::validateType(const QString& type, QString& errorMsg)
{
    static const QStringList validTypes = {"Meuble sur mesure", "Rénovation", "Agencement",
                                           "Restaurant", "Bureau", "Cuisine", "Salle de bain", "Autre"};
    if (type.trimmed().isEmpty()) {
        errorMsg = "Le type de projet est obligatoire.";
        return false;
    }
    if (!validTypes.contains(type)) {
        errorMsg = "Type de projet invalide. Valeurs autorisées : " + validTypes.join(", ");
        return false;
    }
    return true;
}

bool ProjectValidators::validateAdresse(const QString& adresse, QString& errorMsg)
{
    if (adresse.trimmed().isEmpty()) {
        errorMsg = "L'adresse du chantier est obligatoire.";
        return false;
    }
    if (adresse.length() > 200) {
        errorMsg = "L'adresse ne doit pas dépasser 200 caractères.";
        return false;
    }
    // Optional: basic XSS prevention – remove HTML tags
    // (already handled by toHtmlEscaped in PDF generation, but we can clean here)
    return true;
}

// ----------------------------------------------------------------------------
// Combined validators
// ----------------------------------------------------------------------------
bool ProjectValidators::validateAll(const Projet& projet, QString& errorMsg, bool isEdit)
{
    int excludeId = isEdit ? projet.getId() : -1;

    if (!validateProjectName(projet.getNom(), errorMsg, excludeId)) return false;
    if (!validateClientName(projet.getClient(), errorMsg)) return false;
    if (!validateType(projet.getType(), errorMsg)) return false;
    if (!validateDates(projet.getDateProjet(), projet.getDeadline(), errorMsg)) return false;
    if (!validateStatus(projet.getStatus(), errorMsg)) return false;
    if (!validateBudget(projet.getBudget(), errorMsg)) return false;
    if (!validateAdresse(projet.getAdresse(), errorMsg)) return false;
    if (!validateChefProjet(projet.getChefProjet(), errorMsg)) return false;
    return true;
}

bool ProjectValidators::validateAll(const QString& nom, const QString& client,
                                    const QString& type, const QDate& dateDebut,
                                    const QDate& deadline, const QString& status,
                                    double budget, const QString& adresse, int chefId,
                                    QString& errorMsg, int excludeId)
{
    if (!validateProjectName(nom, errorMsg, excludeId)) return false;
    if (!validateClientName(client, errorMsg)) return false;
    if (!validateType(type, errorMsg)) return false;
    if (!validateDates(dateDebut, deadline, errorMsg)) return false;
    if (!validateStatus(status, errorMsg)) return false;
    if (!validateBudget(budget, errorMsg)) return false;
    if (!validateAdresse(adresse, errorMsg)) return false;
    if (!validateChefProjet(chefId, errorMsg)) return false;
    return true;
}
