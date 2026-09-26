#include "polylineitem.h"

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPolygonF>

PolylineItem::PolylineItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    // Ohne das malt ein QDeclarativeItem nichts: es gilt als leer.
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    setSmooth(true);
}

void PolylineItem::setLines(const QVariantList &lines)
{
    m_lines = lines;
    emit linesChanged();
    update();
}

void PolylineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < m_lines.size(); ++i) {
        const QVariantMap line = m_lines.at(i).toMap();
        const QVariantList coords = line.value(QLatin1String("points")).toList();
        if (coords.size() < 4) {
            continue;
        }
        QPolygonF polygon;
        for (int p = 0; p + 1 < coords.size(); p += 2) {
            polygon << QPointF(coords.at(p).toReal(), coords.at(p + 1).toReal());
        }

        const qreal breite = line.value(QLatin1String("width"), 4).toReal();
        const qreal saum = line.value(QLatin1String("casing"), 0).toReal();
        const qreal deckung = line.value(QLatin1String("alpha"), 1.0).toReal();

        painter->setOpacity(1.0);
        if (saum > 0) {
            // Der weisse Saum zuerst, damit die Linie auf der Karte zu sehen
            // bleibt, egal was darunter liegt.
            QPen pen(QColor(255, 255, 255, 230), saum);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            painter->setPen(pen);
            painter->drawPolyline(polygon);
        }

        QPen pen(QColor(line.value(QLatin1String("color"), QLatin1String("#ff3d55")).toString()), breite);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter->setOpacity(deckung);
        painter->setPen(pen);
        painter->drawPolyline(polygon);
    }
    painter->setOpacity(1.0);
}
