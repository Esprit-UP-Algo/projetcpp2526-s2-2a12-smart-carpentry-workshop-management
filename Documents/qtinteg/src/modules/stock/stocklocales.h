#ifndef STOCKLOCALES_H
#define STOCKLOCALES_H

#include <QString>
#include <QStringList>
#include <QList>

// ============================================================================
//  StockLocales — données statiques des locales et emplacements
//  Les coordonnées GPS sont codées ici, jamais en base de données.
// ============================================================================

struct LocaleInfo {
    QString nom;
    double  latitude;
    double  longitude;
    QString gouvernorat; // pour colorier la carte SVG
};

namespace StockLocales {

// 15 locales (entrepôts/dépôts) répartis en Tunisie
inline const QList<LocaleInfo>& locales()
{
    static const QList<LocaleInfo> LOCALES = {
                                              { "Tunis Centre",       36.8190,  10.1658, "tunis"       },
                                              { "Tunis Sud",          36.7800,  10.1700, "tunis"       },
                                              { "Ariana",             36.8625,  10.1956, "ariana"      },
                                              { "Ben Arous",          36.7533,  10.2283, "ben-arous"   },
                                              { "Bizerte",            37.2744,   9.8739, "bizerte"     },
                                              { "Nabeul",             36.4561,  10.7376, "nabeul"      },
                                              { "Sousse",             35.8245,  10.6346, "sousse"      },
                                              { "Monastir",           35.7643,  10.8113, "monastir"    },
                                              { "Sfax Nord",          34.7400,  10.7600, "sfax"        },
                                              { "Sfax Sud",           34.6900,  10.7600, "sfax"        },
                                              { "Gabès",              33.8881,   9.8600, "gabes"       },
                                              { "Gafsa",              34.4250,   8.7842, "gafsa"       },
                                              { "Kairouan",           35.6781,  10.0994, "kairouan"    },
                                              { "Médenine",           33.3549,  10.5055, "medenine"    },
                                              { "Jendouba",           36.5011,   8.7803, "jendouba"    },
                                              };
    return LOCALES;
}

// Retourne les noms uniquement (pour peupler un QComboBox)
inline QStringList localeNames()
{
    QStringList names;
    for (const LocaleInfo& l : locales())
        names << l.nom;
    return names;
}

// Retourne les infos d'une locale par son nom (nullptr si non trouvé)
inline const LocaleInfo* findLocale(const QString& nom)
{
    for (const LocaleInfo& l : locales())
        if (l.nom == nom) return &l;
    return nullptr;
}

// Liste des emplacements disponibles dans un entrepôt (grille 5x4)
inline const QStringList& emplacements()
{
    static const QStringList EMPLACEMENTS = {
        "A1", "A2", "A3", "A4", "A5",
        "B1", "B2", "B3", "B4", "B5",
        "C1", "C2", "C3", "C4", "C5",
        "D1", "D2", "D3", "D4", "D5",
    };
    return EMPLACEMENTS;
}

} // namespace StockLocales

#endif // STOCKLOCALES_H
