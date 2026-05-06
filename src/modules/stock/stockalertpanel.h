#ifndef STOCKALERTPANEL_H
#define STOCKALERTPANEL_H

#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QList>
#include <QLabel>

#include "src/models/stockmaterial.h"

/**
 * StockAlertPanel — panneau flottant de notifications stock.
 * S'ouvre en popup sous la cloche.
 * Affiche une carte par matériau en alerte.
 * Émet alertSelected(id) quand l'utilisateur clique sur une carte.
 */
class StockAlertPanel : public QFrame
{
    Q_OBJECT

public:
    explicit StockAlertPanel(QWidget *parent = nullptr);

    // Recharge la liste des matériaux en alerte et reconstruit les cartes
    void populate(const QList<StockMaterial>& alertMaterials);

    // Positionne le panneau juste sous le widget 'anchor'
    void showUnder(QWidget *anchor);

signals:
    void alertSelected(int materialId); // clic sur une carte

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QWidget     *m_container  = nullptr;
    QVBoxLayout *m_cardLayout = nullptr;
    QScrollArea *m_scroll     = nullptr;
    QLabel      *m_emptyLabel = nullptr;

    void buildCard(const StockMaterial& mat);
    void clearCards();
};

#endif // STOCKALERTPANEL_H
