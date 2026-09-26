// Die Linien auf der Karte.
//
// Auf Sailfish zeichnet ein QtQuick-2-Canvas den Streckenverlauf; QtQuick 1.1
// hat kein Canvas. Dieses Element bekommt die Punkte bereits in
// Bildschirmkoordinaten -- gerechnet wird weiter in QML, wo auch die Kachel-
// Mathematik steht -- und malt sie mit QPainter.
#ifndef POLYLINEITEM_H
#define POLYLINEITEM_H

#include <QDeclarativeItem>
#include <QVariantList>

class PolylineItem : public QDeclarativeItem
{
    Q_OBJECT
    // Je Eintrag eine Karte: points (Zahlenfeld x0, y0, x1, y1, ...), color,
    // width, casing (Breite des weissen Saums, 0 = keiner) und alpha.
    Q_PROPERTY(QVariantList lines READ lines WRITE setLines NOTIFY linesChanged)

public:
    explicit PolylineItem(QDeclarativeItem *parent = 0);

    QVariantList lines() const { return m_lines; }
    void setLines(const QVariantList &lines);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:
    void linesChanged();

private:
    QVariantList m_lines;
};

#endif // POLYLINEITEM_H
