/*
 * Star LC24-10 - 24 wire, Epson LQ command set
 *
 * Typefaces: Courier, Prestige (each with italic), Script, Orator; draft with
 * bold and italic artwork.
 */
#include "escp.h"
#include "pe/rom_fonts.h"

static const escp_typeface lc24_typefaces[] = {
    {0, &pe_font_lc24_courier, NULL, &pe_font_lc24_courier_italic},
    {1, &pe_font_lc24_orator, NULL, NULL},
    {2, &pe_font_lc24_courier, NULL, &pe_font_lc24_courier_italic},
    {3, &pe_font_lc24_prestige, NULL, &pe_font_lc24_prestige_italic},
    {4, &pe_font_lc24_script, NULL, NULL},
    {7, &pe_font_lc24_orator, NULL, NULL},
};
static const pe_typeface lc24_names[] = {
    {0, 0, "Courier"}, {3, 0, "Prestige"}, {4, 0, "Script"}, {7, 0, "Orator"}, {0, 0, NULL},
};
static const escp_caps lc24_caps = {
    .pins = 24,
    .pin_pitch = PE_UNITS_PER_INCH / 180,
    .dot = 18,
    .draft = &pe_font_lc24_draft,
    .lq = &pe_font_lc24_courier,
    .lq_default = 1,
    .typefaces = lc24_typefaces,
    .ntypefaces = 6,
    .gfx8_pitch = PE_UNITS_PER_INCH / 60,
    .has_24bit_gfx = 1,
    .esc_A_unit = PE_UNITS_PER_INCH / 60,
    .draft_italic = &pe_font_lc24_draft_italic,
    .draft_bold = &pe_font_lc24_draft_bold,
};
static void lc24_init(void *st, pe_engine *e) { escp_init((escp *)st, &lc24_caps, e); }
static void lc24_reset(void *st, pe_engine *e) { escp_reset((escp *)st, e); }
static void lc24_byte(void *st, pe_engine *e, uint8_t b) { escp_byte((escp *)st, e, b); }
static void lc24_eoj(void *st, pe_engine *e) { escp_eoj((escp *)st, e); }

const pe_driver pe_driver_lc24 = {
    .id = "lc24",
    .name = "Star LC24-10",
    .description = "24 pin, Courier/Prestige/Script/Orator",
    .state_size = sizeof(escp),
    .dot_diameter = 18,
    .typefaces = lc24_names,
    .init = lc24_init,
    .reset = lc24_reset,
    .byte = lc24_byte,
    .eoj = lc24_eoj,
};
