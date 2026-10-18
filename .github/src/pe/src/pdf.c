/*
 * Minimal PDF writer.  Layout:
 *   1 Catalog  2 Pages  3 Type3 font "F1" (one dot glyph)  4 dot glyph
 *   5 Info     6+2i page content   7+2i page
 * Objects 1 and 2 plus xref/trailer form a "tail" that is rewritten after
 * each page, so the file on disk is always a valid PDF even if the emulator
 * is killed in the middle of a job.
 */
#include "pdf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef PE_HAVE_ZLIB
#include <zlib.h>
#endif

struct pe_pdf {
  char *path;
  FILE *fp;
  int compress;
  long *offs;
  int cap, npages;
  long tail;
};

void pdf_fmt100(char *out, int32_t v) {
  const char *sign = "";
  if (v < 0) { sign = "-"; v = -v; }
  sprintf(out, "%s%d.%02d", sign, (int)(v / 100), (int)(v % 100));
}

struct pe_pdf *pdf_create(const char *path, int compress) {
  struct pe_pdf *p = calloc(1, sizeof *p);
  if (!p) return NULL;
  p->path = malloc(strlen(path) + 1);
  if (!p->path) { free(p); return NULL; }
  strcpy(p->path, path);
  p->compress = compress;
  return p;
}

static int ensure(struct pe_pdf *p, int obj) {
  if (obj < p->cap) return 1;
  int n = p->cap ? p->cap : 64;
  while (n <= obj) n *= 2;
  long *o = realloc(p->offs, (size_t)n * sizeof *o);
  if (!o) return 0;
  p->offs = o;
  p->cap = n;
  return 1;
}

static int begin_obj(struct pe_pdf *p, int obj) {
  if (!ensure(p, obj)) return 0;
  p->offs[obj] = ftell(p->fp);
  fprintf(p->fp, "%d 0 obj\n", obj);
  return 1;
}

static int open_file(struct pe_pdf *p) {
  p->fp = fopen(p->path, "wb+");
  if (!p->fp) return 0;
  fputs("%PDF-1.4\n%\xE2\xE3\xCF\xD3\n", p->fp);

  /* 4: dot glyph, a unit circle centred on the glyph origin */
  static const char dot[] =
      "0 0 -0.5 -0.5 0.5 0.5 d1\n"
      "0.5 0 m 0.5 0.27614 0.27614 0.5 0 0.5 c "
      "-0.27614 0.5 -0.5 0.27614 -0.5 0 c "
      "-0.5 -0.27614 -0.27614 -0.5 0 -0.5 c "
      "0.27614 -0.5 0.5 -0.27614 0.5 0 c f\n";
  if (!begin_obj(p, 4)) return 0;
  fprintf(p->fp, "<< /Length %d >>\nstream\n%sendstream\nendobj\n",
          (int)(sizeof dot - 1), dot);
  /* 3: Type3 font */
  if (!begin_obj(p, 3)) return 0;
  fputs("<< /Type /Font /Subtype /Type3 /FontBBox [-0.5 -0.5 0.5 0.5]\n"
        "/FontMatrix [1 0 0 1 0 0] /CharProcs << /d 4 0 R >>\n"
        "/Encoding << /Type /Encoding /Differences [97 /d] >>\n"
        "/FirstChar 97 /LastChar 97 /Widths [0] >>\nendobj\n", p->fp);
  /* 5: info */
  if (!begin_obj(p, 5)) return 0;
  fputs("<< /Title (Printer output) /Creator (Hatari PDF printer emulation) "
        "/Producer (pe printer engine, NEC Pinwriter P6) >>\nendobj\n", p->fp);
  p->tail = ftell(p->fp);
  return 1;
}

static int write_tail(struct pe_pdf *p) {
  int nobj = 5 + 2 * p->npages;
  fseek(p->fp, p->tail, SEEK_SET);
  if (!begin_obj(p, 2)) return 0;
  fprintf(p->fp, "<< /Type /Pages /Count %d /Kids [", p->npages);
  for (int i = 0; i < p->npages; i++) fprintf(p->fp, "%d 0 R ", 7 + 2 * i);
  fputs("] >>\nendobj\n", p->fp);
  if (!begin_obj(p, 1)) return 0;
  fputs("<< /Type /Catalog /Pages 2 0 R >>\nendobj\n", p->fp);
  long xref = ftell(p->fp);
  fprintf(p->fp, "xref\n0 %d\n0000000000 65535 f \n", nobj + 1);
  for (int i = 1; i <= nobj; i++) fprintf(p->fp, "%010ld 00000 n \n", p->offs[i]);
  fprintf(p->fp, "trailer\n<< /Size %d /Root 1 0 R /Info 5 0 R >>\n"
                 "startxref\n%ld\n%%%%EOF\n", nobj + 1, xref);
  return fflush(p->fp) == 0;
}

int pdf_add_page(struct pe_pdf *p, int32_t w100, int32_t h100,
                 const uint8_t *content, size_t len) {
  if (!p) return 0;
  if (!p->fp && !open_file(p)) return 0;
  fseek(p->fp, p->tail, SEEK_SET);

  const uint8_t *data = content;
  size_t dlen = len;
  const char *filter = "";
  uint8_t *z = NULL;
#ifdef PE_HAVE_ZLIB
  if (p->compress) {
    uLongf zl = compressBound((uLong)len);
    z = malloc(zl);
    if (z && compress2(z, &zl, content, (uLong)len, 6) == Z_OK) {
      data = z; dlen = zl; filter = " /Filter /FlateDecode";
    }
  }
#endif
  int cobj = 6 + 2 * p->npages, pobj = cobj + 1;
  if (!begin_obj(p, cobj)) { free(z); return 0; }
  fprintf(p->fp, "<< /Length %lu%s >>\nstream\n", (unsigned long)dlen, filter);
  fwrite(data, 1, dlen, p->fp);
  fputs("\nendstream\nendobj\n", p->fp);
  free(z);

  char w[32], h[32];
  pdf_fmt100(w, w100);
  pdf_fmt100(h, h100);
  if (!begin_obj(p, pobj)) return 0;
  fprintf(p->fp, "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 %s %s]\n"
                 "/Resources << /Font << /F1 3 0 R >> >> /Contents %d 0 R >>\n"
                 "endobj\n", w, h, cobj);
  p->npages++;
  p->tail = ftell(p->fp);
  return write_tail(p);
}

void pdf_close(struct pe_pdf *p) {
  if (!p) return;
  if (p->fp) fclose(p->fp);
  free(p->offs);
  free(p->path);
  free(p);
}
