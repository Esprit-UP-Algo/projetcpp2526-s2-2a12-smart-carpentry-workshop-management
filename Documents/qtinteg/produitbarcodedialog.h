#ifndef PRODUITBARCODEDIALOG_H
#define PRODUITBARCODEDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QString>
#include <QPixmap>
#include "src/models/produit.h"

// ─────────────────────────────────────────────────────────────────────────────
// Widget de rendu Code128 (dessin pur QPainter, zéro bibliothèque)
// ─────────────────────────────────────────────────────────────────────────────
class BarcodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BarcodeWidget(QWidget *parent = nullptr);
    void setData(const QString& data);
    QPixmap toPixmap() const;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QString          m_data;
    QVector<bool>    m_bars;   // true = barre noire, false = espace
    void             encode();

    // Tables Code128-B
    static QVector<QVector<int>> code128B();
    static QVector<int>          charToCode128B(QChar c);
};

// ─────────────────────────────────────────────────────────────────────────────
// Widget QR Code simplifié (version matricielle pour un ID court)
// ─────────────────────────────────────────────────────────────────────────────
class QRWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QRWidget(QWidget *parent = nullptr);
    void setData(const QString& data);
    QPixmap toPixmap() const;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QString              m_data;
    QVector<QVector<bool>> m_matrix;
    int                  m_size = 0;
    void                 buildMatrix();
    void                 addFinderPattern(int r, int c);
    void                 addTimingPatterns();
    bool                 getDataBit(int idx) const;
    QVector<bool>        encodeData() const;
};

// ─────────────────────────────────────────────────────────────────────────────
// Dialogue principal code-barres
// ─────────────────────────────────────────────────────────────────────────────
class ProduitBarcodeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProduitBarcodeDialog(const Produit& produit, QWidget *parent = nullptr);

private slots:
    void onExportPNG();
    void onPrint();

private:
    void setupUI();

    Produit       m_produit;
    BarcodeWidget *m_barcode = nullptr;
    QRWidget      *m_qr      = nullptr;
};

#endif // PRODUITBARCODEDIALOG_H

