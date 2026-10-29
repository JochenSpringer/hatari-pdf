# Druckeremulationen (Beta 1)

Auswahl in Hatari: Optionen > Devices > PDF setup > Printer model (Pfeiltasten).

| Modell | Nadeln | Schriften | Hinweis |
| --- | --- | --- | --- |
| NEC Pinwriter P6 | 24 | Ersatzschriften (LQMATRIX) | wie bisher, inkl. NEC `FS E` |
| Epson LQ | 24 | wie P6 | Stand der vorletzten Version, ohne NEC-Befehle |
| Epson LQ-800 | 24 | Draft, Roman, Sans serif | ROM-Dumps |
| Epson FX-80 | 9 | Draft (Courier) | ROM-Dump |
| Panasonic KX-P2023 | 24 | Draft, Courier, Prestige, Bold PS, Script | ROM-Dumps |
| Star LC24-10 | 24 | Draft (fett/kursiv), Courier, Prestige (kursiv), Script, Orator | ROM-Dumps |
| Schneider NLQ401 | 9 | Draft, NLQ (2 Durchgaenge) | ROM-Dumps |

## Was ich an den Font-Dateien feststellen musste
* Alle Dumps enthalten nur ASCII 0x21-0x7E. Umlaute und Akzente zeichnet der
  Emulator aus Buchstabe + Punkten selbst (ae oe ue Ae Oe Ue, Akzente, ss als B).
* Bei LQ800 Roman/Sans, KX-P2023 (4 LQ-Fonts), FX-80, NLQ401 (Draft + NLQ) sind
  alle Zeichen auf dieselbe Hoehe gestreckt (Punkt, Apostroph, Kleinbuchstaben
  so hoch wie Grossbuchstaben). `tools/binfont.py` verteilt sie wieder auf
  Grossbuchstaben-, x-Hoehe-, Ober-/Unterlaengen-Baender. Das ist eine Naeherung.
* FX-80-Dump: mehrere Zeichen sind in der Datei selbst defekt (z. B. T ohne
  Stiel, S, N). Das kann der Emulator nicht ausgleichen.
* LC24 Script und LC24 Draft-Italic sind byte-identisch (Script = kursive Draft).
* ESC-k-Nummern fuer KX-P2023 und LC24-10 sind Annahmen (Epson-Numerierung);
  eine Tabelle pro Treiber in `drivers/kxp2023.c` bzw. `drivers/lc24.c`.
Fonts neu erzeugen: `python3 tools/binfont.py` (liest fonts/rom/*.BIN).
