#!/usr/bin/env python3
"""
Convert the reconstructed printer font ROM dumps (fonts/rom/*.BIN) into the
column major tables of pe_font (core/src/fonts/font_<id>.c).

Layouts of the dumps (256 glyphs, code = index):
  24xN   24 rows, N columns, row major, N/8 bytes per row, MSB = left column
  11x8   11 columns of one byte, bit 0 = top wire
  78B    39 columns, upper 8 wires, then 39 columns, lower 8 wires (NLQ)

Most dumps were produced by scaling every glyph to the same box, so an
apostrophe, a period and a lowercase 'a' are all as tall as a capital.  Such
fonts are detected (>= 80 % of the glyphs share one row range) and repaired:
every glyph is scaled vertically into the band its typographic class needs
(capital, x-height, ascender, descender, punctuation).
"""
import collections, os, sys

ROM = os.path.join(os.path.dirname(__file__), "..", "fonts", "rom")
OUT = os.path.join(os.path.dirname(__file__), "..", "core", "src", "fonts")

# id, file, kind, params
FONTS = [
 ("lq800_draft",  "LQ800_RECON_DRAFT_24x16.BIN",  "rows", dict(bpr=2, cell=24, vres=108, base=19)),
 ("lq800_roman",  "LQ800_RECON_ROMAN_24x32.BIN",  "rows", dict(bpr=4, cell=26, vres=0)),
 ("lq800_sans",   "LQ800_RECON_SANS_24x32.BIN",   "rows", dict(bpr=4, cell=26, vres=0)),
 ("fx80_courier", "FX80_RECON_COURIER_11x9.BIN",  "cols8", dict(cell=12, vres=0)),
 ("kxp_draft",    "KXP2023_RECON_DRAFT_24x8.BIN", "rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("kxp_courier",  "KXP2023_RECON_COURIER_24x32.BIN","rows", dict(bpr=4, cell=26, vres=0)),
 ("kxp_prestige", "KXP2023_RECON_PRESTIGE_24x32.BIN","rows", dict(bpr=4, cell=26, vres=0)),
 ("kxp_boldps",   "KXP2023_RECON_BOLDPS_24x32.BIN","rows", dict(bpr=4, cell=26, vres=0)),
 ("kxp_script",   "KXP2023_RECON_SCRIPT_24x32.BIN","rows", dict(bpr=4, cell=26, vres=0)),
 ("lc24_draft",        "LC24_RECON_DRAFT_24x8.BIN",        "rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("lc24_draft_bold",   "LC24_RECON_DRAFT_BOLD_24x8.BIN",   "rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("lc24_draft_italic", "LC24_RECON_DRAFT_ITALIC_24x8.BIN", "rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("lc24_courier",      "LC24_RECON_COURIER_24x8.BIN",      "rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("lc24_courier_italic","LC24_RECON_COURIER_ITALIC_24x8.BIN","rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("lc24_prestige",     "LC24_RECON_PRESTIGE_24x8.BIN",     "rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("lc24_prestige_italic","LC24_RECON_PRESTIGE_ITALIC_24x8.BIN","rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("lc24_orator",       "LC24_RECON_ORATOR_24x8.BIN",       "rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("lc24_script",       "LC24_RECON_SCRIPT_24x8.BIN",       "rows", dict(bpr=1, cell=12, vres=108, base=19)),
 ("nlq401_draft", "NLQ401_RECON_DRAFT_11x8.BIN",  "cols8", dict(cell=12, vres=0)),
 ("nlq401_nlq",   "NLQ401_RECON_NLQ_78B.BIN",     "nlq",  dict(cell=30, vres=144)),
]

# target bands (rows) for the repaired fonts
LAYOUTS = {
 "lq":  dict(pins=24, asc=1, cap=3, x=8,  base=17, desc=22, dot=2),
 "fx":  dict(pins=9,  asc=0, cap=0, x=2,  base=6,  desc=8,  dot=1),
 "nlq": dict(pins=18, asc=0, cap=1, x=5,  base=12, desc=17, dot=1),
}

def classes(L):
    cap, x, asc, base, desc, dot = L["cap"], L["x"], L["asc"], L["base"], L["desc"], L["dot"]
    m = (x + base) // 2
    top2 = cap + max(2, (base - cap) // 3)
    c = {}
    for ch in "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$%&@!?": c[ch] = (cap, base)
    for ch in "acemnorsuvwxz": c[ch] = (x, base)
    for ch in "bdfhkl": c[ch] = (asc, base)
    for ch in "gpqy": c[ch] = (x, desc)
    c["i"] = (cap, base); c["j"] = (cap, desc); c["t"] = (x - (x - cap) // 2, base)
    c["."] = (base - dot + 1, base); c[","] = (base - dot + 1, min(desc, base + 2 * dot))
    c[":"] = (x, base); c[";"] = (x, min(desc, base + 2 * dot))
    for ch in "'\"`^": c[ch] = (cap, top2)
    c["~"] = (x, x + max(2, (base - x) // 2))
    c["-"] = (m, m + max(0, dot - 1)); c["_"] = (min(desc, base + 1), min(desc, base + dot))
    c["="] = (m - 2 * dot, m + dot); c["+"] = (x + 1, base - 1); c["*"] = (cap, cap + (base - cap) // 2)
    c["<"] = (x, base); c[">"] = (x, base)
    c["/"] = (cap, min(desc, base + dot)); c["\\"] = (cap, min(desc, base + dot))
    for ch in "|()[]{}": c[ch] = (asc, desc)
    return c

def load(fid, fn, kind, p):
    d = open(os.path.join(ROM, fn), "rb").read()
    glyphs = {}
    if kind == "rows":
        bpr = p["bpr"]; n = len(d) // 256; nc = bpr * 8
        for code in range(256):
            g = d[code * n:(code + 1) * n]
            glyphs[code] = [[(int.from_bytes(g[r*bpr:(r+1)*bpr], "big") >> (nc-1-c)) & 1 for c in range(nc)] for r in range(24)]
        return glyphs, 24, nc
    if kind == "cols8":
        for code in range(256):
            g = d[code * 11:(code + 1) * 11]
            glyphs[code] = [[(g[c] >> r) & 1 for c in range(11)] for r in range(8)]
        return glyphs, 8, 11
    if kind == "nlq":
        for code in range(256):
            g = d[code * 78:(code + 1) * 78]
            glyphs[code] = [[((g[c] if r < 8 else g[39 + c]) >> (r % 8)) & 1 for c in range(39)] for r in range(16)]
        return glyphs, 16, 39

def bbox_rows(m):
    rs = [i for i, r in enumerate(m) if any(r)]
    return (rs[0], rs[-1]) if rs else None

def normalized(glyphs):
    ext = collections.Counter(bbox_rows(glyphs[c]) for c in range(0x21, 0x7f) if bbox_rows(glyphs[c]))
    return ext.most_common(1)[0][1] >= 0.8 * 94

def rescale(m, ncols, t0, t1, pins):
    b = bbox_rows(m)
    out = [[0] * ncols for _ in range(pins)]
    if not b: return out
    r0, r1 = b; sh = r1 - r0 + 1; th = t1 - t0 + 1
    for i in range(th):
        lo = (i * sh) // th
        hi = max(lo, -(-(i + 1) * sh // th) - 1)
        for c in range(ncols):
            if any(m[r0 + k][c] for k in range(lo, min(hi, sh - 1) + 1)):
                out[t0 + i][c] = 1
    return out

def convert(fid, fn, kind, p, report):
    glyphs, srows, ncols = load(fid, fn, kind, p)
    norm = normalized(glyphs)
    if kind == "rows":
        lay = "lq" if p["bpr"] == 4 else None
    else:
        lay = "fx" if kind == "cols8" else "nlq"
    if norm and lay:
        L = LAYOUTS[lay]; cls = classes(L); pins = L["pins"]; base = L["base"]
        out = {}
        for code in range(0x21, 0x7f):
            ch = chr(code)
            t0, t1 = cls.get(ch, (L["cap"], L["base"]))
            out[code] = rescale(glyphs[code], ncols, t0, t1, pins)
    else:
        pins = srows; base = p.get("base", 17)
        out = {c: glyphs[c] for c in range(0x21, 0x7f)}
    out[0x20] = [[0] * ncols for _ in range(pins)]
    # horizontal extent -> cell
    maxc = max((max((c for r in out[k] for c, v in enumerate(r) if v), default=0) for k in out), default=0) + 1
    cell = max(p["cell"], maxc + 2)
    report.append("%-22s %-34s %s -> %2d Nadeln, %2d Spalten, Zelle %2d%s" % (
        fid, fn, "repariert" if (norm and lay) else "unveraendert", pins, ncols, cell, "" if not p["vres"] else ", %d dpi senkrecht" % p["vres"]))
    # column major words
    codes = sorted(out)
    words = []
    for k in codes:
        m = out[k]
        for c in range(ncols):
            w = 0
            for r in range(pins):
                if m[r][c]: w |= 1 << r
            words.append(w)
    return dict(id=fid, pins=pins, width=ncols, cell=cell, base=base, vres=p["vres"], codes=codes, words=words)

def emit(f):
    path = os.path.join(OUT, "font_%s.c" % f["id"])
    L = []
    L.append("/* Generated by tools/binfont.py from fonts/rom -- do not edit. */")
    L.append('#include "pe/font.h"\n')
    L.append("static const uint16_t %s_codes[%d] = {" % (f["id"], len(f["codes"])))
    L.append("    " + ", ".join("0x%02x" % c for c in f["codes"]) + ",\n};\n")
    L.append("static const uint32_t %s_cols[%d] = {" % (f["id"], len(f["words"])))
    row = []
    for i, w in enumerate(f["words"]):
        row.append("0x%x" % w)
        if (i + 1) % f["width"] == 0:
            L.append("    " + ",".join(row) + ","); row = []
    L.append("};\n")
    L.append("const pe_font pe_font_%s = {" % f["id"])
    L.append('    "%s", %d, %d, %d, %d, %d,' % (f["id"], f["pins"], f["width"], f["cell"], f["base"], len(f["codes"])))
    L.append("    %s_codes, %s_cols, 0, %d," % (f["id"], f["id"], f["vres"]))
    L.append("};")
    open(path, "w").write("\n".join(L) + "\n")

def main():
    rep = []
    fonts = [convert(*f, rep) for f in FONTS]
    for f in fonts: emit(f)
    print("\n".join(rep))
    open(os.path.join(OUT, "..", "..", "include", "pe", "rom_fonts.h"), "w").write(
        "/* Generated by tools/binfont.py */\n#ifndef PE_ROM_FONTS_H\n#define PE_ROM_FONTS_H\n#include \"pe/font.h\"\n"
        + "".join("extern const pe_font pe_font_%s;\n" % f["id"] for f in fonts) + "#endif\n")
    return fonts

if __name__ == "__main__":
    main()
