/*
 * Shared ESC/P interpreter.
 *
 * The Epson FX-80 and the NEC P6 speak dialects of the same language: the
 * command set is nearly identical, they differ in head geometry (9 wires at
 * 1/72" vs 24 wires at 1/180"), available print qualities and the set of
 * bit image densities.  Both drivers are therefore thin caps tables around
 * this interpreter.
 */
#ifndef PE_ESCP_H
#define PE_ESCP_H

#include <stdint.h>

#include "pe/driver.h"
#include "pe/font.h"
#include "pe/pe.h"

/* A letter quality typeface as ESC k selects it. */
typedef struct {
  uint8_t id;              /* ESC k parameter                          */
  const pe_font *fixed;
  const pe_font *prop;     /* proportional artwork, NULL = use fixed   */
} escp_typeface;

typedef struct {
  int pins;                /* print wires: 9 or 24                     */
  int32_t pin_pitch;       /* vertical wire spacing, units             */
  int32_t dot;             /* dot diameter, units                      */
  const pe_font *draft;
  const pe_font *lq;       /* NULL when the printer is draft only      */
  int lq_default;          /* print quality after power up and ESC @   */
  const escp_typeface *typefaces; /* ESC k choices, the first is `lq`  */
  int ntypefaces;
  int32_t gfx8_pitch;      /* vertical pitch of the 8 wire image modes */
  int has_24bit_gfx;       /* ESC * modes 32..73                       */
  int32_t esc_A_unit;      /* ESC A n feeds n * this                   */
} escp_caps;

#define ESCP_MAX_HTAB 32
#define ESCP_MAX_VTAB 16

typedef struct {
  const escp_caps *caps;

  /* parser */
  uint8_t st;
  uint8_t cmd;
  uint8_t prm[4];
  uint8_t nprm, want;
  uint32_t skip; /* payload bytes of an extended command still to swallow */

  /* bit image transfer in progress */
  int32_t gfx_cols;
  uint8_t gfx_bpc, gfx_pins, gfx_col_n;
  uint8_t gfx_col[3];
  int32_t gfx_pitch, gfx_vpitch;
  uint8_t klyz[4]; /* densities currently assigned to ESC K/L/Y/Z      */

  /* tab stops */
  int32_t htab[ESCP_MAX_HTAB];
  int32_t vtab[ESCP_MAX_VTAB];
  uint8_t nhtab, nvtab;

  /* print style */
  uint8_t cpi;      /* 0 = 10, 1 = 12, 2 = 15                          */
  uint8_t condensed, expanded, expanded_line, emphasized, dstrike;
  uint8_t italic, underline, script, proportional, lq, dheight;
  uint8_t typeface;  /* index into caps->typefaces                    */
  uint8_t intl, msb_mode, upper_ok, autofeed;
  uint8_t fs_wide;   /* FS E 1: NEC wide print (1.67 x, 6 cpi at pica) */
  uint8_t chartable; /* codes 128..255: 0 Atari ST, 1 Latin-1, 2 PC 850,
                        3 PC 437 (ESC t); survives ESC @ like a DIP switch */

  /* page geometry, all in units */
  int32_t line_spacing;
  int32_t left_margin, right_margin;
  int32_t form_length, skip_bottom;
  int32_t printable_width, printable_height;

  uint32_t chars_printed;
} escp;

void escp_init(escp *s, const escp_caps *caps, pe_engine *e);
void escp_reset(escp *s, pe_engine *e);
void escp_byte(escp *s, pe_engine *e, uint8_t b);
void escp_eoj(escp *s, pe_engine *e);

#endif /* PE_ESCP_H */
