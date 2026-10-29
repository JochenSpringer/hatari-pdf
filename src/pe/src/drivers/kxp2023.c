/*
 * Panasonic KX-P2023 - 24 wire, Epson LQ command set
 *
 * Typefaces: Courier, Prestige, Bold PS, Script (reconstructed ROM dumps).
 * ESC k numbers follow the Epson LQ convention (assumption, see docs).
 */
#include "escp.h"
#include "pe/rom_fonts.h"

static const escp_typeface kxp_typefaces[] = {
    {0, &pe_font_kxp_courier, NULL, NULL},
    {1, &pe_font_kxp_boldps, NULL, NULL},
    {2, &pe_font_kxp_courier, NULL, NULL},
    {3, &pe_font_kxp_prestige, NULL, NULL},
    {4, &pe_font_kxp_script, NULL, NULL},
};
static const pe_typeface kxp_names[] = {
    {0, 0, "Courier"}, {1, 0, "Bold PS"}, {3, 0, "Prestige"}, {4, 0, "Script"}, {0, 0, NULL},
};
static const escp_caps kxp_caps = {
    .pins = 24,
    .pin_pitch = PE_UNITS_PER_INCH / 180,
    .dot = 18,
    .draft = &pe_font_kxp_draft,
    .lq = &pe_font_kxp_courier,
    .lq_default = 1,
    .typefaces = kxp_typefaces,
    .ntypefaces = 5,
    .gfx8_pitch = PE_UNITS_PER_INCH / 60,
    .has_24bit_gfx = 1,
    .esc_A_unit = PE_UNITS_PER_INCH / 60,
};
static void kxp_init(void *st, pe_engine *e) { escp_init((escp *)st, &kxp_caps, e); }
static void kxp_reset(void *st, pe_engine *e) { escp_reset((escp *)st, e); }
static void kxp_byte(void *st, pe_engine *e, uint8_t b) { escp_byte((escp *)st, e, b); }
static void kxp_eoj(void *st, pe_engine *e) { escp_eoj((escp *)st, e); }

const pe_driver pe_driver_kxp = {
    .id = "kxp2023",
    .name = "Panasonic KX-P2023",
    .description = "24 pin, Courier/Prestige/Bold PS/Script",
    .state_size = sizeof(escp),
    .dot_diameter = 18,
    .typefaces = kxp_names,
    .init = kxp_init,
    .reset = kxp_reset,
    .byte = kxp_byte,
    .eoj = kxp_eoj,
};
