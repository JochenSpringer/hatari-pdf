/* prn2pdf: render a captured printer stream (.prn) to PDF. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pe/driver.h"
#include "pe/pe.h"

int main(int argc, char **argv) {
  const char *model = "p6", *in = NULL, *out = NULL, *dir = NULL;
  char auto_name[1024];
  pe_paper paper = PE_PAPER_A4;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-m") && i + 1 < argc) model = argv[++i];
    else if (!strcmp(argv[i], "-o") && i + 1 < argc) out = argv[++i];
    else if (!strcmp(argv[i], "-d") && i + 1 < argc) dir = argv[++i];
    else if (!strcmp(argv[i], "-e")) paper = PE_PAPER_A4_ENDLOS;
    else if (!strcmp(argv[i], "-l")) paper = PE_PAPER_LETTER;
    else if (!strcmp(argv[i], "-L")) {
      for (const pe_driver *const *d = pe_drivers(); *d; d++)
        printf("%-8s %s\n", (*d)->id, (*d)->name);
      return 0;
    } else in = argv[i];
  }
  if (!out && dir && pe_timestamp_path(auto_name, sizeof auto_name, dir, "print"))
    out = auto_name;
  if (!in || !out) {
    fprintf(stderr, "usage: prn2pdf [-m model] [-e endless] [-l letter] "
                    "(-o out.pdf | -d dir) in.prn   (-L lists models)\n");
    return 2;
  }
  const pe_driver *drv = pe_driver_find(model);
  if (!drv) { fprintf(stderr, "unknown model %s\n", model); return 2; }
  FILE *f = fopen(in, "rb");
  if (!f) { perror(in); return 1; }
  pe_config cfg;
  pe_config_init(&cfg, paper);
  pe_job *j = pe_job_create(drv, &cfg, out);
  if (!j) { fprintf(stderr, "cannot create job\n"); return 1; }
  uint8_t buf[4096];
  size_t n;
  while ((n = fread(buf, 1, sizeof buf, f)) > 0) pe_job_write(j, buf, n);
  fclose(f);
  int pages = pe_job_finish(j);
  printf("%d page(s) -> %s\n", pages, out);
  return pages > 0 ? 0 : 1;
}
