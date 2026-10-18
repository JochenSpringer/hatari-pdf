/*
 * NEC Pinwriter P6 - 24 wires at 1/180", letter quality, 360 dpi graphics.
 *
 * The P6 speaks NEC's ESC/P superset; everything the FX-80 understands works
 * here too, plus the 24 wire bit image modes and the finer vertical motion
 * unit (ESC A counts 1/60" instead of 1/72").
 */
#include "escp.h"

/*
 * ESC k typefaces.  The numbers are Epson's (0 roman, 1 sans serif, 2 courier,
 * 3 prestige, 4 script, 5 OCR-B, 6 OCR-A) plus the ones the P6 plus firmware
 * uses for ITC Souvenir (0x0F), Helvette (0x10), Times (0x12) and Prestige
 * Elite (0x13).  The serif faces all print in Prestige.
 */
static const escp_typeface p6_typefaces[] = {
    {0, &pe_font_prestige, &pe_font_prestige_ps},
    {1, &pe_font_sans, &pe_font_sans_ps},
    {2, &pe_font_prestige, &pe_font_prestige_ps},
    {3, &pe_font_prestige, &pe_font_prestige_ps},
    {4, &pe_font_script, &pe_font_script_ps},
    {5, &pe_font_ocrb, NULL},
    {6, &pe_font_ocra, NULL},
    {0x0F, &pe_font_prestige, &pe_font_prestige_ps},
    {0x10, &pe_font_sans, &pe_font_sans_ps},
    {0x12, &pe_font_prestige, &pe_font_prestige_ps},
    {0x13, &pe_font_prestige, &pe_font_prestige_ps},
};

static const pe_typeface p6_typeface_names[] = {
    {0, 1, "Prestige"}, {1, 1, "Sans serif"}, {4, 1, "Script"},
    {5, 0, "OCR-B"},    {6, 0, "OCR-A"},      {0, 0, NULL},
};

static const escp_caps p6_caps = {
    .pins = 24,
    .pin_pitch = PE_UNITS_PER_INCH / 180,
    .dot = 18, /* ~0.2 mm wire */
    .draft = &pe_font_lq24,
    .lq = &pe_font_prestige,
    .lq_default = 1, /* the P6 powers up in letter quality */
    .typefaces = p6_typefaces,
    .ntypefaces = (int)(sizeof p6_typefaces / sizeof p6_typefaces[0]),
    .gfx8_pitch = PE_UNITS_PER_INCH / 60,
    .has_24bit_gfx = 1,
    .esc_A_unit = PE_UNITS_PER_INCH / 60,
};

static void p6_init(void *st, pe_engine *e) {
  escp_init((escp *)st, &p6_caps, e);
}
static void p6_reset(void *st, pe_engine *e) { escp_reset((escp *)st, e); }
static void p6_byte(void *st, pe_engine *e, uint8_t b) {
  escp_byte((escp *)st, e, b);
}
static void p6_eoj(void *st, pe_engine *e) { escp_eoj((escp *)st, e); }

const pe_driver pe_driver_p6 = {
    .id = "p6",
    .name = "NEC Pinwriter P6",
    .description = "24 pin ESC/P, letter quality, bit image graphics to 360 dpi",
    .state_size = sizeof(escp),
    .dot_diameter = 18,
    .typefaces = p6_typeface_names,
    .init = p6_init,
    .reset = p6_reset,
    .byte = p6_byte,
    .eoj = p6_eoj,
};
