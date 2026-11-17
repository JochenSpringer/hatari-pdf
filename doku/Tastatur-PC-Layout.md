# PC-Tastaturbelegung für den Atari (hATARI 2.8)

Zuschaltbare Funktion: Man tippt am PC, und der Atari bekommt genau die Tastenfolge, die auf
der Atari-Tastatur das gleiche Zeichen erzeugt. Referenz ist der Atari – Zeichen, die es dort
nicht gibt, bleiben unbelegt.

**Ein-/Ausschalten:** Taste **Rollen** (Scroll Lock). Vorgabe beim Start: aus
(`hatari.cfg`, Abschnitt `[Keyboard]`: `bPcLayoutMapping`; `nPcLayoutSource` = 0 Betriebssystem
bzw. Deutsch, 1 Deutsch fest).

**Wirkungsweise:** Aus der gedrückten PC-Taste samt Shift, AltGr und Feststelltaste wird das
Zeichen bestimmt (unter Windows laut eingestellter Tastaturbelegung). In der Atari-Tastentabelle
des geladenen TOS-Images (sonst eingebaute deutsche Tabelle) wird die Taste gesucht, die dieses
Zeichen erzeugt; Shift wird bei Bedarf automatisch gesetzt. Zeichen ohne eigene Atari-Taste
kommen über Alternate + Ziffernblock.

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

### Bekannte Grenzen

* Totzeichen erscheinen sofort, nicht erst nach dem nächsten Zeichen.
* Mit Feststelltaste und gedrückter Shift-Taste erscheinen Großbuchstaben (wie am Atari).
* Enthält ein EmuTOS-Image die Tabellen vieler Länder, wird die deutsche bevorzugt.
* Beim Loslassen von AltGr kann der Atari kurz „Control“ sehen (Windows meldet AltGr als
  Strg+Alt). Ohne Folgen.
* Spiele, die die Tastatur direkt abfragen, sehen bei Zeichen über Alternate + Ziffernblock nur
  Ziffernblock-Tasten.

### Selbsttest (Textfeld im Atari)

1. Rollen drücken, „PC-Tastaturbelegung EIN“ abwarten.
2. Tippen: `abcxyz äöüß ÄÖÜ 1234567890 !"§$%&/()=? +*~#'<>|^,.-;:_`
3. Dann `@ [ ] { } \ ° ² ³ µ` (AltGr-Zeichen).
4. Pos1 / Druck / Ende / Bild hoch / Bild runter ausprobieren.
5. Stimmt ein Zeichen nicht: PC-Taste, erwartetes und erhaltenes Zeichen und TOS-Version notieren
   (und, wenn möglich, `pckeys.log` aus der Diagnose mitschicken).

### Technik

* `src/keymap_pc.c`, `src/includes/keymap_pc.h`: eigentliche Funktion
* `tools/patch_keymap.py` (Bauschritt 5a): hängt `KeymapPc_KeyDown/Up` an den Anfang von
  `Keymap_KeyDown/Up` (keymap.c), `KeymapPc_ShortcutModifier` an den Anfang von
  `ShortCut_CheckKeys` (shortcut.c), ergänzt `configuration.h/.c` und `src/CMakeLists.txt`
  und schreibt `src/includes/keymap_pc_cfg.h`.
