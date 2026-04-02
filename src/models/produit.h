#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>
#include <QDate>

class Produit
{
public:
    Produit();
    Produit(const QString& id, const QString& nom, const QString& categorie,
            double prix, const QString& dimensions,
            const QString& mat, const QDate& date,
            const QString& image);


    // Getters
    QString getId() const { return m_id; }
    QString getNom() const { return m_nom; }
    QString getCategorie() const { return m_categorie; }
    double  getPrix() const { return m_prix; }
    QString getDimensions() const { return m_dimensions; }
    QString getMat() const { return m_mat; }
    QDate   getDate() const { return m_date; }
    QString getImage() const { return m_image; }

    // Setters
    void setId(const QString& v) { m_id = v; }
    void setNom(const QString& v) { m_nom = v; }
    void setCategorie(const QString& v) { m_categorie = v; }
    void setPrix(double v) { m_prix = v; }
    void setDimensions(const QString& v) { m_dimensions = v; }
    void setMat(const QString& v) { m_mat = v; }
    void setDate(const QDate& v) { m_date = v; }
    void setImage(const QString& v) { m_image = v; }

    bool isValid() const;


private:
    QString m_id;
    QString m_nom;
    QString m_categorie;
    double  m_prix = 0.0;
    QString m_dimensions;
    QString m_mat;
    QDate   m_date;
    QString m_image;

};

#endif
