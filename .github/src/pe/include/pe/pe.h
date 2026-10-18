/*
 * Printer emulation core: page engine, PDF output and job API.
 *
 * Units: all coordinates are in 1/2160 inch (the common multiple of the
 * pitches of 9 and 24 wire heads and of the ESC/P vertical units).
 */
#ifndef PE_PE_H
#define PE_PE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_UNITS_PER_INCH 2160

typedef enum {
  PE_PAPER_A4 = 0,        /* single sheet DIN A4, 210 x 297 mm        */
  PE_PAPER_A4_ENDLOS = 1, /* fanfold 12" x 240 mm (A4 hoch), 15 mm Lochrand   */
  PE_PAPER_LETTER = 2
} pe_paper;

typedef struct {
  int32_t page_width, page_height; /* paper size                        */
  int32_t left_offset, top_offset; /* unprintable border                */
  int continuous;                  /* fanfold paper                     */
  int compress;                    /* Flate compress page streams       */
} pe_config;

void pe_config_init(pe_config *cfg, pe_paper paper);

struct pe_pdf;
struct pe_driver;

typedef struct pe_engine {
  pe_config cfg;
  int32_t x, y; /* head position, owned by the driver */

  /* private */
  struct pe_pdf *pdf;
  uint8_t *buf;
  size_t len, cap;
  int in_text;
  int32_t cur_d, px, py;
  unsigned long ink;
  int pages;
} pe_engine;

void pe_engine_dot(pe_engine *e, int32_t x, int32_t y, int32_t diameter);
void pe_engine_column(pe_engine *e, int32_t x, int32_t y, uint32_t bits,
                      int pins, int32_t pitch, int32_t diameter);
void pe_engine_hrule(pe_engine *e, int32_t x0, int32_t x1, int32_t y,
                     int32_t thickness);
/* End the current page.  A page without ink is dropped unless `force`. */
void pe_engine_form_feed(pe_engine *e, int force);

/* Unique output name "<dir>/<prefix>_YYYYMMDD_HHMMSS.pdf" (adds _2, _3 on
 * collision).  Returns 1 on success. */
int pe_timestamp_path(char *out, size_t n, const char *dir, const char *prefix);

/* ---- job API: one print job produces one PDF file ------------------- */
typedef struct pe_job pe_job;

pe_job *pe_job_create(const struct pe_driver *drv, const pe_config *cfg,
                      const char *pdf_path);
void pe_job_write(pe_job *j, const uint8_t *data, size_t n);
void pe_job_reset(pe_job *j); /* hardware INIT line */
/* Finish the job, close the PDF, free everything.  Returns page count. */
int pe_job_finish(pe_job *j);

#ifdef __cplusplus
}
#endif
#endif
