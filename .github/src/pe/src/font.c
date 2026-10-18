#include <stddef.h>
#include "pe/font.h"

const uint32_t *pe_font_glyph(const pe_font *f, uint16_t code, uint8_t *adv) {
  for (int pass = 0; pass < 2; pass++) {
    uint16_t want = pass ? 0x20 : code;
    int lo = 0, hi = (int)f->count - 1;
    while (lo <= hi) {
      int mid = (lo + hi) / 2;
      if (f->codes[mid] == want) {
        if (adv) *adv = f->advance ? f->advance[mid] : f->cell;
        return f->cols + (size_t)mid * f->width;
      }
      if (f->codes[mid] < want) lo = mid + 1; else hi = mid - 1;
    }
  }
  return NULL;
}
