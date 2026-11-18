# Hatari mit PDF-Drucker und PC-Tastaturbelegung – Überblick aller Versionen

**Aktueller Stand: Version 2.8** (Update v28, About-Dialog: Hatari v2.8)
Basis: Hatari 2.6.1, Druckerengine 2.6.

Diese Datei enthält die Anmerkungen **aller bekannten Versionen**, neueste zuerst.
Bei jeder neuen Version wird sie ergänzt und komplett ausgeliefert.
Zählweise: Update v2x = Version 2.x; x wird bei jeder neuen Version um 1 erhöht.
Im About-Dialog steht ab Version 2.8 „Hatari v2.x“ (bis Version 2.7: „Hatari v2.7.2.x“).
Die nächste Version ist **2.9** (Update v29, About: Hatari v2.9).

---

## Aufbau des Repositorys

```
.github/workflows/build.yml     Bauanleitung (GitHub Actions, Windows-Build)
Atari-Treiber/                  Druckertreiber-Dateien für den Atari (NLQ401 D/INT, Star)
doku/                           Dokumentation (Tastatur-PC-Layout.md)
fonts/src/                      Zeichensatzbild des NLQ401
src/                            Quelltexte (werden über Hatari 2.6.1 kopiert)
tools/                          Hilfsprogramme (pngfont.py, binfont.py, patch_keymap.py, prn2pdf.c)
LIESMICH.md                     diese Datei
```

Update-Pakete enthalten nur geänderte Dateien im selben Aufbau; `build.yml` gehört
nach `.github/workflows/`, alles andere in die Hauptebene neben `.github`.
Schritt 3c im Bau prüft Dateien mit Prüfmarken und meldet fehlende oder verschobene.

**Hinweis:** `tools/prn2pdf.c` (Schritt 8 des Baus) war in keinem Update-Paket seit v24
enthalten und muss aus dem bisherigen Repository erhalten bleiben (bei Verlust aus dem Git-Verlauf
zurückholen). Fehlt sie, entsteht keine `prn2pdf.exe`, der Bau läuft aber durch.

---

## Alle Tastenkombinationen

### A. „Rollen“ aus (Hatari wie bisher, plus drei Atari-Tasten)

| PC-Taste | Atari |
|---|---|
| **Pos1** | Clr/Home |
| **Bild hoch** | Help |
| **Bild runter** | Undo |
| alle übrigen Tasten | Hatari-Standard (Tastaturzuordnung aus dem Setup) |
| AltGr + Taste | Hatari-eigene Kürzel (Beenden, Vollbild, Reset …) |

Pos1/Bild hoch/Bild runter gelten in diesem Modus auch zusammen mit Shift
(Shift + Pos1 = Clr). Mit gedrückter Windows-Taste gehen sie an Hatari.

### B. „Rollen“ ein (PC-Tastaturbelegung)

Umschalten mit der Taste **Rollen** (Scroll Lock), Meldung „PC-Tastaturbelegung EIN/AUS“
in der Statusleiste und im Protokoll. Die Einstellung steht auch in der `hatari.cfg`
(`bPcLayoutMapping`).

**Die vier Ebenen des Atari**

| Ebene | PC | Ergebnis am Atari |
|---|---|---|
| 1 normal | Taste | Zeichen der Taste |
| 2 Shift | Shift + Taste | Shift ist im Zeichen enthalten, wird bei Bedarf automatisch gesetzt |
| 3 Control | Strg + Taste | Control + Atari-Taste des Zeichens (Strg+C = Control+C) |
| 4 Alternate | **linke** Alt + Taste | Alternate + Atari-Taste des Zeichens |
| AltGr (PC-Zeichen) | **rechtes** Alt (AltGr) + Taste | das PC-Zeichen, siehe unten |

**AltGr-Zeichen der deutschen PC-Tastatur**

| PC | Zeichen | am Atari erzeugt durch |
|---|---|---|
| AltGr + Q | `@` | Alternate + Ziffernblock 6 4 |
| AltGr + 8 | `[` | Alternate + Ziffernblock 9 1 |
| AltGr + 9 | `]` | Alternate + Ziffernblock 9 3 |
| AltGr + 7 | `{` | Alternate + Ziffernblock 1 2 3 |
| AltGr + 0 | `}` | Alternate + Ziffernblock 1 2 5 |
| AltGr + ß | `\` | Alternate + Ziffernblock 9 2 |
| AltGr + + | `~` | Atari-Taste laut TOS-Tabelle (Taste rechts neben Ä) |
| AltGr + < | `\|` | Atari-Taste laut TOS-Tabelle (Shift + Taste rechts neben Ä) |
| AltGr + 2 | `²` | Alternate + Ziffernblock 2 5 3 |
| AltGr + 3 | `³` | Alternate + Ziffernblock 2 5 4 |
| AltGr + M | `µ` | Alternate + Ziffernblock 2 3 0 |
| AltGr + E | `€` | gibt es am Atari nicht: unbelegt |

Die Ziffernblock-Eingabe (Alternate + ASCII-Code) beherrscht das TOS ab 1.04 und EmuTOS.
Alle anderen AltGr-Kombinationen ohne PC-Zeichen sind unbelegt – **Hatari-eigene AltGr-Kürzel
sind abgeschaltet**, solange „Rollen“ aktiv ist.

**Weitere Zeichen** (ohne AltGr): die Taste für ein Zeichen wird aus der Tastentabelle des
geladenen TOS gesucht, z. B. `#` `'` `^` `<` `>` `;` `:` `_` `+` `*` `ß` `ä ö ü`. Dadurch
stimmt das Zeichen, auch wenn die Taste am Atari an anderer Stelle liegt. Totzeichen (`^`, `´`, `` ` ``)
erscheinen sofort. `°` kommt über Alternate + Ziffernblock 2 4 8, `§` über Shift + 3 (Atari-Taste 3), `´` über Alternate + Ziffernblock 1 8 6.

**Tasten ohne Zeichen**

| PC | Atari |
|---|---|
| Esc, Rück, Tab, Enter | Esc, Backspace, Tab, Return |
| Entf, Einfg | Delete, Insert |
| **Pos1** | Clr/Home |
| **Bild hoch / Bild runter** | seitenweise blättern: Shift + Cursor hoch / Shift + Cursor runter |
| **Druck** | **Help** |
| **Ende** | **Undo** |
| Pfeiltasten | Pfeiltasten |
| F1 … F10 | F1 … F10 (Shift + F1…F10 = F11…F20 des Atari) |
| Ziffernblock 0–9 , . + − * / Enter | Ziffernblock des Atari (immer Ziffern, Num-Lock egal) |
| Num | Ziffernblock `(` |
| Menütaste (neben rechter Strg) | Ziffernblock `)` |
| Feststelltaste | wie bisher (Caps Lock des Atari) |
| Rollen | Umschalter der PC-Tastaturbelegung |
| Shift / Strg / linke Alt | Shift / Control / Alternate des Atari |

**Hatari-Kürzel bei aktiver PC-Tastaturbelegung**

| PC | Wirkung |
|---|---|
| **Windows-Taste + Taste** | ersetzt AltGr + Taste: dieselben Hatari-Kürzel wie sonst mit AltGr (z. B. Windows+Q beenden) |
| F11, F12, Pause | wie bisher direkt an Hatari |
| Tasten der Tastatur-Joystick-Emulation | gehen an Hatari |

Die vollständige Liste der Hatari-Kürzel steht im Setup (F12 → Keyboard) und in `doc/manual.html`.

### Diagnose, falls Zeichen nicht ankommen

Vor dem Start von hatari.exe in der Eingabeaufforderung `set HATARI_PCKEYS_DEBUG=1`
setzen und Hatari mit `hatari.exe --log-file pckeys.log` starten. Für jede Taste steht dann in
`pckeys.log` (Zeilen „PCKEYS: …“), ob sie ankommt und was daraus wird. Beim Bau zeigt Schritt 5a
zusätzlich im Protokoll, an welchen Stellen Hatari AltGr und Tasten behandelt.

---

## Version 2.8 – Update v28: AltGr-Zeichen, Pos1/Bild-Tasten, Tastenübersicht

**Geändert**
- **Hatari-eigene AltGr-Kürzel sind abgeschaltet, solange „Rollen“ aktiv ist.** Dadurch
  liefert AltGr die PC-Zeichen (`@ [ ] { } \ ~ | ² ³ µ`). `ShortCut_CheckKeys()` in
  `shortcut.c` bekommt beim Bau einen Aufruf von `KeymapPc_ShortcutModifier()`; die
  **Windows-Taste ersetzt AltGr** für die Hatari-Kürzel (Windows+Taste statt AltGr+Taste).
  Auch die Ausnahme AltGr+M (Maus fangen) entfällt: AltGr+M ergibt `µ`.
- **Atari-Tasten Clr/Home, Help, Undo:** bei „Rollen“ **aus** liegen sie auf **Pos1**,
  **Bild hoch** und **Bild runter**. Bei „Rollen“ **ein** gelten die typischen PC-Funktionen:
  Pos1 = Clr/Home, Bild hoch/runter = Shift+Cursor hoch/runter (seitenweise blättern);
  Help und Undo liegen dann auf **Druck** und **Ende**.
- **Diagnose:** mit `set HATARI_PCKEYS_DEBUG=1` und `hatari.exe --log-file pckeys.log` wird
  für jede Taste protokolliert, was damit geschieht. Bauschritt 5a listet im Protokoll,
  wo Hatari AltGr/Tasten behandelt („Diagnose“).
- Version: About-Dialog „Hatari v2.8“; neue Prüfmarken für die Dateien in Schritt 3c.
- Bau: Python wird jetzt in der Bauumgebung installiert (`mingw-w64-x86_64-python`); die Schritte 5a und 5b
  rufen `python3` bzw. `python` robust auf. Ursache des Abbruchs mit Exit-Code 127 war
  „python3: command not found“ in Schritt 5a (Version 2.8 wurde dadurch erst mit diesem Stand baubar).
- Bau: Schritt 8 (`prn2pdf.exe`) überspringt sich mit Hinweis, wenn `tools/prn2pdf.c` fehlt, statt den
  ganzen Bau abzubrechen. Hatari selbst baut fehlerfrei (Schritt 7 geprüft am Bauprotokoll).

**Hinweis:** Ursache für das Nichtfunktionieren der AltGr-Zeichen in 2.7 war nicht
nachweisbar (ohne die Hatari-Quellen im Zugriff); 2.8 sperrt die AltGr-Kürzel an der
Stelle, an der Hatari sie auswertet, und liefert Diagnosedaten für den Fall, dass es weiter hakt.

---

## Version 2.7 – Update v27: PC-Tastaturbelegung

**Neu**
- Zuschaltbare PC-Tastaturbelegung: Man tippt am PC, der Atari bekommt die Tastenfolge,
  die auf der Atari-Tastatur dasselbe Zeichen erzeugt. Referenz ist der Atari, Zeichen
  ohne Atari-Entsprechung (z. B. `€`) bleiben unbelegt.
- Ein-/Ausschalten mit der Taste **Rollen** (Scroll Lock) oder in der `hatari.cfg`,
  Abschnitt `[Keyboard]`: `bPcLayoutMapping` (Vorgabe aus) und `nPcLayoutSource`
  (0 = Belegung des Betriebssystems unter Windows, sonst Deutsch; 1 = Deutsch fest).
- Ebenen: Shift = Shift, Control = Strg, Alternate = **linke** Alt-Taste; rechtes Alt = AltGr
  für PC-Zeichen (`@ [ ] { } \ ~ |` …).
- Die Atari-Tastentabelle wird aus dem geladenen TOS-Image gelesen (sonst eingebaute
  deutsche Tabelle). Zeichen ohne eigene Atari-Taste kommen über Alternate + Ziffernblock
  (TOS ab 1.04, EmuTOS).
- Hatari-Kürzel: Windows-Taste + Taste, F11, F12, Pause und AltGr-Kombinationen ohne PC-Zeichen
  bleiben bei Hatari. **AltGr+Q ergibt jetzt `@`** (Beenden: Windows-Taste+Q oder Menü);
  AltGr+M (Maus fangen) bleibt erhalten.
- Einzelheiten, Tastenzuordnung und Selbsttest: `doku/Tastatur-PC-Layout.md`.

**Technik / Bau**
- Neu: `src/keymap_pc.c`, `src/includes/keymap_pc.h`, `src/includes/keymap_pc_cfg.h`,
  `tools/patch_keymap.py`.
- Neuer Bauschritt 5a: `patch_keymap.py` ergänzt `keymap.c`, `configuration.h/.c` und
  `src/CMakeLists.txt` in den Hatari-Quellen (bricht mit klarer Meldung ab, wenn eine
  Stelle nicht gefunden wird). `configuration.c/.h` und `CMakeLists.txt` im Repository
  bleiben unverändert.
- Neue Prüfzeilen in Schritt 3c; `doku` und `LIESMICH.md` kommen in den Auslieferungsordner.

**Offen**
- Kontrollkästchen im Setup-Dialog „Keyboard“ (benötigt `dlgKeyboard.c` aus Hatari v2.6.1).
- Funktion noch nicht gegen die echten Hatari-Quellen gebaut und im Atari getestet.

---

## Version 2.6 – Update v26: ESC J / ESC j / ESC 3 bei 24 Nadeln (Druckerengine 2.6)

**ESC J, ESC j und ESC 3 rechneten bei 24-Nadel-Druckern in 1/216 Zoll statt in 1/180 Zoll.**
Epson/NEC/Star 24-Nadel: n/180 Zoll; nur 9-Nadel-Drucker (FX-80, NLQ401): n/216 Zoll.
TeX druckt die Seite als Bänder aus je 24 Punktzeilen (ESC * 39) und schiebt mit ESC J 24
genau ein Band weiter. Mit 1/216 überlappten die Bänder um 4 Punktzeilen (Zeilen
„gequetscht“), nur Zeilen mit ESC J 30 waren in Ordnung – das ergab die unregelmäßigen
Verzerrungen (NEC P6, TeX-Ausdrucke). Neue Hilfsfunktion `vunit()` in `escp.c`.

- Geändert: `src/pe/src/drivers/escp.c`, `src/pe/src/pdf.c` (Marker „pe printer engine 2.6“),
  `build.yml` (Marker `vunit`, About: Hatari v2.7.2.6).
- `WinApe/escp_vunit.patch`: derselbe Eingriff als Patch für das WinApe-Projekt
  (nur `escp.c`; Amstrad-CPC-Projekt, nutzt dieselbe Engine).

---

## Version 2.5 – Update v25: NLQ401-Draft, Endlospapier (Druckerengine 2.5)

1. **NLQ401, Draft-Schrift: Null (und alle runden Zeichen) korrekt.**
   Im Zeichensatzbild des Druckers (`fonts/src/NLQ401_Charset.png`) liegen die Draft-Punkte
   auf einem Raster von einer halben Spalte (4 Pixel). `tools/pngfont.py` hat sie bisher auf
   ganze Spalten (7,86 Pixel) gerundet; die Eckpunkte der Rundungen rutschten auf ihre
   Nachbarn, die 0 sah offen aus wie „( )“. Der Generator rechnet jetzt auf dem
   Halbspaltenraster (`fit_lattice`); die Schrift hat 14 statt 7 Spalten je Zeichenzelle und
   stimmt Punkt für Punkt mit dem Original überein (die 0 ist ein geschlossenes Achteck).
   Zeilenbreiten ändern sich in allen Modi um höchstens 0,3 %. Die NLQ-Schrift ist nicht
   betroffen (bitgleich wie zuvor).
2. **Endlospapier: NLQ401-Schrift 1 mm tiefer.**
   Neues Feld `fanfold_dy` in `pe_driver` (Einheit 1/2160 Zoll, 85 = 1 mm) und `text_dy` in
   `pe_config`. Bei Endlospapier (grün-weiß und weiß) setzt die Engine die Punkte um diesen
   Betrag tiefer; Zeilenvorschub, Seitenlänge und Lesestreifen bleiben auf ihrem Raster.
   Eingetragen ist 85 für beide NLQ401-Treiber (Intl und DE). Andere Drucker unverändert
   (beim 24-Nadel-Block würde 1 mm die Unterlängen auf die untere Streifenkante setzen);
   bei Bedarf `.fanfold_dy = <Einheiten>` im Treiber eintragen.
3. `src/pe/src/pdf.c`: Version 2.5 (2026-11-10), Makro `PE_CREATOR`
   (Standard „Hatari PDF printer emulation“), gemeinsamer Stand mit dem WinApe-Projekt.
4. `build.yml`: Reparaturschritt 3c suchte in `pdf.c` den Marker `pe printer engine 1.8`;
   jetzt `pe printer engine 2.5`. Neue Prüfmarker: `text_dy`, `fanfold_dy`, `9, 13, 14, 6`
   (neue Draft-Schrift), `fit_lattice`.

Geprüft im WinApe-Projekt (dieselbe Engine): für alle 8 Drucker × 3 Papiersorten stimmen die
Seiteninhalte mit der Referenzausgabe überein. Hatari selbst wurde nicht gebaut.

---

## Version 2.4 und früher – Ausgangsstand (update-v24.zip)

Für diese Stände liegt mir keine eigene LIESMICH vor. Der Stand 2.4 (About: Hatari v2.7.2.4)
enthielt den PDF-Drucker für Hatari 2.6.1 mit der Druckerengine unter `src/pe`:
- Treiber: NEC P6, Epson LQ, LQ-800, FX-80, Panasonic KX-P2023, Star LC24-10, Schneider NLQ401
  (International und Deutsch als eigene Treiber)
- Papierarten Einzelblatt und Endlos (grün-weiß, weiß), Druckgeräusch (`printer_sound*.c`),
  Fertig-Meldung, Einstellungen im Dialog „Devices“ (`dlgDevice.c`)
- Atari-Druckertreiber-Dateien in `Atari-Treiber/`: NLQ401_D_v3.cfg, NLQ401_INT_v3.cfg,
  PRINTER_Star_Einzelblatt.CFG, PRINTER_Star_Endlos.CFG
- Hilfswerkzeuge `tools/pngfont.py`, `tools/binfont.py`, `tools/prn2pdf.c` (eigenständiges
  Programm zum Umwandeln von .prn-Dateien in PDF)

Weitere Einzelheiten früherer Versionen (v1 bis v23) sind hier nicht bekannt; bitte bei
Bedarf aus den alten LIESMICH-Dateien ergänzen.
