#include <stdio.h>
#include <time.h>

#include "pe/pe.h"

/* "<dir>/<prefix>_YYYYMMDD_HHMMSS.pdf"; "_2", "_3" ... if it already exists. */
int pe_timestamp_path(char *out, size_t n, const char *dir, const char *prefix) {
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  char stamp[32];
  if (!tm || !strftime(stamp, sizeof stamp, "%Y%m%d_%H%M%S", tm)) return 0;
  const char *sep = (dir && *dir) ? "/" : "";
  for (int i = 1; i < 1000; i++) {
    int len;
    if (i == 1)
      len = snprintf(out, n, "%s%s%s_%s.pdf", dir ? dir : "", sep, prefix, stamp);
    else
      len = snprintf(out, n, "%s%s%s_%s_%d.pdf", dir ? dir : "", sep, prefix,
                     stamp, i);
    if (len < 0 || (size_t)len >= n) return 0;
    FILE *f = fopen(out, "rb");
    if (!f) return 1;
    fclose(f);
  }
  return 0;
}
