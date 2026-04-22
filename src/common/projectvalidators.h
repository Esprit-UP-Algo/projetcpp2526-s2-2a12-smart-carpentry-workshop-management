#ifndef PROJECTVALIDATORS_H
#define PROJECTVALIDATORS_H

#include <QString>
#include <QDate>

class Projet; // forward declaration

class ProjectValidators
{
public:
    // Individual field validators
    static bool validateProjectName(const QString& name, QString& errorMsg, int excludeId = -1);
    static bool validateClientName(const QString& client, QString& errorMsg);
    static bool validateBudget(double budget, QString& errorMsg);
    static bool validateChefProjet(int chefId, QString& errorMsg);
    static bool validateDates(const QDate& start, const QDate& deadline, QString& errorMsg);
    static bool validateStatus(const QString& status, QString& errorMsg);
    static bool validateType(const QString& type, QString& errorMsg);
    static bool validateAdresse(const QString& adresse, QString& errorMsg);

    // Combined validation for all fields (optionally with a Projet object for edit mode)
    static bool validateAll(const Projet& projet, QString& errorMsg, bool isEdit = false);
    static bool validateAll(const QString& nom, const QString& client, const QString& type,
                            const QDate& dateDebut, const QDate& deadline, const QString& status,
                            double budget, const QString& adresse, int chefId,
                            QString& errorMsg, int excludeId = -1);

private:
    // Helper to check if a project name already exists (excluding a specific ID)
    static bool isProjectNameUnique(const QString& name, int excludeId = -1);
};

#endif // PROJECTVALIDATORS_H
