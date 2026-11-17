#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hATARI 2.8 - PC-Tastaturbelegung in die Hatari-Quellen einhaengen.

Aufruf:  python3 tools/patch_keymap.py <hatari-src>

1. Prueft, welche Hilfsfunktionen die Hatari-Quellen anbieten, und schreibt
   src/includes/keymap_pc_cfg.h (TOS-Image-Auslesen und Statusleisten-Meldung
   werden nur eingeschaltet, wenn die noetigen Namen vorhanden sind).
2. Haengt in keymap.c am Anfang von Keymap_KeyDown() und Keymap_KeyUp() den
   Aufruf von KeymapPc_KeyDown()/KeymapPc_KeyUp() ein.
3. Ergaenzt configuration.h/.c um die Einstellungen bPcLayoutMapping und
   nPcLayoutSource und nimmt keymap_pc.c in src/CMakeLists.txt auf.
4. Haengt in shortcut.c am Anfang von ShortCut_CheckKeys() den Aufruf von
   KeymapPc_ShortcutModifier() ein: Mit aktiver PC-Tastaturbelegung loesen
   AltGr-Kombinationen keine Hatari-Kuerzel mehr aus, die Windows-Taste
   uebernimmt deren Rolle.
5. Schreibt Diagnosezeilen ins Bauprotokoll (wo Hatari Tasten und AltGr
   behandelt).
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


try:
    sys.stdout.reconfigure(errors="replace")
except Exception:
    pass


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


def patch_shortcut(root):
    """ShortCut_CheckKeys(): AltGr-Kuerzel bei aktiver PC-Tastaturbelegung sperren."""
    p = os.path.join(root, "src", "shortcut.c")
    if not os.path.isfile(p):
        print("patch_keymap: WARNUNG shortcut.c nicht gefunden - AltGr-Kuerzel werden nicht gesperrt")
        return
    t = read(p)
    if "KeymapPc_ShortcutModifier" in t:
        print("patch_keymap: shortcut.c ist bereits eingehaengt")
        return
    m = re.search(r"(^|\n)((?:static\s+)?\w+\s+ShortCut_CheckKeys\s*\(([^)]*)\)\s*\{)", t)
    if not m:
        print("patch_keymap: WARNUNG ShortCut_CheckKeys() nicht gefunden - AltGr-Kuerzel werden nicht gesperrt")
        return
    params = [x.strip() for x in m.group(3).split(",")]
    if len(params) < 2 or "*" in params[0]:
        print("patch_keymap: WARNUNG Parameter von ShortCut_CheckKeys() unerwartet: " + m.group(3))
        return
    names = re.findall(r"[A-Za-z_]\w*", params[0])
    if not names:
        print("patch_keymap: WARNUNG Parametername von ShortCut_CheckKeys() nicht erkennbar")
        return
    arg = names[-1]
    ins = "\n\t%s = KeymapPc_ShortcutModifier(%s);\n" % (arg, arg)
    t = t[:m.end()] + ins + t[m.end():]
    last = None
    for last in re.finditer(r"#include[^\n]*\n", t):
        pass
    if last is None:
        print("patch_keymap: WARNUNG keine #include-Zeilen in shortcut.c")
        return
    t = t[:last.end()] + '#include "keymap_pc.h"\n' + t[last.end():]
    write(p, t)
    print("patch_keymap: shortcut.c eingehaengt (Parameter '%s')" % arg)


def diagnose(root):
    """Zeigt im Bauprotokoll, wo Hatari Tasten und AltGr behandelt."""
    pats = [("AltGr/KMOD_RALT/KMOD_MODE", re.compile(r"KMOD_RALT|KMOD_MODE")),
            ("Aufrufe Keymap_KeyDown/Up", re.compile(r"\bKeymap_Key(Down|Up)\s*\(")),
            ("Aufrufe ShortCut_CheckKeys", re.compile(r"\bShortCut_CheckKeys\s*\("))]
    src = os.path.join(root, "src")
    files = []
    for d, _, names in os.walk(src):
        for n in names:
            if n.endswith(".c"):
                files.append(os.path.join(d, n))
    files.sort()
    print("patch_keymap: --- Diagnose -------------------------------------------")
    for title, rx in pats:
        print("patch_keymap: [%s]" % title)
        cnt = 0
        for f in files:
            if os.path.basename(f) in ("keymap_pc.c",):
                continue
            try:
                lines = read(f).split("\n")
            except OSError:
                continue
            for i, l in enumerate(lines, 1):
                if rx.search(l):
                    cnt += 1
                    if cnt <= 25:
                        print("patch_keymap:   %s:%d: %s" % (os.path.relpath(f, root), i, l.strip()[:110]))
        if cnt == 0:
            print("patch_keymap:   (keine Treffer)")
    print("patch_keymap: --- Ende Diagnose --------------------------------------")


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
    patch_shortcut(root)
    diagnose(root)

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
