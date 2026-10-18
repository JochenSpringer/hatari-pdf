#include "escp.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* International character sets (ESC R n)                              */
/* ------------------------------------------------------------------ */

/* The 12 ASCII positions the national variants replace. */
static const uint8_t intl_slots[12] = {0x23, 0x24, 0x40, 0x5B, 0x5C, 0x5D,
                                       0x5E, 0x60, 0x7B, 0x7C, 0x7D, 0x7E};

/* Replacement code points (Latin-1), one row per set.  0 = keep ASCII. */
static const uint16_t intl_sets[13][12] = {
    /* 0  USA     */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 1  France  */ {0, 0, 0xE0, 0xB0, 0xE7, 0xA7, 0, 0, 0xE9, 0xF9, 0xE8, 0xA8},
    /* 2  Germany */ {0, 0, 0xA7, 0xC4, 0xD6, 0xDC, 0, 0, 0xE4, 0xF6, 0xFC, 0xDF},
    /* 3  UK      */ {0xA3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 4  DenmarkI*/ {0, 0, 0, 0xC6, 0xD8, 0xC5, 0, 0, 0xE6, 0xF8, 0xE5, 0},
    /* 5  Sweden  */ {0, 0, 0xC9, 0xC4, 0xD6, 0xC5, 0xDC, 0xE9, 0xE4, 0xF6, 0xE5, 0xFC},
    /* 6  Italy   */ {0, 0, 0, 0xB0, 0, 0xE9, 0, 0xF9, 0xE0, 0xF2, 0xE8, 0xEC},
    /* 7  Spain I */ {0xA4, 0, 0, 0xA1, 0xD1, 0xBF, 0, 0, 0xA8, 0xF1, 0, 0},
    /* 8  Japan   */ {0, 0, 0, 0, 0xA5, 0, 0, 0, 0, 0, 0, 0},
    /* 9  Norway  */ {0, 0, 0xC9, 0xC6, 0xD8, 0xC5, 0xDC, 0xE9, 0xE6, 0xF8, 0xE5, 0xFC},
    /* 10 DenmarkII*/{0, 0, 0xC9, 0xC6, 0xD8, 0xC5, 0xDC, 0xE9, 0xE6, 0xF8, 0xE5, 0xFC},
    /* 11 Spain II*/ {0, 0, 0xE1, 0xA1, 0xD1, 0xBF, 0xE9, 0, 0xED, 0xF1, 0xF3, 0xFA},
    /* 12 LatinAm */ {0, 0, 0xE1, 0xA1, 0xD1, 0xBF, 0xE9, 0xFC, 0xED, 0xF1, 0xF3, 0xFA},
};

static uint16_t translate(const escp *s, uint8_t c) {
  if (s->intl == 0 || s->intl > 12) return c;
  for (int i = 0; i < 12; i++) {
    if (intl_slots[i] == c) {
      uint16_t r = intl_sets[s->intl][i];
      return r ? r : c;
    }
  }
  return c;
}

/* ------------------------------------------------------------------ */
/* Metrics                                                             */
/* ------------------------------------------------------------------ */

/* Cell advance in units for 10/12/15 cpi, normal and condensed. */
static const int32_t pitch_normal[3] = {216, 180, 144};
static const int32_t pitch_cond[3] = {126, 108, 144}; /* 17.14, 20, 15 cpi */

static const pe_font *font_for(const escp *s) {
  if (!s->lq || !s->caps->lq) return s->caps->draft;
  if (s->typeface < s->caps->ntypefaces) {
    const escp_typeface *t = &s->caps->typefaces[s->typeface];
    return s->proportional && t->prop ? t->prop : t->fixed;
  }
  return s->caps->lq;
}

static int32_t char_width(const escp *s) {
  int32_t w = s->condensed ? pitch_cond[s->cpi] : pitch_normal[s->cpi];
  if (s->expanded || s->expanded_line) w *= 2;
  return w;
}

static int32_t cell_height(const escp *s) {
  const pe_font *f = font_for(s);
  int32_t h = (int32_t)f->pins * s->caps->pin_pitch;
  return s->dheight ? h * 2 : h;
}

/* ------------------------------------------------------------------ */
/* Paper movement                                                      */
/* ------------------------------------------------------------------ */

static void form_feed(escp *s, pe_engine *e) {
  pe_engine_form_feed(e, 0);
  e->x = s->left_margin;
  e->y = 0;
}

static void feed(escp *s, pe_engine *e, int32_t units) {
  e->y += units;
  if (e->y < 0) e->y = 0;
  if (e->y + cell_height(s) > s->form_length - s->skip_bottom) form_feed(s, e);
}

static void line_feed(escp *s, pe_engine *e) { feed(s, e, s->line_spacing); }

static void carriage_return(escp *s, pe_engine *e) {
  e->x = s->left_margin;
  if (s->autofeed) line_feed(s, e);
}

/* ------------------------------------------------------------------ */
/* Character imaging                                                   */
/* ------------------------------------------------------------------ */

static void draw_glyph(escp *s, pe_engine *e, uint16_t code, int32_t cw) {
  const pe_font *f = font_for(s);
  uint8_t adv = f->width;
  const uint32_t *g = pe_font_glyph(f, code, &adv);
  if (!g) return;

  const int32_t dot = s->caps->dot;
  const int32_t pin = s->caps->pin_pitch;
  const int32_t x0 = e->x;
  const int32_t y0 = e->y;
  const int32_t colpitch = cw / f->cell ? cw / f->cell : 1;
  /* Emphasized and double strike fire the wire again half a dot away; using
   * the dot size rather than the column pitch keeps the effect visible on
   * the 24 wire heads, whose columns are much finer than their dots. */
  const int32_t smear = dot / 2;
  /* Italics lean the cell by about 1 dot column per 3 wires. */
  const int32_t shear = s->italic ? pin / 3 : 0;
  const int32_t sub_off = s->script == 2 ? (int32_t)f->pins * pin / 3 : 0;

  for (int c = 0; c < f->width; c++) {
    uint32_t bits = g[c];
    if (!bits) continue;
    int32_t cx = x0 + (int32_t)(((int64_t)c * cw) / f->cell);
    for (int r = 0; r < f->pins; r++) {
      if (!(bits & (1u << r))) continue;
      int32_t ry = r;
      int32_t y;
      if (s->script) {
        /* Super/subscript squeeze the cell to two thirds height. */
        y = y0 + sub_off + (int32_t)(((int64_t)ry * pin * 2) / 3);
      } else if (s->dheight) {
        y = y0 + ry * pin * 2;
      } else {
        y = y0 + ry * pin;
      }
      int32_t x = cx + shear * (f->baseline - r);
      pe_engine_dot(e, x, y, dot);
      if (s->emphasized) pe_engine_dot(e, x + smear, y, dot);
      if (s->dstrike) {
        pe_engine_dot(e, x, y + smear, dot);
        if (s->emphasized) pe_engine_dot(e, x + smear, y + smear, dot);
      }
    }
  }
  if (s->underline) {
    int32_t uy = y0 + ((int32_t)f->baseline + 2) * pin;
    pe_engine_hrule(e, x0, x0 + cw - colpitch, uy, dot);
  }
}

static void put_char(escp *s, pe_engine *e, uint8_t c) {
  int32_t cw = char_width(s);
  const pe_font *f = font_for(s);
  uint16_t code = translate(s, c);

  if (s->proportional) {
    uint8_t adv = f->width;
    (void)pe_font_glyph(f, code, &adv);
    int32_t pw = (int32_t)(((int64_t)adv * cw) / f->cell);
    if (pw > 0) cw = pw;
  }

  if (e->x + cw > s->right_margin) {
    carriage_return(s, e);
    if (!s->autofeed) line_feed(s, e);
  }
  draw_glyph(s, e, code, cw);
  e->x += cw;
  s->chars_printed++;
}

/* ------------------------------------------------------------------ */
/* Bit image graphics                                                  */
/* ------------------------------------------------------------------ */

typedef struct {
  uint8_t mode;
  uint16_t dpi;
  uint8_t pins;
} gfx_mode;

static const gfx_mode gfx_modes[] = {
    {0, 60, 8},   {1, 120, 8},  {2, 120, 8},  {3, 240, 8},  {4, 80, 8},
    {5, 72, 8},   {6, 90, 8},   {7, 144, 8},  {32, 60, 24}, {33, 120, 24},
    {38, 90, 24}, {39, 180, 24}, {40, 360, 24}, {71, 180, 24}, {72, 360, 24},
    {73, 360, 24},
};

static const gfx_mode *find_mode(uint8_t m) {
  for (size_t i = 0; i < sizeof gfx_modes / sizeof gfx_modes[0]; i++)
    if (gfx_modes[i].mode == m) return &gfx_modes[i];
  return NULL;
}

static void gfx_start(escp *s, pe_engine *e, uint8_t mode, int32_t cols) {
  const gfx_mode *gm = find_mode(mode);
  (void)e;
  if (!gm || (gm->pins == 24 && !s->caps->has_24bit_gfx)) {
    /*
     * A density this printer cannot produce.  The image is dropped, but the
     * data still has to be consumed - otherwise it would be printed as text,
     * which is how a stream meant for the wrong printer turns into pages of
     * garbage.
     */
    s->gfx_cols = cols;
    s->gfx_pins = 0;
    s->gfx_bpc = gm ? (uint8_t)(gm->pins / 8) : 1;
    s->gfx_col_n = 0;
    return;
  }
  s->gfx_cols = cols;
  s->gfx_pins = gm->pins;
  s->gfx_bpc = (uint8_t)(gm->pins / 8);
  s->gfx_col_n = 0;
  s->gfx_pitch = PE_UNITS_PER_INCH / gm->dpi;
  s->gfx_vpitch =
      gm->pins == 24 ? PE_UNITS_PER_INCH / 180 : s->caps->gfx8_pitch;
}

/* 9 wire graphics (ESC ^) sends two bytes per column, 9 dots used. */
static void gfx_start9(escp *s, uint8_t mode, int32_t cols) {
  s->gfx_cols = cols;
  s->gfx_pins = 9;
  s->gfx_bpc = 2;
  s->gfx_col_n = 0;
  s->gfx_pitch = PE_UNITS_PER_INCH / (mode ? 120 : 60);
  s->gfx_vpitch = s->caps->gfx8_pitch;
}

static void gfx_column(escp *s, pe_engine *e) {
  uint32_t bits = 0;
  if (s->gfx_pins == 0) return; /* unsupported density: swallow the data */
  for (int i = 0; i < s->gfx_bpc; i++) {
    uint8_t b = s->gfx_col[i];
    for (int k = 0; k < 8; k++) {
      if (b & (0x80u >> k)) bits |= 1u << (i * 8 + k);
    }
  }
  if (bits)
    pe_engine_column(e, e->x, e->y, bits, s->gfx_pins, s->gfx_vpitch,
                     s->caps->dot);
  e->x += s->gfx_pitch;
  if (e->x > s->printable_width) e->x = s->printable_width;
}

/* ------------------------------------------------------------------ */
/* Tabs                                                                */
/* ------------------------------------------------------------------ */

static void default_tabs(escp *s) {
  s->nhtab = 0;
  for (int i = 1; i <= 21; i++) {
    int32_t x = (int32_t)i * 8 * pitch_normal[0];
    if (x >= s->printable_width) break;
    s->htab[s->nhtab++] = x;
  }
  s->nvtab = 0;
}

static void do_htab(escp *s, pe_engine *e) {
  for (int i = 0; i < s->nhtab; i++) {
    if (s->htab[i] > e->x) {
      if (s->htab[i] < s->right_margin) e->x = s->htab[i];
      return;
    }
  }
}

static void do_vtab(escp *s, pe_engine *e) {
  for (int i = 0; i < s->nvtab; i++) {
    if (s->vtab[i] > e->y) {
      feed(s, e, s->vtab[i] - e->y);
      return;
    }
  }
  line_feed(s, e);
}

/* ------------------------------------------------------------------ */
/* Parser                                                              */
/* ------------------------------------------------------------------ */

enum {
  ST_DATA = 0,
  ST_ESC,
  ST_PARAM,
  ST_GFX_HDR,
  ST_GFX_DATA,
  ST_HTABS,
  ST_VTABS,
  ST_SKIP /* swallow n bytes of an unsupported extended command */
};

void escp_reset(escp *s, pe_engine *e) {
  const escp_caps *caps = s->caps;
  uint8_t intl = s->intl;
  memset(s, 0, sizeof *s);
  s->caps = caps;
  s->intl = intl;
  s->lq = (uint8_t)caps->lq_default;
  s->line_spacing = PE_UNITS_PER_INCH / 6;
  s->printable_width = e->cfg.page_width - 2 * e->cfg.left_offset;
  s->printable_height = e->cfg.page_height - 2 * e->cfg.top_offset;
  s->left_margin = 0;
  s->right_margin = s->printable_width;
  s->form_length = s->printable_height;
  s->skip_bottom = 0;
  s->klyz[0] = 0; /* ESC K */
  s->klyz[1] = 1; /* ESC L */
  s->klyz[2] = 2; /* ESC Y */
  s->klyz[3] = 3; /* ESC Z */
  s->upper_ok = 1;
  default_tabs(s);
  e->x = 0;
  e->y = 0;
}

void escp_init(escp *s, const escp_caps *caps, pe_engine *e) {
  memset(s, 0, sizeof *s);
  s->caps = caps;
  escp_reset(s, e);
}

/* Parameter byte counts for the fixed length ESC commands. */
static int param_count(escp *s, uint8_t cmd) {
  switch (cmd) {
    case 'K': case 'L': case 'Y': case 'Z': /* handled as graphics */
      return 2;
    case '*': return 3;
    case '^': return 3;
    case '?': return 2;
    case 'S': case 'W': case 'w': case '-': case 'U': case 'R': case 'A':
    case 'J': case 'j': case 'N': case 'Q': case 'l': case '3': case 'I':
    case 'p': case 'x': case 's': case 't': case 'C': case '!': case 'r':
    case 'a': case 25 /* EM */:
      return 1;
    case 'k': return 1;
    case '%': return 1;  /* select ROM / downloaded font - ignored  */
    case ':': return 3;  /* copy ROM font to RAM - ignored          */
    case '$': return 2;
    case '\\': return 2;
    default:
      (void)s;
      return 0;
  }
}

static void master_select(escp *s, uint8_t n) {
  s->cpi = (n & 0x01) ? 1 : 0;
  s->proportional = (n & 0x02) ? 1 : 0;
  s->condensed = (n & 0x04) ? 1 : 0;
  s->emphasized = (n & 0x08) ? 1 : 0;
  s->dstrike = (n & 0x10) ? 1 : 0;
  s->expanded = (n & 0x20) ? 1 : 0;
  s->italic = (n & 0x40) ? 1 : 0;
  s->underline = (n & 0x80) ? 1 : 0;
}

static void exec_cmd(escp *s, pe_engine *e) {
  const uint8_t *p = s->prm;
  switch (s->cmd) {
    case '@': escp_reset(s, e); break;

    /* line spacing */
    case '0': s->line_spacing = PE_UNITS_PER_INCH / 8; break;
    case '1': s->line_spacing = PE_UNITS_PER_INCH * 7 / 72; break;
    case '2': s->line_spacing = PE_UNITS_PER_INCH / 6; break;
    case '3': s->line_spacing = (int32_t)p[0] * (PE_UNITS_PER_INCH / 216); break;
    case 'A': s->line_spacing = (int32_t)p[0] * s->caps->esc_A_unit; break;
    case 'J': feed(s, e, (int32_t)p[0] * (PE_UNITS_PER_INCH / 216)); break;
    case 'j': feed(s, e, -(int32_t)p[0] * (PE_UNITS_PER_INCH / 216)); break;

    /* page layout */
    case 'C':
      if (p[0] == 0) {
        /* ESC C NUL n: form length in inches, second byte pending */
        s->want = 1;
        s->nprm = 0;
        s->cmd = 'c';
        s->st = ST_PARAM;
        return;
      }
      s->form_length = (int32_t)p[0] * s->line_spacing;
      if (s->form_length > s->printable_height)
        s->form_length = s->printable_height;
      break;
    case 'c':
      s->form_length = (int32_t)p[0] * PE_UNITS_PER_INCH;
      if (s->form_length > s->printable_height)
        s->form_length = s->printable_height;
      break;
    case 'N': s->skip_bottom = (int32_t)p[0] * s->line_spacing; break;
    case 'O': s->skip_bottom = 0; break;
    case 'l': {
      int32_t x = (int32_t)p[0] * pitch_normal[s->cpi];
      if (x < s->right_margin) s->left_margin = x;
      break;
    }
    case 'Q': {
      int32_t x = (int32_t)p[0] * pitch_normal[s->cpi];
      if (x > s->left_margin && x <= s->printable_width) s->right_margin = x;
      break;
    }
    case '$': {
      int32_t x = s->left_margin +
                  ((int32_t)p[0] + 256 * (int32_t)p[1]) * (PE_UNITS_PER_INCH / 60);
      if (x <= s->right_margin) e->x = x;
      break;
    }
    case '\\': {
      int32_t n = (int32_t)p[0] + 256 * (int32_t)p[1];
      if (n > 32767) n -= 65536;
      int32_t unit = s->caps->pins == 24 ? PE_UNITS_PER_INCH / 180
                                         : PE_UNITS_PER_INCH / 120;
      int32_t x = e->x + n * unit;
      if (x >= s->left_margin && x <= s->right_margin) e->x = x;
      break;
    }

    /* style */
    case 'P': s->cpi = 0; break;
    case 'M': s->cpi = 1; break;
    case 'g': s->cpi = 2; break;
    case 'p': s->proportional = p[0] & 1; break;
    case 'x': s->lq = p[0] & 1; break;
    case 'E': s->emphasized = 1; break;
    case 'F': s->emphasized = 0; break;
    case 'G': s->dstrike = 1; break;
    case 'H': s->dstrike = 0; break;
    case '4': s->italic = 1; break;
    case '5': s->italic = 0; break;
    case '-': s->underline = p[0] & 1; break;
    case 'S': s->script = (p[0] & 1) ? 2 : 1; break;
    case 'T': s->script = 0; break;
    case 'W': s->expanded = p[0] & 1; s->expanded_line = 0; break;
    case 'w': s->dheight = p[0] & 1; break;
    case '!': master_select(s, p[0]); break;
    case 'R': s->intl = p[0] <= 12 ? p[0] : 0; break;
    case '6': s->upper_ok = 1; break;
    case '7': s->upper_ok = 0; break;
    case '=': s->msb_mode = 1; break;
    case '>': s->msb_mode = 2; break;
    case '#': s->msb_mode = 0; break;

    /* bit images */
    case 'K': case 'L': case 'Y': case 'Z': {
      static const char order[] = "KLYZ";
      int idx = (int)(strchr(order, (char)s->cmd) - order);
      gfx_start(s, e, s->klyz[idx], (int32_t)p[0] + 256 * (int32_t)p[1]);
      s->st = s->gfx_cols > 0 ? ST_GFX_DATA : ST_DATA;
      return;
    }
    case '*':
      gfx_start(s, e, p[0], (int32_t)p[1] + 256 * (int32_t)p[2]);
      s->st = s->gfx_cols > 0 ? ST_GFX_DATA : ST_DATA;
      return;
    case '^':
      gfx_start9(s, p[0], (int32_t)p[1] + 256 * (int32_t)p[2]);
      s->st = ST_GFX_DATA;
      return;
    case '?':
      if (p[0] >= 'K' && p[0] <= 'Z') {
        static const char order[] = "KLYZ";
        const char *q = strchr(order, (char)p[0]);
        if (q) s->klyz[q - order] = p[1];
      }
      break;

    case 'k':
      for (int i = 0; i < s->caps->ntypefaces; i++)
        if (s->caps->typefaces[i].id == p[0]) s->typeface = (uint8_t)i;
      break;

    /* ignored but consumed */
    case 'U': case 's': case 't': case 'I': case 'r': case 'a':
    case '%': case ':': case 25: case '<': case '9': case '8':
      break;
    default:
      break;
  }
  s->st = ST_DATA;
}

void escp_byte(escp *s, pe_engine *e, uint8_t b) {
  switch (s->st) {
    case ST_GFX_DATA:
      s->gfx_col[s->gfx_col_n++] = b;
      if (s->gfx_col_n >= s->gfx_bpc) {
        s->gfx_col_n = 0;
        gfx_column(s, e);
        if (--s->gfx_cols <= 0) s->st = ST_DATA;
      }
      return;

    case ST_HTABS:
      if (b == 0 || s->nhtab >= ESCP_MAX_HTAB) {
        s->st = ST_DATA;
        return;
      }
      s->htab[s->nhtab++] = (int32_t)b * pitch_normal[s->cpi];
      return;

    case ST_VTABS:
      if (b == 0 || s->nvtab >= ESCP_MAX_VTAB) {
        s->st = ST_DATA;
        return;
      }
      s->vtab[s->nvtab++] = (int32_t)b * s->line_spacing;
      return;

    case ST_SKIP:
      if (--s->want == 0) s->st = ST_DATA;
      return;

    case ST_ESC:
      s->cmd = b;
      s->nprm = 0;
      if (b == 'D') {
        s->nhtab = 0;
        s->st = ST_HTABS;
        return;
      }
      if (b == 'B' || b == 'b') {
        s->nvtab = 0;
        s->st = ST_VTABS;
        return;
      }
      if (b == '(') {
        /* ESC/P2 extended command: ESC ( c nL nH <data>.  We cannot render
         * these, but we must not print their payload as text. */
        s->st = ST_SKIP;
        s->want = 3;
        return;
      }
      s->want = (uint8_t)param_count(s, b);
      if (s->want == 0) {
        exec_cmd(s, e);
        return;
      }
      s->st = ST_PARAM;
      return;

    case ST_PARAM:
      s->prm[s->nprm++] = b;
      if (s->nprm >= s->want) exec_cmd(s, e);
      return;

    default:
      break;
  }

  /* ST_DATA */
  if (s->msb_mode == 1) b &= 0x7F;
  else if (s->msb_mode == 2) b |= 0x80;

  switch (b) {
    case 0x07: return;                      /* BEL   */
    case 0x08:                              /* BS    */
      e->x -= char_width(s);
      if (e->x < s->left_margin) e->x = s->left_margin;
      return;
    case 0x09: do_htab(s, e); return;       /* HT    */
    case 0x0A:                              /* LF    */
      line_feed(s, e);
      e->x = s->left_margin;
      s->expanded_line = 0;
      return;
    case 0x0B: do_vtab(s, e); return;       /* VT    */
    case 0x0C:                              /* FF    */
      form_feed(s, e);
      s->expanded_line = 0;
      return;
    case 0x0D:                              /* CR    */
      carriage_return(s, e);
      return;
    case 0x0E: s->expanded_line = 1; return; /* SO   */
    case 0x0F: s->condensed = 1; return;     /* SI   */
    case 0x11: return;                       /* DC1  */
    case 0x12: s->condensed = 0; return;     /* DC2  */
    case 0x13: return;                       /* DC3  */
    case 0x14: s->expanded_line = 0; return; /* DC4  */
    case 0x18: return;                       /* CAN  */
    case 0x1B: s->st = ST_ESC; return;       /* ESC  */
    case 0x7F: return;                       /* DEL  */
    default: break;
  }
  if (b < 0x20) return;
  if (b >= 0x80 && !s->upper_ok) return;
  put_char(s, e, b);
}

void escp_eoj(escp *s, pe_engine *e) { (void)s; (void)e; }
