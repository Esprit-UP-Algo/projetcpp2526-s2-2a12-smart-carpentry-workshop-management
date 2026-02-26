#ifndef STOCKMATERIAL_H
#define STOCKMATERIAL_H

#include <QString>
#include <QDate>

class StockMaterial {
public:
    StockMaterial();
    StockMaterial(const QString& nom, const QString& type,
                  double quantite, double prixUnitaire,
                  const QString& fournisseur, double seuilAlerte,
                  const QDate& lastOrder, double consoMensuelle, const QString& unite = QString());

    // Getters
    int     getId()            const { return m_id; }
    QString getNom()           const { return m_nomMat; }
    QString getType()          const { return m_typeMat; }
    double  getQuantite()      const { return m_quantite; }
    double  getPrixUnitaire()  const { return m_prixUnitaire; }
    QString getFournisseur()   const { return m_fournisseur; }
    double  getSeuilAlerte()   const { return m_seuilAlerte; }
    QDate   getLastOrder()     const { return m_lastOrder; }
    double  getConsoMensuelle()const { return m_consoMensuelle; }

    // Setters
    void setId(int v)                    { m_id = v; }
    void setNom(const QString& v)        { m_nomMat = v; }
    void setType(const QString& v)       { m_typeMat = v; }
    void setQuantite(double v)           { m_quantite = v; }
    void setPrixUnitaire(double v)       { m_prixUnitaire = v; }
    void setFournisseur(const QString& v){ m_fournisseur = v; }
    void setSeuilAlerte(double v)        { m_seuilAlerte = v; }
    void setLastOrder(const QDate& v)    { m_lastOrder = v; }
    void setConsoMensuelle(double v)     { m_consoMensuelle = v; }

    // Getter / Setter pour Unité
    QString getUnite() const { return m_unite; }
    void setUnite(const QString& v) { m_unite = v; }

    bool isBelowAlert() const { return m_quantite < m_seuilAlerte; }
    bool isValid() const;

    bool operator==(const StockMaterial& o) const { return m_id == o.m_id; }

private:
    int     m_id            = 0;
    QString m_nomMat;
    QString m_typeMat;
    double  m_quantite      = 0.0;
    double  m_prixUnitaire  = 0.0;
    QString m_fournisseur;
    double  m_seuilAlerte   = 0.0;
    QDate   m_lastOrder;
    double  m_consoMensuelle= 0.0;
    QString m_unite;
};

#endif // STOCKMATERIAL_H
