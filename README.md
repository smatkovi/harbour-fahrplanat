# Fahrplan AT (harbour-fahrplanat)

Unofficial public transport journey planner for Austria on Sailfish OS.
Native C++/Qt core with a Silica QML user interface. Data source is the
HAFAS "HCI" JSON interface at `fahrplan.oebb.at` — the same backend the
official ÖBB Scotty app uses. This project is not affiliated with ÖBB or HaCon.

## Features (0.1.0)

- Location search with autocomplete (stations, addresses, POIs), history and favourites
- Connection search: from / to / via, departure or arrival time, product filter
  with the exact ÖBB product groups, direct connections only, minimum transfer time,
  wheelchair, bicycle carriage, Einfach-Raus-Ticket
- Results with real-time departure/arrival, delays, platform changes,
  cancellations, earlier/later scrolling
- Connection details: legs, intermediate stops, platforms, remarks,
  disruption messages attached to the connection or a leg
- Disruption messages (HimSearch) with sorting by priority, recency or title
- Map page inside the app: OpenStreetMap raster tiles (own slippy-map QML, no map library)
  with the walking leg or the whole connection drawn from the HAFAS geometry
- Map hand-off: tap a station to show it in Pure Maps (D-Bus `ShowPoi`, `geo:` URI
  fallback). Tap a walking leg to hand the complete footpath (HAFAS polyline) to Pure Maps
  via `ShowRoute` (needs the patch in `contrib/`); stock Pure Maps gets the leg as
  `~/Downloads/fahrplanat-fussweg.gpx` for its GPX router plus the destination as POI
- Configurable backend profile (endpoint, HCI version, extension, client, AID)
  with three presets; request logging for debugging

## Backend profile

Default preset (taken from the ÖBB Android app 9.0.5 configuration):

| Field | Value |
|---|---|
| Endpoint | `https://fahrplan.oebb.at/gate` |
| HCI version | `1.77` |
| Extension | `OEBB.14` |
| Client | `{ "id": "OEBB", "type": "AND", "v": "72" }` |
| Auth | `{ "type": "AID", "aid": "OWDL4fE4ixNiPBBm" }` |

Alternative presets: Öffi/public-transport-enabler (`gate`, HCI 1.88) and
hafas-client (`bin/mgate.exe`, HCI 1.45, iPhone client). Switch in
*Einstellungen* if the server rejects requests; the server error text is shown.

Product class bits used by the ÖBB backend:

| Group | Bits |
|---|---|
| Fernreisezüge (RJ, RJX, ICE, IC, EC) | `0x1005` |
| Nachtreisezüge und Schnellzüge | `0x0008` |
| Regionale Reisezüge | `0x0010` |
| S-Bahnen | `0x0020` |
| U-Bahnen | `0x0100` |
| Straßenbahnen | `0x0200` |
| Busse | `0x0040` |
| Sonstige Verkehre (Rufbus, AST, Seilbahn) | `0x0800` |
| Schifffahrtsverkehre | `0x0080` |
| Schienenersatzverkehre | `0x0002` |
| All (default filter) | `0x1BFF` |

## Building

In the Sailfish SDK (coderus platform SDK container):

    mb2 -t SailfishOS-5.1.0.26-aarch64 build

The RPM ends up in `RPMS/`. For armv7hl or i486 change the target.

## Desktop parser test

`tests/` contains a harness that runs the parser against recorded HCI
responses (e.g. the fixtures of `hafas-client`):

    cd tests && qmake tests.pro && make && ./parsertest fixture.json ...

## Structure

- `src/hafastypes.*` — data structures and German formatting helpers
- `src/hafasclient.*` — HCI request envelope, HTTP, error mapping
- `src/hafasparser.*` — response parsing (common tables, journeys, legs, stops, HIM)
- `src/locationmodel.*`, `src/journeymodel.*`, `src/himmodel.*`, `src/recentmodel.*` — QML models
- `src/appsettings.*` — persisted settings and backend presets
- `qml/pages/*` — Silica pages, `qml/components/*` — shared items, `qml/cover/*` — cover

## License

GPLv3. Protocol description based on public-transport/hafas-client and
schildbach/public-transport-enabler (both open source).
