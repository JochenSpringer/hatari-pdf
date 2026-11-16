/*
  Hatari - keymap_pc.c  (hATARI 2.7.2.5)

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  PC-Tastaturbelegung
  -------------------
  Zuschaltbare Funktion (ConfigureParams.Keyboard.bPcLayoutMapping, Umschalten
  im laufenden Betrieb mit der Taste "Rollen"/Scroll Lock).

  Ist sie aktiv, wird fuer jede Zeichentaste der PC-Tastatur das erzeugte
  Zeichen bestimmt (Shift, AltGr, Feststelltaste beruecksichtigt) und die
  Tastenfolge an den Atari gesendet, die auf dem Atari genau dieses Zeichen
  erzeugt (Atari-Taste plus ggf. Shift).  Referenz ist der Atari: Zeichen, die
  es auf dem Atari nicht gibt, bleiben unbelegt.  Ctrl und linke Alt-Taste
  werden als Control und Alternate des Atari durchgereicht.  Zeichen ohne
  eigene Atari-Taste werden ueber die TOS-Eingabe "Alternate + Ziffernblock"
  erzeugt.

  Die Atari-Tastentabelle wird (wenn moeglich) aus dem geladenen TOS-Image
  gelesen, sonst gilt eine eingebaute Tabelle fuer die deutsche Atari-Tastatur.
*/
const char KeymapPc_fileid[] = "Hatari keymap_pc.c";

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "configuration.h"
#include "ikbd.h"
#include "log.h"
#include "keymap_pc.h"
#include "keymap_pc_cfg.h"

#if KEYMAPPC_HAVE_ROMSCAN
#include "stMemory.h"
#include "tos.h"
#endif
#if KEYMAPPC_HAVE_STATUSBAR
#include "statusbar.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

/* ------------------------------------------------------------------ */
/* Atari-Scancodes der Umschalttasten                                 */
/* ------------------------------------------------------------------ */
#define ST_SC_CTRL   0x1D
#define ST_SC_SHIFT  0x2A
#define ST_SC_ALT    0x38

#define M_SHIFT 1
#define M_CTRL  2
#define M_ALT   4

#define PC_NUMSC  512     /* >= SDL_NUM_SCANCODES */

/* ------------------------------------------------------------------ */
/* Zustand                                                            */
/* ------------------------------------------------------------------ */
static struct
{
	bool lshift, rshift, lctrl, rctrl, lalt, ralt, lgui, rgui;
} pc;

static uint8_t atariMods;            /* Umschalttasten, die wir am Atari gedrueckt halten */
static bool wasEnabled;              /* letzter bekannter Schaltzustand */
static bool toggleHeld;              /* Rollen-Taste wurde von uns verbraucht */

static struct
{
	bool down;      /* wir haben diese PC-Taste behandelt (Taste ist unten) */
	bool valid;     /* ... und dabei einen Atari-Scancode gesendet */
	bool passed;    /* ... wir haben sie an keymap.c weitergegeben */
	uint8_t st;
} held[PC_NUMSC];

/* ------------------------------------------------------------------ */
/* Atari-Zeichensatz (obere Haelfte) -> Unicode, nur gesicherte Werte */
/* ------------------------------------------------------------------ */
static const uint16_t atari_hi[128] =
{
	[0x00] = 0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
	0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
	0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
	0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x00DF, 0x0192,
	0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
	0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
	0x00E3, 0x00F5, 0x00D8, 0x00F8, 0x0153, 0x0152, 0x00C0, 0x00C3,
	0x00D5, 0x00A8, 0x00B4, 0x2020, 0x00B6, 0x00A9, 0x00AE, 0x2122,
	[0x5D] = 0x00A7,   /* 0xDD */
	[0x66] = 0x00B5,   /* 0xE6 */
	[0x71] = 0x00B1,   /* 0xF1 */
	[0x76] = 0x00F7,   /* 0xF6 */
	[0x78] = 0x00B0,   /* 0xF8 */
	[0x7D] = 0x00B2,   /* 0xFD */
	[0x7E] = 0x00B3    /* 0xFE */
};

static uint32_t AtariByteToUni(uint8_t b)
{
	if (b < 0x80)
		return b;
	return atari_hi[b - 0x80];
}

/* Unicode -> Atari-Zeichencode (0 = gibt es nicht) */
static int UniToAtariByte(uint32_t cp)
{
	int i;
	if (cp >= 0x20 && cp < 0x7F)
		return (int)cp;
	for (i = 0; i < 128; i++)
		if (atari_hi[i] == cp && cp != 0)
			return 0x80 + i;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Atari-Tastentabelle: Zeichen -> (Scancode, Shift)                  */
/* ------------------------------------------------------------------ */
typedef struct { uint32_t cp; uint8_t sc; uint8_t shift; } DirectKey;

#define MAX_DIRECT 256
static DirectKey direct[MAX_DIRECT];
static int nDirect;
static const char *tableSource = "-";
static bool tableReady;
static uint32_t tableFingerprint;

/* Eingebaute Tabelle der deutschen Atari-Tastatur (Format wie im TOS:
 * je 128 Bytes, Index = Atari-Scancode, Atari-Zeichensatz) */
static uint8_t de_unshift[128];
static uint8_t de_shift[128];

static void BuildGermanFallback(void)
{
	static const char *lowerLetters = "qwertzuiop";   /* 0x10.. */
	static const char *homeLetters  = "asdfghjkl";    /* 0x1E.. */
	static const char *botLetters   = "yxcvbnm";      /* 0x2C.. */
	int i;

	memset(de_unshift, 0, sizeof(de_unshift));
	memset(de_shift, 0, sizeof(de_shift));
	de_unshift[0x01] = de_shift[0x01] = 0x1B;
	de_unshift[0x0E] = de_shift[0x0E] = 0x08;
	de_unshift[0x0F] = de_shift[0x0F] = 0x09;
	de_unshift[0x1C] = de_shift[0x1C] = 0x0D;
	de_unshift[0x39] = de_shift[0x39] = 0x20;

	for (i = 0; i < 9; i++)
		de_unshift[0x02 + i] = (uint8_t)('1' + i);
	de_unshift[0x0B] = '0';
	{
		static const char shifted[] = "!\"\xDD$%&/()=";   /* 0xDD = Paragraph */
		for (i = 0; i < 10; i++)
			de_shift[0x02 + i] = (uint8_t)shifted[i];
	}
	de_unshift[0x0C] = 0x9E;  de_shift[0x0C] = '?';          /* ss */
	de_unshift[0x0D] = '\'';  de_shift[0x0D] = '`';

	for (i = 0; i < 10; i++)
	{
		de_unshift[0x10 + i] = (uint8_t)lowerLetters[i];
		de_shift[0x10 + i]   = (uint8_t)(lowerLetters[i] - 32);
	}
	de_unshift[0x1A] = 0x81;  de_shift[0x1A] = 0x9A;         /* ue, Ue */
	de_unshift[0x1B] = '+';   de_shift[0x1B] = '*';
	for (i = 0; i < 9; i++)
	{
		de_unshift[0x1E + i] = (uint8_t)homeLetters[i];
		de_shift[0x1E + i]   = (uint8_t)(homeLetters[i] - 32);
	}
	de_unshift[0x27] = 0x94;  de_shift[0x27] = 0x99;         /* oe, Oe */
	de_unshift[0x28] = 0x84;  de_shift[0x28] = 0x8E;         /* ae, Ae */
	de_unshift[0x29] = '#';   de_shift[0x29] = '^';
	de_unshift[0x2B] = '~';   de_shift[0x2B] = '|';
	for (i = 0; i < 7; i++)
	{
		de_unshift[0x2C + i] = (uint8_t)botLetters[i];
		de_shift[0x2C + i]   = (uint8_t)(botLetters[i] - 32);
	}
	de_unshift[0x33] = ',';   de_shift[0x33] = ';';
	de_unshift[0x34] = '.';   de_shift[0x34] = ':';
	de_unshift[0x35] = '-';   de_shift[0x35] = '_';
	de_unshift[0x60] = '<';   de_shift[0x60] = '>';
}

static void AddDirect(uint32_t cp, int sc, int shift)
{
	int i;
	if (cp < 0x20 || cp == 0x7F)
		return;
	for (i = 0; i < nDirect; i++)
		if (direct[i].cp == cp)
			return;                     /* erster Treffer gewinnt */
	if (nDirect >= MAX_DIRECT)
		return;
	direct[nDirect].cp = cp;
	direct[nDirect].sc = (uint8_t)sc;
	direct[nDirect].shift = (uint8_t)shift;
	nDirect++;
}

static void BuildFromTables(const uint8_t *un, const uint8_t *sh)
{
	int sc;
	nDirect = 0;
	/* Hauptblock bis einschl. ISO-Taste 0x60; Ziffernblock bleibt aussen vor */
	for (sc = 0x01; sc <= 0x60; sc++)
		AddDirect(AtariByteToUni(un[sc]), sc, 0);
	for (sc = 0x01; sc <= 0x60; sc++)
		AddDirect(AtariByteToUni(sh[sc]), sc, 1);
}

#if KEYMAPPC_HAVE_ROMSCAN
/* Pruefsumme ueber verstreute ROM-Bytes: erkennt TOS-Wechsel */
static uint32_t RomFingerprint(void)
{
	uint32_t fp, i, step;
	if (TosSize == 0)
		return 0;
	fp = (uint32_t)TosAddress * 31u + (uint32_t)TosSize;
	step = TosSize / 251u;
	if (step == 0)
		step = 1;
	for (i = 0; i < TosSize; i += step)
		fp = fp * 16777619u ^ STMemory_ReadByte((uint32_t)(TosAddress + i));
	return fp;
}

static int CountLetters(const uint8_t *t, bool upper)
{
	static const uint8_t lo[3] = { 0x10, 0x1E, 0x2C };
	static const uint8_t n[3]  = { 10, 9, 7 };
	int a, i, cnt = 0;
	for (a = 0; a < 3; a++)
		for (i = 0; i < n[a]; i++)
		{
			uint8_t c = t[lo[a] + i];
			if (upper ? (c >= 'A' && c <= 'Z') : (c >= 'a' && c <= 'z'))
				cnt++;
		}
	return cnt;
}

static bool IsKeytab(const uint8_t *t, bool shifted)
{
	if (t[0] != 0 || t[1] != 0x1B || t[0x0E] != 0x08 || t[0x0F] != 0x09 ||
	    t[0x1C] != 0x0D || t[0x39] != 0x20)
		return false;
	return CountLetters(t, shifted) >= 22;
}

static bool IsGerman(const uint8_t *un)
{
	return un[0x15] == 'z' && un[0x2C] == 'y' && un[0x28] == 0x84 &&
	       un[0x27] == 0x94 && un[0x1A] == 0x81 && un[0x0C] == 0x9E;
}

/* Sucht im TOS-Image Tabellenpaare (normal + Shift, je 128 Bytes direkt
 * hintereinander). Mehrere Paare (z.B. EmuTOS mit allen Laendern): das
 * deutsche wird bevorzugt, sonst das erste. */
static bool ScanRom(void)
{
	uint8_t *rom;
	uint32_t i, size = TosSize;
	long first = -1, german = -1;

	if (size < 1024 || size > 0x100000)
		return false;
	rom = (uint8_t *)malloc(size);
	if (!rom)
		return false;
	for (i = 0; i < size; i++)
		rom[i] = STMemory_ReadByte((uint32_t)(TosAddress + i));

	for (i = 0; i + 256 <= size; i++)
	{
		if (rom[i + 1] != 0x1B)
			continue;
		if (!IsKeytab(rom + i, false) || !IsKeytab(rom + i + 128, true))
			continue;
		if (first < 0)
			first = (long)i;
		if (german < 0 && IsGerman(rom + i))
			german = (long)i;
	}
	if (first >= 0)
	{
		long use = (german >= 0) ? german : first;
		BuildFromTables(rom + use, rom + use + 128);
		tableSource = (german >= 0) ? "TOS-Image (deutsche Tabelle)" : "TOS-Image";
		free(rom);
		return true;
	}
	free(rom);
	return false;
}
#endif /* KEYMAPPC_HAVE_ROMSCAN */

static void EnsureTable(void)
{
#if KEYMAPPC_HAVE_ROMSCAN
	uint32_t fp = RomFingerprint();
	if (tableReady && fp == tableFingerprint)
		return;
	tableFingerprint = fp;
	tableReady = true;
	if (ScanRom())
	{
		Log_Printf(LOG_INFO, "PC-Tastaturbelegung: Atari-Tastentabelle aus %s, %d Zeichen\n",
		           tableSource, nDirect);
		return;
	}
#else
	if (tableReady)
		return;
	tableReady = true;
#endif
	BuildGermanFallback();
	BuildFromTables(de_unshift, de_shift);
	tableSource = "eingebaute deutsche Tabelle";
	Log_Printf(LOG_INFO, "PC-Tastaturbelegung: Atari-Tastentabelle aus %s, %d Zeichen\n",
	           tableSource, nDirect);
}

/* Ergebnis der Suche nach einem Zeichen auf dem Atari */
typedef enum { A_NONE, A_DIRECT, A_NUMPAD } AKind;
typedef struct { AKind kind; uint8_t sc; uint8_t shift; int code; } AtariKey;

static AtariKey LookupAtari(uint32_t cp)
{
	AtariKey r;
	int i, code;
	memset(&r, 0, sizeof(r));
	for (i = 0; i < nDirect; i++)
		if (direct[i].cp == cp)
		{
			r.kind = A_DIRECT;
			r.sc = direct[i].sc;
			r.shift = direct[i].shift;
			return r;
		}
	code = UniToAtariByte(cp);
	if (code >= 1)
	{
		r.kind = A_NUMPAD;
		r.code = code;
	}
	return r;
}

/* ------------------------------------------------------------------ */
/* PC-Seite: Zeichen einer Taste                                      */
/* ------------------------------------------------------------------ */
static uint16_t pcde[PC_NUMSC][3];    /* normal, Shift, AltGr */
static bool pcdeInit;

static bool IsLetterCp(uint32_t c)
{
	return (c >= 'a' && c <= 'z') || c == 0xE4 || c == 0xF6 || c == 0xFC;
}

static void InitGermanPc(void)
{
	int i;
	if (pcdeInit)
		return;
	pcdeInit = true;
	memset(pcde, 0, sizeof(pcde));
	for (i = 0; i < 26; i++)
	{
		pcde[SDL_SCANCODE_A + i][0] = (uint16_t)('a' + i);
		pcde[SDL_SCANCODE_A + i][1] = (uint16_t)('A' + i);
	}
	/* Deutsche PC-Tastatur: Y und Z vertauscht */
	pcde[SDL_SCANCODE_Y][0] = 'z';  pcde[SDL_SCANCODE_Y][1] = 'Z';
	pcde[SDL_SCANCODE_Z][0] = 'y';  pcde[SDL_SCANCODE_Z][1] = 'Y';
	pcde[SDL_SCANCODE_Q][2] = '@';
	pcde[SDL_SCANCODE_E][2] = 0x20AC;
	pcde[SDL_SCANCODE_M][2] = 0x00B5;

	for (i = 0; i < 9; i++)
	{
		pcde[SDL_SCANCODE_1 + i][0] = (uint16_t)('1' + i);
	}
	pcde[SDL_SCANCODE_0][0] = '0';
	pcde[SDL_SCANCODE_1][1] = '!';
	pcde[SDL_SCANCODE_2][1] = '"';   pcde[SDL_SCANCODE_2][2] = 0x00B2;
	pcde[SDL_SCANCODE_3][1] = 0x00A7; pcde[SDL_SCANCODE_3][2] = 0x00B3;
	pcde[SDL_SCANCODE_4][1] = '$';
	pcde[SDL_SCANCODE_5][1] = '%';
	pcde[SDL_SCANCODE_6][1] = '&';
	pcde[SDL_SCANCODE_7][1] = '/';   pcde[SDL_SCANCODE_7][2] = '{';
	pcde[SDL_SCANCODE_8][1] = '(';   pcde[SDL_SCANCODE_8][2] = '[';
	pcde[SDL_SCANCODE_9][1] = ')';   pcde[SDL_SCANCODE_9][2] = ']';
	pcde[SDL_SCANCODE_0][1] = '=';   pcde[SDL_SCANCODE_0][2] = '}';

	pcde[SDL_SCANCODE_GRAVE][0] = '^';  pcde[SDL_SCANCODE_GRAVE][1] = 0x00B0;
	pcde[SDL_SCANCODE_MINUS][0] = 0x00DF; pcde[SDL_SCANCODE_MINUS][1] = '?';
	pcde[SDL_SCANCODE_MINUS][2] = '\\';
	pcde[SDL_SCANCODE_EQUALS][0] = 0x00B4; pcde[SDL_SCANCODE_EQUALS][1] = '`';
	pcde[SDL_SCANCODE_LEFTBRACKET][0] = 0x00FC; pcde[SDL_SCANCODE_LEFTBRACKET][1] = 0x00DC;
	pcde[SDL_SCANCODE_RIGHTBRACKET][0] = '+'; pcde[SDL_SCANCODE_RIGHTBRACKET][1] = '*';
	pcde[SDL_SCANCODE_RIGHTBRACKET][2] = '~';
	pcde[SDL_SCANCODE_SEMICOLON][0] = 0x00F6; pcde[SDL_SCANCODE_SEMICOLON][1] = 0x00D6;
	pcde[SDL_SCANCODE_APOSTROPHE][0] = 0x00E4; pcde[SDL_SCANCODE_APOSTROPHE][1] = 0x00C4;
	pcde[SDL_SCANCODE_NONUSHASH][0] = '#';  pcde[SDL_SCANCODE_NONUSHASH][1] = '\'';
	pcde[SDL_SCANCODE_BACKSLASH][0] = '#';  pcde[SDL_SCANCODE_BACKSLASH][1] = '\'';
	pcde[SDL_SCANCODE_NONUSBACKSLASH][0] = '<';
	pcde[SDL_SCANCODE_NONUSBACKSLASH][1] = '>';
	pcde[SDL_SCANCODE_NONUSBACKSLASH][2] = '|';
	pcde[SDL_SCANCODE_COMMA][0] = ',';  pcde[SDL_SCANCODE_COMMA][1] = ';';
	pcde[SDL_SCANCODE_PERIOD][0] = '.'; pcde[SDL_SCANCODE_PERIOD][1] = ':';
	pcde[SDL_SCANCODE_SLASH][0] = '-';  pcde[SDL_SCANCODE_SLASH][1] = '_';
	pcde[SDL_SCANCODE_SPACE][0] = ' ';  pcde[SDL_SCANCODE_SPACE][1] = ' ';
}

static bool IsCharKey(int sc)
{
	InitGermanPc();
	return pcde[sc][0] != 0;
}

#ifdef _WIN32
/* SDL-Scancode -> PC-Scancode (Set 1) der Zeichentasten */
static uint16_t Set1For(int sc)
{
	if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z)
	{
		static const uint8_t az[26] = {
			0x1E,0x30,0x2E,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,
			0x31,0x18,0x19,0x10,0x13,0x1F,0x14,0x16,0x2F,0x11,0x2D,0x15,0x2C };
		return az[sc - SDL_SCANCODE_A];
	}
	if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9)
		return (uint16_t)(0x02 + (sc - SDL_SCANCODE_1));
	switch (sc)
	{
	case SDL_SCANCODE_0:             return 0x0B;
	case SDL_SCANCODE_GRAVE:         return 0x29;
	case SDL_SCANCODE_MINUS:         return 0x0C;
	case SDL_SCANCODE_EQUALS:        return 0x0D;
	case SDL_SCANCODE_LEFTBRACKET:   return 0x1A;
	case SDL_SCANCODE_RIGHTBRACKET:  return 0x1B;
	case SDL_SCANCODE_SEMICOLON:     return 0x27;
	case SDL_SCANCODE_APOSTROPHE:    return 0x28;
	case SDL_SCANCODE_BACKSLASH:     return 0x2B;
	case SDL_SCANCODE_NONUSHASH:     return 0x2B;
	case SDL_SCANCODE_COMMA:         return 0x33;
	case SDL_SCANCODE_PERIOD:        return 0x34;
	case SDL_SCANCODE_SLASH:         return 0x35;
	case SDL_SCANCODE_NONUSBACKSLASH:return 0x56;
	case SDL_SCANCODE_SPACE:         return 0x39;
	default:                         return 0;
	}
}

/* Zeichen laut aktiver Windows-Tastaturbelegung (0 = keines) */
static uint32_t CharFromWindows(int sc, bool shift, bool altgr, bool caps)
{
	HKL hkl = GetKeyboardLayout(0);
	UINT set1 = Set1For(sc);
	UINT vk;
	BYTE ks[256];
	WCHAR buf[8];
	int n;

	if (!set1)
		return 0;
	vk = MapVirtualKeyExW(set1, 3 /* MAPVK_VSC_TO_VK_EX */, hkl);
	if (!vk)
		return 0;
	memset(ks, 0, sizeof(ks));
	if (shift)
		ks[VK_SHIFT] = ks[VK_LSHIFT] = 0x80;
	if (altgr)
	{
		ks[VK_CONTROL] = ks[VK_LCONTROL] = 0x80;
		ks[VK_MENU] = ks[VK_RMENU] = 0x80;
	}
	if (caps)
		ks[VK_CAPITAL] = 0x01;
	memset(buf, 0, sizeof(buf));
	n = ToUnicodeEx(vk, set1, ks, buf, 8, 0x4 /* Totzeichen-Zustand nicht aendern */, hkl);
	if (n < 0)
	{
		/* Totzeichen: Zustand zuruecksetzen, das Akzentzeichen selbst benutzen */
		BYTE none[256];
		WCHAR junk[8];
		memset(none, 0, sizeof(none));
		ToUnicodeEx(VK_SPACE, 0x39, none, junk, 8, 0x4, hkl);
		return (buf[0] >= 0x20) ? buf[0] : 0;
	}
	if (n == 0 || buf[0] < 0x20)
		return 0;
	return buf[0];
}
#endif /* _WIN32 */

static uint32_t CharFromGerman(int sc, bool shift, bool altgr, bool caps)
{
	uint32_t c;
	InitGermanPc();
	if (altgr)
		return pcde[sc][2];
	c = pcde[sc][0];
	if (IsLetterCp(c))
		shift = shift != caps;
	return shift ? pcde[sc][1] : c;
}

static uint32_t PcChar(int sc, bool shift, bool altgr, bool caps)
{
#ifdef _WIN32
	if (ConfigureParams.Keyboard.nPcLayoutSource == 0)
		return CharFromWindows(sc, shift, altgr, caps);
#endif
	return CharFromGerman(sc, shift, altgr, caps);
}

/* ------------------------------------------------------------------ */
/* Tasten ohne Zeichen: feste Zuordnung PC -> Atari                   */
/* ------------------------------------------------------------------ */
static uint8_t specialSt[PC_NUMSC];
static bool specialInit;

static void InitSpecial(void)
{
	int i;
	if (specialInit)
		return;
	specialInit = true;
	memset(specialSt, 0, sizeof(specialSt));
	specialSt[SDL_SCANCODE_ESCAPE]    = 0x01;
	specialSt[SDL_SCANCODE_BACKSPACE] = 0x0E;
	specialSt[SDL_SCANCODE_TAB]       = 0x0F;
	specialSt[SDL_SCANCODE_RETURN]    = 0x1C;
	specialSt[SDL_SCANCODE_DELETE]    = 0x53;
	specialSt[SDL_SCANCODE_INSERT]    = 0x52;
	specialSt[SDL_SCANCODE_HOME]      = 0x47;   /* Clr/Home */
	specialSt[SDL_SCANCODE_PAGEUP]    = 0x62;   /* Help */
	specialSt[SDL_SCANCODE_PAGEDOWN]  = 0x61;   /* Undo */
	specialSt[SDL_SCANCODE_UP]        = 0x48;
	specialSt[SDL_SCANCODE_DOWN]      = 0x50;
	specialSt[SDL_SCANCODE_LEFT]      = 0x4B;
	specialSt[SDL_SCANCODE_RIGHT]     = 0x4D;
	for (i = 0; i < 10; i++)
		specialSt[SDL_SCANCODE_F1 + i] = (uint8_t)(0x3B + i);
	specialSt[SDL_SCANCODE_KP_0] = 0x70;
	specialSt[SDL_SCANCODE_KP_1] = 0x6D;
	specialSt[SDL_SCANCODE_KP_2] = 0x6E;
	specialSt[SDL_SCANCODE_KP_3] = 0x6F;
	specialSt[SDL_SCANCODE_KP_4] = 0x6A;
	specialSt[SDL_SCANCODE_KP_5] = 0x6B;
	specialSt[SDL_SCANCODE_KP_6] = 0x6C;
	specialSt[SDL_SCANCODE_KP_7] = 0x67;
	specialSt[SDL_SCANCODE_KP_8] = 0x68;
	specialSt[SDL_SCANCODE_KP_9] = 0x69;
	specialSt[SDL_SCANCODE_KP_PERIOD]   = 0x71;
	specialSt[SDL_SCANCODE_KP_ENTER]    = 0x72;
	specialSt[SDL_SCANCODE_KP_PLUS]     = 0x4E;
	specialSt[SDL_SCANCODE_KP_MINUS]    = 0x4A;
	specialSt[SDL_SCANCODE_KP_DIVIDE]   = 0x65;
	specialSt[SDL_SCANCODE_KP_MULTIPLY] = 0x66;
	specialSt[SDL_SCANCODE_NUMLOCKCLEAR]= 0x63;   /* Ziffernblock ( */
	specialSt[SDL_SCANCODE_APPLICATION] = 0x64;   /* Ziffernblock ) */
}

/* ------------------------------------------------------------------ */
/* Senden an den Atari                                                */
/* ------------------------------------------------------------------ */
static void SendKey(int st, bool press)
{
	IKBD_PressSTKey((uint8_t)st, press);
}

/* Umschalttasten am Atari auf den gewuenschten Stand bringen */
static void SetMods(uint8_t want)
{
	static const struct { uint8_t bit; uint8_t sc; } mods[3] =
		{ { M_CTRL, ST_SC_CTRL }, { M_ALT, ST_SC_ALT }, { M_SHIFT, ST_SC_SHIFT } };
	int i;
	for (i = 0; i < 3; i++)       /* erst loslassen ... */
		if ((atariMods & mods[i].bit) && !(want & mods[i].bit))
			SendKey(mods[i].sc, false);
	for (i = 0; i < 3; i++)       /* ... dann druecken */
		if (!(atariMods & mods[i].bit) && (want & mods[i].bit))
			SendKey(mods[i].sc, true);
	atariMods = want;
}

static bool AltGrDown(void)
{
	return pc.ralt;
}

static uint8_t Baseline(void)
{
	uint8_t m = 0;
	bool altgr = AltGrDown();
	if (pc.lshift || pc.rshift)
		m |= M_SHIFT;
	if (pc.rctrl || (pc.lctrl && !altgr))    /* AltGr = Strg+Alt unter Windows */
		m |= M_CTRL;
	if (pc.lalt && !altgr)
		m |= M_ALT;
	return m;
}

static void ReleaseAll(void)
{
	int i;
	for (i = 0; i < PC_NUMSC; i++)
	{
		if (held[i].down && held[i].valid)
			SendKey(held[i].st, false);
		held[i].down = held[i].valid = held[i].passed = false;
	}
	SetMods(0);
}

static void Notify(bool on)
{
	Log_Printf(LOG_WARN, "PC-Tastaturbelegung: %s\n", on ? "EIN" : "AUS");
#if KEYMAPPC_HAVE_STATUSBAR
	Statusbar_AddMessage(on ? "PC-Tastaturbelegung EIN" : "PC-Tastaturbelegung AUS", 2500);
#endif
}

/* Schaltzustand pruefen (Konfiguration kann sich jederzeit aendern) */
static bool CheckEnabled(void)
{
	bool en = ConfigureParams.Keyboard.bPcLayoutMapping;
	if (en != wasEnabled)
	{
		wasEnabled = en;
		ReleaseAll();
		if (en)
		{
			Uint16 m = (Uint16)SDL_GetModState();
			memset(&pc, 0, sizeof(pc));
			pc.lshift = (m & KMOD_LSHIFT) != 0;
			pc.rshift = (m & KMOD_RSHIFT) != 0;
			pc.lctrl  = (m & KMOD_LCTRL) != 0;
			pc.rctrl  = (m & KMOD_RCTRL) != 0;
			pc.lalt   = (m & KMOD_LALT) != 0;
			pc.ralt   = (m & KMOD_RALT) != 0;
			EnsureTable();
			SetMods(Baseline());
		}
	}
	return en;
}

/* Tastaturbedienung per Joystick-Emulation hat Vorrang */
static bool IsJoystickKey(int sym)
{
	int i, k;
	if (sym == 0)
		return false;
	for (i = 0; i < JOYSTICK_COUNT; i++)
	{
		const JOYSTICK *j = &ConfigureParams.Joysticks.Joy[i];
		if (j->nJoystickMode != JOYSTICK_KEYBOARD)
			continue;
		if (sym == j->nKeyCodeUp || sym == j->nKeyCodeDown ||
		    sym == j->nKeyCodeLeft || sym == j->nKeyCodeRight ||
		    sym == j->nKeyCodeFire || sym == j->nKeyCodeB ||
		    sym == j->nKeyCodeC || sym == j->nKeyCodeOption ||
		    sym == j->nKeyCodePause || sym == j->nKeyCodeStar ||
		    sym == j->nKeyCodeHash)
			return true;
		for (k = 0; k < 10; k++)
			if (sym == j->nKeyCodeNum[k])
				return true;
	}
	return false;
}

/* Umschalttasten des PC verfolgen. true = war eine Umschalttaste. */
static bool TrackModifier(int sc, bool down)
{
	switch (sc)
	{
	case SDL_SCANCODE_LSHIFT: pc.lshift = down; break;
	case SDL_SCANCODE_RSHIFT: pc.rshift = down; break;
	case SDL_SCANCODE_LCTRL:  pc.lctrl  = down; break;
	case SDL_SCANCODE_RCTRL:  pc.rctrl  = down; break;
	case SDL_SCANCODE_LALT:   pc.lalt   = down; break;
	case SDL_SCANCODE_RALT:   pc.ralt   = down; break;
	case SDL_SCANCODE_LGUI:   pc.lgui   = down; return true;
	case SDL_SCANCODE_RGUI:   pc.rgui   = down; return true;
	default: return false;
	}
	SetMods(Baseline());
	return true;
}

static void SendNumpadCode(int code)
{
	static const uint8_t kp[10] = { 0x70, 0x6D, 0x6E, 0x6F, 0x6A, 0x6B, 0x6C, 0x67, 0x68, 0x69 };
	char digits[8];
	int i;
	SetMods(M_ALT);
	snprintf(digits, sizeof(digits), "%d", code);
	for (i = 0; digits[i]; i++)
	{
		SendKey(kp[digits[i] - '0'], true);
		SendKey(kp[digits[i] - '0'], false);
	}
	SetMods(Baseline());        /* Alt loslassen: TOS fuegt jetzt das Zeichen ein */
}

/* ------------------------------------------------------------------ */
/* Einstiegspunkte                                                    */
/* ------------------------------------------------------------------ */
bool KeymapPc_KeyDown(const SDL_Keysym *ks)
{
	int sc = (int)ks->scancode;
	bool en, altgr, shift, ctrl, alt, caps;
	uint32_t cp;
	AtariKey a;

	if (sc <= 0 || sc >= PC_NUMSC)
		return false;

	en = CheckEnabled();

	/* Umschalten mit "Rollen" - immer aktiv */
	if (sc == SDL_SCANCODE_SCROLLLOCK)
	{
		if (!toggleHeld)
		{
			toggleHeld = true;
			ConfigureParams.Keyboard.bPcLayoutMapping = !ConfigureParams.Keyboard.bPcLayoutMapping;
			CheckEnabled();
			Notify(ConfigureParams.Keyboard.bPcLayoutMapping);
		}
		return true;
	}
	if (!en)
		return false;

	InitSpecial();

	if (sc == SDL_SCANCODE_CAPSLOCK)
	{
		held[sc].passed = true;
		return false;                     /* Feststelltaste: wie bisher */
	}
	if (TrackModifier(sc, true))
		return true;

	if (pc.lgui || pc.rgui)               /* Win-Taste + X bleibt Hatari-Shortcut */
	{
		held[sc].passed = true;
		return false;
	}
	if (IsJoystickKey((int)ks->sym))
	{
		held[sc].passed = true;
		return false;
	}
	if (held[sc].down)
		return true;                      /* Wiederholung ignorieren */

	altgr = AltGrDown();
	shift = pc.lshift || pc.rshift;
	ctrl  = (Baseline() & M_CTRL) != 0;
	alt   = (Baseline() & M_ALT) != 0;
	caps  = (ks->mod & KMOD_CAPS) != 0;

	/* Tasten mit fester Atari-Zuordnung */
	if (specialSt[sc])
	{
		SetMods(Baseline());
		SendKey(specialSt[sc], true);
		held[sc].down = held[sc].valid = true;
		held[sc].st = specialSt[sc];
		return true;
	}

	/* Zeichentasten */
	if (!IsCharKey(sc))
	{
		held[sc].passed = true;
		return false;                     /* z.B. F11, F12, Pause: Hatari-Standard */
	}
	if (altgr && sc == SDL_SCANCODE_M)    /* AltGr+M = Maus fangen (Hatari) */
	{
		held[sc].passed = true;
		return false;
	}
	cp = PcChar(sc, shift, altgr, caps);
	if (cp == 0)
	{
		if (altgr)
		{
			held[sc].passed = true;
			return false;                 /* AltGr-Shortcuts von Hatari */
		}
		held[sc].down = true;             /* unbelegt */
		return true;
	}
	a = LookupAtari(cp);
	if (a.kind == A_NONE)
	{
		if (altgr)
		{
			held[sc].passed = true;
			return false;
		}
		held[sc].down = true;             /* gibt es auf dem Atari nicht */
		return true;
	}
	if (a.kind == A_NUMPAD)
	{
		held[sc].down = true;
		if (!ctrl && !alt)
			SendNumpadCode(a.code);
		return true;
	}
	SetMods((uint8_t)((a.shift ? M_SHIFT : 0) | (ctrl ? M_CTRL : 0) | (alt ? M_ALT : 0)));
	SendKey(a.sc, true);
	held[sc].down = held[sc].valid = true;
	held[sc].st = a.sc;
	return true;
}

bool KeymapPc_KeyUp(const SDL_Keysym *ks)
{
	int sc = (int)ks->scancode;
	bool en;

	if (sc <= 0 || sc >= PC_NUMSC)
		return false;
	en = CheckEnabled();

	if (sc == SDL_SCANCODE_SCROLLLOCK)
	{
		bool used = toggleHeld;
		toggleHeld = false;
		return used || en;
	}
	if (!en)
		return false;

	if (held[sc].passed)
	{
		held[sc].passed = false;
		return false;
	}
	if (held[sc].down)
	{
		if (held[sc].valid)
			SendKey(held[sc].st, false);
		held[sc].down = held[sc].valid = false;
		SetMods(Baseline());
		return true;
	}
	if (TrackModifier(sc, false))
		return true;
	return false;                         /* Taste war vor dem Einschalten unten */
}
