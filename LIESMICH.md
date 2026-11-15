# Hatari – Update v26 (Druckerengine 2.6)

Nur geänderte Dateien, gleicher Aufbau wie v25 (Ordner `src`, Datei `build.yml`).

## Änderung
**ESC J, ESC j und ESC 3 rechneten bei 24-Nadel-Druckern in 1/216 Zoll statt in 1/180 Zoll.**
Epson/NEC/Star 24-Nadel: n/180 Zoll; nur 9-Nadel-Drucker (FX-80, NLQ401): n/216 Zoll.
TeX druckt die Seite als Bänder aus je 24 Punktzeilen (ESC * 39) und schiebt mit ESC J 24 genau ein Band weiter.
Mit 1/216 überlappten die Bänder um 4 Punktzeilen (Zeilen "gequetscht"), nur Zeilen mit ESC J 30 waren in Ordnung –
das ergab die unregelmäßigen Verzerrungen. Jetzt gibt es die Hilfsfunktion `vunit()` in `escp.c`.

`WinApe/escp_vunit.patch` ist derselbe Eingriff als Patch für das WinApe-Projekt (nur `escp.c`).
