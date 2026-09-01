#ifndef HAFASPARSER_H
#define HAFASPARSER_H

#include "hafastypes.h"

#include <QJsonArray>
#include <QJsonObject>

// Access to the `common` lookup tables of a HAFAS service result. All lists in
// the result reference these tables by index (locX, prodX, remX, himX, ...).
class HafasCommon
{
public:
    explicit HafasCommon(const QJsonObject &res);

    HafasLocation location(int index) const;
    HafasProduct product(int index) const;
    HafasRemark remark(int index) const;
    HafasWarning warning(int index) const;
    QString operatorName(int index) const;
    QString iconColor(int index, bool foreground) const;
    QList<HafasPoint> polyline(int index) const;

    int warningCount() const { return m_himL.size(); }
    QJsonObject rawWarning(int index) const { return m_himL.at(index).toObject(); }

private:
    QJsonObject m_common;
    QJsonArray m_locL;
    QJsonArray m_prodL;
    QJsonArray m_opL;
    QJsonArray m_remL;
    QJsonArray m_himL;
    QJsonArray m_icoL;
    QJsonArray m_polyL;
};

namespace HafasParser
{
    // "yyyyMMdd" + "HHmmss" (optionally prefixed with a day offset, e.g. "01123000")
    QDateTime parseDateTime(const QString &date, const QString &time);
    // "HHmmss" or "ddHHmmss" -> minutes
    int parseDuration(const QString &dur);

    HafasLocation parseLocation(const QJsonObject &l);
    // prefix is 'd' for departure fields (dTimeS, dPlatfS ...) or 'a' for arrival fields
    HafasTime parseTime(const QJsonObject &o, const QString &date, QChar prefix);
    HafasStop parseStop(const QJsonObject &st, const QString &date, const HafasCommon &common);
    HafasLeg parseLeg(const QJsonObject &sec, const QString &date, const HafasCommon &common);
    HafasJourney parseJourney(const QJsonObject &con, const HafasCommon &common);
    HafasWarning parseWarning(const QJsonObject &w, const HafasCommon &common);

    // Resolves msgL entries ({type:"REM", remX} / {type:"HIM", himX}) into lists.
    void parseMessages(const QJsonArray &msgL, const HafasCommon &common,
                       QList<HafasRemark> &remarks, QList<HafasWarning> &warnings);

    // Parses a complete HimSearch result (msgL inline, msgRefL or common.himL).
    QList<HafasWarning> parseHimSearch(const QJsonObject &res);

    // Decodes a Google encoded polyline (precision 1e-5) as used in common.polyL[].crdEncYX
    QList<HafasPoint> decodePolyline(const QString &encoded);
}

#endif // HAFASPARSER_H
