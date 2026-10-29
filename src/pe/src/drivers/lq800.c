/*
 * Epson LQ-800 - 24 wire ESC/P, draft + Roman + Sans serif
 *
 * Fonts from the reconstructed character generator dumps (fonts/rom).
 */
#include "escp.h"
#include "pe/rom_fonts.h"

static const escp_typeface lq800_typefaces[] = {
    {0, &pe_font_lq800_roman, NULL, NULL},
    {1, &pe_font_lq800_sans, NULL, NULL},
};
static const pe_typeface lq800_names[] = {
    {0, 0, "Roman"}, {1, 0, "Sans serif"}, {0, 0, NULL},
};
static const escp_caps lq800_caps = {
    .pins = 24,
    .pin_pitch = PE_UNITS_PER_INCH / 180,
    .dot = 18,
    .draft = &pe_font_lq800_draft,
    .lq = &pe_font_lq800_roman,
    .lq_default = 1,
    .typefaces = lq800_typefaces,
    .ntypefaces = 2,
    .gfx8_pitch = PE_UNITS_PER_INCH / 60,
    .has_24bit_gfx = 1,
    .esc_A_unit = PE_UNITS_PER_INCH / 60,
};
static void lq800_init(void *st, pe_engine *e) { escp_init((escp *)st, &lq800_caps, e); }
static void lq800_reset(void *st, pe_engine *e) { escp_reset((escp *)st, e); }
static void lq800_byte(void *st, pe_engine *e, uint8_t b) { escp_byte((escp *)st, e, b); }
static void lq800_eoj(void *st, pe_engine *e) { escp_eoj((escp *)st, e); }

const pe_driver pe_driver_lq800 = {
    .id = "lq800",
    .name = "Epson LQ-800",
    .description = "24 pin ESC/P, Roman and Sans serif, bit images to 360 dpi",
    .state_size = sizeof(escp),
    .dot_diameter = 18,
    .typefaces = lq800_names,
    .init = lq800_init,
    .reset = lq800_reset,
    .byte = lq800_byte,
    .eoj = lq800_eoj,
};
