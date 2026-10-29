/*
 * Epson FX-80 - 9 wire ESC/P, draft only
 *
 * Font from the reconstructed character generator dump (fonts/rom).
 */
#include "escp.h"
#include "pe/rom_fonts.h"

static const pe_typeface fx80_names[] = {{0, 0, NULL}};
static const escp_caps fx80_caps = {
    .pins = 9,
    .pin_pitch = PE_UNITS_PER_INCH / 72,
    .dot = 30,
    .draft = &pe_font_fx80_courier,
    .lq = NULL,
    .gfx8_pitch = PE_UNITS_PER_INCH / 72,
    .has_24bit_gfx = 0,
    .esc_A_unit = PE_UNITS_PER_INCH / 72,
};
static void fx80_init(void *st, pe_engine *e) { escp_init((escp *)st, &fx80_caps, e); }
static void fx80_reset(void *st, pe_engine *e) { escp_reset((escp *)st, e); }
static void fx80_byte(void *st, pe_engine *e, uint8_t b) { escp_byte((escp *)st, e, b); }
static void fx80_eoj(void *st, pe_engine *e) { escp_eoj((escp *)st, e); }

const pe_driver pe_driver_fx80 = {
    .id = "fx80",
    .name = "Epson FX-80",
    .description = "9 pin ESC/P, draft, bit images to 240 dpi",
    .state_size = sizeof(escp),
    .dot_diameter = 30,
    .typefaces = fx80_names,
    .init = fx80_init,
    .reset = fx80_reset,
    .byte = fx80_byte,
    .eoj = fx80_eoj,
};
