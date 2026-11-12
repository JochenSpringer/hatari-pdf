# Hatari – Update v25 (Druckerengine 2.5)

Enthält nur die geänderten Dateien, im selben Aufbau wie `update-v24.zip` (Ordner `src`, `tools`, Datei `build.yml`).
In die Hauptebene des Repositorys hochladen.

## Änderungen

1. **NLQ401, Draft-Schrift: Null (und alle runden Zeichen) korrekt**
   Im Zeichensatzbild des Druckers (`fonts/src/NLQ401_Charset.png`) liegen die Draft-Punkte auf einem Raster von einer
   halben Spalte (4 Pixel). `tools/pngfont.py` hat sie bisher auf ganze Spalten (7,86 Pixel) gerundet; die Eckpunkte der
   Rundungen rutschten dadurch auf ihre Nachbarn, die 0 sah offen aus wie „( )“. Der Generator rechnet jetzt auf dem
   Halbspaltenraster (`fit_lattice`); die Schrift hat 14 statt 7 Spalten je Zeichenzelle und stimmt Punkt für Punkt mit
   dem Original überein (die 0 ist ein geschlossenes Achteck, je zwei Punkte oben und unten). Zeilenbreiten ändern sich
   in allen Modi (normal, schmal, 12/15 cpi, breit, fett, proportional) um höchstens 0,3 %. Die NLQ-Schrift ist nicht
   betroffen (ihr Raster ist fein genug); der Generator erzeugt sie bitgleich wie zuvor.
2. **Endlospapier: NLQ401-Schrift 1 mm tiefer**
   Neues Feld `fanfold_dy` in `pe_driver` (Einheit 1/2160 Zoll, 85 = 1 mm) und `text_dy` in `pe_config`. Bei Endlospapier
   (grün-weiß und weiß) setzt die Engine die Punkte um diesen Betrag tiefer; Zeilenvorschub, Seitenlänge und Lesestreifen
   bleiben auf ihrem Raster. Eingetragen ist 85 für beide NLQ401-Treiber (Intl und DE). Alle anderen Drucker bleiben
   unverändert: Beim 24-Nadel-Block (P6, LQ …) würde 1 mm die Unterlängen auf die untere Streifenkante setzen.
   Bei Bedarf trägst du für einen anderen Drucker einfach `.fanfold_dy = <Einheiten>` im Treiber ein.
3. `src/pe/src/pdf.c`: Version 2.5 (2026-11-10); enthält das Makro `PE_CREATOR` (Standard „Hatari PDF printer emulation“),
   Stand der gemeinsamen Engine mit dem WinApe-Projekt.
4. `build.yml`: Der Reparaturschritt 3c suchte in `pdf.c` den Marker `pe printer engine 1.8`; die Datei trägt aber
   2.x, der Schritt konnte also „FEHLT“ melden. Der Marker lautet jetzt `pe printer engine 2.5`.
   Neue Prüfmarker: `text_dy`, `fanfold_dy`, `9, 13, 14, 6` (neue Draft-Schrift), `fit_lattice`.

## Geprüft

Dieselbe Engine läuft im WinApe-Projekt; dort stimmen für alle 8 Drucker × 3 Papiersorten die Seiteninhalte der
freistehenden Variante mit der Referenzausgabe überein. Hatari selbst konnte ich nicht bauen.
