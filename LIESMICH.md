# Hatari mit PDF-Drucker – Gesamtstand Version 2.7.2.7

Vollständiger Stand des Repositorys: v24 + v25 + v26 + v27 (siehe `doku/Aenderungen.txt`).

## Aufbau

```
.github/workflows/build.yml     Bauanleitung (GitHub Actions)
Atari-Treiber/                  Druckertreiber-Dateien für den Atari
doku/                           Dokumentation (Änderungen, PC-Tastaturbelegung)
fonts/src/                      Zeichensatzbild des NLQ401
src/                            Quelltexte (werden über Hatari 2.6.1 kopiert)
tools/                          Hilfsprogramme (pngfont.py, binfont.py, patch_keymap.py)
LIESMICH.md                     diese Datei
```

## Hinweis

`tools/prn2pdf.c` (Schritt 8 des Baus) gehört ebenfalls nach `tools/`. Sie war in
keinem der Update-Pakete enthalten und muss aus deinem bisherigen Repository
übernommen werden.

Die Taste **Rollen** schaltet die PC-Tastaturbelegung um (Vorgabe aus), siehe
`doku/Tastatur-PC-Layout.md`.
