# Pure Maps: ShowRoute D-Bus method

`pure-maps-showroute.patch` (against Pure Maps 3.5.1, master as of 2026-09-01)
adds `ShowRoute(String route_json)` to the `io.github.rinigus.PureMaps`
D-Bus interface. Fahrplan AT calls it to display the complete walking leg;
without the patch Fahrplan AT falls back to writing the leg as
`~/Downloads/fahrplanat-fussweg.gpx` (for Pure Maps' GPX router) and showing
the destination as POI.

Apply in the Pure Maps source tree:

    git apply pure-maps-showroute.patch

Route JSON format (the same the routers produce):

    {"x": [16.37, ...], "y": [48.20, ...], "mode": "foot",
     "maneuvers": [{"x":..,"y":..,"icon":"depart","narrative":".."}, ...],
     "locations": [{"x":..,"y":..,"text":"Start"}, {"x":..,"y":..,"text":"Ziel","destination":true}],
     "location_indexes": [0, n-1], "language": "de", "provider": "Fahrplan AT"}

Manual test:

    dbus-send --session --print-reply --dest=io.github.rinigus.PureMaps \
      /io/github/rinigus/PureMaps io.github.rinigus.PureMaps.ShowRoute \
      string:'{"x":[16.3753,16.3830],"y":[48.1852,48.1890],"mode":"foot"}'
