/*
 * Epson LQ - generic 24 wire ESC/P letter quality printer
 *
 * The emulation used before the NEC specific FS commands were added: the
 * letter quality artwork of the P6 stand-in fonts, Epson ESC k numbering.
 */
#include "escp.h"
#include "pe/rom_fonts.h"

static const escp_typeface eplq_typefaces[] = {
    {0, &pe_font_prestige, &pe_font_prestige_ps, NULL},  /* Roman      */
    {1, &pe_font_sans, &pe_font_sans_ps, NULL},          /* Sans serif */
    {2, &pe_font_prestige, &pe_font_prestige_ps, NULL},  /* Courier    */
    {3, &pe_font_prestige, &pe_font_prestige_ps, NULL},  /* Prestige   */
    {4, &pe_font_script, &pe_font_script_ps, NULL},      /* Script     */
    {5, &pe_font_ocrb, NULL, NULL},
    {6, &pe_font_ocra, NULL, NULL},
};
static const pe_typeface eplq_names[] = {
    {0, 1, "Roman"}, {1, 1, "Sans serif"}, {4, 1, "Script"},
    {5, 0, "OCR-B"}, {6, 0, "OCR-A"},      {0, 0, NULL},
};
static const escp_caps eplq_caps = {
    .pins = 24,
    .pin_pitch = PE_UNITS_PER_INCH / 180,
    .dot = 18,
    .draft = &pe_font_lq24,
    .lq = &pe_font_prestige,
    .lq_default = 1,
    .typefaces = eplq_typefaces,
    .ntypefaces = (int)(sizeof eplq_typefaces / sizeof eplq_typefaces[0]),
    .gfx8_pitch = PE_UNITS_PER_INCH / 60,
    .has_24bit_gfx = 1,
    .esc_A_unit = PE_UNITS_PER_INCH / 60,
    .nec_fs = 0,
};
static void eplq_init(void *st, pe_engine *e) { escp_init((escp *)st, &eplq_caps, e); }
static void eplq_reset(void *st, pe_engine *e) { escp_reset((escp *)st, e); }
static void eplq_byte(void *st, pe_engine *e, uint8_t b) { escp_byte((escp *)st, e, b); }
static void eplq_eoj(void *st, pe_engine *e) { escp_eoj((escp *)st, e); }

const pe_driver pe_driver_eplq = {
    .id = "eplq",
    .name = "Epson LQ",
    .description = "24 pin ESC/P, letter quality",
    .state_size = sizeof(escp),
    .dot_diameter = 18,
    .typefaces = eplq_names,
    .init = eplq_init,
    .reset = eplq_reset,
    .byte = eplq_byte,
    .eoj = eplq_eoj,
};
