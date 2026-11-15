# PC-Tastaturbelegung für den Atari (hATARI 2.7.2.7)

Zuschaltbare Funktion: Man tippt am PC, und der Atari bekommt genau die
Tastenfolge, die auf der Atari-Tastatur das gleiche Zeichen erzeugt.
Referenz ist der Atari – Zeichen, die es dort nicht gibt, bleiben unbelegt.

## Ein- und Ausschalten

| Weg | Wirkung |
|---|---|
| Taste **Rollen** (Scroll Lock) | schaltet im laufenden Betrieb um, Meldung in der Statusleiste und im Protokoll |
| `hatari.cfg`, Abschnitt `[Keyboard]`: `bPcLayoutMapping = TRUE` | Funktion beim Start aktiv |
| `hatari.cfg`: `nPcLayoutSource = 0` | Zeichen der PC-Tastatur laut Betriebssystem (Windows), sonst Deutsch |
| `hatari.cfg`: `nPcLayoutSource = 1` | immer die eingebaute deutsche PC-Belegung |

Vorgabe: **aus**. Dann verhält sich hATARI wie bisher. Die Einstellung wird
mit „Save config“ bzw. beim Speichern der Konfiguration gesichert.
Ein Kontrollkästchen im Setup-Dialog „Keyboard“ gibt es noch nicht (siehe
„Offen“).

## Was passiert beim Tippen

1. Aus der gedrückten PC-Taste samt Shift, AltGr und Feststelltaste wird das
   Zeichen bestimmt (unter Windows laut eingestellter Tastaturbelegung, sonst
   deutsch).
2. In der Atari-Tastentabelle wird die Taste gesucht, die dieses Zeichen
   erzeugt. Shift wird bei Bedarf automatisch gedrückt oder losgelassen,
   unabhängig vom PC. Beispiel: `#` ist am PC eine eigene Taste, am Atari
   (deutsch) liegt es an einer anderen Stelle – es kommt trotzdem `#` an.
3. Die Atari-Tastentabelle wird aus dem **geladenen TOS-Image** gelesen
   (TOS 1.x/2.x/3.x/4.x, EmuTOS). Findet sich dort keine Tabelle, gilt eine
   eingebaute Tabelle der deutschen Atari-Tastatur. Im Protokoll steht, welche
   Quelle benutzt wurde („PC-Tastaturbelegung: Atari-Tastentabelle aus …“).
4. Zeichen, die der Atari-Zeichensatz kennt, aber keine eigene Taste haben,
   werden mit der TOS-Eingabe **Alternate + Ziffernblock** erzeugt (ASCII-Code
   auf dem Ziffernblock tippen, Alternate loslassen). Das betrifft am
   deutschen PC vor allem `@ [ ] { } \ ° ² ³ ´`. Voraussetzung: TOS 1.04 oder
   neuer (EmuTOS auch). In Programmen, die die Tastatur direkt abfragen
   (viele Spiele), kommen dabei nur die Ziffernblock-Tasten an.
5. Zeichen, die der Atari nicht kennt (z. B. `€`), bleiben unbelegt.

## Die vier Ebenen des Atari

| Ebene | am PC | Hinweis |
|---|---|---|
| 1 normal | Taste | |
| 2 Shift | Shift + Taste | Shift ist für den Atari bereits im Zeichen enthalten |
| 3 Control | Strg + Taste | Strg und Taste werden an den Atari durchgereicht |
| 4 Alternate | **linke** Alt-Taste + Taste | Rechtes Alt = AltGr, wird für PC-Zeichen gebraucht |

Control und Alternate gelten für das Zeichen der PC-Taste, also z. B. Strg+C
= Control + Atari-Taste „C“, Alt+X = Alternate + Atari-Taste „X“.

## Tasten ohne Zeichen

| PC | Atari |
|---|---|
| Esc, Rück, Tab, Enter | Esc, Backspace, Tab, Return |
| Einfg, Entf, Pos1 | Insert, Delete, Clr/Home |
| Bild↑ | Help |
| Bild↓ | Undo |
| Pfeiltasten | Pfeiltasten |
| F1 – F10 | F1 – F10 (Shift + F1…F10 = F11…F20 des Atari) |
| Ziffernblock 0–9 , . + − * / Enter | Ziffernblock des Atari (immer als Ziffern, Num-Lock egal) |
| Num | Ziffernblock `(` |
| Menütaste (neben rechter Strg) | Ziffernblock `)` |
| Feststelltaste | wie bisher (Caps Lock des Atari) |
| Rollen | Umschalter dieser Funktion |

Unbelegt bleiben u. a. Ende, Druck, Pause, Windows-Tasten (siehe unten).

## Hatari-Tastenkombinationen

* **Windows-Taste + Taste** und **F11/F12/Pause** gehen weiter an Hatari
  (Vollbild, Menü, Pause …).
* **AltGr + Taste** geht an Hatari, wenn die Kombination am PC kein Zeichen
  erzeugt (z. B. AltGr+F, AltGr+C).
* **AltGr + M** (Maus fangen) bleibt bewusst erhalten; das `µ` ist deshalb nicht
  über AltGr+M erreichbar.
* Achtung: **AltGr + Q** ergibt jetzt `@`. „Beenden“ also über Windows-Taste+Q
  oder das Menü.
* Tasten, die für die Joystick-Emulation per Tastatur eingestellt sind, gehen
  weiter an Hatari.

## Bekannte Grenzen

* Totzeichen (`^`, `´`, `` ` ``) erscheinen sofort, nicht erst nach dem
  nächsten Zeichen.
* Mit Feststelltaste und gedrückter Shift-Taste erscheinen Großbuchstaben
  (so verhält sich auch der Atari).
* Enthält ein EmuTOS-Image die Tabellen vieler Länder, wird die deutsche
  Tabelle bevorzugt.
* Beim Loslassen von AltGr kann der Atari kurz „Control“ sehen (Windows
  meldet AltGr als Strg+Alt). Ohne Folgen.

## Selbsttest (Textfeld des Desktops, z. B. in einem Editor des Atari)

1. Rollen drücken, Meldung „PC-Tastaturbelegung EIN“ abwarten.
2. Zeile tippen: `abcxyz äöüß ÄÖÜ 1234567890 !"§$%&/()=? +*~#'<>|^,.-;:_`
3. Dann `@ [ ] { } \ ° ² ³` (AltGr-Zeichen).
4. Stimmt ein Zeichen nicht, bitte notieren, welche PC-Taste welches Zeichen
   am Atari ergibt, und die TOS-Version nennen.

## Offen

* Kontrollkästchen im Setup-Dialog „Keyboard“ (benötigt `dlgKeyboard.c`).
* Weitere PC-Layouts ohne Windows (unter Windows wird die eingestellte
  Belegung des Betriebssystems benutzt).

## Technik

* `src/keymap_pc.c`, `src/includes/keymap_pc.h`: eigentliche Funktion
* `tools/patch_keymap.py`: hängt beim Bau `KeymapPc_KeyDown/Up` an den Anfang
  von `Keymap_KeyDown/Up` in `keymap.c` und schreibt
  `src/includes/keymap_pc_cfg.h` (TOS-Image-Auslesen nur, wenn die Hatari-Quellen
  die nötigen Funktionen bieten)
* `src/configuration.c/.h`: neue Einstellungen `bPcLayoutMapping`, `nPcLayoutSource`
