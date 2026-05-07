#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>
#include <QDate>
#include <QByteArray>

class Produit
{
public:
    Produit();
    Produit(const QString& id, const QString& nom, const QString& categorie,
            double prix, const QString& dimensions, const QString& matUtilise,
            const QDate& dateCreation, const QString& projetId);

    // Getters
    QString    getId()           const { return m_id; }
    QString    getNom()          const { return m_nom; }
    QString    getCategorie()    const { return m_categorie; }
    double     getPrix()         const { return m_prix; }
    QString    getDimensions()   const { return m_dimensions; }
    QString    getMatUtilise()   const { return m_matUtilise; }
    QDate      getDateCreation() const { return m_dateCreation; }
    QByteArray getImage()        const { return m_image; }
    QString    getProjetId()     const { return m_projetId; }

    bool hasImage() const { return !m_image.isEmpty(); }

    // Setters
    void setId           (const QString&    v) { m_id           = v; }
    void setNom          (const QString&    v) { m_nom          = v; }
    void setCategorie    (const QString&    v) { m_categorie    = v; }
    void setPrix         (double            v) { m_prix         = v; }
    void setDimensions   (const QString&    v) { m_dimensions   = v; }
    void setMatUtilise   (const QString&    v) { m_matUtilise   = v; }
    void setDateCreation (const QDate&      v) { m_dateCreation = v; }
    void setImage        (const QByteArray& v) { m_image        = v; }
    void setProjetId     (const QString&    v) { m_projetId     = v; }

    bool isValid() const;

private:
    QString    m_id;
    QString    m_nom;
    QString    m_categorie;
    double     m_prix         = 0.0;
    QString    m_dimensions;
    QString    m_matUtilise;
    QDate      m_dateCreation;
    QByteArray m_image;         // image stockée en binaire en mémoire
    QString    m_projetId;      // FK → PROJET(ID_PROJET)
};

#endif // PRODUIT_H
