#include <string.h>

#include "pe/driver.h"

static const pe_driver *const drivers[] = {
    &pe_driver_p6,
    NULL,
};

const pe_driver *const *pe_drivers(void) { return drivers; }

const pe_driver *pe_driver_find(const char *id) {
  for (const pe_driver *const *d = drivers; *d; d++)
    if (!strcmp((*d)->id, id)) return *d;
  return NULL;
}
