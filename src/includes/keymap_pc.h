/*
  Hatari - keymap_pc.h  (hATARI 2.8)

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  PC-Tastaturbelegung: Zeichen der PC-Tastatur werden in die Tastenfolge
  umgesetzt, die dasselbe Zeichen auf der Atari-Tastatur erzeugt.
*/
#ifndef HATARI_KEYMAP_PC_H
#define HATARI_KEYMAP_PC_H

#include <SDL.h>
#include <stdbool.h>

/* Rueckgabe true = Taste wurde vollstaendig behandelt, Hatari-Standard
 * (keymap.c) darf sie nicht mehr verarbeiten. */
extern bool KeymapPc_KeyDown(const SDL_Keysym *keysym);
extern bool KeymapPc_KeyUp(const SDL_Keysym *keysym);

/* Fuer ShortCut_CheckKeys(): AltGr-Kuerzel abschalten, Windows-Taste als Ersatz */
extern int KeymapPc_ShortcutModifier(int modkey);

#endif
