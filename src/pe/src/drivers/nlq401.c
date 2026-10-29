/*
 * Schneider / Amstrad NLQ401 - 9 wire, draft and near letter quality
 *
 * Epson FX-80 compatible; ESC x 1 selects the NLQ artwork (two passes).
 */
#include "escp.h"
#include "pe/rom_fonts.h"

static const escp_typeface nlq401_typefaces[] = {
    {0, &pe_font_nlq401_nlq, NULL, NULL},
};
static const pe_typeface nlq401_names[] = {{0, 0, "NLQ"}, {0, 0, NULL}};
static const escp_caps nlq401_caps = {
    .pins = 9,
    .pin_pitch = PE_UNITS_PER_INCH / 72,
    .dot = 30,
    .draft = &pe_font_nlq401_draft,
    .lq = &pe_font_nlq401_nlq,
    .lq_default = 0,
    .typefaces = nlq401_typefaces,
    .ntypefaces = 1,
    .gfx8_pitch = PE_UNITS_PER_INCH / 72,
    .has_24bit_gfx = 0,
    .esc_A_unit = PE_UNITS_PER_INCH / 72,
};
static void nlq401_init(void *st, pe_engine *e) { escp_init((escp *)st, &nlq401_caps, e); }
static void nlq401_reset(void *st, pe_engine *e) { escp_reset((escp *)st, e); }
static void nlq401_byte(void *st, pe_engine *e, uint8_t b) { escp_byte((escp *)st, e, b); }
static void nlq401_eoj(void *st, pe_engine *e) { escp_eoj((escp *)st, e); }

const pe_driver pe_driver_nlq401 = {
    .id = "nlq401",
    .name = "Schneider NLQ401",
    .description = "9 pin ESC/P, draft and NLQ",
    .state_size = sizeof(escp),
    .dot_diameter = 30,
    .typefaces = nlq401_names,
    .init = nlq401_init,
    .reset = nlq401_reset,
    .byte = nlq401_byte,
    .eoj = nlq401_eoj,
};
