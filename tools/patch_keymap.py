#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hATARI 2.7.2.7 - PC-Tastaturbelegung in die Hatari-Quellen einhaengen.

Aufruf:  python3 tools/patch_keymap.py <hatari-src>

1. Prueft, welche Hilfsfunktionen die Hatari-Quellen anbieten, und schreibt
   src/includes/keymap_pc_cfg.h (TOS-Image-Auslesen und Statusleisten-Meldung
   werden nur eingeschaltet, wenn die noetigen Namen vorhanden sind).
2. Haengt in keymap.c am Anfang von Keymap_KeyDown() und Keymap_KeyUp() den
   Aufruf von KeymapPc_KeyDown()/KeymapPc_KeyUp() ein.
3. Ergaenzt configuration.h/.c um die Einstellungen bPcLayoutMapping und
   nPcLayoutSource und nimmt keymap_pc.c in src/CMakeLists.txt auf.
Alle Eingriffe sind wiederholbar (bereits Eingehaengtes wird erkannt) und
brechen mit Fehlermeldung ab, wenn die Einhaengestelle nicht gefunden wird.
Die Zieldateien werden erst NACH dem Kopieren der Projektdateien (Schritt 5)
bearbeitet, daher haengt das Paket nicht vom Stand von configuration.c/.h und
CMakeLists.txt ab.
"""
import os
import re
import sys


def read(path):
    with open(path, encoding="latin-1", newline="") as f:
        return f.read()


def write(path, text):
    with open(path, "w", encoding="latin-1", newline="") as f:
        f.write(text)


def fail(msg):
    print("FEHLER patch_keymap: " + msg)
    sys.exit(1)


def has(root, header, pattern):
    p = os.path.join(root, "src", "includes", header)
    if not os.path.isfile(p):
        return False
    return re.search(pattern, read(p)) is not None


def patch_config(root):
    """configuration.h / configuration.c um die neuen Einstellungen erweitern."""
    h = os.path.join(root, "src", "includes", "configuration.h")
    c = os.path.join(root, "src", "configuration.c")
    for p in (h, c):
        if not os.path.isfile(p):
            fail(p + " nicht gefunden")

    t = read(h)
    if "bPcLayoutMapping" not in t:
        m = re.search(r"([ \t]*char\s+szMappingFileName\s*\[[^\]]*\]\s*;[^\n]*\n)", t)
        if not m:
            fail("szMappingFileName in configuration.h nicht gefunden")
        add = ("  bool bPcLayoutMapping;   /* PC-Tastaturbelegung (hATARI 2.7.2.7) */\n"
               "  int nPcLayoutSource;     /* 0 = Betriebssystem (Windows) bzw. Deutsch, 1 = Deutsch fest */\n")
        t = t[:m.end()] + add + t[m.end():]
        write(h, t)
        print("patch_keymap: configuration.h erweitert")

    t = read(c)
    if "bPcLayoutMapping" not in t:
        m = re.search(r'[ \t]*\{\s*"szMappingFileName"[^\n]*\n', t)
        if not m:
            fail("Tag szMappingFileName in configuration.c nicht gefunden")
        add = ('\t{ "bPcLayoutMapping", Bool_Tag, &ConfigureParams.Keyboard.bPcLayoutMapping },\n'
               '\t{ "nPcLayoutSource", Int_Tag, &ConfigureParams.Keyboard.nPcLayoutSource },\n')
        t = t[:m.end()] + add + t[m.end():]
        m = re.search(r"[ \t]*strcpy\(\s*ConfigureParams\.Keyboard\.szMappingFileName[^\n]*\n", t)
        if not m:
            fail("Vorgabewert von szMappingFileName in configuration.c nicht gefunden")
        add = ("\tConfigureParams.Keyboard.bPcLayoutMapping = false;\n"
               "\tConfigureParams.Keyboard.nPcLayoutSource = 0;\n")
        t = t[:m.end()] + add + t[m.end():]
        write(c, t)
        print("patch_keymap: configuration.c erweitert")


def patch_cmake(root):
    """keymap_pc.c in die Quellenliste von src/CMakeLists.txt aufnehmen."""
    p = os.path.join(root, "src", "CMakeLists.txt")
    if not os.path.isfile(p):
        fail(p + " nicht gefunden")
    t = read(p)
    if "keymap_pc.c" in t:
        return
    t2, n = re.subn(r"\bkeymap\.c\b", "keymap.c keymap_pc.c", t, count=1)
    if n != 1:
        fail("keymap.c in src/CMakeLists.txt nicht gefunden")
    write(p, t2)
    print("patch_keymap: CMakeLists.txt erweitert")


def main():
    if len(sys.argv) != 2:
        fail("Aufruf: patch_keymap.py <hatari-src>")
    root = sys.argv[1]
    keymap = os.path.join(root, "src", "keymap.c")
    if not os.path.isfile(keymap):
        fail("src/keymap.c nicht gefunden in " + root)

    # --- 1. Faehigkeiten pruefen -------------------------------------------
    if not has(root, "ikbd.h", r"\bIKBD_PressSTKey\b"):
        fail("IKBD_PressSTKey() fehlt in ikbd.h - Hatari-Version nicht unterstuetzt")
    romscan = (has(root, "stMemory.h", r"\bSTMemory_ReadByte\b")
               and has(root, "tos.h", r"\bTosAddress\b")
               and has(root, "tos.h", r"\bTosSize\b"))
    statusbar = has(root, "statusbar.h", r"\bStatusbar_AddMessage\b")
    print("patch_keymap: TOS-Image auslesen : " + ("ja" if romscan else "nein (eingebaute Tabelle)"))
    print("patch_keymap: Statusleiste       : " + ("ja" if statusbar else "nein"))

    cfg = os.path.join(root, "src", "includes", "keymap_pc_cfg.h")
    write(cfg,
          "/* automatisch erzeugt von tools/patch_keymap.py */\n"
          "#define KEYMAPPC_HAVE_ROMSCAN   %d\n"
          "#define KEYMAPPC_HAVE_STATUSBAR %d\n" % (1 if romscan else 0, 1 if statusbar else 0))

    patch_config(root)
    patch_cmake(root)

    # --- 2. keymap.c einhaengen --------------------------------------------
    src = read(keymap)
    if "KeymapPc_KeyDown" in src:
        print("patch_keymap: keymap.c ist bereits eingehaengt")
        return

    def hook(text, func, call):
        pat = re.compile(r"(^|\n)(void\s+" + func + r"\s*\(([^)]*)\)\s*\{)")
        m = pat.search(text)
        if not m:
            fail("Funktion " + func + "() in keymap.c nicht gefunden")
        names = re.findall(r"[A-Za-z_]\w*", m.group(3))
        if not names:
            fail("Parametername von " + func + "() nicht erkennbar")
        arg = names[-1]
        ins = "\n\tif (%s(%s))\n\t\treturn;\n" % (call, arg)
        return text[:m.end()] + ins + text[m.end():]

    src = hook(src, "Keymap_KeyDown", "KeymapPc_KeyDown")
    src = hook(src, "Keymap_KeyUp", "KeymapPc_KeyUp")

    inc = '#include "keymap_pc.h"\n'
    m = re.search(r'#include\s+"keymap\.h"[^\n]*\n', src)
    if m:
        src = src[:m.end()] + inc + src[m.end():]
    else:
        last = None
        for last in re.finditer(r"#include[^\n]*\n", src):
            pass
        if last is None:
            fail("keine #include-Zeilen in keymap.c")
        src = src[:last.end()] + inc + src[last.end():]

    write(keymap, src)
    print("patch_keymap: keymap.c eingehaengt")


if __name__ == "__main__":
    main()
