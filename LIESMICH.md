# Hatari – Update v27 (Version 2.7.2.7, PC-Tastaturbelegung)

Nur geänderte/neue Dateien, gleicher Aufbau wie v26 (Ordner `src`, `tools`,
`doku`, Datei `build.yml`). Basis ist v26 (Druckerengine 2.6).

## Neu
* `src/keymap_pc.c`, `src/includes/keymap_pc.h`, `src/includes/keymap_pc_cfg.h`
* `tools/patch_keymap.py` (Schritt 5a im Bau)
* `doku/Tastatur-PC-Layout.md`, `doku/Aenderungen.txt`
* `build.yml` (Prüfzeilen, Schritt 5a, Version 2.7.2.7, Doku im Auslieferungsordner)

Nicht ersetzt werden `configuration.c/.h` und `src/CMakeLists.txt`: die kleinen
Ergänzungen dafür macht Schritt 5a beim Bau selbst.

Bedienung: Taste **Rollen** schaltet die PC-Tastaturbelegung um (Vorgabe aus).
Einzelheiten in `doku/Tastatur-PC-Layout.md`.
