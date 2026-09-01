// Desktop test: run the parser over recorded HCI responses (hafas-client fixtures).
// Build: cd tests && qmake && make && ./parsertest <fixture.json>...
#include "../src/hafasparser.h"
#include "../src/hafastypes.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

static QJsonObject loadRes(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }
    const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
    if (root.contains(QStringLiteral("svcResL"))) {
        return root.value(QStringLiteral("svcResL")).toArray().first().toObject().value(QStringLiteral("res")).toObject();
    }
    return root;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    out.setCodec("UTF-8");
    int failures = 0;

    // Unit checks for date/time handling
    const QDateTime dt = HafasParser::parseDateTime(QStringLiteral("20260901"), QStringLiteral("01123000"));
    out << "parseDateTime day offset: " << dt.toString(Qt::ISODate) << "\n";
    if (dt.date().day() != 2 || dt.time().hour() != 12 || dt.time().minute() != 30) {
        out << "FAIL day offset\n";
        ++failures;
    }
    if (HafasParser::parseDuration(QStringLiteral("010500")) != 65 || HafasParser::parseDuration(QStringLiteral("01010500")) != 1505) {
        out << "FAIL parseDuration\n";
        ++failures;
    }
    out << HafasFormat::duration(65) << " | " << HafasFormat::duration(45) << " | " << HafasFormat::delay(5) << " | "
        << HafasFormat::productNames(0x1005 | 0x20) << "\n";
    out << HafasFormat::htmlToStyled(QStringLiteral("Hallo<br/>Welt <b>fett</b><p>Absatz</p><ul><li>eins</li></ul>")) << "\n";

    for (int i = 1; i < argc; ++i) {
        const QString path = QString::fromLocal8Bit(argv[i]);
        const QJsonObject res = loadRes(path);
        out << "\n=== " << path << " ===\n";
        if (res.isEmpty()) {
            out << "FAIL: could not load\n";
            ++failures;
            continue;
        }
        HafasCommon common(res);

        if (res.contains(QStringLiteral("outConL"))) {
            const QJsonArray outConL = res.value(QStringLiteral("outConL")).toArray();
            out << "journeys: " << outConL.size() << " ctxB=" << res.value(QStringLiteral("outCtxScrB")).toString().left(20) << "\n";
            for (int k = 0; k < outConL.size() && k < 3; ++k) {
                const HafasJourney j = HafasParser::parseJourney(outConL.at(k).toObject(), common);
                out << "  " << HafasFormat::date(j.dep.effective()) << " " << HafasFormat::time(j.dep.scheduled)
                    << (j.dep.hasRealtime() ? " (" + HafasFormat::delay(j.dep.delayMinutes()) + ")" : QString())
                    << " -> " << HafasFormat::time(j.arr.scheduled)
                    << (j.arr.hasRealtime() ? " (" + HafasFormat::delay(j.arr.delayMinutes()) + ")" : QString())
                    << "  " << HafasFormat::duration(j.durationMinutes) << ", " << j.changes << " Umstiege, "
                    << j.origin.name << " -> " << j.destination.name << ", Gl. " << j.dep.platform()
                    << ", lines: " << j.lineNames().join(QStringLiteral(" · "))
                    << ", warnings: " << j.warningCount() << ", cancelled: " << j.cancelled << "\n";
                for (const HafasLeg &l : j.legs) {
                    out << "     leg " << l.type << " " << (l.hasProduct ? l.product.displayName() : QStringLiteral("-"))
                        << " [" << l.product.color << "] " << l.origin.name << " " << HafasFormat::time(l.dep.effective())
                        << " Gl." << l.dep.platform() << " -> " << l.destination.name << " "
                        << HafasFormat::time(l.arr.effective()) << " Gl." << l.arr.platform()
                        << " dir=" << l.direction << " stops=" << l.stops.size() << " dist=" << l.distance
                        << " dur=" << l.durationMinutes << " remarks=" << l.remarks.size() << " warn=" << l.warnings.size() << "\n";
                    for (const HafasRemark &r : l.remarks) {
                        out << "        rem[" << r.type << "/" << r.code << "] " << r.text.left(70) << "\n";
                    }
                    for (const HafasWarning &w : l.warnings) {
                        out << "        him " << w.head.left(70) << "\n";
                    }
                }
                const QVariantMap v = j.toVariant();
                if (v.value(QStringLiteral("legs")).toList().size() != j.legs.size()) {
                    out << "FAIL toVariant legs\n";
                    ++failures;
                }
            }
        }
        if (res.contains(QStringLiteral("journey"))) {
            // JourneyDetails style result: parse the stop list through parseLeg-like path
            const QJsonObject jny = res.value(QStringLiteral("journey")).toObject();
            const QString date = jny.value(QStringLiteral("date")).toString();
            const HafasProduct p = common.product(jny.value(QStringLiteral("prodX")).toInt(-1));
            out << "trip: " << p.displayName() << " (" << p.catOutL << ", cls " << p.cls << ", color " << p.color
                << ", op " << p.operatorName << ") -> " << jny.value(QStringLiteral("dirTxt")).toString() << "\n";
            const QJsonArray stopL = jny.value(QStringLiteral("stopL")).toArray();
            for (int k = 0; k < stopL.size(); ++k) {
                const HafasStop s = HafasParser::parseStop(stopL.at(k).toObject(), date, common);
                out << "   " << HafasFormat::time(s.arr.scheduled) << "/" << HafasFormat::time(s.dep.scheduled)
                    << " " << s.loc.name << " Gl. " << s.dep.platform() << (s.dep.platformChanged() ? "*" : "")
                    << (s.passBy ? " (Durchfahrt)" : "") << "\n";
            }
            QList<HafasRemark> rems;
            QList<HafasWarning> warns;
            HafasParser::parseMessages(jny.value(QStringLiteral("msgL")).toArray(), common, rems, warns);
            out << "   remarks: " << rems.size() << ", warnings: " << warns.size() << "\n";
            for (const HafasRemark &r : rems) {
                out << "     rem[" << r.type << "/" << r.code << "] " << r.text.left(80) << "\n";
            }
            for (const HafasWarning &w : warns) {
                out << "     him " << w.head << " | " << HafasFormat::dateTime(w.validFrom) << " - "
                    << HafasFormat::dateTime(w.validUntil) << "\n";
            }
        }
        if (res.contains(QStringLiteral("msgL")) || res.contains(QStringLiteral("msgRefL"))) {
            const QList<HafasWarning> warns = HafasParser::parseHimSearch(res);
            out << "him messages: " << warns.size() << "\n";
            for (int k = 0; k < warns.size() && k < 5; ++k) {
                const HafasWarning &w = warns.at(k);
                out << "   [" << w.prio << "] " << w.head.left(60) << " | " << w.company << " | "
                    << HafasFormat::productNames(w.prod) << " | until " << HafasFormat::dateTime(w.validUntil)
                    << " | lines " << w.affectedLines.join(QStringLiteral(",")) << " | " << w.fromName << "\n";
            }
            if (warns.isEmpty()) {
                out << "FAIL no HIM messages parsed\n";
                ++failures;
            }
        }
        if (res.contains(QStringLiteral("match"))) {
            const QJsonArray locL = res.value(QStringLiteral("match")).toObject().value(QStringLiteral("locL")).toArray();
            for (int k = 0; k < locL.size(); ++k) {
                const HafasLocation l = HafasParser::parseLocation(locL.at(k).toObject());
                out << "  loc " << l.type << " " << l.name << " " << l.extId << " " << l.lat << "," << l.lon << " "
                    << HafasFormat::productNames(l.products) << "\n";
            }
        }
    }
    out << "\n" << (failures ? "FAILURES: " : "OK ") << failures << "\n";
    return failures ? 1 : 0;
}
