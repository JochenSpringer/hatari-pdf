/* Page engine: turns dots and rules into PDF page content. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdf.h"
#include "pe/driver.h"
#include "pe/pe.h"

/* 1/2160 inch = 1/30 pt  ->  hundredths of a point */
static int32_t pt100(int32_t u) { return (int32_t)(((int64_t)u * 10 + 1) / 3); }

void pe_config_init(pe_config *c, pe_paper paper) {
  memset(c, 0, sizeof *c);
  c->compress = 1;
  switch (paper) {
    case PE_PAPER_LETTER:
      c->page_width = PE_UNITS_PER_INCH * 17 / 2;
      c->page_height = PE_UNITS_PER_INCH * 11;
      break;
    case PE_PAPER_A4_ENDLOS_GRUEN:
      c->greenbar = 1;
      /* fall through */
    case PE_PAPER_A4_ENDLOS:
      /* DIN-Computerpapier "12 Zoll x 240 mm (A4 hoch)": 240 mm wide with a
       * 15 mm tractor strip (Fuehrungslochrand) on each side, 12 inch pitch
       * between the cross perforations.  Printing area = 210 mm. */
      c->continuous = 1;
      c->page_width = 20409;                   /* 240 mm */
      c->page_height = PE_UNITS_PER_INCH * 12; /* 12 inch = 304.8 mm */
      break;
    default:
      c->page_width = 17858;  /* 210 mm */
      c->page_height = 25256; /* 297 mm */
      break;
  }
  /* single sheet: 1.3 mm the head cannot reach; fanfold: the tractor strips */
  c->left_offset = c->continuous ? 1276 /* 15 mm */ : PE_UNITS_PER_INCH / 20;
  c->top_offset = PE_UNITS_PER_INCH / 6; /* 4.2 mm top of form */
}

static void put(pe_engine *e, const char *s, size_t n) {
  if (e->len + n > e->cap) {
    size_t c = e->cap ? e->cap : 65536;
    while (c < e->len + n) c *= 2;
    uint8_t *b = realloc(e->buf, c);
    if (!b) return; /* out of memory: drop output */
    e->buf = b;
    e->cap = c;
  }
  memcpy(e->buf + e->len, s, n);
  e->len += n;
}
static void puts_(pe_engine *e, const char *s) { put(e, s, strlen(s)); }

static int32_t page_h100(const pe_engine *e) { return pt100(e->cfg.page_height); }

static void end_text(pe_engine *e) {
  if (e->in_text) { puts_(e, "ET\n"); e->in_text = 0; }
}

void pe_engine_dot(pe_engine *e, int32_t x, int32_t y, int32_t d) {
  if (d <= 0) return;
  int32_t X = pt100(e->cfg.left_offset + x);
  int32_t Y = page_h100(e) - pt100(e->cfg.top_offset + y);
  char a[32], b[32], line[96];
  if (!e->in_text) {
    puts_(e, "BT\n");
    e->in_text = 1;
    e->px = e->py = 0;
    e->cur_d = 0;
  }
  if (e->cur_d != d) {
    int32_t s = pt100(d);
    if (s < 1) s = 1;
    pdf_fmt100(a, s);
    snprintf(line, sizeof line, "/F1 %s Tf\n", a);
    puts_(e, line);
    e->cur_d = d;
  }
  pdf_fmt100(a, X - e->px);
  pdf_fmt100(b, Y - e->py);
  e->px = X;
  e->py = Y;
  snprintf(line, sizeof line, "%s %s Td (a) Tj\n", a, b);
  puts_(e, line);
  e->ink++;
}

void pe_engine_column(pe_engine *e, int32_t x, int32_t y, uint32_t bits,
                      int pins, int32_t pitch, int32_t d) {
  for (int i = 0; i < pins && i < 32; i++)
    if (bits & (1u << i)) pe_engine_dot(e, x, y + i * pitch, d);
}

void pe_engine_hrule(pe_engine *e, int32_t x0, int32_t x1, int32_t y,
                     int32_t th) {
  if (x1 < x0) { int32_t t = x0; x0 = x1; x1 = t; }
  if (th <= 0) th = 1;
  end_text(e);
  int32_t X = pt100(e->cfg.left_offset + x0);
  int32_t W = pt100(x1 - x0) + pt100(th);
  int32_t H = pt100(th);
  int32_t Y = page_h100(e) - pt100(e->cfg.top_offset + y) - H / 2;
  char a[24], b[24], c[24], d[24], line[128];
  pdf_fmt100(a, X - H / 2);
  pdf_fmt100(b, Y);
  pdf_fmt100(c, W);
  pdf_fmt100(d, H);
  snprintf(line, sizeof line, "%s %s %s %s re f\n", a, b, c, d);
  puts_(e, line);
  e->ink++;
}

/*
 * Fanfold paper: draw the tractor strips the way the sheet looks in the
 * printer - transport holes (4 mm, 12.7 mm pitch, centred 7.5 mm from the
 * edge), the longitudinal perforation at the strip border and the cross
 * perforation at the top and bottom of the form.  Light grey, behind the dots.
 */
static void circle(pe_engine *t, int32_t cx, int32_t cy, int32_t r) {
  int32_t k = (int32_t)((int64_t)r * 5523 / 10000);
  char b[8][32], line[640];
  pdf_fmt100(b[0], cx + r); pdf_fmt100(b[1], cy);
  pdf_fmt100(b[2], cy + k); pdf_fmt100(b[3], cx + k);
  pdf_fmt100(b[4], cy + r); pdf_fmt100(b[5], cx);
  pdf_fmt100(b[6], cx - k); pdf_fmt100(b[7], cx - r);
  snprintf(line, sizeof line, "%s %s m %s %s %s %s %s %s c ", b[0], b[1], b[0],
           b[2], b[3], b[4], b[5], b[4]);
  puts_(t, line);
  snprintf(line, sizeof line, "%s %s %s %s %s %s c ", b[6], b[4], b[7], b[2],
           b[7], b[1]);
  puts_(t, line);
  pdf_fmt100(b[2], cy - k); pdf_fmt100(b[4], cy - r);
  snprintf(line, sizeof line, "%s %s %s %s %s %s c %s %s %s %s %s %s c S\n",
           b[7], b[2], b[6], b[4], b[5], b[4], b[3], b[4], b[0], b[2], b[0],
           b[1]);
  puts_(t, line);
}

static void hline(pe_engine *t, int32_t x0, int32_t x1, int32_t y) {
  char a[32], b[32], c[32], line[256];
  pdf_fmt100(a, x0);
  pdf_fmt100(b, y);
  pdf_fmt100(c, x1);
  snprintf(line, sizeof line, "%s %s m %s %s l S\n", a, b, c, b);
  puts_(t, line);
}

/*
 * Fanfold paper as it comes out of the printer: tractor strips with 4 mm
 * transport holes every 1/2 inch, the perforation to tear them off, the
 * cross perforation between the sheets and, for "Lesestreifen" paper, a
 * green bar on every second line (6 lines per inch).
 */
static void tractor_strips(pe_engine *t, int32_t w100, int32_t h100,
                           int32_t top100, int green) {
  const int32_t strip = 4252, hole_x = 2126, hole_r = 567, pitch = 3600;
  const int32_t row = 1200; /* one line at 6 lpi = 1/6 inch = 12 pt */
  char a[32], b[32], c[32], d[32], line[200];

  if (green) {
    puts_(t, "q 0.86 0.94 0.86 rg\n");
    for (int32_t y = top100 + row; y + row <= h100; y += 2 * row) {
      pdf_fmt100(a, strip);
      pdf_fmt100(b, h100 - y - row);
      pdf_fmt100(c, w100 - 2 * strip);
      pdf_fmt100(d, row);
      snprintf(line, sizeof line, "%s %s %s %s re f\n", a, b, c, d);
      puts_(t, line);
    }
    puts_(t, "0.16 0.52 0.30 RG 0.5 w\n");
    for (int32_t y = top100 + row; y + row <= h100; y += 2 * row) {
      hline(t, strip, w100 - strip, h100 - y);
      hline(t, strip, w100 - strip, h100 - y - row);
    }
    puts_(t, "Q\n");
  }

  puts_(t, "q 0 G 0.8 w\n");
  for (int32_t y = pitch / 2; y < h100; y += pitch) {
    circle(t, hole_x, y, hole_r);
    circle(t, w100 - hole_x, y, hole_r);
  }
  puts_(t, "[3 2] 0 d 0.4 w\n");
  pdf_fmt100(a, strip);
  pdf_fmt100(b, h100);
  snprintf(line, sizeof line, "%s 0 m %s %s l S\n", a, a, b);
  puts_(t, line);
  pdf_fmt100(a, w100 - strip);
  snprintf(line, sizeof line, "%s 0 m %s %s l S\n", a, a, b);
  puts_(t, line);
  pdf_fmt100(a, w100);
  pdf_fmt100(b, h100 - 30);
  snprintf(line, sizeof line, "0 30 m %s 30 l S 0 %s m %s %s l S\n", a, b, a, b);
  puts_(t, line);
  puts_(t, "Q\n");
}

static void flush_page(pe_engine *e) {
  end_text(e);
  if (!e->pdf) return;
  if (e->cfg.continuous) {
    pe_engine t;
    memset(&t, 0, sizeof t);
    tractor_strips(&t, pt100(e->cfg.page_width), page_h100(e),
                   pt100(e->cfg.top_offset), e->cfg.greenbar);
    put(&t, (const char *)e->buf, e->len);
    free(e->buf);
    e->buf = t.buf;
    e->len = t.len;
    e->cap = t.cap;
  }
  if (pdf_add_page(e->pdf, pt100(e->cfg.page_width), page_h100(e), e->buf,
                   e->len))
    e->pages++;
  e->len = 0;
  e->ink = 0;
}

void pe_engine_form_feed(pe_engine *e, int force) {
  if (e->ink || force) flush_page(e);
}

/* ------------------------------------------------------------------ */

struct pe_job {
  const pe_driver *drv;
  pe_engine e;
  void *state;
};

pe_job *pe_job_create(const pe_driver *drv, const pe_config *cfg,
                      const char *path) {
  pe_job *j = calloc(1, sizeof *j);
  if (!j) return NULL;
  j->drv = drv;
  j->e.cfg = *cfg;
  j->e.pdf = pdf_create(path, cfg->compress);
  j->state = calloc(1, drv->state_size);
  if (!j->e.pdf || !j->state) {
    pdf_close(j->e.pdf);
    free(j->state);
    free(j);
    return NULL;
  }
  drv->init(j->state, &j->e);
  return j;
}

void pe_job_write(pe_job *j, const uint8_t *d, size_t n) {
  for (size_t i = 0; i < n; i++) j->drv->byte(j->state, &j->e, d[i]);
}

void pe_job_reset(pe_job *j) { j->drv->reset(j->state, &j->e); }

int pe_job_finish(pe_job *j) {
  if (!j) return 0;
  j->drv->eoj(j->state, &j->e);
  pe_engine_form_feed(&j->e, 0);
  int pages = j->e.pages;
  pdf_close(j->e.pdf);
  free(j->e.buf);
  free(j->state);
  free(j);
  return pages;
}
